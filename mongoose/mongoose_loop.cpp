#include "mongoose_loop.h"
#include "mongoose.h"
#include "net.h"

using namespace std;
using namespace cs;

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

	mg_log_set(MG_LL_DEBUG);  // Set debug log level
	mg_mgr_init(&mgr);

	web_init(&mgr);
	while (s_sig_num == 0) {
		mg_mgr_poll(&mgr, 50);
	}

	mg_mgr_free(&mgr);

	return nullptr;
}

