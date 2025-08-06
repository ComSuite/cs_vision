#pragma once
#include "OllamaDetector.h"

namespace cs
{
	class OllamaTextPromptDetector : public OllamaDetector
	{
	public:
		OllamaTextPromptDetector() : OllamaDetector() {};
		~OllamaTextPromptDetector() {};
		virtual int init(object_detector_environment& env) override;
		virtual void clear() override;
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) override;
		virtual void parse(const std::string& response, int& current_id) override;
	};
}

