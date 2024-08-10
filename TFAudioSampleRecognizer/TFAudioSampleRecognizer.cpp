/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2023, Alexander Epstine (a@epstine.com)
 **************************************************************************************
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "TFAudioSampleRecognizer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <exception>
#include <iterator>
#include <algorithm>
#include <string.h>

#include <opencv2/plot.hpp>

#ifdef __HAS_CUDA__
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudawarping.hpp>
#endif

using namespace std;
using namespace cs;

TFAudioSampleRecognizer::TFAudioSampleRecognizer()
{
   std::cout << TfLiteVersion() << std::endl;
}

TFAudioSampleRecognizer::~TFAudioSampleRecognizer()
{
    clear();
}

int TFAudioSampleRecognizer::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu)
{
    return init(model_path, label_path, rules_path, is_use_gpu);
}

int TFAudioSampleRecognizer::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
    model = TfLiteModelCreateFromFile(model_path);
    if (!model)
        return 0;

    options = TfLiteInterpreterOptionsCreate();

    if (!options) {
        clear();
        return 0;
    }

    TfLiteInterpreterOptionsSetNumThreads(options, 2);
    TfLiteInterpreterOptionsSetErrorReporter(options, default_error_reporter, NULL);

    interpreter = TfLiteInterpreterCreate(model, options);
    if (!interpreter) {
        clear();
        return 0;
    }

    int32_t in_tensor_count = TfLiteInterpreterGetInputTensorCount(interpreter);
    std::cout << "Input Tensor Count: " << in_tensor_count << std::endl;
    if (in_tensor_count != 1) {
        clear();
        return 0;
    }

    int32_t out_tensor_count = TfLiteInterpreterGetOutputTensorCount(interpreter);
    std::cout << "Output Tensor Count: " << out_tensor_count << std::endl;
    if (out_tensor_count != 1) {
        clear();
        return 0;
    }

    TfLiteStatus status = TfLiteInterpreterAllocateTensors(interpreter);
    if (status != kTfLiteOk) {
        clear();
        return 0;
    }

    input_tensor = TfLiteInterpreterGetInputTensor(interpreter, 0);
    width = input_tensor->bytes;
    height = 1;
    cout << "Input tensor info:" << endl;
    cout << input_tensor->name << endl;
    cout << input_tensor->dims->size << endl;
    cout << input_tensor->type << endl;
    cout << input_tensor->bytes << endl;

    output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);
    cout << "Output tensor info:" << endl;
    cout << output_tensor->name << endl;
    cout << output_tensor->dims->size << endl;
    cout << output_tensor->type << endl;
    cout << output_tensor->bytes << endl;

    load_labels(label_path);

    return 1;
}

void TFAudioSampleRecognizer::clear()
{
    if (interpreter)
        TfLiteInterpreterDelete(interpreter);

    if (options)
        TfLiteInterpreterOptionsDelete(options);

    if (model)
        TfLiteModelDelete(model);

    model = NULL;
    options = NULL;
    interpreter = NULL;
    input_tensor = NULL;
    metal_delegate = NULL;
}

int TFAudioSampleRecognizer::detect(cv::Mat* input, int& current_id, bool is_draw)
{
    if (input == nullptr)
        return 0;

    clear_last_detections();

    char* dst = (char*)input_tensor->data.f;
    memcpy(dst, input->data, width);

    if (TfLiteInterpreterInvoke(interpreter) != kTfLiteOk) {
        cout << "Error invoking detection model" << endl;
        return 0;
    }

    int detected_index = -1;
    float cur = 0;
    for (int i = 0; i < output_tensor->bytes / sizeof(float); i++) {
        if (output_tensor->data.f[i] > cur) {
            cur = output_tensor->data.f[i];
            detected_index = i;
        }
    }

    DetectionItem* item = new DetectionItem();
    item->id = current_id;
    item->detector_id = id;
    current_id++;
    item->kind = ObjectDetectorKind::AUDIO_RECOGNIZER_TFLITE;
    if (detected_index >= 0)
        item->label = trim(labels[detected_index]);
    else
        item->label = "none";

    item->class_id = detected_index;
    item->score = cur;
    item->box.x = 0;
    item->box.y = 0;
    item->box.width = 0;
    item->box.height = 0;
    item->neural_network_id = neural_network_id;

    last_detections.push_back(item);

    return 1;
}

int TFAudioSampleRecognizer::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
    if (input == nullptr)
        return 0;

    cv::Mat _input;
    input->download(_input);

    return detect(&_input, current_id, is_draw);
}

void window_function(cv::Mat& data, float** vals, float& min, float& max)
{
    float multiplier = 0;
    int frames_per_buffer = data.cols; // / sizeof(float)
    *vals = (float*)malloc(data.cols * sizeof(float));

    float* p = (float*)data.ptr();
    float* v = *vals;
    for (int i = 0; i < data.cols / sizeof(float); i++) {
        multiplier = (float)(0.5 * (1 - cos(2 * M_PI * i / (frames_per_buffer))));
        *v = *p * multiplier;
        if (*v < min)
            min = *v;
        if (*v > max)
            max = *v;
        p++;
        v++;
    }
}

#ifdef __HAS_CUDA__
void TFAudioSampleRecognizer::draw_detection(cv::cuda::GpuMat* detect_frame, DetectionItem* detection, cv::Scalar& background_color, bool is_show_mask)
#else
void TFAudioSampleRecognizer::draw_detection(cv::Mat* detect_frame, DetectionItem* detection)
#endif
{
    cv::Mat input;
    detect_frame->download(input);
    int num_channels = 1;
    float* vals = nullptr;
    float min = 0;
    float max = 0;

    window_function(input, &vals, min, max);
    if (min == max)
        return;
    
    float* p = vals;
    int n = (input.cols / sizeof(float));
    cv::Mat frame(640, n, CV_8UC3);
    frame.setTo(background_color);

    for (int i = 0; i < n - 1; i++) {
        int x0 = i;
        int y0 = (int)((320.0 / (max - min)) * (*p)) + 320;
        p++;
        int x1 = i + 1;
        int y1 = (int)((320.0 / (max - min)) * (*p)) + 320;
        line(frame, cv::Point(x0, y0), cv::Point(x1, y1), color, 3);
    }

    cv::resize(frame, frame, cv::Size(640, 640), 0, 0, cv::INTER_AREA);
    draw_label(frame, detection->label, 10, 10, background_color, color);

    detect_frame->upload(frame);

    free(vals);
}
