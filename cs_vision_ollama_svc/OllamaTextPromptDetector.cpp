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
		return 0; // No prompt provided
	}

	std::cout << "OllamaTextPromptDetector::detect: Using prompt: " << prompt << std::endl;

	return 1;
}

void OllamaTextPromptDetector::parse(const std::string& response, int& current_id)
{
}

