/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2021, Alexander Epstine (a@epstine.com)
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

#include "TRTYOLOv5ObjectDetector.h"

using namespace std;
using namespace cv;
using namespace cs;
using namespace rapidjson;

class TRTLogger : public yolov5::Logger {
public:
    void print(const yolov5::LogLevel& level, const char* msg) override {

    }
};

TRTYOLOv5ObjectDetector::TRTYOLOv5ObjectDetector()
{
    //cout << "TFLite version: " << TfLiteVersion() << endl;
}

TRTYOLOv5ObjectDetector::~TRTYOLOv5ObjectDetector()
{
    clear();
}

int TRTYOLOv5ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu)
{
    load_rules(rules_path);

    detector.init(yolov5::DetectorFlag::PREPROCESSOR_CVCUDA);
    detector.loadEngine(model_path, input_tensor_name, output_tensor_name);
    shared_ptr<TRTLogger> logger(new TRTLogger);
    detector.setLogger(logger);

    if (classes.loadFromFile(label_path) == yolov5::RESULT_SUCCESS)
        detector.setClasses(classes);

    width = detector.inferenceSize().width;
    height = detector.inferenceSize().height;

    return 1;
}

int TRTYOLOv5ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
    return init(model_path, label_path, rules_path, "images", "output", is_use_gpu);
}

void TRTYOLOv5ObjectDetector::clear()
{

}

int TRTYOLOv5ObjectDetector::detect_batch(const std::vector<Mat*>& input, int& current_id, bool is_draw)
{
    clear_last_detections();
    std::vector<std::vector<yolov5::Detection>> detections;

    //if (detector.detectBatch(images, &detections) != yolov5::Result::RESULT_SUCCESS)
    //    return 0;

    for (auto detection : detections) {
        for (auto detect : detection) {

        }
    }

    return 0;
}

void TRTYOLOv5ObjectDetector::postprocess(std::vector<yolov5::Detection>& detections, int& current_id, bool is_draw, cv::Mat* image)
{
    for (auto detection : detections) {
        DetectionItem* item = new DetectionItem();
        item->color = color;
        if (check_rule(detection.classId(), detection.score(), item->color)) {
            item->id = current_id;
            current_id++;

            item->detector_id = id;
            item->kind = ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLOv5;
            item->class_id = detection.classId();
            item->label = trim(detection.className());
            item->score = detection.score();
            item->box = detection.boundingBox();
            item->neural_network_id = neural_network_id;

            last_detections.push_back(item);

            if (is_draw && image != nullptr) {
                cv::rectangle(*image, item->box, cv::Scalar(255, 0, 0), 2);
                cv::putText(*image, detection.className(), cv::Point(item->box.x, item->box.y), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
            }
        }
        else
            delete item;
    }
}

int TRTYOLOv5ObjectDetector::detect(cv::cuda::GpuMat* image, int& current_id, bool is_draw)
{
    if (!image)
        return 0;

    clear_last_detections();
    std::vector<yolov5::Detection> detections;

    if (detector.detect(*image, &detections) != yolov5::Result::RESULT_SUCCESS)
        return 0;

    postprocess(detections, current_id, false);

    return 1;
}

int TRTYOLOv5ObjectDetector::detect(Mat* image, int& current_id, bool is_draw)
{
    if (!image)
        return 0;

    clear_last_detections();
    std::vector<yolov5::Detection> detections;

    if (detector.detect(*image, &detections) != yolov5::Result::RESULT_SUCCESS)
        return 0;

    postprocess(detections, current_id, is_draw);

    return 1;
}


