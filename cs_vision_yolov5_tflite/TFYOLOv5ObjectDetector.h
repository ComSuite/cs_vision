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

#include "yolov5_tflite.h"
#include "IObjectDetector.h"
#include "JsonWrapper.h"

namespace cs
{
	class TFYOLOv5ObjectDetector : public IObjectDetector, JsonWrapper
	{
	public:
		TFYOLOv5ObjectDetector();
		virtual ~TFYOLOv5ObjectDetector();

		virtual int init(object_detector_environment& env) override;

		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false, std::list<DetectionItem*>* detections = nullptr) override;
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false) override;
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override { return 0; };
	private:
		Prediction out_pred;
		YOLOV5 yolo_model;
	};
}

