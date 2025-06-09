#pragma once
#include "OllamaDetector.h"

namespace cs
{
	class QwenDetector : public OllamaDetector
	{
	public:
		virtual void parse(const std::string& response, int& current_id) override
		{
			// Qwen's response parsing logic
			// This is a placeholder; actual implementation will depend on Qwen's response format
			std::cout << "Parsing Qwen response: " << response << std::endl;
		}
	};
}

