#include "mongoose_loop.h"
#include "mongoose.h"
#include "net.h"
#include "settings.h"

using namespace std;
using namespace cs;

#ifdef __LINUX__
#ifndef _strdup
#define _strdup strdup
#endif
#endif

static int s_sig_num;
static void signal_handler(int sig_num) 
{
	signal(sig_num, signal_handler);
	s_sig_num = sig_num;
}

void* cs::mongoose_thread_func(void* arg)
{
	if (arg == nullptr)
		return nullptr;

	device_settings* settings = static_cast<device_settings*>(arg);
	if (settings == nullptr)
		return nullptr;

	struct mg_mgr mgr;
	signal(SIGPIPE, SIG_IGN);
	//signal(SIGINT, signal_handler);
	//signal(SIGTERM, signal_handler);

	http_server_params server_params;
	server_params.device_name = _strdup(settings->name.c_str());
	server_params.root_dir = _strdup(settings->http_server->root_dir.c_str());
	server_params.cert_dir = _strdup(settings->http_server->cert_dir.c_str());
	server_params.http_port = settings->http_server->http_port;
	server_params.https_port = settings->http_server->https_port;
	server_params.home_page = _strdup(settings->http_server->home_page.c_str());

	mg_log_set(MG_LL_DEBUG);  // Set debug log level
	mg_mgr_init(&mgr);
	mgr.userdata = &server_params;

	web_init(&mgr, &server_params);
	while (s_sig_num == 0) {
		mg_mgr_poll(&mgr, 50);
	}

	mg_mgr_free(&mgr);

	return nullptr;
}

