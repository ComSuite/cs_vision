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

#include "TRTYoloObjectDetector.h"

using namespace cs;
using namespace cv;
using namespace std;

class TRTYOLOLogger : public nvinfer1::ILogger {
	void log(Severity severity, const char* msg) noexcept override {
		if (severity <= Severity::kWARNING)
			std::cout << msg << std::endl;
	}
} trt_yolo_logger;

TRTYoloObjectDetector::TRTYoloObjectDetector()
{

}

TRTYoloObjectDetector::~TRTYoloObjectDetector()
{

}

int TRTYoloObjectDetector::init(object_detector_environment& env)
{
	load_rules(env.rules_path.c_str());
	load_labels(env.label_path.c_str());

	detector = make_unique<TRTYolo>(env.model_path, trt_yolo_logger);

	return 1;
}

void TRTYoloObjectDetector::clear()
{

}

void TRTYoloObjectDetector::postprocess(std::vector<Detection>* detections, int& current_id, bool is_draw, int model_h, int model_w, cv::Mat* image)
{
	if (detections == nullptr)
		return;

	if (detections->size() <= 0)
		return;

	const float ratio_h = model_h / (float)image->rows;
	const float ratio_w = model_w / (float)image->cols;

	for (auto detection : *detections) {
		DetectionItem* item = new DetectionItem();
		item->color = color;
		if (check_rule(detection.class_id, detection.conf, item->color)) {
			item->id = current_id;
			current_id++;

			item->kind = ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLO;
			item->class_id = detection.class_id;
			item->detector_id = id;

			std::string label;
			get_rule_label(detection.class_id, label);
			item->priority = get_rule_priority(detection.class_id);
			item->label = trim(label);
			item->score = detection.conf;
			item->box = detection.bbox;

			if (ratio_h > ratio_w)
			{
				item->box.x = item->box.x / ratio_w;
				item->box.y = (item->box.y - (model_h - ratio_w * image->rows) / 2) / ratio_w;
				item->box.width = item->box.width / ratio_w;
				item->box.height = item->box.height / ratio_w;
			}
			else
			{
				item->box.x = (item->box.x - (model_w - ratio_h * image->cols) / 2) / ratio_h;
				item->box.y = item->box.y / ratio_h;
				item->box.width = item->box.width / ratio_h;
				item->box.height = item->box.height / ratio_h;
			}

			item->neural_network_id = neural_network_id;

			last_detections.push_back(item);
		}
		else
			delete item;
	}
}

int TRTYoloObjectDetector::detect(cv::Mat* input, int& current_id, bool is_draw, std::list<DetectionItem*>* detections)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();

	vector<Detection> objects;
	detector->preprocess(*input);
	detector->infer();
	detector->postprocess(objects);

	postprocess(&objects, current_id, is_draw, detector->get_model_height(), detector->get_model_width(), input);

	return objects.size() > 0;
}

int TRTYoloObjectDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	cv::Mat img;
	input->download(img);

	return detect(&img, current_id, is_draw);
}

int TRTYoloObjectDetector::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	return 0;
}

