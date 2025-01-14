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

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/photo.hpp>
#include "mjpeg_streamer.hpp"
#include "IVideoStreamer.h"

namespace cs 
{
	class HTTPVideoStreamer : public IVideoStreamer
	{
	public:
		HTTPVideoStreamer();
		virtual ~HTTPVideoStreamer();

		virtual void init(int port, const char* channel_name, int width = 0, int height = 0, int fps = 0) override {};
		virtual int open(int port, int tunneling_port = 0) override;
		virtual void show_frame(cv::Mat& frame, const char* channel) override;
	private:
		std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
		nadjieb::MJPEGStreamer* streamer = NULL;
	};
}

