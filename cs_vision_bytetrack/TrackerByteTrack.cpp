#include "TrackerByteTrack.h"

using namespace cs;

int TrackerByteTrack::init(object_detector_environment& env)
{
	load_rules(env.rules_path.c_str());
	load_labels(env.label_path.c_str());

	tracker = new BYTETracker(env.fps, env.fps);
	if (tracker == nullptr) {
		std::cerr << "Failed to create BYTETracker instance." << std::endl;
		return 0;
	}

	return 1;
}

void TrackerByteTrack::clear()
{
	if (tracker != nullptr)
	{
		delete tracker;
		tracker = nullptr;
	}

	clear_last_detections();
}

int TrackerByteTrack::detect(cv::Mat* input, int& current_id, bool is_draw, std::list<DetectionItem*>* detections)
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

    std::vector<STrack> output_stracks = tracker->update(input_detections);

    for (auto i = 0; i < output_stracks.size(); i++)
    {
        DetectionItem* item = new DetectionItem();
        item->color = color;
		item->id = current_id;
		current_id++;

		item->kind = ObjectDetectorKind::OBJECT_DETECTOR_MOT_BYTETRACK;
		item->class_id = input_detections[i]->class_id;
		item->detector_id = id;

		std::string label;
		get_rule_label(item->class_id, label);
		item->priority = get_rule_priority(item->class_id);
		item->label = trim(label);
		item->score = output_stracks[i].score;
		item->box.y = output_stracks[i].tlwh[0];
		item->box.x = output_stracks[i].tlwh[1];
		item->box.width = output_stracks[i].tlwh[2];
		item->box.height = output_stracks[i].tlwh[3];

		item->neural_network_id = neural_network_id;
		last_detections.push_back(item);
    }


	return last_detections.size() > 0;
}

void TrackerByteTrack::parse(const std::string& payload, int& current_id)
{
}

