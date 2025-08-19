#pragma once

#include <NvInfer.h>
#if NV_TENSORRT_MAJOR < 10
#include <NvInferRuntime.h>
#endif
#include <opencv2/opencv.hpp>
#include <vector>

struct Detection
{
    float conf;
    int class_id;
    cv::Rect bbox;
};

class TensorRT
{
public:
    void preprocess(cv::Mat& image, float* input_buffer);

    int get_model_width() { return input_w; }
    int get_model_height() { return input_h; }

    nvinfer1::ICudaEngine* get_engine() { return engine; }
protected:
    cudaStream_t stream;
    nvinfer1::IRuntime* runtime;
    nvinfer1::ICudaEngine* engine;
    nvinfer1::IExecutionContext* context;

    int input_w;
    int input_h;
    int num_detections;
    int detection_attribute_size;
    int num_classes = 80;
    const int MAX_IMAGE_SIZE = 4096 * 4096;
    float conf_threshold = 0.3f;
    float nms_threshold = 0.4f;
};

class TRTYolo : public TensorRT
{
public:
    TRTYolo();
    TRTYolo(std::string model_path, nvinfer1::ILogger& logger);
    ~TRTYolo();

    void infer();
    void preprocess(cv::Mat& image) {
        TensorRT::preprocess(image, gpu_buffers[0]);
    }

    void postprocess(std::vector<Detection>& output);
    void build(std::string onnx_path, nvinfer1::ILogger& logger);
    bool save_engine(const std::string& filename);

private:
    void init(std::string engine_path, nvinfer1::ILogger& logger);

    float* gpu_buffers[2];               
    float* cpu_output_buffer = nullptr;

    std::vector<cv::Rect> boxes;
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<int> nms_result;

    std::vector<cv::Scalar> colors;
};