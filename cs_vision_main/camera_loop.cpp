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

#include "camera_loop.h"
#include <thread>
#include <iostream>
#include <fstream>
#ifdef __LINUX__
#include <signal.h>
#endif
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/mat.hpp>

#ifdef __HAS_CUDA__
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudawarping.hpp>
#endif

#include "ICamera.h"
#include "OpenCVCamera.h"
#include "OpenCVCamera_GPU.h"
#ifdef __WITH_AUDIO_PROCESSING__
//#include "PortAudioMicrophone.h"
#endif
#include "cv_utils.h"
#include "std_utils.h"
#include <atomic>
#include "IObjectDetector.h"
#include "MQTTClient.h"
#include "DetectorEnvironment.h"
#ifdef __WITH_VIDEO_STREAMER__
#include "HTTPVideoStreamer.h"
#include "SimpleGStreamerVideoStreamer.h"
#include "RTSPVideoStreamer.h"
#endif
#include "camera_loop_utils.h"
#ifdef __WITH_SCRIPT_LANG__
#include "CSScript.h"
#endif

using namespace std;
using namespace cv;
#ifdef __HAS_CUDA__
using namespace cv::cuda;
#endif
using namespace cs;

void cpu_preprocessing(Mat& frame, camera_settings* set, Mat& image, Size& border_dims);

void create_video_streamer(DetectorEnvironment* environment, camera_settings* set, ICamera* capture)
{
#ifdef __WITH_VIDEO_STREAMER__
	if (set->video_stream_mode != VIDEO_STREAM_MODE::VIDEO_STREAM_MODE_NONE) {
		switch (set->video_stream_engine) {
		case VIDEO_STREAM_ENGINE::VIDEO_STREAM_ENGINE_HTTP_MJPG:
			environment->video_streamer = new HTTPVideoStreamer();
			break;
		case VIDEO_STREAM_ENGINE::VIDEO_STREAM_ENGINE_SIMPLE_GSTREAMER:
			environment->video_streamer = new SimpleGStreamerVideoStreamer(); 
			break;
		case VIDEO_STREAM_ENGINE::VIDEO_STREAM_ENGINE_LIVE555:
			environment->video_streamer = new RTSPVideoStreamer();
			break;
		}

		if (environment->video_streamer != nullptr) {
			if (set->video_stream_login.length() > 0 && set->video_stream_password.length() > 0)
				environment->video_streamer->add_user_credentials(set->video_stream_login.c_str(), set->video_stream_password.c_str());
			try {
				environment->video_streamer->init(set->video_stream_port, set->video_stream_channel.c_str(), capture->get_width(), capture->get_height(), capture->get_fps());
			}
			catch (const std::runtime_error& e) {
				cout << "Error: " << e.what() << endl;
			}

			environment->video_streamer->open(set->video_stream_port);
			cout << "[Video Streamer] Publishing to port: " << set->video_stream_port << " Channel: " << set->video_stream_channel << " Mode: " << static_cast<int>(set->video_stream_mode) << endl;
			environment->video_stream_channel = set->video_stream_channel;
			environment->video_stream_mode = set->video_stream_mode;
		}
	}
#endif 
}

bool connect_to_mqtt_broker(camera_settings* settings, DetectorEnvironment* env)
{
	env->mqtt_client = new MQTTClient();
	if (env->mqtt_client == NULL) {
		return false;
	}

	if (!env->mqtt_client->connect(settings->mqtt_client_name.c_str(), settings->mqtt_broker_ip.c_str(), settings->mqtt_broker_port)) {
		delete env->mqtt_client;
		env->mqtt_client = NULL;
		return false;
	}

	env->mqtt_client->start_background_loop();

	return true;
}

