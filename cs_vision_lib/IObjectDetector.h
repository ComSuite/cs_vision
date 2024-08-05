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

#pragma once

#include <list>
#include <map>
#include <vector>
#include <variant>
#include <string>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include "std_utils.h"
#include "types.h"
#include "rapidjson.h"
#include "JsonWrapper.h"

void default_error_reporter(void* user_data, const char* format, va_list args);

namespace cs
{

	class DetectionRule
	{
	public:
		std::variant<int, std::string> object;
		float score = 0;
		std::string description = "";
		std::string on_detect = "";
		bool execute_always = false;
		cv::Scalar color = cv::Scalar(255, 255, 255);

		bool check(std::variant<int, std::string> item, float sc) { return item == object && sc >= score; };
	};

	class DetectionItem
	{
	public:
		static const int RESULTS_MAPPING_RULE_NONE = 0;
		static const int RESULTS_MAPPING_RULE_ASIS = 1;
		static const int RESULTS_MAPPING_RULE_NORM = 2;

		DetectionItem() {};
		DetectionItem(DetectionItem* item)
		{
			if (item == nullptr)
				return;

			this->id = item->id;
			this->detector_id = item->detector_id;
			this->kind = item->kind;
			this->event = item->event;
			this->id = item->id;
			this->neural_network_id = item->neural_network_id;
			this->class_id = item->class_id;
			this->label = item->label;
			this->score = item->score;
			this->box = item->box;
			this->predecessor_id = item->predecessor_id;
			this->predecessor_detector_id = item->predecessor_detector_id;
			this->predecessor_class = item->predecessor_class;
			this->color = item->color;

			this->original_x = item->original_x;
			this->original_y = item->original_y;
			this->frame_w = item->frame_w;
			this->frame_h = item->frame_h;
			this->mapping_rule = item->mapping_rule;
			this->is_draw = item->is_draw;
			this->scale_factor = item->scale_factor;
			this->is_send_result = item->is_send_result;
		}

		ObjectDetectorKind kind = ObjectDetectorKind::OBJECT_DETECTOR_NONE;
		ObjectDetectorEvent event = ObjectDetectorEvent::OBJECT_DETECTOR_EVENT_NONE;
		int id = -1;
		int neural_network_id = -1;
		int detector_id = -1;
		int class_id = -1;
		std::string label = "";
		float score = 0;
		cv::Rect2d box;
		bool is_draw = false;
		int predecessor_id = -1;
		int predecessor_detector_id = -1;
		int predecessor_class = -1;
		cv::Scalar color = cv::Scalar(255, 255, 255);

		int original_x = 0;
		int original_y = 0;
		int frame_w = 0;
		int frame_h = 0;
		int mapping_rule = RESULTS_MAPPING_RULE_NONE;
		int scale_factor = 1;
		bool is_send_result = false;

		int get_id() { return id; }
		int get_neural_network_id() { return neural_network_id; }
		int get_class_id() { return class_id; }
		float get_score() { return score; }

		void set_color(int r, int g, int b) { this->color = cv::Scalar(r, g, b); }
		void set_is_send_result(bool send) { is_send_result = send; }
	};

	class IObjectDetector : public JsonWrapper
	{
	public:
		IObjectDetector() {};
		virtual ~IObjectDetector() {};

		virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false) = 0;
		virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false) = 0;
		virtual int init(void* param, bool is_use_gpu = false) = 0;
		virtual void clear() = 0;
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) = 0;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) = 0;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) = 0;

		int infer(cv::Mat* input, int& current_id, bool show_mean, bool is_draw = true);
		float get_mean_detect_duration();

#ifdef __HAS_CUDA__
		virtual void draw_detection(cv::cuda::GpuMat* detect_frame, DetectionItem* detection, bool is_show_mask);
		void draw_mask(DetectionItem* det, cv::cuda::GpuMat* frame, const cv::Scalar& color = cv::Scalar(255, 255, 255));
#else
		virtual void draw_detection(cv::Mat* detect_frame, DetectionItem* detection);
		void draw_mask(DetectionItem* det, cv::Mat* frame, const cv::Scalar color = cv::Scalar(255, 255, 255));
#endif

		void draw_label(cv::Mat& input_image, std::string& label, int left, int top);

		std::string name = "";
		int id = 0;
		int neural_network_id = -1;

		int predecessor_id = -1; //id -1 - should use original image. another - list of results of executed predecessor
		int predecessor_class = -1;

		int width = 0;
		int height = 0;
		int channels = 0;

		bool is_send_results = false;
		bool is_use_gpu = false;
		bool is_draw_detections = false;
		int scale_factor = 1;

		cv::Scalar color = cv::Scalar(255, 255, 255);

		std::string on_detect = "";
		bool execute_always = false;
#ifdef __WITH_SCRIPT_LANG__
		cs::SCRIPT_EXECUTE_MODE execute_mode = cs::SCRIPT_EXECUTE_MODE::SCRIPT_EXECUTE_MODE_NONE;
#endif

		int results_mapping_rule = DetectionItem::RESULTS_MAPPING_RULE_ASIS;

		std::list<DetectionItem*> last_detections;
		void clear_last_detections()
		{
			::clear<DetectionItem, std::list>(last_detections);
		}

		virtual bool get_is_check_proportions() { return true; };
	protected:
		std::vector<std::string> labels;
		std::map<int, DetectionRule*> rules;

		void ProcessInputWithFloatModel(uint8_t* input, float* buffer, const int width, const int height, const int channels);
		cv::Mat ProcessOutputWithFloatModel(float* input, const int width, const int height, const int channels);
		void load_labels_txt(const char* label_path);
		void load_labels_coco_json(const char* label_path);
		void load_labels(const char* label_path);
		void load_rules(const char* rules_path);

		DetectionRule* get_rule(int ind);
		bool get_rule_label(int ind, std::string& label);
		float get_rule_score(int ind);
		bool check_rule(int ind, float score, cv::Scalar& color);

		float detect_time = 0;
		int64_t detections_count = 0;
	private:
		bool if_json(const char* label_path);
		int parse(rapidjson::Document& root) override;
	};
}
