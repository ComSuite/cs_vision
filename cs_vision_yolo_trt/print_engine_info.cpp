#include "print_engine_info.h"
#include <iostream>
#include <string>

inline std::string datatype_to_string(nvinfer1::DataType type)
{
    switch (type) {
    case nvinfer1::DataType::kFLOAT:
        return "fp32";
    case nvinfer1::DataType::kHALF:
        return "fp16";
    case nvinfer1::DataType::kINT8:
        return "INT8";
    case nvinfer1::DataType::kINT32:
        return "INT32";
    case nvinfer1::DataType::kBOOL:
        return "bool8";
#if NV_TENSORRT_MAJOR >= 10
    case nvinfer1::DataType::kUINT8:
        return "UINT8";
#endif
    }

    return "unknown";
}

void print_engine_info(nvinfer1::ICudaEngine* engine)
{
    std::cout << std::endl << "===== TensorRT Engine Information =====" << std::endl;
    std::cout << "Name: " << engine->getName() << std::endl;

#if NV_TENSORRT_MAJOR >= 10 || TRT_BUILD_RTX == 21
    int num_tensors = engine->getNbIOTensors();
    std::cout << "Number of tensors: " << num_tensors << std::endl;

    for (int i = 0; i < num_tensors; i++) {
        const char* tensor_name = engine->getIOTensorName(i);
        std::cout << "\nTensor " << i << ":" << std::endl;
        std::cout << "  Name: " << tensor_name << std::endl;

        nvinfer1::TensorIOMode io_mode = engine->getTensorIOMode(tensor_name);
        switch (io_mode) {
        case nvinfer1::TensorIOMode::kINPUT:
            std::cout << "  Type: Input" << std::endl;
            break;
        case nvinfer1::TensorIOMode::kOUTPUT:
            std::cout << "  Type: Output" << std::endl;
            break;
        default:
            std::cout << "  Type: Unknown" << std::endl;
            break;
        }

        nvinfer1::Dims dims = engine->getTensorShape(tensor_name);
        std::cout << "  Dimensions: (";
        for (int j = 0; j < dims.nbDims; j++) {
            std::cout << dims.d[j];
            if (j < dims.nbDims - 1)
                std::cout << ", ";
        }
        std::cout << ")" << std::endl;

        std::cout << "  Data Type: ";
        std::cout << datatype_to_string(engine->getTensorDataType(tensor_name)) << std::endl << std::endl;
    }
#endif
    int numLayers = engine->getNbLayers();
    std::cout << "\nNumber of layers: " << numLayers << std::endl;

    std::cout << "=====================================" << std::endl << std::endl;
}

