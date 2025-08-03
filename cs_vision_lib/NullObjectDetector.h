#pragma once

#include "IObjectDetector.h"

namespace cs
{
	class NullObjectDetector : public IObjectDetector
	{
	public:
		virtual int init(object_detector_environment& env) override { return 1; }

		virtual void clear() { };
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) { return 0; }
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) { return 0; }
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override { return 0; }
	};
}

