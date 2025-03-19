#include "engine_information.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cuda_runtime.h>

using namespace std;
using namespace cs;

// Helper function to check CUDA errors
#define CHECK_CUDA(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// Helper function to check TensorRT return status
#define CHECK(status) \
    do { \
        if (!status) { \
            std::cerr << "TensorRT error: " << status << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// Print engine information
void engine_information::printEngineInfo(const nvinfer1::ICudaEngine* engine) 
{
    std::cout << "===== TensorRT Engine Information =====" << std::endl;
	std::cout << "Name: " << engine->getName() << std::endl;

    // Print number of bindings (inputs + outputs)
    int numBindings = engine->getNbIOTensors();
    std::cout << "Number of tensors: " << numBindings << std::endl;

    for (int i = 0; i < numBindings; ++i) {
        const char* bindingName = engine->getIOTensorName(i);
        std::cout << "\nTensor " << i << ":" << std::endl;
        std::cout << "  Name: " << bindingName << std::endl;

        // Check if the binding is an input or output
        nvinfer1::TensorIOMode iomode = engine->getTensorIOMode(bindingName);
        switch (iomode) {
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

        // Get binding dimensions
        nvinfer1::Dims dims = engine->getTensorShape(bindingName);
        std::cout << "  Dimensions: (";
        for (int j = 0; j < dims.nbDims; ++j) {
            std::cout << dims.d[j];
            if (j < dims.nbDims - 1) std::cout << ", ";
        }
        std::cout << ")" << std::endl;

        // Get data type of the binding
        nvinfer1::DataType dtype = engine->getTensorDataType(bindingName);
        std::cout << "  Data Type: ";
        switch (dtype) {
        case nvinfer1::DataType::kFLOAT: std::cout << "FP32"; break;
        case nvinfer1::DataType::kHALF: std::cout << "FP16"; break;
        case nvinfer1::DataType::kINT8: std::cout << "INT8"; break;
        case nvinfer1::DataType::kINT32: std::cout << "INT32"; break;
        case nvinfer1::DataType::kBOOL: std::cout << "BOOL"; break;
		case nvinfer1::DataType::kUINT8: std::cout << "UINT8"; break;
        default: std::cout << "Unknown"; break;
        }
        std::cout << std::endl;
    }

    // Print number of layers
    int numLayers = engine->getNbLayers();
    std::cout << "\nNumber of layers: " << numLayers << std::endl;

	// Print layers
    std::cout << "=====================================" << std::endl;
}

void engine_information::print(const std::string& engine_name) 
{
    // Initialize TensorRT logger
    class Logger : public nvinfer1::ILogger {
        void log(Severity severity, const char* msg) noexcept override {
            if (severity <= Severity::kWARNING) {
                std::cout << "[TensorRT] " << msg << std::endl;
            }
        }
    } logger;

    // Load the TensorRT engine from a file
    //std::string engineFile = "model.engine";
    std::ifstream file(engine_name, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open engine file: " << engine_name << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> engineData(fileSize);
    file.read(engineData.data(), fileSize);
    file.close();

    // Create a runtime and deserialize the engine
    nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(logger);
    if (!runtime) {
        std::cerr << "Failed to create TensorRT runtime." << std::endl;
        return;
    }

    nvinfer1::ICudaEngine* engine = runtime->deserializeCudaEngine((void*)engineData.data(), fileSize);
    if (!engine) {
        std::cerr << "Failed to deserialize CUDA engine." << std::endl;
        delete runtime; // ->destroy();
        return;
    }

    // Print engine information
    printEngineInfo(engine);

    // Clean up
    delete engine;
    delete runtime; // ->destroy();
}
