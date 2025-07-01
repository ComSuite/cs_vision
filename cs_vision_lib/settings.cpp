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

#include "settings.h"
#include "cv_utils.h"
#include <iostream>

using namespace cs;
using namespace rapidjson;

//******************************************************************************************
// dynamic_settings
int dynamic_settings::parse(rapidjson::Value& root)
{
	if (root.HasMember("additional")) {
		auto additional = root["additional"].GetArray();
		std::variant<int, std::string, float, double, bool> def;
		for (auto& tr : additional) {
			if (!tr.IsObject())
				continue;

			if (!tr.HasMember("name") || !tr.HasMember("val"))
				continue;

			std::string name = json_get_string(tr, "name", "");
			auto val = json_get_variant(tr, "val", def);
			settings[name] = val;
		}
	}

	return settings.size() > 0;
}

//******************************************************************************************
detector_settings::detector_settings(detector_settings& settings)
{
	model_path = settings.model_path;
	labels_path = settings.labels_path;
	rules_path = settings.rules_path;
	model_width = settings.model_width;
	model_height = settings.model_height;
	model_chnls = settings.model_chnls;
}

int detector_settings::parse(rapidjson::Value& root)
{
	additional.parse(root);

	name = json_get_string(root, "name", name.c_str());
	id = json_get_int(root, "id", id);
	neural_network_id = json_get_int(root, "neural_network_id", neural_network_id);
	kind = json_get_int(root, "kind", id);

	input_tensor_name = json_get_string(root, "input_tensor_name", input_tensor_name.c_str());
	output_tensor_name = json_get_string(root, "output_tensor_name", output_tensor_name.c_str());

	is_send_results = json_get_bool(root, "is_send_results", is_send_results);
	is_use_gpu = json_get_bool(root, "is_use_gpu", is_use_gpu);
	is_draw_detections = json_get_bool(root, "is_draw_detections", is_draw_detections);
	results_mapping_rule = json_get_int(root, "results_mapping_rule", results_mapping_rule);

	if (root.HasMember("predecessor")) {
		if (root["predecessor"].IsObject()) {
			auto pred = root["predecessor"].GetObject();
			predecessor_id = json_get_int(pred, "id", -1);
			predecessor_class = json_get_int(pred, "class", -1);
		}
	}

	labels_path = json_get_string(root, "labels_path", labels_path.c_str());
	model_path = json_get_string(root, "model_path", model_path.c_str());
	rules_path = json_get_string(root, "rules_path", rules_path.c_str());

	model_width = json_get_int(root, "width", model_width);
	model_height = json_get_int(root, "height", model_height);
	model_chnls = json_get_int(root, "chnls", model_chnls);

	color = cv_string_to_color(json_get_string(root, "color", "0x00FFFFFF"));
	//unsigned int x = std::stoul(str_color, nullptr, 16);
	//color = cv::Scalar(x & 0x00FF0000, x & 0x0000FF00 >> 2, x & 0x000000FF);

#ifdef __WITH_SCRIPT_LANG__
	on_detect = json_get_string(root, "on_detect", on_detect.c_str());
	execute_always = json_get_bool(root, "execute_always", execute_always);
	execute_mode = static_cast<SCRIPT_EXECUTE_MODE>(json_get_int(root, "execute_mode", static_cast<int>(execute_mode)));
#endif

	return 1;
}

//******************************************************************************************
int camera_settings::parse(rapidjson::Value& root)
{
	additional.parse(root);

	int dev = json_get_int(root, "device", -1);
	if (dev >= 0)
		device = dev;
	else
		device = json_get_string(root, "device", "");

	id = json_get_string(root, "id", id.c_str());
	name = json_get_string(root, "name", name.c_str());
	attempts_count = json_get_int(root, "connection_attempts_count", attempts_count);
	rotate_angle = json_get_double(root, "rotate_angle", rotate_angle);
	is_sort_results = json_get_bool(root, "is_sort_results", is_sort_results);
	is_flip = json_get_bool(root, "is_flip", is_flip);
	is_convert_to_gray = json_get_int(root, "is_convert_to_gray", is_convert_to_gray);
	is_use_gpu = json_get_bool(root, "is_use_gpu", is_use_gpu);
	frame_width = json_get_int(root, "frame_width", frame_width);
	frame_height = json_get_int(root, "frame_height", frame_height);
	resize_x = json_get_int(root, "resize_x", resize_x);
	resize_y = json_get_int(root, "resize_y", resize_y);
	//is_show_mask = json_get_bool(root, "is_show_mask", is_show_mask);

	mqtt = json_get_bool(root, "mqtt", mqtt);
	mqtt_client_name = json_get_string(root, "mqtt_client_name", mqtt_client_name.c_str());
	mqtt_broker_ip = json_get_string(root, "mqtt_broker_ip", mqtt_broker_ip.c_str());
	mqtt_broker_port = json_get_int(root, "mqtt_broker_port", mqtt_broker_port);
	mqtt_detection_topic = json_get_string(root, "mqtt_detection_topic", mqtt_detection_topic.c_str());
	mqtt_is_send_empty = json_get_bool(root, "mqtt_is_send_empty", mqtt_is_send_empty);

	video_stream_port = json_get_int(root, "video_stream_port", video_stream_port);
	video_stream_channel = json_get_string(root, "video_stream_channel", video_stream_channel.c_str());
	video_stream_login = json_get_string(root, "video_stream_login", video_stream_login.c_str());
	video_stream_password = json_get_string(root, "video_stream_password", video_stream_password.c_str());

	video_stream_mode = static_cast<VIDEO_STREAM_MODE>(json_get_int(root, "video_stream_mode", static_cast<int>(video_stream_mode)));
	video_stream_engine = static_cast<VIDEO_STREAM_ENGINE>(json_get_int(root, "video_stream_engine", static_cast<int>(video_stream_engine)));
	video_stream_max_output_fps = json_get_int(root, "video_stream_max_output_fps", video_stream_max_output_fps);

	is_use_super_resolution = json_get_bool(root, "is_use_super_resolution", is_use_super_resolution);
	super_resolution_name = json_get_string(root, "super_resolution_name", super_resolution_name.c_str());
	super_resolution_model_path = json_get_string(root, "super_resolution_model_path", super_resolution_model_path.c_str());
	super_resolution_factor = json_get_int(root, "super_resolution_factor", super_resolution_factor);

	input_kind = static_cast<INPUT_OUTPUT_DEVICE_KIND>(json_get_int(root, "input_kind", static_cast<int>(input_kind)));
	output_kind = static_cast<INPUT_OUTPUT_DEVICE_KIND>(json_get_int(root, "output_kind", static_cast<int>(output_kind)));

	background_color = cv_string_to_color(json_get_string(root, "background_color", "0x00FFFFFF"));
	aliases_path = json_get_string(root, "aliases_path", aliases_path.c_str());

#ifdef __WITH_SCRIPT_LANG__
	on_postprocess = json_get_string(root, "on_postprocess", on_postprocess.c_str());
	execute_always = json_get_bool(root, "execute_always", execute_always);
	execute_mode = static_cast<SCRIPT_EXECUTE_MODE>(json_get_int(root, "execute_mode", static_cast<int>(execute_mode)));
#endif

	if (root.HasMember("detectors")) {
		auto _detectors = root["detectors"].GetArray();
		for (auto& tr : _detectors) {
			detector_settings* detector = new detector_settings();
			if (detector != NULL) {
				if (detector->parse(tr))
					detectors.push_back(detector);
				else
					delete detector;
			}
		}
	}

	return 1;
}

