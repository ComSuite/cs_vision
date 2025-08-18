#pragma once

#include "IObjectDetector.h"

namespace cs
{
	class TRTRetinaNetObjectDetector : public IObjectDetector
	{
	public:
		TRTRetinaNetObjectDetector();
		virtual ~TRTRetinaNetObjectDetector();

		virtual int init(object_detector_environment& env) override;

		virtual void clear();

		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;
		virtual void parse(const std::string& payload, int& current_id);

		virtual void set_prompt(const std::string& prompt) {
		}
	};
}

