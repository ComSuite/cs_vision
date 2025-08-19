#include "trt_yolo.h"
#include "logging.h"
#include "cuda_utils.h"
#include "macros.h"
#include "preprocess.h"
#include <NvOnnxParser.h>
#include <fstream>
#include <iostream>
#ifdef __WITH_FILESYSTEM_CXX__
#include <filesystem>
#endif
#include "std_utils.h"
#include "print_engine_info.h"

using namespace nvinfer1;
using namespace std;
using namespace cv;

static Logger logger;
#define isFP16 true
#define warmup true

TensorRT::TensorRT(std::string model_path, nvinfer1::ILogger& logger)
{
#ifdef __WITH_FILESYSTEM_CXX__
    if (to_lower(std::filesystem::path(model_path).extension().string()) == ".onnx") {
#else
    if (model_path.find(".onnx") != std::string::npos) {
#endif
        build_engine(model_path, logger);
        save_engine(model_path);
    }
    else {
        init_engine(model_path, logger);
    }

#if NV_TENSORRT_MAJOR < 10 && TRT_BUILD_RTX != 21
    auto input_dims = engine->getBindingDimensions(0);
    input_h = input_dims.d[2];
    input_w = input_dims.d[3];
#else
    auto input_dims = engine->getTensorShape(engine->getIOTensorName(0));
    input_h = input_dims.d[2];
    input_w = input_dims.d[3];
#endif
}

TensorRT::~TensorRT()
{
    CUDA_CHECK(cudaStreamSynchronize(stream));
    CUDA_CHECK(cudaStreamDestroy(stream));

    cuda_preprocess_destroy();
    delete context;
    delete engine;
    delete runtime;
}

void TensorRT::preprocess(Mat& image, float* input_buffer)
{
    cuda_preprocess(image.ptr(), image.cols, image.rows, input_buffer, input_w, input_h, stream); //gpu_buffers[0]
#ifdef TRT_BUILD_RTX == 21
    CUDA_CHECK(cudaStreamSynchronize(stream));
#endif
}

bool TensorRT::init_engine(std::string engine_path, nvinfer1::ILogger& logger)
{
    ifstream engineStream(engine_path, ios::binary);
    engineStream.seekg(0, ios::end);
    const size_t modelSize = engineStream.tellg();
    engineStream.seekg(0, ios::beg);
    unique_ptr<char[]> engineData(new char[modelSize]);
    engineStream.read(engineData.get(), modelSize);
    engineStream.close();

    runtime = createInferRuntime(logger);
    engine = runtime->deserializeCudaEngine(engineData.get(), modelSize);
    if (engine == nullptr) {
        cout << "Failed to deserialize engine" << endl;
        return false;
    }

    print_engine_info(engine);
    cuda_preprocess_init(MAX_IMAGE_SIZE);
    context = engine->createExecutionContext();

    return true;
}

void TensorRT::build_engine(std::string onnx_path, nvinfer1::ILogger& logger)
{
    auto builder = createInferBuilder(logger);
    const auto explicitBatch = 1U << static_cast<uint32_t>(NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    INetworkDefinition* network = builder->createNetworkV2(explicitBatch);
    IBuilderConfig* config = builder->createBuilderConfig();
    if (isFP16) {
        config->setFlag(BuilderFlag::kFP16);
    }
    nvonnxparser::IParser* parser = nvonnxparser::createParser(*network, logger);
    bool parsed = parser->parseFromFile(onnx_path.c_str(), static_cast<int>(nvinfer1::ILogger::Severity::kINFO));
    IHostMemory* plan{ builder->buildSerializedNetwork(*network, *config) };

    runtime = createInferRuntime(logger);

    engine = runtime->deserializeCudaEngine(plan->data(), plan->size());

    context = engine->createExecutionContext();

    delete network;
    delete config;
    delete parser;
    delete plan;
}

bool TensorRT::save_engine(const std::string& filename)
{
    std::string engine_path;
    size_t dotIndex = filename.find_last_of(".");
    if (dotIndex != std::string::npos) {
        engine_path = filename.substr(0, dotIndex) + ".engine";
    }
    else {
        return false;
    }

    if (engine) {
        nvinfer1::IHostMemory* data = engine->serialize();
        std::ofstream file;
        file.open(engine_path, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            std::cout << "Create engine file" << engine_path << " failed" << std::endl;
            return 0;
        }

        file.write((const char*)data->data(), data->size());
        file.close();

        delete data;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
TRTYolo::TRTYolo(string model_path, nvinfer1::ILogger& logger) : TensorRT(model_path, logger)
{
	init();
}

TRTYolo::~TRTYolo()
{
    for (int i = 0; i < 2; i++) {
        CUDA_CHECK(cudaFree(gpu_buffers[i]));
    }

    if (cpu_output_buffer != nullptr) {
        delete[] cpu_output_buffer;
    }
}

void TRTYolo::init()
{
#if NV_TENSORRT_MAJOR < 10 && TRT_BUILD_RTX != 21
    auto input_dims = engine->getBindingDimensions(0);
    input_h = input_dims.d[2];
    input_w = input_dims.d[3];
	
    detection_attribute_size = engine->getBindingDimensions(1).d[1];
    num_detections = engine->getBindingDimensions(1).d[2];
    num_classes = detection_attribute_size - 4;
#else
    auto input_dims = engine->getTensorShape(engine->getIOTensorName(0));
    input_h = input_dims.d[2];
    input_w = input_dims.d[3];
	
	auto output_dims = engine->getTensorShape(engine->getIOTensorName(1));
    detection_attribute_size = output_dims.d[1];
    num_detections = output_dims.d[2];
    num_classes = detection_attribute_size - 4;
#endif

#ifdef TRT_BUILD_RTX == 21
    cpu_output_buffer = new float[detection_attribute_size * num_detections];

    CUDA_CHECK(cudaMalloc(&gpu_buffers[0], 3 * input_w * input_h * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&gpu_buffers[1], detection_attribute_size * num_detections * sizeof(float)));
#else
    CUDA_CHECK(cudaHostAlloc(&gpu_buffers[0], 3 * input_w * input_h * sizeof(float), cudaHostAllocWriteCombined));
    CUDA_CHECK(cudaHostAlloc(&gpu_buffers[1], detection_attribute_size * num_detections * sizeof(float), cudaHostAllocPortable));
#endif

#if NV_TENSORRT_MAJOR >= 10 || TRT_BUILD_RTX == 21
	context->setTensorAddress(engine->getIOTensorName(0), gpu_buffers[0]);
	context->setTensorAddress(engine->getIOTensorName(1), gpu_buffers[1]);
#endif

    CUDA_CHECK(cudaStreamCreate(&stream));

    if (warmup) {
        for (int i = 0; i < 10; i++) {
            this->infer();
        }
        printf("model warmup 10 times\n");
    }
}

void TRTYolo::infer()
{
#if NV_TENSORRT_MAJOR < 10 && TRT_BUILD_RTX != 21
    context->enqueueV2((void**)gpu_buffers, stream, nullptr);
#else
    this->context->enqueueV3(this->stream);
#endif
}

void TRTYolo::preprocess(cv::Mat& image) {
    TensorRT::preprocess(image, gpu_buffers[0]);
}

void TRTYolo::postprocess(vector<Detection>& output)
{
#ifdef TRT_BUILD_RTX == 21
    CUDA_CHECK(cudaMemcpyAsync(cpu_output_buffer, gpu_buffers[1], num_detections * detection_attribute_size * sizeof(float), cudaMemcpyDeviceToHost, stream));
    CUDA_CHECK(cudaStreamSynchronize(stream));
#endif

    boxes.clear();
    class_ids.clear();
    confidences.clear();

#ifdef TRT_BUILD_RTX == 21
    const Mat det_output(detection_attribute_size, num_detections, CV_32F, cpu_output_buffer);
#else
    const Mat det_output(detection_attribute_size, num_detections, CV_32F, gpu_buffers[1]);
#endif

    Point class_id_point;
    double score;
    Rect box;

    for (int i = 0; i < det_output.cols; ++i) {
        minMaxLoc(det_output.col(i).rowRange(4, 4 + num_classes), nullptr, &score, nullptr, &class_id_point);

        if (score > conf_threshold) {
            const float cx = det_output.at<float>(0, i);
            const float cy = det_output.at<float>(1, i);
            const float ow = det_output.at<float>(2, i);
            const float oh = det_output.at<float>(3, i);

            box.x = static_cast<int>((cx - 0.5 * ow));
            box.y = static_cast<int>((cy - 0.5 * oh));
            box.width = static_cast<int>(ow);
            box.height = static_cast<int>(oh);

            boxes.push_back(box);
            class_ids.push_back(class_id_point.y);
            confidences.push_back(score);
        }
    }

    nms_result.clear();
    dnn::NMSBoxes(boxes, confidences, conf_threshold, nms_threshold, nms_result);

    for (int i = 0; i < nms_result.size(); i++) {
        Detection result;
        int idx = nms_result[i];
        result.class_id = class_ids[idx];
        result.conf = confidences[idx];
        result.bbox = boxes[idx];
        output.push_back(result);
    }
}



