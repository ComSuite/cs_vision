#include "TRTRetinaNetObjectDetector.h"
#include <vector>

using namespace cs;

class TRTRetinaNetLogger : public nvinfer1::ILogger {
	void log(Severity severity, const char* msg) noexcept override {
		if (severity <= Severity::kWARNING)
			std::cout << msg << std::endl;
	}
} trt_retina_logger;

TRTRetinaNetObjectDetector::TRTRetinaNetObjectDetector()
{
}

TRTRetinaNetObjectDetector::~TRTRetinaNetObjectDetector()
{
	clear();
}

int TRTRetinaNetObjectDetector::init(object_detector_environment& env)
{
	load_rules(env.rules_path.c_str());
	load_labels(env.label_path.c_str());

	detector = make_unique<TRTRetinaNet>(env.model_path, trt_retina_logger);

	return 1;
}

void TRTRetinaNetObjectDetector::clear()
{
	// Implement any necessary cleanup logic here
}

int TRTRetinaNetObjectDetector::detect(cv::Mat* input, int& current_id, bool is_draw, std::list<DetectionItem*>* detections)
{
	if (input == nullptr)
		return 0;

	clear_last_detections();

	std::vector<Detection> objects;
	detector->preprocess(*input, nullptr);
	detector->infer();
	//detector->postprocess(objects);

	//postprocess(&objects, current_id, is_draw, detector->get_model_height(), detector->get_model_width(), input);

	return objects.size() > 0;
}

int TRTRetinaNetObjectDetector::detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw)
{
	// Implement batch detection logic here
	return 0; // Return the number of detections or an appropriate status code
}

int TRTRetinaNetObjectDetector::detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw)
{
	if (input == nullptr)
		return 0;

	cv::Mat img;
	input->download(img);

	return detect(&img, current_id, is_draw);
}

void TRTRetinaNetObjectDetector::parse(const std::string & payload, int& current_id)
{
	// Implement parsing logic here if needed
}


