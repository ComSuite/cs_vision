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

	if (command == nullptr) {
		command = new command_processor();
		if (command == nullptr)
			return;
	}

	((JsonWrapper*)command)->parse(payload.c_str());
	switch (command->command_id)
	{
	case command_processor::COMMAND_ID_SET_LLM_PROMPT:
	{
		auto val = command->get_item_value("prompt");
		if (val != nullptr) {
			std::string prompt = std::get<std::string>(val->value);
			requests.push_back(prompt);
		}

		break;
	}
	default:
		break;
	}
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

	return 1;
}

bool MQTTRequest::is_opened()
{
	return mqtt != nullptr && mqtt->is_connect;
}

int MQTTRequest::get_frame(cv::Mat& frame, bool convert_to_gray)
{
	frame.release();
	if (requests.empty())
		return 0;
	
	std::string request = requests.front();
	requests.erase(requests.begin());

	std::cout << "Processing request: " << request << std::endl;
	// Simulate filling the frame with dummy data
	frame = cv::Mat::zeros(1, request.size(), CV_8UC3);
	memcpy(frame.data, request.c_str(), request.size());
	
	return 1;
}

int MQTTRequest::get_frame(const char* name, cv::Mat& frame, bool convert_to_gray)
{
	return get_frame(frame, convert_to_gray);
}