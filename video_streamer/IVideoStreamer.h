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
#include "fps_counter.h"

namespace cs
{
	class IVideoStreamer
	{
	public:
		IVideoStreamer() {};
		virtual ~IVideoStreamer() {};

		virtual void init(int port, const char* channel_name, int width = 0, int heigth = 0, int fps = 0) = 0;
		virtual int open(int port, int tunneling_port = 0) = 0;
		virtual void show_frame(cv::Mat& frame, const char* channel) = 0;
		virtual void add_user_credentials(const char* user, const char* password) {};

		void show(cv::Mat& frame, const char* channel)
		{
			if (fps != nullptr) {
				if (fps->get_fps() > max_output_fps) {
					return; 
				}
				else
					fps->tick("IVideoStreamer::show", channel);
			}
			
			show_frame(frame, channel);
		}

		void set_max_output_fps(unsigned int new_fps)
		{
			max_output_fps = new_fps;

			if (new_fps != 0) {
				if (fps == nullptr) {
					fps = new fps_counter();
					fps->init(default_fps_counter_step);
				}
			} 
			else {
				if (fps != nullptr) {
					delete fps;
					fps = nullptr;
				}
			}
		}
	private:
		unsigned int max_output_fps = 0;
		fps_counter* fps = nullptr;
	};
}


