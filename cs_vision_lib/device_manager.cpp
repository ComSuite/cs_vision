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

#include "device_manager.h"
#include <cstdio>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#ifdef __LINUX__
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>
#endif
#include "std_utils.h"

using namespace std;
using namespace cs;

void cs::send_response(MQTTClient* mqtt, device_settings* settings, command_processor* command, int error_code)
{
    if (mqtt == nullptr) {
        return;
    }

    mqtt->send_command_response(command->command_id, settings->id.c_str(), settings->mqtt_response_topic.c_str(), command->req_id, error_code, "");
}

void cs::send_ping(MQTTClient* mqtt, device_settings* settings, command_processor* command)
{
    if (mqtt == nullptr || settings == nullptr) {
        return;
    }

    mqtt->send_command_response(command_processor::COMMAND_ID_PING, settings->id.c_str(), settings->mqtt_ping_topic.c_str(), command->req_id, 0, "ping");
}

void cs::send_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command)
{
    if (mqtt == nullptr || settings == nullptr || command == nullptr) {
        return;
    }

    JsonWriter* json_writer = new JsonWriter();
    if (json_writer == nullptr) {
        send_response(mqtt, settings, command, cs::error_codes::__ERROR_NOT_ENOUGH_MEMORY);
        delete mqtt;
        return;
    }

    int ret = json_writer->load(settings->get_file_path());
    if (ret) {
        string json = "";
        list<string> secrets;
        txt_load_to<list>(settings->secrets_dictionary.c_str(), secrets);
        command->hide_secrets(json_writer, secrets, json);
        mqtt->send(settings->mqtt_settings_get_topic.c_str(), json.c_str());

        send_response(mqtt, settings, command, cs::error_codes::__ERROR_NO_ERROR);
    }
    else
        send_response(mqtt, settings, command, cs::error_codes::__ERROR_INTERNAL_ERROR);

    delete json_writer;
}

void cs::set_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command)
{
    if (mqtt == nullptr || settings == nullptr || command == nullptr) {
        return;
    }

    ReadOnlyValues* checker = nullptr;

    if (settings->is_use_readonly_checker)
        checker = new ReadOnlyValues(settings->readonly_checker_dictionary.c_str());

    JsonWriter* writer = new JsonWriter();
    if (writer != nullptr) {
        writer->load(settings->get_file_path());
        if (command->execute(writer, checker, settings->get_file_path(), settings->is_create_backup))
            send_response(mqtt, settings, command, cs::error_codes::__ERROR_NO_ERROR);
        else
            send_response(mqtt, settings, command, cs::error_codes::__ERROR_INTERNAL_ERROR);

        delete writer;
    }
    else
        send_response(mqtt, settings, command, cs::error_codes::__ERROR_NOT_ENOUGH_MEMORY);

    if (checker != nullptr)
        delete checker;
}

void cs::on_message_callback(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{
    processed_command_params* params = (processed_command_params*)data;
    if (params == nullptr || params->settings == nullptr || payload == nullptr) {
        cout << "Invalid parameters in on_message_callback" << endl;
        return;
	}

    command_processor* command = new command_processor();
    if (command == nullptr) {
        return;
    }

    ((JsonWrapper*)command)->parse(payload);
    if (command->device_id.compare(params->settings->id) != 0) {
        cout << "Device ID: " << params->settings->id << " not matched with command device id: " << command->device_id << endl;
        delete command;
        return;
    }

    params->commands_queue.push_back(command);
}

void cs::process_command(MQTTClient* mqtt, processed_command_params* params)
{
    if (mqtt == nullptr || params == nullptr || params->settings == nullptr) {
        return;
    }

    while (params->commands_queue.size() > 0) {
        command_processor* command = *params->commands_queue.begin();
        params->commands_queue.pop_front();
        if (command == nullptr)
            return;

        switch (command->command_id) {
        case command_processor::COMMAND_ID_RESTART:
            send_response(mqtt, params->settings, command, error_codes::__ERROR_NO_ERROR);
            this_thread::sleep_for(chrono::milliseconds(100));
            exit(0);
            break;
        case command_processor::COMMAND_ID_REBOOT:
#ifdef __LINUX__
            sync();
            reboot(RB_AUTOBOOT);
#endif
            break;
        case command_processor::COMMAND_ID_GET_SETTINGS:
            send_settings(mqtt, params->settings, command);
            break;
        case command_processor::COMMAND_ID_SET_SETTINGS:
            set_settings(mqtt, params->settings, command);
            break;
        case command_processor::COMMAND_ID_RESTORE_FROM_BACKUP:
            if (FileBackup::restore(params->settings->get_file_path(), FileBackup::default_backup_extension.c_str()))
                send_response(mqtt, params->settings, command, error_codes::__ERROR_NO_ERROR);
            else
                send_response(mqtt, params->settings, command, error_codes::__ERROR_CAN_NOT_RESTORE_FROM_BACKUP);
            break;
        case command_processor::COMMAND_ID_PING:
            send_ping(mqtt, params->settings, command);
            break;
        case command_processor::COMMAND_ID_GET_CONFIGURATION:
        {
			std::string config;
            device_configuration::get_config(params->settings, config);
            if (!config.empty()) {
                mqtt->send(params->settings->mqtt_response_topic.c_str(), config.c_str());
                send_response(mqtt, params->settings, command, error_codes::__ERROR_NO_ERROR);
            }
            else {
                send_response(mqtt, params->settings, command, error_codes::__ERROR_INTERNAL_ERROR);
			}
        }
            break;
        default:
            send_response(mqtt, params->settings, command, error_codes::__ERROR_BAD_COMMAND_ID);
            break;
        }

        delete command;
    }
}
