/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2025, Alexander Epstine (a@epstine.com)
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

#include <list>
#include "settings.h"
#include "camera_loop.h"
#include "MQTTClient.h"
#include "JsonWriter.h"
#include "command_processor.h"
#include "device_configuration.h"
#include "ReadOnlyValues.h"
#include "FileBackup.h"

namespace cs
{
	class processed_command_params
	{
	public:
		device_settings* settings = nullptr;
		std::list<command_processor*> commands_queue;
	};

	void send_response(MQTTClient* mqtt, device_settings* settings, command_processor* command, int error_code);
	void send_ping(MQTTClient* mqtt, device_settings* settings, command_processor* command);
	void send_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command);
	void set_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command);
	void on_message_callback(struct mosquitto* mosq, const char* topic, const char* payload, void* data);
	void process_command(MQTTClient* mqtt, processed_command_params* params);
}

