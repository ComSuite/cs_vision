/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2021 - 2025, Alexander Epstine (a@epstine.com)
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

#include "TrackerDeepSORT.h"
#include "FeatureTensor.h"

using namespace cs;	

int TrackerDeepSORT::init(object_detector_environment& env)
{
	load_rules(env.rules_path.c_str());
	load_labels(env.label_path.c_str());

	model_path = env.model_path;
	input_tensor_name = env.input_tensor_name;
	output_tensor_name = env.output_tensor_name;

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
	if (input == nullptr || input->empty()) {
		return 0;
	}

	if (detections == nullptr || detections->empty()) {
		return 0;
	}

	clear_last_detections();

	std::vector<DETECTION_ROW> input_detections;
	cv::Mat frame = *input;

	for (auto& detection : *detections) {
		if (detection == nullptr)
			continue;

		if (detection->class_id == this->predecessor_class || this->predecessor_class < 0) {
			cv::rectangle(frame, detection->box, cv::Scalar(255, 0, 0), 2);

			DETECTION_ROW detect;
			detect.tlwh = DETECTBOX(detection->box.x, detection->box.y, detection->box.width, detection->box.height);
			detect.confidence = detection->score;
			detect.class_id = detection->class_id;

			input_detections.push_back(detect);
		}
	}

	wchar_t* model_path_w = const_cast<wchar_t*>(ascii_to_wchar(model_path.c_str()));
	if (FeatureTensor::getInstance(model_path_w)->getRectsFeature(frame, input_detections, this->input_tensor_name.c_str(), this->output_tensor_name.c_str())) {
		tracker->predict();
		tracker->update(input_detections);

		std::vector<RESULT_DATA> result;
		for (int i = 0; i < tracker->tracks.size(); i++) { //Track& track : tracker->tracks
			if (!tracker->tracks[i].is_confirmed() || tracker->tracks[i].time_since_update > 1)
				continue;

			DetectionItem* item = new DetectionItem();
			item->color = color;
			item->id = current_id;
			current_id++;

			item->kind = ObjectDetectorKind::OBJECT_DETECTOR_MOT_BYTETRACK;
			item->class_id = input_detections[i].class_id;
			item->detector_id = id;

			std::string label;
			get_rule_label(item->class_id, label);
			item->priority = get_rule_priority(item->class_id);
			item->label = trim(label);
			item->score = input_detections[i].confidence;
			item->box.y = tracker->tracks[i].to_tlwh()[1];
			item->box.x = tracker->tracks[i].to_tlwh()[0];
			item->box.width = tracker->tracks[i].to_tlwh()[2];
			item->box.height = tracker->tracks[i].to_tlwh()[3];

			item->neural_network_id = neural_network_id;
			last_detections.push_back(item);
		}
	}

	return 1;
}

void TrackerDeepSORT::parse(const std::string& payload, int& current_id)
{
}

