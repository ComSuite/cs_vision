/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2024, Alexander Epstine (a@epstine.com)
 **************************************************************************************
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "mongoose_loop.h"
#include "credentials_storage.h"
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
	if (arg == nullptr)
		return nullptr;

	http_server_thread_arg* _arg = static_cast<http_server_thread_arg*>(arg);

	device_settings* settings = _arg->settings;
	BaseQueue<fps_counter_info>* queue = _arg->queue;

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
	server_params.settings_file_path = _strdup(settings->get_file_path());

	mg_log_set(MG_LL_DEBUG);  // Set debug log level
	mg_mgr_init(&mgr);
	mgr.userdata = &server_params;

	web_init(&mgr, &server_params);
	while (s_sig_num == 0) {
		if (queue != nullptr) {
			auto info = queue->try_pop();
			if (info != nullptr) {
				server_params.fps = info->counter;
			}
		}

		mg_mgr_poll(&mgr, 50);
	}

	mg_mgr_free(&mgr);

	return nullptr;
}

