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

#include "ICamera.h"
#ifdef __HAS_CUDA__
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>
#endif
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <iostream>
#include <thread>

using namespace std;
using namespace cv;
using namespace cs;

int ICamera::start_save_video(const int id, const char* output_movie_name)
{
	int ret = 1;

	try {
		Size size(get_width(), get_height());
		video_writer = new VideoWriter(output_movie_name, VideoWriter::fourcc('M', 'P', '4', 'V'), 17, size, false);
	}
	catch(Exception e) {
		ret = 0;
	}

	return ret;
}

int ICamera::save_frame(cv::Mat& frame)
{
	int ret = 1;

	if (video_writer == NULL)
		return ret;

	if (!frame.empty()) {
		video_writer->write(frame);
	}
	else
		ret = 0;

	return ret;
}

#ifdef __HAS_CUDA__
int ICamera::save_frame(cv::cuda::GpuMat& in_frame)
{
	Mat frame;
	in_frame.download(frame);

	return save_frame(frame);
}
#endif

int ICamera::close()
{ 
	if (video_writer != NULL) {
		cout << "Sve video started" << endl;
		video_writer->release();
		cout << "Video writer closed" << endl;
	}

	return 0; 
};
