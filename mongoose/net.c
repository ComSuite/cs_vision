// Copyright (c) 2023 Cesanta Software Limited
// All rights reserved

#include "net.h"
// Authenticated user.
// A user can be authenticated by:
//   - a name:pass pair, passed in a header Authorization: Basic .....
//   - an access_token, passed in a header Cookie: access_token=....
// When a user is shown a login screen, she enters a user:pass. If successful,
// a server responds with a http-only access_token cookie set.
struct user {
  const char *name, *pass, *access_token;
};

// Settings
struct settings {
  bool log_enabled;
  int log_level;
  long brightness;
  char *device_name;
};

static struct settings s_settings = {true, 1, 57, NULL};

static const char *s_json_header =
    "Content-Type: application/json\r\n"
    "Cache-Control: no-cache\r\n";

int ui_event_next(int no, struct ui_event *e) {
  if (no < 0 || no >= MAX_EVENTS_NO) return 0;

  srand((unsigned) no);
  e->type = (uint8_t) rand() % 4;
  e->prio = (uint8_t) rand() % 3;
  e->timestamp =
      (unsigned long) ((int64_t) mg_now() - 86400 * 1000 /* one day back */ +
                       no * 300 * 1000 /* 5 mins between alerts */ +
                       1000 * (rand() % 300) /* randomize event time */) /
      1000UL;

  mg_snprintf(e->text, MAX_EVENT_TEXT_SIZE, "event#%d", no);
  return no + 1;
}

static void timer_sntp_fn(void *param) {  // SNTP timer function. Sync up time
  mg_sntp_connect(param, "udp://time.google.com:123", NULL, NULL);
}

// Parse HTTP requests, return authenticated user or NULL
static struct user* authenticate(struct mg_http_message *hm, struct http_server_params* server_params) {
    if (server_params == NULL)
        return NULL;

    char login[64], pass[64];
    struct user* result = NULL;
    mg_http_creds(hm, login, sizeof(login), pass, sizeof(pass));
    MG_VERBOSE(("user [%s] pass [%s]", login, pass));

    char* token;
    if (server_params->callback(CREDENTIALS_OPERATION_CHECK_CREDENTIALS, server_params->credentials, login, pass, &token)) {
        result = (struct user*)malloc(sizeof(struct user));
        if (result != NULL) {
            result->access_token = _strdup(token);
            result->name = _strdup(login);
        }
    }
    else {
        token = _strdup(pass);
        memset(pass, 0x00, sizeof(pass));
        if (server_params->callback(CREDENTIALS_OPERATION_CHECK_TOKEN, server_params->credentials, login, pass, &token)) {
            result = (struct user*)malloc(sizeof(struct user));
            if (result != NULL) {
                result->access_token = _strdup(token);
                result->name = _strdup(login);
            }
        }
    }

    return result;
}

static void handle_login(struct mg_connection *c, struct user *u) {
  char cookie[256];
  mg_snprintf(cookie, sizeof(cookie),
              "Set-Cookie: access_token=%s; Path=/; "
              "%sHttpOnly; SameSite=Lax; Max-Age=%d\r\n",
              u->access_token, c->is_tls ? "Secure; " : "", 3600 * 24);
  mg_http_reply(c, 200, cookie, "{%m:%m}", MG_ESC("user"), MG_ESC(u->name));
}

static void handle_logout(struct mg_connection *c) {
  char cookie[256];
  mg_snprintf(cookie, sizeof(cookie),
              "Set-Cookie: access_token=; Path=/; "
              "Expires=Thu, 01 Jan 1970 00:00:00 UTC; "
              "%sHttpOnly; Max-Age=0; \r\n",
              c->is_tls ? "Secure; " : "");
  mg_http_reply(c, 200, cookie, "true\n");
}

static void handle_debug(struct mg_connection *c, struct mg_http_message *hm) {
  int level = mg_json_get_long(hm->body, "$.level", MG_LL_DEBUG);
  mg_log_set(level);
  mg_http_reply(c, 200, "", "Debug level set to %d\n", level);
}

