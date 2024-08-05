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

#include "tensorflow/lite/c/c_api.h"
#include "tensorflow/lite/c/c_api_experimental.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "IObjectDetector.h"

namespace cs
{
	class TFAudioSampleRecognizer : public IObjectDetector
	{
	public:
		TFAudioSampleRecognizer();
		virtual ~TFAudioSampleRecognizer();

		virtual int init(const char* model_path, const char* label_path, const char* rules_path, bool is_use_gpu = false) override;
		virtual int init(const char* model_path, const char* label_path, const char* rules_path, const char* input_tensor_name, const char* output_tensor_name, bool is_use_gpu = false) override;
		virtual int init(void* param, bool is_use_gpu = false) override { return 0; };
		virtual void clear();
		virtual int detect(cv::Mat* input, int& current_id, bool is_draw = false);
		virtual int detect(cv::cuda::GpuMat* input, int& current_id, bool is_draw = false);
		virtual int detect_batch(const std::vector<cv::Mat*>& input, int& current_id, bool is_draw = false) override { return 0; }

#ifdef __HAS_CUDA__
		virtual void draw_detection(cv::cuda::GpuMat* detect_frame, DetectionItem* detection, cv::Scalar& background_color, bool is_show_mask) override;
#else
		virtual void draw_detection(cv::Mat* detect_frame, DetectionItem* detectioncv::Scalar& background_color, bool is_show_mask) override;
#endif
	private:
		TfLiteModel* model = nullptr;
		TfLiteInterpreterOptions* options = nullptr;
		TfLiteDelegate* metal_delegate = nullptr;
		TfLiteInterpreter* interpreter = nullptr;
		TfLiteTensor* input_tensor = nullptr;
		const TfLiteTensor* output_tensor = nullptr;
	};
}

