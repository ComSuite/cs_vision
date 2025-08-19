#pragma once

#include <NvInfer.h>
#if NV_TENSORRT_MAJOR < 10
#include <NvInferRuntime.h>
#endif

void print_engine_info(nvinfer1::ICudaEngine* engine);