int http_server_settings::parse(rapidjson::Value& root)
{
	device_name = json_get_string(root, "device_name", device_name.c_str());
	root_dir = json_get_string(root, "root_dir", root_dir.c_str());
	http_port = json_get_int(root, "http_port", http_port);
	https_port = json_get_int(root, "https_port", https_port);
	cert_dir = json_get_string(root, "cert_dir", cert_dir.c_str());
	home_page = json_get_string(root, "home_page", home_page.c_str());

	return 1;
}

//******************************************************************************************
int device_settings::parse(Document& root)
{
	int ret = 1;

	try {
		if (root.IsNull())
			return 0;

		if (json_get_int(root, "config_version_high", 0) != config_version_high || json_get_int(root, "config_version_low", 0) != config_version_low) {
			std::cout << "Invalid configurationn file version. Current is: " << config_version_high << "." << config_version_low << std::endl;
			return 0;
		}

		device_kind = json_get_int(root, "device_kind", device_kind);

		if (!root.HasMember("settings"))
			return 0;

		if (!root["settings"].IsObject())
			return 0;

		auto settings = root["settings"].GetObject();

		id = json_get_string(settings, "id", id.c_str());
		name = json_get_string(settings, "name", name.c_str());

		mqtt = json_get_bool(settings, "mqtt", mqtt);
		mqtt_client_name = json_get_string(settings, "mqtt_client_name", mqtt_client_name.c_str());
		mqtt_broker_ip = json_get_string(settings, "mqtt_broker_ip", mqtt_broker_ip.c_str());
		mqtt_broker_port = json_get_int(settings, "mqtt_broker_port", mqtt_broker_port);
		mqtt_command_topic = json_get_string(settings, "mqtt_command_topic", mqtt_command_topic.c_str());
		mqtt_response_topic = json_get_string(settings, "mqtt_response_topic", mqtt_response_topic.c_str());
		mqtt_settings_get_topic = json_get_string(settings, "mqtt_settings_get_topic", mqtt_settings_get_topic.c_str());
		mqtt_error_topic = json_get_string(settings, "mqtt_error_topic", mqtt_error_topic.c_str());
		mqtt_ping_topic = json_get_string(settings, "mqtt_ping_topic", mqtt_ping_topic.c_str());

		is_use_readonly_checker = json_get_bool(root, "is_use_readonly_checker", is_use_readonly_checker);
		readonly_checker_dictionary = json_get_string(settings, "readonly_checker_dictionary", readonly_checker_dictionary.c_str());
		secrets_dictionary = json_get_string(settings, "secrets_dictionary", secrets_dictionary.c_str());
		is_create_backup = json_get_bool(root, "is_create_backup", is_create_backup);

		if (settings.HasMember("cameras")) {
			if (settings["cameras"].IsArray()) {
				auto cams = settings["cameras"].GetArray();
				for (auto& cam : cams) {
					camera_settings* cam_set = new camera_settings();
					if (cam_set != NULL) {
						if (cam_set->parse(cam))
							cameras.push_back(cam_set);
						else
							delete cam_set;
					}
				}
			}
		}

		if (settings.HasMember("http_server")) {
			if (settings["http_server"].IsObject()) {
				http_server = new http_server_settings();
				if (http_server != nullptr) {
					http_server->parse(settings["http_server"]);
				}
			}
		}
	}
	catch (...) {
		ret = 0;
	}

	return ret;
}

