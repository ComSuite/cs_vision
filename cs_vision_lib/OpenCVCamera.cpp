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

#include "OpenCVCamera.h"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;
using namespace cs;

OpenCVCamera::OpenCVCamera()
{

}

OpenCVCamera::~OpenCVCamera()
{
	close();
}

int OpenCVCamera::info()
{
	int ret = 1;

	fps = int(capture->get(CAP_PROP_FPS));
	width = int(capture->get(CAP_PROP_FRAME_WIDTH));
	height = int(capture->get(CAP_PROP_FRAME_HEIGHT));

	cout << "Frame rate: " << fps << " Width: " << width << " Heigth: " << height << endl;

	return width > 0 && height > 0;
}

inline bool file_exists(const char* path)
{
	ifstream f(path);
	return f.good();
}

void OpenCVCamera::bring_to_start()
{
	if (capture != nullptr && capture->isOpened() && source_is_file) {
		capture->set(CAP_PROP_POS_FRAMES, 0);
	}
}

bool OpenCVCamera::is_end_of_file()
{
	if (!source_is_file)
		return false;

	if (capture->get(CAP_PROP_POS_FRAMES) == capture->get(CAP_PROP_FRAME_COUNT))
		return true;
	else
		return false;
}

int OpenCVCamera::open(camera_settings* settings, void* param)
{
	if (settings == nullptr)
		return 0;

	auto device = settings->device;
	if (std::holds_alternative<int>(device)) {
		return open(std::get<int>(device), settings->frame_width, settings->frame_height);
	}
	else if (std::holds_alternative<std::string>(device)) {
		return open(std::get<string>(device).c_str());
	}

	return 0;
}

int OpenCVCamera::open(std::variant<std::string, int> device, const int frame_width, const int frame_height)
{
	if (std::holds_alternative<int>(device)) {
		return open(std::get<int>(device), frame_width, frame_height);
	}
	else if (std::holds_alternative<std::string>(device)) {
		return open(std::get<string>(device).c_str());
	}

	return 0;
}

int OpenCVCamera::open(const char* name)
{
	close();

	source_is_file = false;
	device = name;

	try {
		capture = new VideoCapture(name); //, CAP_V4L
		if (capture->isOpened()) {
			cout << "Video from: " << name << endl;
			source_is_file = file_exists(name);
			return info();
		}
	}
	catch (...) {

	}

	return 0;
}

int OpenCVCamera::open(int id, const int frame_width, const int frame_height)
{
	close();

	source_is_file = false;
	device = id;

	try {
#ifdef __LINUX__
		capture = new VideoCapture(id, CAP_V4L); //, CAP_OPENCV_MJPEG , CAP_V4L
#else
		capture = new VideoCapture(id, CAP_DSHOW);
#endif
		if (capture == nullptr)
			return 0;

		cout << "OpenCV backend: " << capture->getBackendName() << endl;
		if (frame_width > 0 || frame_height > 0) {
			capture->set(CAP_PROP_FRAME_WIDTH, frame_width);
			capture->set(CAP_PROP_FRAME_HEIGHT, frame_height);
		}

		if (capture->isOpened()) {
			cout << "Video from device: #" << id << endl;
			return info();
		}
		else
			cout << "Can not open camera #" << id << endl;
	}
	catch (...) {

	}

	return 0;
}

int OpenCVCamera::close()
{
	int ret = 0;

	if (capture != nullptr) {
		if (capture->isOpened())
			capture->release();
		delete capture;

		capture = nullptr;
	}

	return ret;
}

int OpenCVCamera::prepare()
{
	int ret = 1;

	return ret;
}

int OpenCVCamera::get_frame(const char* name, cv::Mat& frame, bool convert_to_gray)
{
	return 0;
}

int OpenCVCamera::get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray)
{
	return 0;
}

int OpenCVCamera::get_frame(cv::Mat& frame_out, bool convert_to_gray)
{
	int ret = 1;

	if (capture && capture->isOpened()) {
		*capture >> frame_out;

		if (!frame_out.empty()) {
			if (convert_to_gray)
				cvtColor(frame_out, frame_out, COLOR_BGR2GRAY);
		}
		else {
			open(device);
			ret = 0;
		}
	}
	else {
		if (capture != nullptr)
			open(device);

		ret = 0;
	}

	return ret;
}

bool OpenCVCamera::is_opened()
{
	return capture->isOpened();
}

int OpenCVCamera::get_frame(cv::cuda::GpuMat& frame_out, bool convert_to_gray)
{
	//Mat frame;

	int ret = get_frame(frame, convert_to_gray);
	if (ret == 1)
		frame_out.upload(frame);

	return ret;
}

int OpenCVCamera::save_to_file()
{
	int ret = 0;

	return ret;
}

int OpenCVCamera::get_width()
{
	return width;
}

int OpenCVCamera::get_height()
{
	return height;
}

