#include "trt_retinanet.h"

TRTRetinaNet::TRTRetinaNet(std::string model_path, nvinfer1::ILogger& logger) : TensorRT(model_path, logger) 
{
	init();
}

void TRTRetinaNet::init()
{

}

void TRTRetinaNet::infer()
{
#if NV_TENSORRT_MAJOR < 10 && TRT_BUILD_RTX != 21
    context->enqueueV2((void**)gpu_buffers, stream, nullptr);
#else
    this->context->enqueueV3(this->stream);
#endif
}
