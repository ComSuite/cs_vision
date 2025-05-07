#pragma once

#include "IObjectDetector.h"

namespace cs
{
	class GemmaDetector : public IObjectDetector
	{
	public:
		GemmaDetector();
		~GemmaDetector();

		virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false)  override;
		virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false)  override;
		virtual int init(void* param, bool is_use_gpu = false)  override { return 0; };
		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;
	private:
		std::string endpoint = "";
		std::string model = "";
		std::string prompt = "";
	};
}
