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

#include <cstdio>
#include <iostream>
#include <thread>
#include <pthread.h>
#ifdef __LINUX__
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>
#endif
#include "settings.h"
#include "camera_loop.h"
#include "MQTTClient.h"
#include "JsonWriter.h"
#include "command_processor.h"
#include "ReadOnlyValues.h"
#include "std_utils.h"
#include "FileBackup.h"
#ifdef __WITH_SCRIPT_LANG__
#include "CSScript.h"
#endif
#ifdef __WITH_AUDIO_PROCESSING__
#include "portaudio_stream.h"
#endif
#ifdef __WITH_MONGOOSE_SERVER__
#include "mongoose_loop.h"
#endif

using namespace std;
using namespace cs;

void send_response(MQTTClient* mqtt, device_settings* settings, command_processor* command, int error_code)
{
    if (mqtt == nullptr) {
        return;
    }

    mqtt->send_command_response(command->command_id, settings->id.c_str(), settings->mqtt_response_topic.c_str(), command->req_id, error_code, "");
}

void send_ping(MQTTClient* mqtt, device_settings* settings, command_processor* command)
{
    if (mqtt == nullptr || settings == nullptr) {
        return;
    }

    mqtt->send_command_response(command_processor::COMMAND_ID_PING, settings->id.c_str(), settings->mqtt_ping_topic.c_str(), command->req_id, 0, "ping");
}

void send_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command)
{
    if (mqtt == nullptr || settings == nullptr || command == nullptr) {
        return;
    }

    JsonWriter* json_writer = new JsonWriter();
    if (json_writer == nullptr) {
        send_response(mqtt, settings, command, error_codes::ERROR_NOT_ENOUGH_MEMORY);
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

        send_response(mqtt, settings, command, cs::error_codes::ERROR_NO_ERROR);
    }
    else
        send_response(mqtt, settings, command, cs::error_codes::ERROR_INTERNAL_ERROR);

    delete json_writer;
}

void set_settings(MQTTClient* mqtt, device_settings* settings, command_processor* command)
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
            send_response(mqtt, settings, command, error_codes::ERROR_NO_ERROR);
        else
            send_response(mqtt, settings, command, error_codes::ERROR_INTERNAL_ERROR);

        delete writer;
    }
    else
        send_response(mqtt, settings, command, error_codes::ERROR_NOT_ENOUGH_MEMORY);

    if (checker != nullptr)
        delete checker;
}

list<command_processor*> commands_queue;

