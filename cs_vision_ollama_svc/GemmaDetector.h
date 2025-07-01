#pragma once
#include "OllamaDetector.h"
#include <opencv2/imgproc.hpp>

namespace cs
{
	class GemmaDetector : public OllamaDetector
	{
	public:
		virtual void parse(const std::string& response, int& current_id) override
		{
			if (response.find("Yes") != std::string::npos)
			{
				DetectionItem* item = new DetectionItem();
				item->id = current_id;
				current_id++;

				item->kind = ObjectDetectorKind::OBJECT_DETECTOR_SVC_GEMMA3;
				item->detector_id = id;

				item->label = "Weapon!!!";
				item->neural_network_id = neural_network_id;

				last_detections.push_back(item);
			}
		}

		virtual void draw_detection(cv::Mat* detect_frame, DetectionItem* detection) override
		{
			if (detect_frame == nullptr || detection == nullptr)
				return;

			cv::putText(*detect_frame, detection->label, cv::Point(10, 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, detection->color, 2);
		}
	};
}


