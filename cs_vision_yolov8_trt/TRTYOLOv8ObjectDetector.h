/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
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
 */

#pragma once

#include "JsonWrapper.h"
#include "IObjectDetector.h"
#include "yolov8.h"

namespace cs
{
	class TRTYOLOv8ObjectDetector : public IObjectDetector
	{
	public:
		TRTYOLOv8ObjectDetector();
		virtual ~TRTYOLOv8ObjectDetector();

		virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false)  override;
		virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false)  override;
		virtual int init(void* param, bool is_use_gpu = false)  override { return 0; };
		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;
	private:
		const char* default_input_tensor_name = "";
		const char* default_output_tensor_name = "";

		std::unique_ptr<YoloV8> detector = nullptr;
		YoloV8Config config;

		void postprocess(std::vector<Object>& detections, int& current_id, bool is_draw, cv::Mat* image);
	};
}