static size_t print_int_arr(void (*out)(char, void *), void *ptr, va_list *ap) {
  size_t i, len = 0, num = va_arg(*ap, size_t);  // Number of items in the array
  int *arr = va_arg(*ap, int *);              // Array ptr
  for (i = 0; i < num; i++) {
    len += mg_xprintf(out, ptr, "%s%d", i == 0 ? "" : ",", arr[i]);
  }
  return len;
}

static void handle_stats_get(struct mg_connection *c) {
  int points[] = {21, 22, 22, 19, 18, 20, 23, 23, 22, 22, 22, 23, 22};
  mg_http_reply(c, 200, s_json_header, "{%m:%d,%m:%d,%m:[%M]}\n",
                MG_ESC("temperature"), 21,  //
                MG_ESC("humidity"), 67,     //
                MG_ESC("points"), print_int_arr,
                sizeof(points) / sizeof(points[0]), points);
}

static size_t print_events(void (*out)(char, void *), void *ptr, va_list *ap) {
  size_t len = 0;
  struct ui_event ev;
  int pageno = va_arg(*ap, int);
  int no = (pageno - 1) * EVENTS_PER_PAGE;
  int end = no + EVENTS_PER_PAGE;

  while ((no = ui_event_next(no, &ev)) != 0 && no <= end) {
    len += mg_xprintf(out, ptr, "%s{%m:%lu,%m:%d,%m:%d,%m:%m}\n",  //
                      len == 0 ? "" : ",",                         //
                      MG_ESC("time"), ev.timestamp,                //
                      MG_ESC("type"), ev.type,                     //
                      MG_ESC("prio"), ev.prio,                     //
                      MG_ESC("text"), MG_ESC(ev.text));
  }

  return len;
}

static void handle_events_get(struct mg_connection *c,
                              struct mg_http_message *hm) {
  int pageno = mg_json_get_long(hm->body, "$.page", 1);
  mg_http_reply(c, 200, s_json_header, "{%m:[%M], %m:%d}\n", MG_ESC("arr"),
                print_events, pageno, MG_ESC("totalCount"), MAX_EVENTS_NO);
}

static void handle_settings_set(struct mg_connection *c, struct mg_str body) {
  struct settings settings;
  char *s = mg_json_get_str(body, "$.device_name");
  bool ok = true;
  memset(&settings, 0, sizeof(settings));
  mg_json_get_bool(body, "$.log_enabled", &settings.log_enabled);
  settings.log_level = mg_json_get_long(body, "$.log_level", 0);
  settings.brightness = mg_json_get_long(body, "$.brightness", 0);
  if (s && strlen(s) < MAX_DEVICE_NAME) {
    free(settings.device_name);
    settings.device_name = s;
  } else {
    free(s);
  }
  s_settings = settings; // Save to the device flash
  mg_http_reply(c, 200, s_json_header, 
                "{%m:%s,%m:%m}",                          //
                MG_ESC("status"), ok ? "true" : "false",  //
                MG_ESC("message"), MG_ESC(ok ? "Success" : "Failed"));
}