bool init_detectors_environment(DetectorEnvironment* environment, camera_settings* set, ICamera* capture)
{
	if (!environment)
		return false;

	if (!set)
		return false;

	environment->is_sort_results = set->is_sort_results;
	environment->mqtt = set->mqtt;
	environment->mqtt_detection_topic = set->mqtt_detection_topic;
	environment->mqtt_is_send_empty = set->mqtt_is_send_empty;
	environment->background_color = set->background_color;
	if (::is_file_exists(set->aliases_path.c_str())) {
		environment->field_aliases = new aliases();
		if (environment->field_aliases != nullptr) {
			environment->field_aliases->set_id(environment->camera_id.c_str());
			environment->field_aliases->load(set->aliases_path.c_str());
		}
	}

	environment->http_server_queue = set->http_server_queue;

	create_video_streamer(environment, set, capture);
	connect_to_mqtt_broker(set, environment);

#ifdef __WITH_SCRIPT_LANG__
	environment->script = new cs::CSScript();
	if (environment->script != nullptr) {
		environment->script->init(environment->mqtt_client->get_handle());

		environment->on_postprocess = set->on_postprocess;
		environment->execute_always = set->execute_always;
		environment->execute_mode = set->execute_mode;
	}
#endif
	
	for (auto& detector : set->detectors) {
		IObjectDetector* _detector = create_detector(detector->kind);
		if (_detector != nullptr) {
			_detector->init(detector->model_path.c_str(), detector->labels_path.c_str(), detector->rules_path.c_str(), detector->input_tensor_name.c_str(), detector->output_tensor_name.c_str(), detector->is_use_gpu);
			_detector->name = detector->name;
			_detector->id = detector->id;
			_detector->neural_network_id = detector->neural_network_id;
			_detector->is_send_results = detector->is_send_results;
			_detector->is_use_gpu = detector->is_use_gpu;
			_detector->predecessor_id = detector->predecessor_id;
			_detector->predecessor_class = detector->predecessor_class;
			_detector->is_draw_detections = detector->is_draw_detections;
			_detector->results_mapping_rule = detector->results_mapping_rule;
			_detector->color = detector->color;
#ifdef __WITH_SCRIPT_LANG__
			_detector->on_detect = detector->on_detect;
			_detector->execute_always = detector->execute_always;
			_detector->execute_mode = detector->execute_mode;
#endif
			if (set->super_resolution_factor > 0 && set->is_use_super_resolution)
				_detector->scale_factor = set->super_resolution_factor;
			else
				_detector->scale_factor = 1;

			environment->detectors.push_back(_detector);
		}
	}

	environment->is_show_mask = set->is_show_mask;

	if (set->is_use_super_resolution) {
		environment->super_resolution = new cv::dnn_superres::DnnSuperResImpl();
		if (environment->super_resolution != nullptr) {
			environment->super_resolution->readModel(set->super_resolution_model_path);
			environment->super_resolution->setModel(set->super_resolution_name, set->super_resolution_factor);
		}

		cout << "Using super resolution DNN: " << set->super_resolution_model_path << " Name: " << set->super_resolution_name << " Scale factor: " << set->super_resolution_factor << endl;
	}
	else 
		cout << "Engine working without super resolution DNN" << endl;

	environment->image_writer = new SampleImageWriter("sr_", ".jpg");

	return 1; // environment->detectors.size();
}

void send_results_thread(DetectorEnvironment* env, list<DetectionItem*>& detections)
{
	if (env->mqtt_client == nullptr)
		return;

	env->mqtt_client->send_detection(env->camera_id.c_str(), env->mqtt_detection_topic.c_str(), detections, env->field_aliases);
}

void draw_detections(DetectorEnvironment* env, cv::Mat* detect_frame, list<DetectionItem*>& detections, bool is_show_mask)
{
	if (detect_frame == nullptr || env == nullptr)
		return;

	for (auto det : detections) {
		if (det->is_draw) {
			IObjectDetector* detector = env->get_detector(det->detector_id);
			if (detector != nullptr) {
				detector->draw_detection(detect_frame, det, env->background_color, is_show_mask);
			}
		}
	}
}

class detecting_image {
public:
	detecting_image(Mat* img, bool is_del_img, int x, int y, int id, int scale)
	{
		predecessor_id = id;
		image = img;
		is_delete_img = is_del_img;
		original_x = x;
		original_y = y;
		scale_factor = scale;
	}

