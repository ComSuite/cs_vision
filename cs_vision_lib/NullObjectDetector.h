#pragma once

#include "IObjectDetector.h"

namespace cs
{
	class NullObjectDetector : public IObjectDetector
	{
	public:
		virtual int init(object_detector_environment& env) override { return 1; }

		//virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false) override { return 1; }
		//virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false) override { return 1; }
		//virtual int init(void* param, bool is_use_gpu = false) override { return 0; };
		virtual void clear() { };
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) { return 0; }
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) { return 0; }
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override { return 0; }
	};
}

