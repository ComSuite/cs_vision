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
#include <semaphore.h>
#include <fcntl.h>
#ifdef __LINUX__
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>
#endif
#include "settings.h"
#include "camera_loop.h"
#include "MQTTClient.h"
#include "device_manager.h";
#include "std_utils.h"
#ifdef __WITH_SCRIPT_LANG__
#include "CSScript.h"
#endif
#ifdef __WITH_AUDIO_PROCESSING__
//#include "portaudio_stream.h"
#endif
#ifdef __WITH_MONGOOSE_SERVER__
#include "mongoose_loop.h"
#endif

using namespace std;
using namespace cs;

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

bool check_process_instance(sem_t** semaphore)
{
    *semaphore = sem_open("cs_vision_instance", O_CREAT | O_EXCL, 0644, 1);
    if (*semaphore == SEM_FAILED)
    {
        if (errno == EEXIST)
            return false;
    }

    return true;
}

#ifdef __LINUX__
static void signal_handler(int sig_num)
{
	cout << "Signal: " << sig_num << " received. Exit." << endl;
    exit(0);
}
#endif

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
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
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
		http_arg->secrets_file_path = settings->secrets_dictionary;
        http_arg->settings = settings->http_server;
        http_arg->queue = new BaseQueue<fps_counter_info>();
    }
#endif
	processed_command_params* command_params = new processed_command_params();
    if (command_params == nullptr) {
        cout << "Not enough memory. Programm terminated!" << endl;
		delete settings;
        return 1;
	}

	command_params->settings = settings;

    MQTTClient* mqtt = nullptr;
    if (settings->mqtt && settings->mqtt_broker_ip.size() > 0 && settings->mqtt_broker_port > 0) {
        mqtt = new MQTTClient();
        if (mqtt != nullptr) {
            if (!mqtt->connect(settings->mqtt_client_name.c_str(), settings->mqtt_broker_ip.c_str(), settings->mqtt_broker_port)) {
                cout << "Cannot connect to Mosquitto broker IP: " << settings->mqtt_broker_ip.c_str() << " port: " << settings->mqtt_broker_port << endl;
            }

            mqtt->subscribe(settings->mqtt_command_topic.c_str(), command_params, on_message_callback);
        }
        else
            cout << "Cannot allocate memory for command`s topic MQTT client. Broker IP: " << settings->mqtt_broker_ip.c_str() << " port: " << settings->mqtt_broker_port << endl;
    }

    list<camera_thread_description*> camera_threads;
    for (auto& camera : settings->cameras) {
        if (camera->is_enabled) {
#ifdef __WITH_MONGOOSE_SERVER__
            if (http_arg != nullptr) {
                camera->http_server_queue = http_arg->queue;
            }
#endif

            camera_thread_description* descr = new camera_thread_description();

            camera_loop_params* cam_prams = new camera_loop_params();
            cam_prams->settings = camera;
            cam_prams->mqtt_client = mqtt;

            pthread_create(&descr->camera_thread, NULL, camera_loop, cam_prams); //camera
            pthread_detach(descr->camera_thread);
            descr->camera_set = camera;

            camera_threads.push_back(descr);
        }
    }

#ifdef __WITH_MONGOOSE_SERVER__
    pthread_t http_thread;

    if (http_arg != nullptr) {
        http_arg->camera_count = static_cast<int>(settings->cameras.size());

        pthread_create(&http_thread, NULL, mongoose_thread_func, http_arg);
        pthread_detach(http_thread);
    }
#endif

    auto begin_time = std::chrono::steady_clock::now();

    //if (mqtt != nullptr)
	//	mqtt->start_background_loop(); // Start MQTT background loop

    for (;;) {
        if (mqtt != nullptr)
            mqtt->loop();

        process_command(mqtt, command_params);

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