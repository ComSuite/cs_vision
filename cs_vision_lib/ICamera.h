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

#include <variant>
#include <atomic>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include "settings.h"

namespace cs
{
	class ICamera
	{
	public:
		ICamera() { video_writer = NULL; };
		virtual ~ICamera() { close(); };

		virtual int info() = 0;

		virtual int open(std::variant<std::string, int> device, const int frame_width = 0, const int frame_height = 0) = 0;
		virtual int open(const int id, const int frame_width = 0, const int frame_height = 0) = 0;
		virtual int open(const char* name) = 0;
		virtual int open(const int id, int attempts_count);
		virtual int open(const char* name, int attempts_count);
		virtual int open(camera_settings* settings)
		{
			if (settings == nullptr)
				return -1;
			auto device = settings->device;
			//source_is_file = settings->source_is_file;
			return open(device, settings->frame_width, settings->frame_height);
		}

		int start_save_video(const int id, const char* output_movie_name);
		virtual int close();
		virtual int prepare() = 0;
		//virtual int grab() = 0;
		//virtual int grab(const char* name) = 0;
		virtual int save_to_file() = 0;

#ifdef  __HAS_CUDA__
		int save_frame(cv::cuda::GpuMat& frame);
		virtual int get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray) = 0;
		virtual int get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray) = 0;
#endif 

		int save_frame(cv::Mat& frame);
		virtual int get_frame(const char* name, cv::Mat& frame, bool convert_to_gray) = 0;
		virtual int get_frame(cv::Mat& frame, bool convert_to_gray) = 0;

		virtual int get_width() = 0;
		virtual int get_height() = 0;
		virtual int get_fps() = 0;

		virtual bool is_end_of_file() = 0;

		virtual bool is_opened() = 0;
		virtual void bring_to_start() = 0;

		virtual bool is_ready() { return ready_flag; }
		virtual void set_ready(bool val) { ready_flag = val; }
		virtual void set_detector_buffer(size_t length) {}

		bool source_is_file = false;
	private:
		cv::VideoWriter* video_writer = NULL;
	protected:
		std::variant<std::string, int> device;
		std::atomic_bool ready_flag = true;
	};
}

