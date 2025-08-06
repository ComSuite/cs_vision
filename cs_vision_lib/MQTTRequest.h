#pragma once
#include <string>
#include <variant>
#include "ICamera.h"
#include "MQTTWrapper.h"
#include <command_processor.h>

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

		virtual int get_frame(const char* name, cv::Mat& frame, bool convert_to_gray) override;
		virtual int get_frame(cv::Mat& frame, bool convert_to_gray) override;

		virtual int get_width() override { return 5000; }
		virtual int get_height() override { return 1; }
		virtual int get_fps() override { return 0; }

		virtual bool is_end_of_file() override { return 0; }

		virtual bool is_opened() override;
		virtual void bring_to_start() override {}

		void on_request(const std::string& topic, const std::string& payload);
	private:
		cs::MQTTWrapper* mqtt;
		std::string mqtt_request_topic = "";

		std::vector<std::string> requests;
		command_processor command;
	};
}

