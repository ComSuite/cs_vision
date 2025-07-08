#include "MQTTRequest.h"

using namespace cs;

void on_request_message(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{

}

int MQTTRequest::open(camera_settings* settings) 
{
	if (settings == nullptr)
		return 0;

	settings->additional.get<std::string>("mqtt_request_topic", mqtt_request_topic);
	if (!mqtt_request_topic.empty())
		mqtt.subscribe(mqtt_request_topic.c_str(), settings, on_request_message);
}

