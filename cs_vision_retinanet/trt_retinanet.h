#pragma once

#include "trt_yolo.h"

class TRTRetinaNet : public TensorRT
{
public:
	TRTRetinaNet(std::string model_path, nvinfer1::ILogger& logger);
	virtual ~TRTRetinaNet() {}

	void infer();
private:
	void init(); 
};

