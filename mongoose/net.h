// Copyright (c) 2023 Cesanta Software Limited
// All rights reserved
#pragma once

#include "mongoose.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(HTTP_URL)
#define HTTP_URL "http://0.0.0.0:8000"
#endif

#if !defined(HTTPS_URL)
#define HTTPS_URL "https://0.0.0.0:8443"
#endif

#define MAX_DEVICE_NAME 40
#define MAX_EVENTS_NO 400
#define MAX_EVENT_TEXT_SIZE 10
#define EVENTS_PER_PAGE 20

// Event log entry
struct ui_event {
  uint8_t type, prio;
  unsigned long timestamp;
  char text[10];
};

#define CREDENTIALS_OPERATION_CHECK_CREDENTIALS	1
#define CREDENTIALS_OPERATION_CHECK_TOKEN	2
#define CREDENTIALS_OPERATION_ADD_USER 3
#define CREDENTIALS_OPERATION_CHANGE_PASSWORD 4
#define CREDENTIALS_OPERATION_REMOVE_TOKEN 5

struct http_fps_item
{
	char* id;
	unsigned int counter;
};

struct http_system_info
{
	char* device_name;
	unsigned int cpu;
	unsigned int memory;
	int temp;
	unsigned int gpu;
};

struct http_server_params
{
	char* device_name;
	char* root_dir;
	int http_port;
	int https_port;
	char* cert_dir;
	char* home_page;
	char* settings_file_path;

	struct http_system_info system_info;
	int num_counter;
	struct http_fps_item* counters;

	void* credentials;
	bool (*callback)(int operation, void* credentials, const char* login, int login_max_len, const char* password, char** token);
};

void web_init(struct mg_mgr *mgr, struct http_server_params* params);
void init_http_server_params(struct http_server_params* params, const char* device_name, const char* root_dir, const int http_port, const int https_port, const char* cert_dir, const char* home_page);

#ifdef __cplusplus
}
#endif
