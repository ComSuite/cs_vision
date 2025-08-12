/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief       MQTT wrapper
 *
 **************************************************************************************
 * Copyright (c) 2021 - 2025, Alexander Epstine (a@epstine.com)
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

#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <mosquitto.h>

namespace cs
{
	typedef void (*on_message)(struct mosquitto* mosq, const char* topic, const char* payload, void* data);

	struct send_params{
		struct mosquitto* mosq;
		char* payload;
		char* topic;
		bool retained;
	};

	class MQTTSubscriber
	{
	public:
		std::string topic = "";
		void* user_data = nullptr;
		on_message callback = nullptr;
	};

	class MQTTWrapper
	{
	public:
		MQTTWrapper();
		MQTTWrapper(struct mosquitto* mosq);
		virtual ~MQTTWrapper();

		int connect(const char* client, const char* host, int port);
		int connect(const char* client, const char* login, const char* password, const char* host, int port);
		int send(const char* topic, const char* payload);
		int send(const char* topic, const char* payload, bool retained);
		int subscribe(const char* topic, void* data, on_message callback);
		int unsubscribe(const char* topic, on_message callback);
		int loop();
		int start_background_loop();
		int disconnect();
		int cleanup();
		struct mosquitto* get_handle() { return mosq; };

		bool is_connect = false;
		static std::vector<MQTTSubscriber> callbacks;
	protected:
		std::chrono::system_clock::time_point last_ping_time;
		struct mosquitto* mosq = NULL;
	};
}
