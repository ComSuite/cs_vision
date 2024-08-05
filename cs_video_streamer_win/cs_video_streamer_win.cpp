/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief       HTTP MJPEG Video streamer
 *
 **************************************************************************************
 * Copyright (c) 2024, Alexander Epstine (a@epstine.com)
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
 *
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include "HTTPVideoStreamer.h"
#include "cxxopts.hpp"

using namespace cv;
using namespace std;
using namespace cs;

#include <queue>
#include <mutex>

template<class T>
class BaseQueue
{
public:
	BaseQueue() {}

	void push(T* elem) {
		if (elem == nullptr)
			return;

		m.lock();
		q.push(elem);
		m.unlock();
	}

	T* pop()
	{
		T* elem = nullptr;

		m.lock();
		if (!q.empty()) {
			elem = q.front();
			q.pop();
		}
		m.unlock();

		return elem;
	}
private:
	std::queue<T*> q;
	std::mutex m;
};

class fps_counter
{
public:
	void tick(const char* prompt)
	{
		counter++;
		if (ticker == 50) {
			ticker = 0;
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			long long c = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
			if (c > 0)
				std::cout << prompt << counter / c << std::endl;
		}
		else
			ticker++;
	}

	void init()
	{
		counter = 0;
		ticker = 0;
		begin = std::chrono::steady_clock::now();
	}
private:
	int64_t counter = 0;
	std::chrono::steady_clock::time_point begin;
	int ticker = 0;
};

class thread_param 
{
public:
	string path = "";
	int port = 8088;
	string channel = "camera";
	bool is_loop = false;
	bool is_display = true;
	bool is_fps = false;
	bool is_file = false;
	fps_counter fps_cnt;
	BaseQueue<cv::Mat> queue;

	int fps = 0;
	int width = 0;
	int height = 0;

	void print()
	{
		std::cout << "path: " << path << std::endl;
		std::cout << "port: " << port << std::endl;
		std::cout << "channel: " << channel << std::endl;
		std::cout << "loop: " << is_loop << std::endl;
		std::cout << "display: " << is_display << std::endl;
		std::cout << "fps: " << is_fps << std::endl;
	}
};

void print_copyright()
{
	cout << "Video Streamer for Windows" << endl;
	cout << "Copyright (C) 2024 by Alex Epstine (a@epstine.com). All rights reserved." << endl;
}

void parse_command_line(int argc, char* argv[], thread_param* param)
{
	cxxopts::Options options("", "");
	options.allow_unrecognised_options();
	options.add_options()
		("h,help", "Print help")
		("f,file", "Path to video file", cxxopts::value<string>())
		("p,port", "Port number", cxxopts::value<int>())
		("c,channel", "Channel name with first slush", cxxopts::value<string>())
		("l,loop", "Loop video")
		("m,movie", "Show movie")
		("s,fps", "Show FPS");

	auto result = options.parse(argc, argv);

	if (result.count("help")) {
		cout << options.help() << endl;
		exit(0);
	}

	if (result.count("file")) {
		param->path = result["file"].as<string>();
	}
	else {
		cout << options.help() << endl;
		exit(0);
	}

	if (result.count("port")) {
		param->port = result["port"].as<int>();
	}
	else {
		cout << options.help() << endl;
		exit(0);
	}

	if (result.count("channel")) {
		param->channel = result["channel"].as<string>();
	}
	else {
		cout << options.help() << endl;
		exit(0);
	}

	param->is_loop = result.count("loop");
	param->is_display = result.count("movie");
	param->is_fps = result.count("fps");
}

/*
std::atomic_flag sync = ATOMIC_FLAG_INIT;

void stream_thread_func(Mat* frame, const char* channel, IVideoStreamer* streamer, thread_param* param)
{
	streamer->show_frame(*frame, channel);
	sync.clear(); // = 1;

	delete frame;
}
*/

void stream_frame(IVideoStreamer* streamer, thread_param* param)
{
	while (true) {
		Mat* frame = param->queue.pop();
		if (frame != nullptr) {
			if (param->is_fps)
				param->fps_cnt.tick("FPS: ");

			streamer->show_frame(*frame, param->channel.c_str());
			delete frame;

				//Mat* show_frame = new Mat(*frame);
				//thread stream_tread(stream_thread_func, show_frame, channel, streamer, param);
				//stream_tread.detach();
		}
	}
}

void thread_func(void* arg)
{
	thread_param* param = (thread_param*)arg;

	IVideoStreamer* streamer = new HTTPVideoStreamer();
	if (streamer == nullptr) {
		cout << "Not enough memory" << endl;
		return;
	}

	streamer->open(param->port);

	cv::VideoCapture* capture = new VideoCapture(param->path, CAP_FFMPEG);
	if (capture == nullptr) {
		cout << "Not enough memory" << endl;
		delete streamer;
		return;
	}

	if (capture->isOpened()) {
		param->fps = int(capture->get(CAP_PROP_FPS));
		param->width = int(capture->get(CAP_PROP_FRAME_WIDTH));
		param->height = int(capture->get(CAP_PROP_FRAME_HEIGHT));

		cout << endl << "Source details:" << endl << "width: " << param->width << endl << "height: " << param->height << endl << "fps: " << param->fps << endl << endl;
	}
	else {
		cout << "Can`t open video source: " << param->path << endl;
		delete streamer;
		delete capture;
		return;
	}

	param->fps_cnt.init();
	param->is_file = std::filesystem::exists(param->path);

	std::thread stream_thread(stream_frame, streamer, param);
	stream_thread.detach();

	cv::Mat frame;
	for (;;) {
		*capture >> frame;
		if (!frame.empty()) {
			param->queue.push(new Mat(frame));
			//stream_frame_(&frame, param->channel.c_str(), streamer, param);

			if (param->is_display)
				imshow("Camera Emulator", frame);
		}

		if (capture->get(CAP_PROP_POS_FRAMES) == capture->get(CAP_PROP_FRAME_COUNT)) {
			if (param->is_loop)
				capture->set(CAP_PROP_POS_FRAMES, 0);
			else
				break;
		}

		//if (param->is_file && param->fps != 0) {
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000 / param->fps / 3));
		//}

		if (waitKey(1) == 27) //1000 / param->fps
			break;
	}

	delete streamer;
	delete capture;
}

int main(int argc, char** argv)
{
	print_copyright();

	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

	thread_param* param = new thread_param();
	if (param == nullptr) {
		return 1;
	}

	parse_command_line(argc, argv, param);

	param->print();

	std::thread detect_thread(thread_func, param);
	detect_thread.join();

	delete param;

	return 0;
}

