/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2024, Alexander Epstine (a@epstine.com)
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

#include "OCVYOLOv8ObjectDetector.h"

using namespace cs;
using namespace rapidjson;
using namespace std;
using namespace cv;

OCVYOLOv8ObjectDetector::OCVYOLOv8ObjectDetector()
{

}

OCVYOLOv8ObjectDetector::~OCVYOLOv8ObjectDetector()
{

}

int OCVYOLOv8ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
	return 0;
}

int OCVYOLOv8ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu)
{
	detector = new Inference(model_path, cv::Size(640, 640), label_path, is_use_gpu);

    width = 640;
    height = 640;

	return 0;

}

void OCVYOLOv8ObjectDetector::clear()
{

}

int OCVYOLOv8ObjectDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
    return 1;
}

int OCVYOLOv8ObjectDetector::detect(cv::Mat* input, int& current_id, bool is_draw)
{
    if (input == nullptr)
		return 0;

    std::vector<Detection> output = detector->runInference(*input);

    for (auto& item : output)
    {
        DetectionItem* detection = new DetectionItem();
        detection->color = color;
        if (check_rule(item.class_id, item.confidence, detection->color)) {
            detection->id = current_id;
            current_id++;

            detection->detector_id = id;
            detection->kind = ObjectDetectorKind::OBJECT_DETECTOR_OPENCV_YOLOv8;
            detection->class_id = item.class_id;
            detection->label = trim(item.className);
            detection->score = item.confidence;
            detection->box = item.box;
            detection->neural_network_id = neural_network_id;

            last_detections.push_back(detection);

            if (is_draw) {
                cv::rectangle(*input, detection->box, cv::Scalar(255, 0, 0), 2);
                cv::putText(*input, detection->label, cv::Point(detection->box.x, detection->box.y), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
            }
        }
        else
            delete detection;
    }

	return output.size() > 0;
}

int OCVYOLOv8ObjectDetector::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	return 0;
}