	virtual ~detecting_image()
	{
		if (image != nullptr && is_delete_img) {
			delete image;
			image = nullptr;
		}
	}

	int predecessor_id = -1;
	Mat* image = nullptr;
	bool is_delete_img = false;

	int original_x = 0;
	int original_y = 0;
	int scale_factor = 1;
};

void stream_thread_func(DetectorEnvironment* env)
{
#ifdef __WITH_VIDEO_STREAMER__
	if (env == nullptr)
		return;

	if (env->video_stream_channel.length() == 0 || env->video_streamer == nullptr)
		return;

	while (true) {
		if (!env->is_can_show && !env->show_frame.empty()) {
			env->is_can_show = true;
			env->video_streamer->show_frame(env->show_frame, env->video_stream_channel.c_str());
		}
	}
#endif
}

void stream_frame_(Mat* frame, DetectorEnvironment* env)
{
	if (env->is_can_show) {
		frame->copyTo(env->show_frame);
		env->is_can_show = false;
	}
}

#ifdef __WITH_SCRIPT_LANG__
#ifdef __HAS_CUDA__
void execute_script(DetectorEnvironment* env, cv::cuda::GpuMat* detect_frame, SCRIPT_EXECUTE_MODE mode, bool is_always, const char* script_name, std::list<DetectionItem*>* detections)
#else
void execute_script(DetectorEnvironment* env, cv::Mat* detect_frame, SCRIPT_EXECUTE_MODE mode, bool is_always, const char* script_name, std::list<DetectionItem*>* detections)
#endif
{
	if (env == nullptr || script_name == nullptr || detections == nullptr || detect_frame == nullptr)
		return;

	if (env->script != nullptr) {
		if (mode != SCRIPT_EXECUTE_MODE::SCRIPT_EXECUTE_MODE_NONE && (detections->size() > 0 || is_always)) {
			if (!env->script->execute_script(script_name, detections, detect_frame)) {

			}
		}
	}
}
#endif

