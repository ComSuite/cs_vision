#include "HaarCascadeClassifier.h"

using namespace std;
using namespace cv;
using namespace cv::cuda;
using namespace cs;

HaarCascadeClassifier::HaarCascadeClassifier()
{

}

HaarCascadeClassifier::~HaarCascadeClassifier()
{

}

int HaarCascadeClassifier::init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu)
{
	/*
	if (is_use_gpu) {
		cuda_detector = cv::cuda::CascadeClassifier::create(model_path);
		if (cuda_detector != nullptr) {
			if (!cuda_detector->empty()) {
				return 1;
			}
		}
	}
	else {
	*/
		cpu_detector = new cv::CascadeClassifier(model_path);
		if (cpu_detector != nullptr) {
			if (!cpu_detector->empty()) {
				return 1;
			}
		}
	//}

	return 0;
}

int HaarCascadeClassifier::init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu)
{
	init(model_path, label_path, rules_path, is_use_gpu);
}

void HaarCascadeClassifier::clear()
{

}

void HaarCascadeClassifier::postprocess(std::vector<cv::Rect>& features, int& current_id, bool is_draw, cv::Mat* image)
{
	for (auto f : features) {
		if (f.empty())
			continue;

		DetectionItem* item = new DetectionItem();
		if (item != nullptr) {
			item->color = color;

			DetectionItem* item = new DetectionItem();
			item->id = current_id;
			item->detector_id = id;
			current_id++;
			item->kind = ObjectDetectorKind::HAAR_CASCADE_CLASSIFIER;
			//item->label = trim(labels[ind]);
			item->class_id = 0;
			item->score = 1;
			item->box.x = f.x;
			item->box.y = f.y;
			item->box.width = f.width;
			item->box.height = f.height;
			item->neural_network_id = neural_network_id;

			last_detections.push_back(item);

			if (is_draw && image != nullptr) {

			}
		}
	}
}

int HaarCascadeClassifier::detect(cv::Mat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();

	if (cpu_detector != nullptr) {
		std::vector<cv::Rect> features;

		cpu_detector->detectMultiScale(*input, features);
		postprocess(features, current_id, is_draw, input);

		return 1;
	}

}

int HaarCascadeClassifier::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();
	cv::Mat in;
	input->download(in);
	return detect(&in, current_id, is_draw);
	
	if (cuda_detector != nullptr) {
		std::vector<cv::Rect> features;

		cuda_detector->detectMultiScale(*input, features);
		postprocess(features, current_id, is_draw, nullptr);

		return 1;
	}
}

int HaarCascadeClassifier::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	return 0;
}


