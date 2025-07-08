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

		int info() override { return 0; }

		int open(std::variant<std::string, int> device, const int frame_width = 0, const int frame_height = 0) override { return 0; }
		int open(const int id, const int frame_width = 0, const int frame_height = 0) override { return 0; }
		int open(const char* name) override { return 0; }
		int open(const int id, int attempts_count) override { return 0; }
		int open(const char* name, int attempts_count) override { return 0; }
		int open(camera_settings* settings) override;

		int close() override { return 0; }

		int prepare() override { return 0; }
		int save_to_file() override { return 0; }
	private:
		cs::MQTTWrapper mqtt;
		std::string mqtt_request_topic = "";
	};
}