void detect_func(DetectorEnvironment* env)
{
	if (env == nullptr)
		return;

	std::list<DetectionItem*> detections;
	int id = 0;
	int scale_factor = 1;

	for (auto& detector : env->detectors) {
		list<detecting_image*> images;
		detecting_image* di = nullptr;
		scale_factor = 1;

		if (detector->predecessor_id < 0 || detector->predecessor_class < 0) {
			di = new detecting_image(env->detect_frame, false, 0, 0, -1, 1);
			images.push_back(di);
		}
		else {
			auto pred_detector = env->get_detector(detector->predecessor_id);
			if (pred_detector != nullptr) {
				for (auto& item : pred_detector->last_detections) {
					if (item->class_id == detector->predecessor_class) {
#ifdef __HAS_CUDA__
						Mat* img = nullptr;
						if (env->super_resolution != nullptr) {
							img = new Mat();
							Mat src, dst;
							(*env->detect_frame)(item->box).copyTo(src);
							env->super_resolution->upsample(src, dst);
							dst.copyTo(*img); //  ->upload(dst);
							scale_factor = detector->scale_factor;
						}
						else
							img = new Mat((*env->detect_frame)(item->box));
#else
						Mat* img = nullptr;
						if (env->super_resolution != nullptr) {
							img = new Mat();
							env->super_resolution->upsample((*(env->detect_frame))(item->box), *img);
							scale_factor = detector->scale_factor;
						}
						else
							img = new Mat((*(env->detect_frame))(item->box));
#endif
						di = new detecting_image(img, true, item->box.x, item->box.y, item->id, scale_factor); 

						images.push_back(di);
					}
				}
			}
		}

		for (auto& img : images) {
			if (img != nullptr && img->image != nullptr && !img->image->empty()) {
				if (detector->detect(img->image, id, false) == 1) { 
					for (auto& d : detector->last_detections) {
						DetectionItem* detection_item = new DetectionItem(d);

						detection_item->predecessor_detector_id = detector->predecessor_id;
						detection_item->predecessor_class = detector->predecessor_class;
						detection_item->predecessor_id = img->predecessor_id;

						detection_item->original_x = img->original_x;
						detection_item->original_y = img->original_y;

						detection_item->is_draw = detector->is_draw_detections;
						detection_item->frame_w = env->detect_frame->cols;
						detection_item->frame_h = env->detect_frame->rows;
						detection_item->mapping_rule = detector->results_mapping_rule;
						detection_item->scale_factor = img->scale_factor;
						detection_item->is_send_result = detector->is_send_results;

						detections.push_back(detection_item);
					}
				}
			}
		}

#ifdef __WITH_SCRIPT_LANG__
		//execute_script(env, &env->detect_frame, detector->execute_mode, detector->execute_always, detector->on_detect.c_str(), &detector->last_detections);
#endif

		if (detector->predecessor_id >= 0 && detector->predecessor_class >= 0)
			clear<detecting_image, std::list>(images);
	}

	for (auto& d : detections) {
		int scale_factor = 1;
		if (d->scale_factor > 1)
			scale_factor = d->scale_factor;

		d->box.x = d->box.x / scale_factor + d->original_x;
		d->box.y = d->box.y / scale_factor + d->original_y;
		d->box.width = d->box.width / scale_factor;
		d->box.height = d->box.height / scale_factor;
	}

#ifdef __WITH_SCRIPT_LANG__
	//execute_script(env, &env->detect_frame, env->execute_mode, env->execute_always, env->on_postprocess.c_str(), &detections);
#endif

#ifdef _DEBUG_
	if (detections.size() > 0) {
		//env->kpi.tick(detections.size());
	}
#endif

	if (env->is_sort_results) {
		detections.sort([](DetectionItem* a, DetectionItem* b) { return a->priority < b->priority; });
	}

	if (detections.size() > 0 || env->mqtt_is_send_empty) {
		send_results_thread(env, detections);
	}

#ifdef __WITH_VIDEO_STREAMER__
	if (env->video_stream_mode == VIDEO_STREAM_MODE::VIDEO_STREAM_MODE_DETECTOR && env->video_streamer != nullptr) {
		draw_detections(env, env->detect_frame, detections, env->is_show_mask);
		stream_frame_(env->detect_frame, env);
	}
#endif

	clear<DetectionItem, std::list>(detections);
}

void thread_func(DetectorEnvironment* env)
{
	while (true) {
		if (!env->detector_ready) {
			detect_func(env);
#ifdef _DEBUG_
			env->fps.tick("##########Output FPS: ", env->camera_id.c_str(), env->http_server_queue);
#endif
			env->detector_ready = true;
		}
	}
}

void process_frame(ICamera* capture, cs::camera_settings* set, DetectorEnvironment* environment, Mat& frame)
{
	if (environment->detector_ready) {
		environment->original_size = frame.size();

#ifdef __HAS_CUDA__
		//cv::cuda::GpuMat* image = new cv::cuda::GpuMat();

		//if (set->is_use_gpu)
		//	gpu_preprocessing(frame, set, environment->detect_frame, environment->border_dims);
		//else {
		//	Mat img;
		//	cpu_preprocessing(frame, set, img, environment->border_dims);
		//	environment->detect_frame.upload(img);
		//}
#else
		//cv::Mat* image = new cv::Mat();

		if (set->is_use_gpu) {
			gpu_preprocessing(frame, set, environment->detect_frame, environment->border_dims);
		}
		else {
			cpu_preprocessing(frame, set, environment->detect_frame, environment->border_dims);
		}
#endif
		environment->detect_frame = &frame;

#ifdef __WITH_VIDEO_STREAMER__
		if (set->video_stream_mode == VIDEO_STREAM_MODE::VIDEO_STREAM_MODE_SOURCE && environment->video_streamer != nullptr) {
			stream_frame_(environment->detect_frame, environment);
		}
#endif

		capture->set_ready(false);
		environment->detector_ready = false;
	}
}

