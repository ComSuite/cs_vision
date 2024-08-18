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

	cout << "login: " << login << "password: " << password << endl;
	return false;
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
	server_params.fps = 0;

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

