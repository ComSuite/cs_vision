#include "QwenDetector.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/rapidjson.h"

using namespace rapidjson;
using namespace cs;
using namespace std;

void on_detector_message(struct mosquitto* mosq, const char* topic, const char* payload, void* data)
{
	if (data == nullptr)
		return;

	QwenDetector* detector = static_cast<QwenDetector*>(data);

	command_processor* command = detector->get_command_processor();
	if (command == nullptr) {
		return;
	}

	((JsonWrapper*)command)->parse(payload);
	switch (command->command_id)
	{
	case command_processor::COMMAND_ID_SET_LLM_PROMPT:
	{
		auto val = command->get_item_value("prompt");
		if (val != nullptr) {
			std::string prompt = std::get<std::string>(val->value);
			detector->set_prompt(prompt);
			cout << "QwenDetector::on_detector_message: Set prompt: " << prompt << endl;
		}
		else {
			cout << "QwenDetector::on_detector_message: No prompt found in command" << endl;
		}
		break;
	}
	default:
		break;
	}
}

int QwenDetector::init(object_detector_environment& env)
{
	OllamaDetector::init(env);

	if (env.mqtt_wrapper == nullptr) {
		return 0;
	}

	command = new command_processor();
	if (command == nullptr) {
		return 0;
	}

	if (env.additional != nullptr) {
		std::string topic = env.additional->get<std::string>("mqtt_request_topic", "");
		
		mqtt_subscribe(env.mqtt_wrapper, topic.c_str(), this, on_detector_message);
	}

	return 0;
}

void QwenDetector::parse(const std::string& response, int& current_id)
{
	const std::string pattern_begin = "```json";
	const std::string pattern_end = "```";

	std::string resp = response;
	auto pos = resp.find(pattern_begin);
	if (pos != std::string::npos) {
		resp.erase(pos, pattern_begin.length());
	}

	pos = resp.find(pattern_end);
	if (pos != std::string::npos) {
		resp.erase(pos, pattern_end.length());
	}

	trim(resp);

	Document root;
	try {
		if (!root.Parse(resp.c_str()).HasParseError()) {
			if (root.IsArray()) {
				for (auto& item : root.GetArray()) {
					if (item.IsObject()) {
						auto box = item["bbox_2d"].GetArray();
						auto label = item["label"].GetString();

						DetectionItem* item = new DetectionItem();
						item->id = current_id;
						current_id++;

						item->kind = ObjectDetectorKind::OBJECT_DETECTOR_QWEN;
						item->detector_id = id;

						item->label = trim(label);
						item->neural_network_id = neural_network_id;

						double k = 0.50;

						item->box.width = (box[2].GetFloat() / k - box[0].GetFloat()) / k;
						item->box.height = (box[3].GetFloat() / k - box[1].GetFloat()) / k;
						item->box.x = box[0].GetFloat() / k;
						item->box.y = box[1].GetFloat() / k;
						//cout << "QwenDetector::parse: " << item->label << " " << item->box.x << " " << item->box.y << " " << item->box.width << " " << item->box.height << endl;

						last_detections.push_back(item);
					}
				}
			}
		}
	}
	catch (...) {

	}
}
