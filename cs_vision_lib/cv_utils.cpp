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

#include "cv_utils.h"
#ifdef __HAS_CUDA__
#include "opencv2/core/cuda.hpp"
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaarithm.hpp>
#endif

using namespace cv;
using namespace cv::cuda;

Mat cpu_rotate(Mat& src, double angle)
{
    Mat dst;

    Point2f pt(src.cols / 2., src.rows / 2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine(src, dst, r, Size(src.cols, src.rows));

    return dst;
}

#ifdef __HAS_CUDA__
GpuMat gpu_rotate(GpuMat& src, double angle)
{
    GpuMat dst;

    Point2f pt(src.cols / 2., src.rows / 2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    cuda::warpAffine(src, dst, r.getUMat(ACCESS_READ), Size(src.cols, src.rows), INTER_LINEAR);

    return dst;
}
#endif

void cpu_rotate(cv::Mat& src, cv::Mat& dst, double angle)
{
    Point2f pt(static_cast<float>(src.cols / 2.), static_cast<float>(src.rows / 2.));
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(src, dst, r, Size(src.cols, src.rows));
}

#ifdef __HAS_CUDA__
void gpu_rotate(cv::cuda::GpuMat& src, cv::cuda::GpuMat& dst, double angle)
{
    Point2f pt(src.cols / 2., src.rows / 2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    cuda::warpAffine(src, dst, r.getUMat(ACCESS_READ), Size(src.cols, src.rows), INTER_LINEAR);
}
#endif

Mat cpu_resize(Mat& src, int width, int height, int inter) 
{
    Size dim, hw;
    hw = src.size();

    if (width == 0 && height == 0)
        return src;

    if (width == 0) {
        float r = float(height) / float(hw.height);
        dim = Size((hw.width * r), height);
    }
    else {
        float r = float(width) / float(hw.width);
        dim = Size(width, (hw.height * r));
    }

    Mat resized;
    cv::resize(src, resized, dim, 0, 0, inter);

    return resized;
}

#ifdef __HAS_CUDA__
GpuMat gpu_resize(GpuMat& src, int width, int height, int inter)
{
    Size dim, hw;
    hw = src.size();

    if (width == 0 && height == 0)
        return src;

    if (width == 0) {
        float r = float(height) / float(hw.height);
        dim = Size((hw.width * r), height);
    }
    else {
        float r = float(width) / float(hw.width);
        dim = Size(width, (hw.height * r));
    }

    GpuMat resized;
    cuda::resize(src, resized, dim, 0, 0, inter);

    return resized;
}
#endif

// from cpu_resize and gpu_resize
void cpu_calc_original(Rect& src, Rect& orig, Rect& resized)
{


}

// from OpenCV resize
void cpu_calc_original1(Rect& src, Rect& orig, Rect& resized)
{
    float Cw = resized.width / orig.width;
    float Ch = resized.height / orig.height;

    src.x = src.x * Cw;
    src.y = src.y * Ch;
    src.width = src.width * Cw;
    src.height = src.height * Ch;
}

void cpu_resize(cv::Mat& src, cv::Mat& dst, int width, int height, int inter)
{
    //Size dim, hw;
    //hw = src.size();

    if (width == 0 && height == 0) {
        src.copyTo(dst);
        return;
    }

    float c = width / height;
    float nwidth = src.size().width;
    float nheight = src.size().height;
    if (src.size().width > src.size().height)
    {
        nheight = src.size().height;
        nwidth = nheight * c;
    }

    int x0, x1, y0, y1;
    x0 = (src.size().width - nwidth) / 2;
    y0 = (src.size().height - nheight) / 2;
    x1 = x0 + nwidth;
    y1 = y0 + nheight;

    Mat cropped = src(Range(y0, y1), Range(x0, x1));

    //if (width == 0) {
    //    float r = float(height) / float(hw.height);
    //    dim = Size((hw.width * r), height);
    //}
    //else {
    //    float r = float(width) / float(hw.width);
    //    dim = Size(width, (hw.height * r));
    //}

    cv::resize(cropped, dst, Size(width, height), 0, 0, inter);
}

#ifdef __HAS_CUDA__
void gpu_resize(cv::cuda::GpuMat& src, cv::cuda::GpuMat& dst, int width, int height, int inter)
{
    //Size dim, hw;
    //hw = src.size();

    if (width == 0 && height == 0) {
        src.copyTo(dst);
        return;
    }

    float c = width / height;
    float nwidth = src.size().width;
    float nheight = src.size().height;
    if (src.size().width > src.size().height)
    {
        nheight = src.size().height;
        nwidth = nheight * c;
    }

    int x0, x1, y0, y1;
    x0 = (src.size().width - nwidth) / 2;
    y0 = (src.size().height - nheight) / 2;
    x1 = x0 + nwidth;
    y1 = y0 + nheight;

    GpuMat cropped = src(Range(y0, y1), Range(x0, x1));

/*
    if (width == 0) {
        float r = float(height) / float(hw.height);
        dim = Size((hw.width * r), height);
    }
    else {
        float r = float(width) / float(hw.width);
        dim = Size(width, (hw.height * r));
    }
*/

    cuda::resize(cropped, dst, Size(width, height), 0, 0, inter);
}
#endif

void cpu_create_square_letterbox(Mat& src, Mat& dst, int letterbox_size, Size& border_dims)
{
    float a = max(src.cols, src.rows);
    float r = letterbox_size / a;

    Mat resized;
    cv::resize(src, resized, Size(0, 0), r, r, INTER_CUBIC); //INTER_AREA

    float d = (letterbox_size - resized.rows) / 2;
    border_dims.height = static_cast<int>(std::round(d));
    border_dims.width = 0;
    cv::copyMakeBorder(resized, dst, border_dims.height, border_dims.height, border_dims.width, border_dims.width, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
}

#ifdef __HAS_CUDA__
void gpu_create_square_letterbox(GpuMat& src, GpuMat& dst, int letterbox_size, Size& border_dims)
{
    float a = max(src.cols, src.rows);
    float r = letterbox_size / a;

    GpuMat resized;
    cv::cuda::resize(src, resized, Size(0, 0), r, r, INTER_CUBIC); //INTER_AREA
    
    float d = (letterbox_size - resized.rows) / 2;
    border_dims.height = static_cast<int>(std::round(d));
    border_dims.width = 0;
    cv::cuda::copyMakeBorder(resized, dst, static_cast<int>(std::round(d)), static_cast<int>(std::round(d)), 0, 0, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
}
#endif

cv::Scalar cv_string_to_color(const char* str_color)
{
    unsigned int x = std::stoul(str_color, nullptr, 16);
    return cv::Scalar(x & 0x00FF0000, x & 0x0000FF00 >> 2, x & 0x000000FF);
}

cv::Scalar generate_contrast_color(const cv::Scalar& color)
{
    return cv::Scalar(255 - color[0], 255 - color[1], 255 - color[2]);
}

