#pragma once
#include "OllamaDetector.h"

namespace cs
{
	class QwenDetector : public OllamaDetector
	{
	public:
		virtual int init(object_detector_environment& env) override;
		virtual void parse(const std::string& response, int& current_id) override;

		command_processor* get_command_processor() { return command; }
	private:
		command_processor* command = nullptr;
	};
}