char* load_file(const char* path) {
    FILE* file;
    char* buffer;
    long file_size;

    if (path == NULL)
        return NULL;

    file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    buffer = (char*)malloc((file_size + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0'; // Null-terminate the string

    // Print the file content
    printf("%s\n", buffer);

    // Clean up
    fclose(file);

    return buffer;
}

static void handle_settings_get(struct mg_connection *c, struct http_server_params* server_params) {
    char* json = load_file(server_params->settings_file_path);
    if (json == NULL)
        return;

    mg_http_reply(c, 200, s_json_header, json);

    free(json);
}

static void handle_firmware_upload(struct mg_connection *c,
                                   struct mg_http_message *hm) {
  char name[64], offset[20], total[20];
  struct mg_str data = hm->body;
  long ofs = -1, tot = -1;
  name[0] = offset[0] = '\0';
  mg_http_get_var(&hm->query, "name", name, sizeof(name));
  mg_http_get_var(&hm->query, "offset", offset, sizeof(offset));
  mg_http_get_var(&hm->query, "total", total, sizeof(total));
  MG_INFO(("File %s, offset %s, len %lu", name, offset, data.len));
  if ((ofs = mg_json_get_long(mg_str(offset), "$", -1)) < 0 ||
      (tot = mg_json_get_long(mg_str(total), "$", -1)) < 0) {
    mg_http_reply(c, 500, "", "offset and total not set\n");
  } else if (ofs == 0 && mg_ota_begin((size_t) tot) == false) {
    mg_http_reply(c, 500, "", "mg_ota_begin(%ld) failed\n", tot);
  } else if (data.len > 0 && mg_ota_write(data.buf, data.len) == false) {
    mg_http_reply(c, 500, "", "mg_ota_write(%lu) @%ld failed\n", data.len, ofs);
    mg_ota_end();
  } else if (data.len == 0 && mg_ota_end() == false) {
    mg_http_reply(c, 500, "", "mg_ota_end() failed\n", tot);
  } else {
    mg_http_reply(c, 200, s_json_header, "true\n");
    if (data.len == 0) {
      // Successful mg_ota_end() called, schedule device reboot
      mg_timer_add(c->mgr, 500, 0, (void (*)(void *)) mg_device_reset, NULL);
    }
  }
}

static void handle_firmware_commit(struct mg_connection *c) {
  mg_http_reply(c, 200, s_json_header, "%s\n",
                mg_ota_commit() ? "true" : "false");
}

static void handle_firmware_rollback(struct mg_connection *c) {
  mg_http_reply(c, 200, s_json_header, "%s\n",
                mg_ota_rollback() ? "true" : "false");
}

static size_t print_status(void (*out)(char, void *), void *ptr, va_list *ap) {
  int fw = va_arg(*ap, int);
  return mg_xprintf(out, ptr, "{%m:%d,%m:%c%lx%c,%m:%u,%m:%u}\n",
                    MG_ESC("status"), mg_ota_status(fw), MG_ESC("crc32"), '"',
                    mg_ota_crc32(fw), '"', MG_ESC("size"), mg_ota_size(fw),
                    MG_ESC("timestamp"), mg_ota_timestamp(fw));
}

static void handle_firmware_status(struct mg_connection *c) {
  mg_http_reply(c, 200, s_json_header, "[%M,%M]\n", print_status,
                MG_FIRMWARE_CURRENT, print_status, MG_FIRMWARE_PREVIOUS);
}

static void handle_device_reset(struct mg_connection *c) {
  mg_http_reply(c, 200, s_json_header, "true\n");
  mg_timer_add(c->mgr, 500, 0, (void (*)(void *)) mg_device_reset, NULL);
}

static void handle_device_eraselast(struct mg_connection *c) {
  size_t ss = mg_flash_sector_size(), size = mg_flash_size();
  char *base = (char *) mg_flash_start(), *last = base + size - ss;
  if (mg_flash_bank() == 2) last -= size / 2;
  mg_flash_erase(last);
  mg_http_reply(c, 200, s_json_header, "true\n");
}

static void handle_status(struct mg_connection* c, struct http_server_params* server_params)
{
    mg_http_reply(c, 200, s_json_header, "{\"fps\": %d, \"memory\" : 1020}\n", server_params->fps);
}

// HTTP request handler function
static void fn(struct mg_connection *c, int ev, void *ev_data, struct http_server_params* server_params) {
  if (ev == MG_EV_ACCEPT) {
    if (c->fn_data != NULL) {  // TLS listener!
      struct mg_tls_opts opts = {0};
      opts.cert = mg_unpacked("/certs/server_cert.pem");
      opts.key = mg_unpacked("/certs/server_key.pem");
      mg_tls_init(c, &opts);
    }
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct user *u = authenticate(hm, server_params);

    if (mg_match(hm->uri, mg_str("/api/#"), NULL) && u == NULL) {
      mg_http_reply(c, 403, "", "Not Authorized\n");
    } else if (mg_match(hm->uri, mg_str("/api/login"), NULL)) {
      handle_login(c, u);
    } else if (mg_match(hm->uri, mg_str("/api/login"), NULL)) {
      handle_login(c, u);
    } else if (mg_match(hm->uri, mg_str("/api/status/get"), NULL)) {
        handle_status(c, server_params);
    } else if (mg_match(hm->uri, mg_str("/api/logout"), NULL)) {
      handle_logout(c);
    } else if (mg_match(hm->uri, mg_str("/api/debug"), NULL)) {
      handle_debug(c, hm);
    } else if (mg_match(hm->uri, mg_str("/api/stats/get"), NULL)) {
      handle_stats_get(c);
    } else if (mg_match(hm->uri, mg_str("/api/events/get"), NULL)) {
      handle_events_get(c, hm);
    } else if (mg_match(hm->uri, mg_str("/api/settings/get"), NULL)) {
      handle_settings_get(c, server_params);
    } else if (mg_match(hm->uri, mg_str("/api/settings/set"), NULL)) {
      handle_settings_set(c, hm->body);
    } else if (mg_match(hm->uri, mg_str("/api/firmware/upload"), NULL)) {
      handle_firmware_upload(c, hm);
    } else if (mg_match(hm->uri, mg_str("/api/firmware/commit"), NULL)) {
      handle_firmware_commit(c);
    } else if (mg_match(hm->uri, mg_str("/api/firmware/rollback"), NULL)) {
      handle_firmware_rollback(c);
    } else if (mg_match(hm->uri, mg_str("/api/firmware/status"), NULL)) {
      handle_firmware_status(c);
    } else if (mg_match(hm->uri, mg_str("/api/device/reset"), NULL)) {
      handle_device_reset(c);
    } else if (mg_match(hm->uri, mg_str("/api/device/eraselast"), NULL)) {
      handle_device_eraselast(c);
    } else {
      struct mg_http_serve_opts opts;
      memset(&opts, 0, sizeof(opts));
#if MG_ARCH == MG_ARCH_UNIX || MG_ARCH == MG_ARCH_WIN32
      if (server_params != NULL && server_params->root_dir != NULL)
          opts.root_dir = server_params->root_dir;
      else
        opts.root_dir = "web_root";  // On workstations, use filesystem
#else
      if (server_params != NULL && server_params->root_dir != NULL)
          opts.root_dir = server_params->root_dir;
      else
        opts.root_dir = "/web_root";  // On embedded, use packed files
      opts.fs = &mg_fs_packed;
#endif
      mg_http_serve_dir(c, ev_data, &opts);
    }
    MG_DEBUG(("%lu %.*s %.*s -> %.*s", c->id, (int) hm->method.len,
              hm->method.buf, (int) hm->uri.len, hm->uri.buf, (int) 3,
              &c->send.buf[9]));
  }
}

void web_init(struct mg_mgr *mgr, struct http_server_params* params) {
    if (params != NULL && params->device_name != NULL)
        s_settings.device_name = strdup(params->device_name);
    else
        s_settings.device_name = strdup("MyDevice");

  mg_http_listen(mgr, HTTP_URL, fn, NULL, params);
  mg_http_listen(mgr, HTTPS_URL, fn, (void *) 1, params);
  mg_timer_add(mgr, 3600 * 1000, MG_TIMER_RUN_NOW | MG_TIMER_REPEAT,
               timer_sntp_fn, mgr);
}

void init_http_server_params(struct http_server_params* params, const char* device_name, const char* root_dir, const int http_port, const int https_port, const char* cert_dir, const char* home_page)
{
    if (params == NULL)
        return;

    params->device_name = strdup(device_name);
    params->root_dir = strdup(root_dir);
    params->http_port = http_port;
    params->https_port = https_port;
    params->cert_dir = strdup(cert_dir);
    params->home_page = strdup(home_page);
}
