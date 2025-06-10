#include "QwenDetector.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/rapidjson.h"

using namespace rapidjson;
using namespace cs;
using namespace std;

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

						item->box.width = (box[2].GetFloat() - box[0].GetFloat()) / 0.30;
						item->box.height = (box[3].GetFloat() - box[1].GetFloat()) / 0.30;
						item->box.x = box[0].GetFloat() / 0.30;
						item->box.y = box[1].GetFloat() / 0.30;
						cout << "QwenDetector::parse: " << item->label << " " << item->box.x << " " << item->box.y << " " << item->box.width << " " << item->box.height << endl;

						last_detections.push_back(item);
					}
				}
			}
		}
	}
	catch (...) {

	}
}
