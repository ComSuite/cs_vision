/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2025, Alexander Epstine (a@epstine.com)
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

#include "IVideoStreamer.h"

class FFmpegH264Source;
class RTSPServer;
class TaskScheduler;
class UsageEnvironment;

namespace cs
{
	class RTSPVideoStreamer : public IVideoStreamer
	{
	public:
		RTSPVideoStreamer();
		virtual ~RTSPVideoStreamer();

		virtual void init(int port, const char* channel_name, int width = 0, int height = 0, int fps = 0) override;
		virtual int open(int port, int tunneling_port = 0) override;
		virtual void show_frame(cv::Mat& frame, const char* channel) override;

	private:
		FFmpegH264Source* source = nullptr;
		RTSPServer* rtspServer = nullptr;
		TaskScheduler* scheduler;
		UsageEnvironment* env;
	};
}

