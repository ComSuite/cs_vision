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

#include <string>
#include <map>
#include <variant>
#include <list>
#include <opencv2/core.hpp>
#include "JsonWrapper.h"
#include "types.h"
#include "fps_counter.h"
#include "http_server_settings.h"

namespace cs
{
	class dynamic_settings : protected cs::JsonWrapper
	{
	public:
		template<typename T>
		T get(const std::string& key, T default_value = T()) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<T>(it->second)) {
					return std::get<T>(it->second);
				}
			}

			return default_value;
		}

		double get_double(const std::string& key, double default_value = 0.0f) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<double>(it->second)) {
					return std::get<double>(it->second);
				}
			}

			return default_value;
		}

		float get_float(const std::string& key, float default_value = 0.0f) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<float>(it->second)) {
					return std::get<float>(it->second);
				}
			}

			return default_value;
		}

		int get_int(const std::string& key, int default_value = 0) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<int>(it->second)) {
					return std::get<int>(it->second);
				}
			}

			return default_value;
		}

		std::string get_string(const std::string& key, const std::string& default_value = "") const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<std::string>(it->second)) {
					return std::get<std::string>(it->second);
				}
			}

			return default_value;
		}

		bool get_bool(const std::string& key, bool default_value = false) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<bool>(it->second)) {
					return std::get<bool>(it->second);
				}
			}

			return default_value;
		}

		int parse(rapidjson::Value& root);
	private:
		std::map<std::string, std::variant<int, std::string, float, double, bool>> settings;
	};

	class detector_settings : protected cs::JsonWrapper
	{
	public:
		detector_settings() {};
		detector_settings(detector_settings& settings);
		virtual ~detector_settings() {};

		std::string name = "";
		int id = 0;
		int neural_network_id = 0;
		int kind = -1;

		int predecessor_id = -1; //id -1 - should use original image. another - list of results of executed predecessor
		int predecessor_class = -1;

		dynamic_settings additional;

		std::string input_tensor_name = "";
		std::string output_tensor_name = "";

		std::string model_path = "";
		std::string labels_path = "";
		std::string rules_path = "";

		int model_width = 0;
		int model_height = 0;
		int model_chnls = 0;

		bool is_use_gpu = false;
		bool is_send_results = false;

		bool is_draw_detections = false;
		int results_mapping_rule = 0;

		cv::Scalar color = cv::Scalar(255, 255, 255);

#ifdef __WITH_SCRIPT_LANG__
		std::string on_detect = "";
		bool execute_always = false;
		cs::SCRIPT_EXECUTE_MODE execute_mode = cs::SCRIPT_EXECUTE_MODE::SCRIPT_EXECUTE_MODE_NONE;
#endif

		int parse(rapidjson::Value& root);
	};

	class camera_settings : public cs::JsonWrapper
	{
	public:
		std::list<detector_settings*> detectors;

		std::string id = "";
		std::string name = "";

		dynamic_settings additional;

		bool is_display = false;
		std::variant<std::string, int> device = 0;
		bool is_use_gpu = false;
		bool is_flip = false;
		bool is_sort_results = false;
		double rotate_angle = 0;
		int is_convert_to_gray = 0;
		int frame_width = 0;
		int frame_height = 0;
		int resize_x = 0;
		int resize_y = 0;
		//bool is_show_mask = false;

		bool mqtt = true; // to do: should be changed to false
		std::string mqtt_client_name = "";
		std::string mqtt_broker_ip = "";
		int mqtt_broker_port = 0;
		std::string mqtt_detection_topic = "";
		bool mqtt_is_send_empty = false;

		int video_stream_port = 0;
		std::string	video_stream_channel = "";
		std::string video_stream_login = "";
		std::string video_stream_password = "";
		VIDEO_STREAM_MODE video_stream_mode = VIDEO_STREAM_MODE::VIDEO_STREAM_MODE_NONE;
		VIDEO_STREAM_ENGINE video_stream_engine = VIDEO_STREAM_ENGINE::VIDEO_STREAM_ENGINE_NONE;
		unsigned int video_stream_max_output_fps = 0;

		bool is_use_super_resolution = false;
		std::string super_resolution_name = "";
		std::string super_resolution_model_path = "";
		int super_resolution_factor = 1;

#ifdef __WITH_SCRIPT_LANG__
		std::string on_preprocess = "";
		std::string on_postprocess = "";
		bool execute_always = false;
		cs::SCRIPT_EXECUTE_MODE execute_mode = cs::SCRIPT_EXECUTE_MODE::SCRIPT_EXECUTE_MODE_NONE;
#endif

		INPUT_OUTPUT_DEVICE_KIND input_kind = INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_NONE;
		INPUT_OUTPUT_DEVICE_KIND output_kind = INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_NONE;

		cv::Scalar background_color = cv::Scalar(255, 255, 255);
		std::string aliases_path = "";

		BaseQueue<fps_counter_info>* http_server_queue = nullptr;

		int get_attempts_count() { return attempts_count; };
		int get_is_convert_to_gray() { return is_convert_to_gray; };
		bool get_is_flip() { return is_flip; };
		double get_rotate_angle() { return rotate_angle; };

		int parse(rapidjson::Value& root);
	private:
		int attempts_count = 1;
	};

	class device_settings : public cs::JsonWrapper
	{
	public:
		static const int config_version_high = 1;
		static const int config_version_low = 10;

		std::string id = "";
		std::string name = "";
		int device_kind = 0;
		std::list<camera_settings*> cameras;

		bool mqtt = true; // to do: should be changed to false
		std::string mqtt_client_name = "";
		std::string mqtt_broker_ip = "";
		int mqtt_broker_port = 0;
		std::string mqtt_command_topic = "";
		std::string mqtt_response_topic = "";
		std::string mqtt_settings_get_topic = "";
		std::string mqtt_error_topic = "";
		std::string mqtt_ping_topic = "";

		bool is_use_readonly_checker = false;
		std::string readonly_checker_dictionary = "";
		std::string secrets_dictionary = "";
		bool is_create_backup = true;

		http_server_settings* http_server = nullptr;
	protected:
		int parse(rapidjson::Document& root) override;
	};
}

