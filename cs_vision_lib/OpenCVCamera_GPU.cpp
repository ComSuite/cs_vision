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

#include "OpenCVCamera_GPU.h"

using namespace cs;
using namespace cv;
using namespace cv::cuda;
using namespace std;

OpenCVCamera_GPU::OpenCVCamera_GPU()
{
}

OpenCVCamera_GPU::~OpenCVCamera_GPU()
{
	if (capture != nullptr)
		delete capture;
}

int OpenCVCamera_GPU::info()
{
	if (capture == nullptr)
		return 0;

	cv::cudacodec::FormatInfo info = capture->format();

	width = info.width;
	height = info.height;
	fps = info.fps;

	return 1;
}

int OpenCVCamera_GPU::open(std::variant<std::string, int> device, const int frame_width, const int frame_height)
{
	if (std::holds_alternative<int>(device)) {
		return open(std::get<int>(device), frame_width, frame_height);
	}
	else if (std::holds_alternative<std::string>(device)) {
		return open(std::get<string>(device).c_str());
	}
}

int OpenCVCamera_GPU::open(const int id, const int frame_width, const int frame_height)
{
	if (capture == nullptr)
		return 0;

	return 1;
}

int OpenCVCamera_GPU::open(const char* name)
{
	capture = cv::cudacodec::createVideoReader(std::string(name));

	if (capture == nullptr)
		return 0;

	return 1;
}

int OpenCVCamera_GPU::close()
{
	if (capture == nullptr)
		return 0;

	return 1;
}

int OpenCVCamera_GPU::prepare()
{
	return 0;
}

int OpenCVCamera_GPU::save_to_file()
{
	return 0;
}

int OpenCVCamera_GPU::get_frame(const char* name, cv::Mat& frame, bool convert_to_gray)
{
	return capture->retrieve(frame);
}

int OpenCVCamera_GPU::get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray)
{
	return capture->retrieve(frame);
}

int OpenCVCamera_GPU::get_frame(cv::Mat& frame, bool convert_to_gray)
{
	return capture->retrieve(frame);
}

int OpenCVCamera_GPU::get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray)
{
	return capture->retrieve(frame);
}

int OpenCVCamera_GPU::get_width()
{
	return width;
}

int OpenCVCamera_GPU::get_height()
{
	return height;
}

bool OpenCVCamera_GPU::is_end_of_file()
{
	return false;
}

void OpenCVCamera_GPU::bring_to_start()
{

}

bool OpenCVCamera_GPU::is_opened()
{
	return false;
}
