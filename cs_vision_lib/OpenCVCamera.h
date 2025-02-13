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
#include <variant>
#include "ICamera.h"
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/opengl.hpp>
#include <opencv2/cudacodec.hpp>

namespace cs
{
	class OpenCVCamera : public ICamera
	{
	public:
		OpenCVCamera();
		virtual ~OpenCVCamera();

		int info();
		int open(std::variant<std::string, int> device, const int frame_width = 0, const int frame_height = 0) override;
		int open(const int id, const int frame_width = 0, const int frame_height = 0);
		int open(const char* name);
		virtual int close();
		virtual int prepare();
		//int grab();
		//int grab(const char* name);
		int save_to_file();
		//char* get_frame();
		int get_frame(const char* name, cv::Mat& frame, bool convert_to_gray);
		int get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray);
		int get_frame(cv::Mat& frame, bool convert_to_gray);
		int get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray);

		int get_width();
		int get_height();
		int get_fps() { return fps; };

		virtual bool is_end_of_file() override;
		virtual void bring_to_start() override;
		virtual bool is_opened() override;

		virtual void set_ready(bool val) override { ready_flag = true; }
	private:
		int fps = 0;
		int width = 0;
		int height = 0;
		cv::Mat frame;
		cv::VideoCapture* capture = NULL;
	};
}
