/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2021, Alexander Epstine (a@epstine.com)
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

#include "MQTTWrapper.h"
#ifdef __linux__
#include <unistd.h>
#endif
#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace cs;

//on_message MQTTWrapper::on_message_callback = NULL;
std::vector<MQTTSubscriber> MQTTWrapper::callbacks;

void mqtt_on_connect(struct mosquitto* mosq, void* data, int);
void mqtt_on_disconnect(struct mosquitto* mosq, void* data, int);
void mqtt_on_message(struct mosquitto* mosq, void* data, const struct mosquitto_message* msg);
void mqtt_on_subscribe(struct mosquitto* mosq, void* data, int mid, int qos_count, const int* granted_qos);

MQTTWrapper::MQTTWrapper()
{
	mosquitto_lib_init();
}

MQTTWrapper::MQTTWrapper(struct mosquitto* mosq)
{
	this->mosq = mosq;
}

MQTTWrapper::~MQTTWrapper()
{
}

int MQTTWrapper::cleanup()
{
	return mosquitto_lib_cleanup();
}

int MQTTWrapper::connect(const char* client, const char* host, int port)
{
	cout << "Trying connect to Mosquitto broker. Client: " << client << " Host: " << host << " Port: " << port << endl;

	mosq = mosquitto_new(client, true, this);
	if (mosq == NULL)
	{
		cout << "mosquitto_new returned NULL" << endl;
		return 0;
	}

	mosquitto_disconnect_callback_set(mosq, mqtt_on_disconnect);
	mosquitto_connect_callback_set(mosq, mqtt_on_connect);
	mosquitto_message_callback_set(mosq, mqtt_on_message);
	mosquitto_subscribe_callback_set(mosq, mqtt_on_subscribe);

	if (mosquitto_connect(mosq, host, port, 100) == MOSQ_ERR_SUCCESS) {
		while(!is_connect) {
			int rc = mosquitto_loop(mosq, 100, 1);
			if (rc != MOSQ_ERR_SUCCESS) {
				cout << "Mosquitto loop error: " << rc << " : " << mosquitto_strerror(rc) << endl;
				this_thread::sleep_for(chrono::milliseconds(100));
			}
		}
		cout << "Mosquitto server connected" << endl;
	}
	else {
		cout << "Failed to connect Mosquitto server" << endl;
		return 0;
	}

	mosquitto_threaded_set(mosq, true);

	return 1;
}

int MQTTWrapper::connect(const char* client, const char* login, const char* password, const char* host, int port)
{
	if (login != NULL && password != NULL) {
		if (mosquitto_username_pw_set(mosq, login, password) != MOSQ_ERR_SUCCESS) {
			return 0;
		}
	}

	return connect(client, host, port);
}

int MQTTWrapper::disconnect()
{
	if (mosq == NULL)
		return 0;

	if (is_connect) {
		mosquitto_destroy(mosq);
	}

	return 1;
}

int MQTTWrapper::send(const char* topic, const char* payload, bool retained)
{
	if (topic == NULL || payload == NULL)
		return 0;

	if (strlen(topic) == 0 || strlen(payload) == 0)
		return 0;

	int err = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, retained);
	if (err != MOSQ_ERR_SUCCESS) {
		cout << "<mosquitto_publish> Mosquitto Error: " << err << " : " << mosquitto_strerror(err) << " System Error: " << errno << endl;
		return 0;
	}
	else {
		try {
			mosquitto_loop_write(mosq, 1);
		}
		catch (...) {
			cout << "Error in mosquitto_loop_write" << endl;
		}
	}

	return 1;
}

int MQTTWrapper::send(const char* topic, const char* payload)
{
	return send(topic, payload, false);
}

int MQTTWrapper::subscribe(const char* topic, void* data, on_message callback)
{
	if (topic == NULL || strlen(topic) == 0 || callback == nullptr)
		return 0;

	int ret = mosquitto_subscribe(mosq, NULL, topic, 1);
	if (ret == MOSQ_ERR_SUCCESS) {
		cout << "Subscribed to topic: " << topic << endl;
		//mosquitto_message_callback_set(mosq, mqtt_on_message);
	}

	MQTTWrapper::callbacks.push_back({ std::string(topic), data, callback });

	return ret;
}

int MQTTWrapper::unsubscribe(const char* topic, on_message callback)
{
	if (topic == nullptr || strlen(topic) == 0 || callback == nullptr)
		return 0;

	for (auto it = MQTTWrapper::callbacks.begin(); it != MQTTWrapper::callbacks.end(); ) {
		if (it->topic == topic && it->callback == callback) {
			it = MQTTWrapper::callbacks.erase(it);
			break; 
		}
		else {
			it++;
		}
	}

	for (auto it = MQTTWrapper::callbacks.begin(); it != MQTTWrapper::callbacks.end(); ) {
		if (it->topic == topic) {
			return 0;
		}
		else {
			it++;
		}
	}

	return mosquitto_unsubscribe(mosq, NULL, topic);
}

int MQTTWrapper::loop()
{
	if (mosquitto_loop(mosq, 100, 1) != MOSQ_ERR_SUCCESS) {
		cout << "!!!!!!!!!!Reconnect!!!!!!!!!!!" << endl;
		this_thread::sleep_for(chrono::milliseconds(20));
		//mosquitto_reconnect(mosq);
	}

	return 1;
}

int MQTTWrapper::start_background_loop()
{
	return mosquitto_loop_start(mosq);
}

//************************************************************************************************
// Native callbacks
//************************************************************************************************
void mqtt_on_message(struct mosquitto* mosq, void* data, const struct mosquitto_message* msg)
{
	//if (MQTTWrapper::on_message_callback != NULL)
	//	MQTTWrapper::on_message_callback(mosq, msg->topic, (const char*)msg->payload, data);

	for (auto& subscriber : MQTTWrapper::callbacks) {
		if (subscriber.topic == msg->topic && subscriber.callback != NULL) {
			subscriber.callback(mosq, msg->topic, (const char*)msg->payload, subscriber.user_data);
		}
	}
}

void mqtt_on_connect(struct mosquitto* mosq, void* data, int)
{
	if (data == NULL) {
		cout << "mqtt_on_connect: data is NULL" << endl;
		return;
	}

	cout << "mqtt_on_connect" << endl;
	MQTTWrapper* mqtt = (MQTTWrapper*)data;
	mqtt->is_connect = true;
}

void mqtt_on_disconnect(struct mosquitto* mosq, void* data, int)
{
	cout << "mqtt_on_disconnect" << endl;
	mosquitto_reconnect(mosq);
}

void mqtt_on_subscribe(struct mosquitto* mosq, void* data, int mid, int qos_count, const int* granted_qos)
{
	cout << "mqtt_on_subscribe: mid: " << mid << " qos_count: " << qos_count << " granted_qos: ";
	for (int i = 0; i < qos_count; i++) {
		cout << granted_qos[i] << " ";
	}
	cout << endl;
}

