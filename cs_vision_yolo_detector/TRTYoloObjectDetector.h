#pragma once

#include "JsonWrapper.h"
#include "IObjectDetector.h"
#include "yolov11.h"

namespace cs
{
	class TRTYoloObjectDetector : public IObjectDetector
	{
	public:
		TRTYoloObjectDetector();
		virtual ~TRTYoloObjectDetector();

		virtual int init(object_detector_environment& env) override;

		//virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false)  override;
		//virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false)  override;
		//virtual int init(void* param, bool is_use_gpu = false)  override { return 0; };

		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;
	private:
		const char* default_input_tensor_name = "";
		const char* default_output_tensor_name = "";

		std::unique_ptr<YOLOv11> detector = nullptr;

		void postprocess(std::vector<Detection>* detections, int& current_id, bool is_draw, int model_h, int model_w, cv::Mat* image);
	};
}

