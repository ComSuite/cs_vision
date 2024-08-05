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

#include <yolov5_detector.hpp>
#include "IObjectDetector.h"
#include "JsonWrapper.h"

namespace cs 
{
	class TRTYOLOv5ObjectDetector : public IObjectDetector
	{
	public:
		TRTYOLOv5ObjectDetector();
		virtual ~TRTYOLOv5ObjectDetector();

		virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false)  override;
		virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false)  override;
		virtual int init(void* param, bool is_use_gpu = false)  override { return 0; };
		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override;
	private:
		yolov5::Detector detector;
		yolov5::Classes classes;

		void postprocess(std::vector<yolov5::Detection>& detections, int& current_id, bool is_draw, cv::Mat* image = nullptr);
	};
}

