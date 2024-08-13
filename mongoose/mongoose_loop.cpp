#include "mongoose_loop.h"
#include "mongoose.h"
#include "net.h"

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
	struct mg_mgr mgr;

	signal(SIGPIPE, SIG_IGN);
	//signal(SIGINT, signal_handler);
	//signal(SIGTERM, signal_handler);

	http_server_params server_params;
	server_params.device_name = _strdup("ComSuite Vision");
	server_params.root_dir = _strdup("G:\\Projects\\comsuite\\sources\\cs_vision\\web_root\\web_root");

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