ICamera* create_input_device(cs::camera_settings* set)
{
	if (set == nullptr)
		return nullptr;

	switch (set->input_kind) {
	case INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_CAMERA:
#ifdef __WITH_GPU_CAMERA__
#if defined(HAVE_OPENCV_CUDACODEC)
		return new OpenCVCamera_GPU();
#else
		return new OpenCVCamera();
#endif
#else
		return new OpenCVCamera();
#endif
		break;
//#ifdef __WITH_AUDIO_PROCESSING__
//	case INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_MICROPHONE:
//		return new PortAudioMicrophone();
//		break;
//#endif
	}

	return nullptr;
}

void* camera_loop(void* arg)
{
#ifdef __LINUX__
	signal(SIGPIPE, SIG_IGN);
	signal(0, SIG_IGN);
#endif
	cs::camera_settings* set = (cs::camera_settings*)arg;
	if (set == nullptr)
		return NULL;

	ICamera* capture = create_input_device(set);
	if (!capture) {
		return NULL;
	}
	capture->prepare();

	if (!capture->open(set->device, set->frame_width, set->frame_height) || capture->get_height() <= 0 || capture->get_width() <= 0) {
		cout << "Can not open capture: " << get<string>(set->device).c_str() << endl;
		delete capture;
		return NULL;
	}

	DetectorEnvironment environment;
	if (!init_detectors_environment(&environment, set, capture)) {
		delete capture;
		return NULL;
	}

	gpu_setup_device();

	for (auto detector : environment.detectors) {
		capture->set_detector_buffer(detector->width * detector->height);
	}

	Mat frame(capture->get_height(), capture->get_width(), CV_8UC3);
	Mat fake(capture->get_height(), capture->get_width(), CV_8UC3);

	environment.mqtt_client->loop();

#ifdef _DEBUG_
	fps_counter fps;
	if (set->input_kind == INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_CAMERA) {
		environment.fps.init();
		environment.kpi.init();

		fps.init();
	}
#endif
	environment.detector_ready = true;

	thread detect_tread(thread_func, &environment);
	detect_tread.detach();

	thread stream_tread(stream_thread_func, &environment); //show_frame, 
	stream_tread.detach();

	for (;;) {
		if (!capture->is_ready()) {
			cout << "Capture is not ready" << endl;
			continue;
		}

		int ret = 0;
		if (environment.detector_ready) {
			ret = capture->get_frame(frame, set->get_is_convert_to_gray());
		}
		else {
			capture->get_frame(fake, false);
		}

#ifdef _DEBUG_
		if (set->input_kind == INPUT_OUTPUT_DEVICE_KIND::INPUT_OUTPUT_DEVICE_KIND_CAMERA) {
			fps.tick("!!!!!!!!!Input FPS: ", "", nullptr);
		}
#endif

		if (ret != 0) {
			if (!frame.empty()) {
				process_frame(capture, set, &environment, frame);
			}

			if (capture->source_is_file && capture->is_end_of_file()) {
				capture->bring_to_start();
				cout << "Reopen capture" << endl;
			}

			if (capture->source_is_file)
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / capture->get_fps()));
		}
	}
	
	delete capture;
	//delete environment;

	return NULL;
}

void cpu_preprocessing(Mat& frame, camera_settings* set, Mat& image, Size& border_dims)
{
	border_dims.width = 0;
	border_dims.height = 0;

	if (set->is_flip) {
		cv::flip(frame, frame, 1);
	}

	if (set->get_rotate_angle() != 0) {
		Mat rotate;
		cpu_rotate(frame, rotate, set->get_rotate_angle());
		rotate.copyTo(frame);
	}

	/*
	if (set->resize_x != 0 && set->resize_y != 0) {
		if (set->resize_x == set->resize_y && frame.cols != frame.rows) 
			cpu_create_square_letterbox(frame, image, set->resize_x, border_dims);
		else
			cv::resize(frame, image, Size(set->resize_x, set->resize_y), 0, 0, INTER_AREA);
	}
	else
	*/
	frame.copyTo(image);
}

