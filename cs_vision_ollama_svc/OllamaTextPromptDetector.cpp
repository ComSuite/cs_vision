#include "OllamaTextPromptDetector.h"

using namespace cs;

int OllamaTextPromptDetector::init(object_detector_environment& env)
{
	OllamaDetector::init(env);

	return 1;
}

void OllamaTextPromptDetector::clear()
{

}

int OllamaTextPromptDetector::detect(cv::Mat* input, int& current_id, bool is_draw, std::list<DetectionItem*>* detections)
{
	const char* prompt = (const char*)input->data;
	if (prompt == nullptr || strlen(prompt) == 0) {
		return 0; 
	}

	set_prompt(prompt);
	OllamaDetector::detect(nullptr, current_id, is_draw, detections);

	return 1;
}

void OllamaTextPromptDetector::parse(const std::string& response, int& current_id)
{
	std::cout << "\a";

	DetectionItem* item = new DetectionItem();
	item->id = current_id;
	current_id++;
	item->label = response;
	item->score = 1.0f;

	last_detections.push_back(item);
}

