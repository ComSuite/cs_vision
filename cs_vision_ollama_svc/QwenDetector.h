#pragma once
#include "OllamaDetector.h"

namespace cs
{
	class QwenDetector : public OllamaDetector
	{
	public:
		virtual int init(object_detector_environment& env) override;
		virtual void parse(const std::string& response, int& current_id) override;
	};
}

