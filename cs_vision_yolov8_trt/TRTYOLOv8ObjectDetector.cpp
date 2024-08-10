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

#include "TRTYOLOv8ObjectDetector.h"

using namespace cs;
using namespace cv;
using namespace std;

TRTYOLOv8ObjectDetector::TRTYOLOv8ObjectDetector()
{

}

TRTYOLOv8ObjectDetector::~TRTYOLOv8ObjectDetector()
{

}

int TRTYOLOv8ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
	return init(model_path, label_path, rules_path, default_input_tensor_name, default_output_tensor_name, is_use_gpu);
}

int TRTYOLOv8ObjectDetector::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu) 
{
	load_rules(rules_path);
	load_labels(label_path);
	config.classNames.clear();
	config.classNames = labels;

	detector = make_unique<YoloV8>(model_path, config);

	return 1;
}

void TRTYOLOv8ObjectDetector::clear()
{

}

void TRTYOLOv8ObjectDetector::postprocess(std::vector<Object>* detections, int& current_id, bool is_draw, cv::Mat* image)
{
	if (detections == nullptr)
		return; 

	if (detections->size() <= 0)
		return;

	for (auto detection : *detections) {
		DetectionItem* item = new DetectionItem();
		item->color = color;
		if (check_rule(detection.label, detection.probability, item->color)) {
			item->id = current_id;
			current_id++;

			item->kind = ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLOv8;
			item->class_id = detection.label;
			item->detector_id = id;

			std::string label;
			get_rule_label(detection.label, label);
			item->label = trim(label);
			item->score = detection.probability;
			item->box = detection.rect;
			item->neural_network_id = neural_network_id;

			last_detections.push_back(item);
		}
		else
			delete item;
	}

	if (is_draw && image != nullptr) {
		detector->drawObjectLabels(*image, *detections);
	}
}

int TRTYOLOv8ObjectDetector::detect(cv::Mat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();
	auto detections = detector->detectObjects(*input);
	postprocess(&detections, current_id, is_draw, input);

	return detections.size() > 0;
}

int TRTYOLOv8ObjectDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();
	auto detections = detector->detectObjects(*input);

	if (is_draw) {
		cv::Mat img;
		input->download(img);
		postprocess(&detections, current_id, is_draw, &img);
		input->upload(img);
	}
	else
		postprocess(&detections, current_id, false, nullptr);

	return detections.size() > 0;
}

int TRTYOLOv8ObjectDetector::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	return 0;
}