void on_message_callback(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{
    device_settings* settings = (device_settings*)data;
    if (settings == nullptr)
        return;

    command_processor* command = new command_processor();
    if (command == nullptr) {
        return;
    }

    ((JsonWrapper*)command)->parse(payload);
    if (command->device_id.compare(settings->id) != 0) {
        cout << "Device ID: " << settings->id << " not matched with command device id: " << command->device_id << endl;
        delete command;
        return;
    }

    commands_queue.push_back(command);
}

void process_command(MQTTClient* mqtt, device_settings* settings)
{
    if (mqtt == nullptr || settings == nullptr) {
        return;
    }

    while (commands_queue.size() > 0) {
        command_processor* command = *commands_queue.begin();
        commands_queue.pop_front();
        if (command == nullptr)
            return;

        switch (command->command_id) {
        case command_processor::COMMAND_ID_RESTART:
            send_response(mqtt, settings, command, error_codes::ERROR_NO_ERROR);
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
            send_settings(mqtt, settings, command);
            break;
        case command_processor::COMMAND_ID_SET_SETTINGS:
            set_settings(mqtt, settings, command);
            break;
        case command_processor::COMMAND_ID_RESTORE_FROM_BACKUP:
            if (FileBackup::restore(settings->get_file_path(), FileBackup::default_backup_extension.c_str()))
                send_response(mqtt, settings, command, error_codes::ERROR_NO_ERROR);
            else
                send_response(mqtt, settings, command, error_codes::ERROR_CAN_NOT_RESTORE_FROM_BACKUP);
            break;
        case command_processor::COMMAND_ID_PING:
            send_ping(mqtt, settings, command);
            break;
        default:
            send_response(mqtt, settings, command, error_codes::ERROR_BAD_COMMAND_ID);
            break;
        }

        delete command;
    }
}

class camera_thread_description {
public:
    pthread_t  camera_thread;
    camera_settings* camera_set;
};

int pthread_exists(pthread_t tid) {
    if (!pthread_equal(tid, (pthread_t)(0x0000)))
        return pthread_kill(tid, 0) == 0;
    else
        return 1;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "\nError! Usage: <path to settings file>" << endl << endl;
        return 1;
    }

#ifndef _DEBUG_
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
#endif

#ifdef __LINUX__
    signal(SIGPIPE, SIG_IGN);
#endif

    device_settings* settings = new device_settings();
    if (settings == nullptr) {
        cout << "Not enough memory. Programm terminated!" << endl;
        return 1;
    }

    if (!settings->load(argv[1])) {
        cout << "Invalid settings file: <" << argv[1] << ">!" << endl;
        return 1;
    }

#ifdef __WITH_MONGOOSE_SERVER__
    http_server_thread_arg* http_arg = new http_server_thread_arg();
    if (http_arg != nullptr) {
        http_arg->settings_file_path = settings->get_file_path();
        http_arg->settings = settings->http_server;
        http_arg->queue = new BaseQueue<fps_counter_info>();
    }
#endif

    list<camera_thread_description*> camera_threads;
    for (auto& camera : settings->cameras) {
#ifdef __WITH_MONGOOSE_SERVER__
        if (http_arg != nullptr) {
            camera->http_server_queue = http_arg->queue;
        }
#endif

        camera_thread_description* descr = new camera_thread_description();

        pthread_create(&descr->camera_thread, NULL, camera_loop, camera);
        pthread_detach(descr->camera_thread);
        descr->camera_set = camera;

        camera_threads.push_back(descr);
    }

    MQTTClient* mqtt = nullptr;
    if (settings->mqtt_broker_ip.size() > 0 && settings->mqtt_broker_port > 0) {
        mqtt = new MQTTClient();
        if (mqtt != nullptr) {
            if (!mqtt->connect(settings->mqtt_client_name.c_str(), settings->mqtt_broker_ip.c_str(), settings->mqtt_broker_port)) {
                cout << "Cannot connect to Mosquitto broker IP: " << settings->mqtt_broker_ip.c_str() << " port: " << settings->mqtt_broker_port << endl;
            }

            mqtt->subscribe(settings->mqtt_command_topic.c_str(), settings, on_message_callback);
        }
        else
            cout << "Cannot allocate memory for command`s topic MQTT client. Broker IP: " << settings->mqtt_broker_ip.c_str() << " port: " << settings->mqtt_broker_port << endl;
    }

#ifdef __WITH_MONGOOSE_SERVER__
    pthread_t http_thread;

    if (http_arg != nullptr) {
        pthread_create(&http_thread, NULL, mongoose_thread_func, http_arg);
        pthread_detach(http_thread);
    }
#endif

    auto begin_time = std::chrono::steady_clock::now();

    for (;;) {
        if (mqtt != nullptr)
            mqtt->loop();

        process_command(mqtt, settings);

        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin_time).count() > 10) {
            for (auto& descr : camera_threads) {
                if (!pthread_exists(descr->camera_thread)) {
                    pthread_create(&descr->camera_thread, NULL, camera_loop, descr->camera_set);
                    pthread_detach(descr->camera_thread);
                }
            }
        }
    }

    clear<camera_thread_description, std::list>(camera_threads);
    delete settings;
    if (mqtt != nullptr)
        delete mqtt;
#ifdef __WITH_MONGOOSE_SERVER__
    if (http_arg != nullptr) {
        delete http_arg->queue;
    }
#endif

    return 0;
}