#pragma once
#include <string>
#include <variant>
#include "ICamera.h"
#include "MQTTWrapper.h"

namespace cs
{
	class MQTTRequest : public ICamera
	{
	public:
		MQTTRequest() : ICamera() {};
		virtual ~MQTTRequest() {};

		virtual int info() override { return 0; }
		virtual int open(camera_settings* settings, void* param = nullptr) override;
		virtual int close() override { return 0; }
		virtual int prepare() override { return 0; }
		virtual int save_to_file() override { return 0; }

#ifdef  __HAS_CUDA__
		virtual int get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray) override { return 0; }
		virtual int get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray) override { return 0; }
#endif 

		virtual int get_frame(const char* name, cv::Mat& frame, bool convert_to_gray) override { return 0; }
		virtual int get_frame(cv::Mat& frame, bool convert_to_gray) override { return 0; }

		virtual int get_width() override { return 0; }
		virtual int get_height() override { return 0; }
		virtual int get_fps() override { return 0; }

		virtual bool is_end_of_file() override { return 0; }

		virtual bool is_opened() override { return 0; }
		virtual void bring_to_start() override { }

		void on_request_message(const std::string& topic, const std::string& payload);
	private:
		cs::MQTTWrapper* mqtt;
		std::string mqtt_request_topic = "";
	};
}

