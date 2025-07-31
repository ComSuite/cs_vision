#include "MQTTRequest.h"

using namespace cs;

void on_request_message(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{
	if (data == nullptr)
		return;

	MQTTRequest* request = (MQTTRequest*)data;
	request->on_request(topic, payload);
}

void MQTTRequest::on_request(const std::string& topic, const std::string& payload)
{
	if (topic != mqtt_request_topic)
		return;

	std::cout << "Received request on topic: " << topic << " Payload: " << payload << std::endl;
	// Process the request payload here

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

