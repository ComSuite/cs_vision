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
#include <atomic>
#include <map>
#include <opencv2/dnn_superres.hpp>
#include "IObjectDetector.h"
#include "std_utils.h"
#include "MQTTClient.h"
#ifdef __WITH_VIDEO_STREAMER__
#include "IVideoStreamer.h"
#endif
#ifdef _DEBUG_
#include "fps_counter.h"
#include "kpi_counter.h"
#endif
#include "SampleImageWriter.h"
#include "BaseQueue.h"
#ifdef __WITH_SCRIPT_LANG__
#include "CSScript.h"
#endif
#include "aliases.h"
#include "BaseQueue.h"
#include "fps_counter.h"

namespace cs
{
	class DetectorEnvironment
	{
	public:
		virtual ~DetectorEnvironment()
		{
			clear<IObjectDetector, std::list>(detectors);

			//if (show_frame != nullptr)
			//	delete show_frame;
#ifdef __WITH_VIDEO_STREAMER__
			if (video_streamer != nullptr)
				delete video_streamer;
#endif
			if (mqtt_client != nullptr)
				delete mqtt_client;

			if (super_resolution != nullptr)
				delete super_resolution;

			if (image_writer != nullptr)
				delete image_writer;
		}

		std::atomic_bool detector_ready = true;
		std::mutex detector_mutex;

		std::list<IObjectDetector*> detectors; //to do: shold be changed to map<int, IObjectDetector*>?

		std::atomic_bool is_can_show = true;
/*
#ifdef __HAS_CUDA__
		//BaseQueue<cv::cuda::GpuMat> queue;
		cv::cuda::GpuMat detect_frame;
		cv::cuda::GpuMat show_frame;
#else
		//BaseQueue<cvMat> queue;
		cv::Mat detect_frame;
		cv::Mat show_frame;
#endif
*/
		cv::Mat* detect_frame;
		cv::Mat show_frame;

		std::string topic = "";
		std::string camera_id = "";
		bool is_sort_results = false;
		int resize_x = 0;
		int resize_y = 0;
		bool is_show_mask = false;
		bool is_draw_detections = false;
		cv::Size original_size;
		cv::Size border_dims;
		cv::Scalar background_color = cv::Scalar(255, 255, 255);
		aliases* field_aliases = nullptr;

		IObjectDetector* get_detector(int detector_id)
		{
			for (auto& det : detectors) {
				if (det->id == detector_id)
					return det;
			}

			return nullptr;
		}

		cv::dnn_superres::DnnSuperResImpl* super_resolution = nullptr;

		bool mqtt = false;
		MQTTClient* mqtt_client = nullptr;
		std::string mqtt_detection_topic = "";
		bool mqtt_is_send_empty = false;
#ifdef __WITH_VIDEO_STREAMER__
		cs::IVideoStreamer* video_streamer = nullptr;
#endif
		std::string video_stream_channel = "";
		VIDEO_STREAM_MODE video_stream_mode = VIDEO_STREAM_MODE::VIDEO_STREAM_MODE_NONE;

		cs::SampleImageWriter* image_writer = nullptr;

		BaseQueue<fps_counter_info>* http_server_queue = nullptr;

#ifdef _DEBUG_
		fps_counter fps;
		kpi_counter kpi;
#endif

#ifdef __WITH_SCRIPT_LANG__
		cs::CSScript* script = nullptr;

		std::string on_postprocess = "";
		bool execute_always = false;
		cs::SCRIPT_EXECUTE_MODE execute_mode = cs::SCRIPT_EXECUTE_MODE::SCRIPT_EXECUTE_MODE_NONE;
#endif
	};
}
