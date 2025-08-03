#pragma once

#include "IObjectDetector.h"
#include "tracker.h"

namespace cs
{
	class TrackerDeepSORT : public IObjectDetector
	{
		virtual int init(object_detector_environment& env) override;

		virtual void clear();

		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override { return 0; };

		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override { return 0; };

		virtual void parse(const std::string& payload, int& current_id);
	private:
		tracker* tracker = nullptr;
	};
}

