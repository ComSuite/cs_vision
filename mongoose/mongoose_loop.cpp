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
#include "uuid.h"
#include "system_usage.h"
#include <chrono>

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

static uuids::uuid get_uuid()
{
	std::random_device rd;
	auto seed_data = std::array<int, std::mt19937::state_size> {};
	std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
	std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
	std::mt19937 generator(seq);

	return uuids::uuid_random_generator{ generator }();
}

static bool credentials_callback(int operation, void* credentials, const char* login, int login_max_len, const char* password, char** token)
{
	credentials_storage* cred = (credentials_storage*)credentials;
	if (cred == NULL || token == NULL) //login == NULL ||
		return false;

	switch (operation) {
	case CREDENTIALS_OPERATION_CHECK_CREDENTIALS:
		if (cred->check(login, password)) {
			std::string st = uuids::to_string(get_uuid());
			cred->add_token(st.c_str(), login);
			*token = _strdup(st.c_str());
			return true;
		} 
		break;
	case CREDENTIALS_OPERATION_CHECK_TOKEN:
		return cred->check_token(*token, (char*)login, login_max_len);
	case CREDENTIALS_OPERATION_ADD_USER:
		break;
	case CREDENTIALS_OPERATION_CHANGE_PASSWORD:
		break;
	case CREDENTIALS_OPERATION_REMOVE_TOKEN:
		return cred->del_token(*token);
	}

	return false;
}

http_fps_item* find_counter(http_fps_item* counters, int num_counters, const char* id)
{
	for (int i = 0; i < num_counters; i++) {
		if (counters[i].id != NULL) {
			if (strcmp(counters[i].id, id) == 0)
				return &counters[i];
		}
	}

	for (int i = 0; i < num_counters; i++) {
		if (counters[i].id == NULL) {
			counters[i].id = _strdup(id);
			return &counters[i];
		}
	}

	return NULL;
}

void* cs::mongoose_thread_func(void* arg)
{
	if (arg == nullptr)
		return nullptr;

	http_server_thread_arg* _arg = static_cast<http_server_thread_arg*>(arg);

	http_server_settings* settings = _arg->settings;
	BaseQueue<fps_counter_info>* queue = _arg->queue;

	if (settings == nullptr)
		return nullptr;

	struct mg_mgr mgr;
	signal(SIGPIPE, SIG_IGN);
	//signal(SIGINT, signal_handler);
	//signal(SIGTERM, signal_handler);

	credentials_storage* credentials = new credentials_storage();

	http_server_params server_params;
	server_params.device_name = _strdup(settings->device_name.c_str());
	server_params.root_dir = _strdup(settings->root_dir.c_str());
	server_params.cert_dir = _strdup(settings->cert_dir.c_str());
	server_params.http_port = settings->http_port;
	server_params.https_port = settings->https_port;
	server_params.home_page = _strdup(settings->home_page.c_str());
	server_params.settings_file_path = _strdup(_arg->settings_file_path.c_str());
	server_params.callback = credentials_callback;
	server_params.credentials = credentials;

	server_params.counters = (http_fps_item*)malloc(_arg->camera_count * sizeof(http_fps_item));
	if (server_params.counters != NULL) {
		server_params.num_counter = _arg->camera_count;
		for (int i = 0; i < server_params.num_counter; i++) {
			server_params.counters[i].counter = 0;
			server_params.counters[i].id = NULL;
		}
	}
	else {
		server_params.num_counter = 0;
	}

	system_usage sys;
	sys.init();

	mg_log_set(MG_LL_DEBUG); //MG_LL_ERROR
	mg_mgr_init(&mgr);
	mgr.userdata = &server_params;

	web_init(&mgr, &server_params);
	auto begin_time = std::chrono::steady_clock::now();
	while (s_sig_num == 0) {
		if (queue != nullptr) {
			auto info = queue->try_pop();
			if (info != nullptr) {
				http_fps_item* counter = find_counter(server_params.counters, server_params.num_counter, info->id.c_str());
				if (counter != NULL) {
					counter->counter = info->counter;
				}

				delete info;
			}

			server_params.system_info.memory = sys.get_free_memory();
			server_params.system_info.gpu = sys.get_gpu_usage();


			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin_time).count() > 1500) {
				server_params.system_info.cpu = sys.get_cpu_usage();
				begin_time = std::chrono::steady_clock::now();
			}
		}

		mg_mgr_poll(&mgr, 50);
	}

	mg_mgr_free(&mgr);

	return nullptr;
}

