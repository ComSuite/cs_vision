#pragma once
#include <NvInfer.h>
#include <string>

namespace cs
{
	class engine_information
	{
	public:
		static void printEngineInfo(const nvinfer1::ICudaEngine* engine);
		static void print(const std::string& engine_name);
	};
}

