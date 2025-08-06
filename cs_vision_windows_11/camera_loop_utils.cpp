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

#include "camera_loop_utils.h"
//#include "OCVYOLOv8ObjectDetector.h"
#include "NullObjectDetector.h"
#include "TFYOLOv5ObjectDetector.h"
#include "TRTYOLOObjectDetector.h"
#include "GemmaDetector.h"
#include "QwenDetector.h"
#include "OllamaTextPromptDetector.h"
#include "TrackerByteTrack.h"
#include "TrackerDeepSORT.h"
#include "cv_utils.h"
#ifdef __HAS_CUDA__
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudawarping.hpp>
#endif

using namespace cs;
using namespace cv;
using namespace cv::cuda;

IObjectDetector* create_detector(int kind)
{
	switch (static_cast<ObjectDetectorKind>(kind)) {
	case ObjectDetectorKind::OBJECT_DETECTOR_NONE: return new NullObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORFLOW_YOLOv5: return new TFYOLOv5ObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLO: return new TRTYoloObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_SVC_GEMMA3: return new GemmaDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_SVC_QWEN: return new QwenDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_MOT_BYTETRACK: return new TrackerByteTrack();
	case ObjectDetectorKind::OBJECT_DETECTOR_MOT_DEEPSORT: return new TrackerDeepSORT();
	case ObjectDetectorKind::OBJECT_DETECTOR_OLLAMA_PROMPT: return new OllamaTextPromptDetector();
	}

	return nullptr;
}

void gpu_preprocessing(Mat& src, camera_settings* set, Mat& image, Size& border_dims)
{
#ifdef __HAS_CUDA__
	GpuMat frame;

	gpu_preprocessing(src, set, frame, border_dims);

	frame.download(image);
#endif
}

#ifdef __HAS_CUDA__
void gpu_preprocessing(cv::Mat& src, cs::camera_settings* set, cv::cuda::GpuMat& image, cv::Size& border_dims)
{
	image.upload(src);

	if (set->is_flip) {
		cuda::flip(image, image, 1);
	}

	if (set->get_rotate_angle() != 0) {
		GpuMat rotate;
		gpu_rotate(image, rotate, set->get_rotate_angle());
		rotate.copyTo(image);
	}
}
#endif

void gpu_setup_device()
{
#ifdef __HAS_CUDA__
	//cudaDeviceProp prop;
	//cudaGetDeviceProperties(&prop, 0);
	//if (prop.canMapHostMemory) {
		//cudaSetDeviceFlags(cudaDeviceMapHost);
		//cudaHostAlloc(&a_h, nBytes, cudaHostAllocMapped);
		//cudaHostGetDevicePointer(&a_map, a_h, 0);
		//kernel << <gridSize, blockSize >> > (a_map);
	//}
#endif
}