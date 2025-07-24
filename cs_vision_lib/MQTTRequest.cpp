#include "MQTTRequest.h"

using namespace cs;

void on_request_message(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{
	if (data == nullptr)
		return;

	MQTTRequest* request = (MQTTRequest*)data;
}

void MQTTRequest::on_request_message(const std::string& topic, const std::string& payload)
{

}

int MQTTRequest::open(camera_settings* settings, void* param)
{
	if (settings == nullptr || param == nullptr)
		return 0;

	mqtt = new MQTTWrapper(((MQTTWrapper*)param)->get_handle());
	if (mqtt == nullptr)
		return 0;

	mqtt_request_topic = settings->additional.get<std::string>("mqtt_request_topic", mqtt_request_topic);
	if (!mqtt_request_topic.empty())
		mqtt->subscribe(mqtt_request_topic.c_str(), this, on_request_message);
}

