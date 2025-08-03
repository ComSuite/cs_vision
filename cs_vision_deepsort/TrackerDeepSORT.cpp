#include "TrackerDeepSORT.h"
#include "FeatureTensor.h"

using namespace cs;	

int TrackerDeepSORT::init(object_detector_environment& env)
{
	load_rules(env.rules_path.c_str());
	load_labels(env.label_path.c_str());

	tracker = new ::tracker(0.5, 30, 0.7, 30, 3);
	if (tracker == nullptr) {
		std::cerr << "Failed to create tracker instance." << std::endl;
		return 0;
	}


	return 1;
}

void TrackerDeepSORT::clear()
{

}

int TrackerDeepSORT::detect(cv::Mat* input, int& current_id, bool is_draw, std::list<DetectionItem*>* detections)
{
	if (input == nullptr || input->empty())
	{
		std::cerr << "Input frame is empty or null." << std::endl;
		return 0;
	}

	if (detections == nullptr || detections->empty())
	{
		std::cerr << "No detections provided." << std::endl;
		return 0;
	}

	clear_last_detections();
	std::vector<DetectionItem*> input_detections;
	for (auto& detection : *detections) {
		if (detection == nullptr)
			continue;

		if (detection->class_id == this->predecessor_class || this->predecessor_class < 0) {
			input_detections.push_back(detection);
		}
	}

	std::cout << "begin track" << std::endl;
	if (FeatureTensor::getInstance()->getRectsFeature(*input, input_detections))
	{
		std::cout << "get feature succeed!" << std::endl;
		tracker->predict();
		tracker->update(input_detections);

		std::vector<RESULT_DATA> result;
		for (Track& track : tracker->tracks) {
			if (!track.is_confirmed() || track.time_since_update > 1) continue;
			result.push_back(std::make_pair(track.track_id, track.to_tlwh()));
		}

		for (unsigned int k = 0; k < input_detections.size(); k++)
		{
			//DETECTBOX tmpbox = detections[k].tlwh;
			//cv::Rect rect(tmpbox(0), tmpbox(1), tmpbox(2), tmpbox(3));
			//cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 4);

			//for (unsigned int k = 0; k < result.size(); k++)
			//{
			//	DETECTBOX tmp = result[k].second;
			//	cv::Rect rect = cv::Rect(tmp(0), tmp(1), tmp(2), tmp(3));
			//	rectangle(frame, rect, cv::Scalar(255, 255, 0), 2);

			//	std::string label = cv::format("%d", result[k].first);
			//	cv::putText(frame, label, cv::Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
			//}
		}
	}


	return 1;
}

void TrackerDeepSORT::parse(const std::string& payload, int& current_id)
{
}

