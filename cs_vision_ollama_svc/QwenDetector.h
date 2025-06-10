#pragma once
#include "OllamaDetector.h"

namespace cs
{
	class QwenDetector : public OllamaDetector
	{
	public:
		virtual void parse(const std::string& response, int& current_id) override;
	};
}

