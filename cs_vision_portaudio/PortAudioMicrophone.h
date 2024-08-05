/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2023, Alexander Epstine (a@epstine.com)
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

#include "ICamera.h"
#include <portaudio.h>
#include "audio_types.h"

namespace cs
{
	class PortAudioMicrophone : public ICamera
	{
	public:
		PortAudioMicrophone();
		virtual ~PortAudioMicrophone();

		virtual int info();
		virtual int open(std::variant<std::string, int> device);
		virtual int open(const int id);
		virtual int open(const char* name);
		virtual int close();
		virtual int prepare();
		virtual int save_to_file();

#ifdef  __HAS_CUDA__
		virtual int get_frame(cv::cuda::GpuMat& frame, bool convert_to_gray);
		virtual int get_frame(const char* name, cv::cuda::GpuMat& frame, bool convert_to_gray);
#endif 
		virtual int get_frame(const char* name, cv::Mat& frame, bool convert_to_gray);
		virtual int get_frame(cv::Mat& frame, bool convert_to_gray);
		virtual int get_width();
		virtual int get_height();
		virtual int get_fps();

		virtual bool is_end_of_file() override { return 0; }
		virtual bool is_opened() override;
		virtual void bring_to_start() override {}

		virtual void set_detector_buffer(size_t length) override;

		void set_buffer(char* data, uint length);
	private:
		audio_callback_data* audio_data = nullptr;
		PaStream* stream = nullptr;
		int channels = 0;
		char* detector_buffer = nullptr;
		size_t detector_cur_position = 0;
		size_t detector_input_length = 0;
	};
}

