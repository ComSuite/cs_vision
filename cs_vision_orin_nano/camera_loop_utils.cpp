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
#include "NullObjectDetector.h"
#include "TRTYOLOv5ObjectDetector.h"
#include "TRTYOLOv8ObjectDetector.h"
//#include "OCVYOLOv8ObjectDetector.h"
#include "TFYOLOv5ObjectDetector.h"
//#include "TFAudioSampleRecognizer.h"
//#include "HaarCascadeClassifier.h"
#include "TRTYOLOv11ObjectDetector.h"
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
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLOv5: return new TRTYOLOv5ObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_OPENCV_YOLOv5: return nullptr;
	//case ObjectDetectorKind::OBJECT_DETECTOR_OPENCV_YOLOv8: return new OCVYOLOv8ObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORFLOW_YOLOv5: return new TFYOLOv5ObjectDetector();
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLOv8: return new TRTYOLOv8ObjectDetector();
	//case ObjectDetectorKind::AUDIO_RECOGNIZER_TFLITE: return new TFAudioSampleRecognizer();
	//case ObjectDetectorKind::HAAR_CASCADE_CLASSIFIER: return new HaarCascadeClassifier();
	case ObjectDetectorKind::OBJECT_DETECTOR_TENSORRT_YOLOv11: return new TRTYOLOv11ObjectDetector();
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
	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);
	if (prop.canMapHostMemory) {
		//cudaSetDeviceFlags(cudaDeviceMapHost);
		//cudaHostAlloc(&a_h, nBytes, cudaHostAllocMapped);
		//cudaHostGetDevicePointer(&a_map, a_h, 0);
		//kernel << <gridSize, blockSize >> > (a_map);
	}
#endif
}