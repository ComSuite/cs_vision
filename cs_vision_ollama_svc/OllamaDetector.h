/**
 * @file		GemmaDetector.h
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

#include "IObjectDetector.h"
#include "command_processor.h"

namespace cs
{
	class OllamaDetector : public IObjectDetector
	{
	public:
		OllamaDetector();
		~OllamaDetector();

		virtual int init(object_detector_environment& env) override;

		virtual void clear();

		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;

		virtual void parse(const std::string& payload, int& current_id);

		virtual void set_prompt(const std::string& prompt) {
			this->prompt = prompt;
		}
	private:
		std::string endpoint = "";
		std::string model = "";
		std::string prompt = "";
	};
}
