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

#include <opencv2/opencv.hpp>

cv::Scalar generate_contrast_color(const cv::Scalar& color);
cv::Mat cpu_rotate(cv::Mat& src, double angle);
void cpu_rotate(cv::Mat& src, cv::Mat& dst, double angle);
cv::Mat cpu_resize(cv::Mat& src, int width, int height, int inter);
void cpu_resize(cv::Mat& src, cv::Mat& dst, int width, int height, int inter);
void cpu_calc_original1(cv::Rect& src, cv::Rect& orig, cv::Rect& resized);
void cpu_create_square_letterbox(cv::Mat& src, cv::Mat& dst, int letterbox_size, cv::Size& border_dims);
cv::Scalar cv_string_to_color(const char* str_color);

#ifdef __HAS_CUDA__
cv::cuda::GpuMat gpu_rotate(cv::cuda::GpuMat& src, double angle);
void gpu_rotate(cv::cuda::GpuMat& src, cv::cuda::GpuMat& dst, double angle);
cv::cuda::GpuMat gpu_resize(cv::cuda::GpuMat& src, int width, int height, int inter);
void gpu_resize(cv::cuda::GpuMat& src, cv::cuda::GpuMat& dst, int width, int height, int inter);
void gpu_create_square_letterbox(cv::cuda::GpuMat& src, cv::cuda::GpuMat& dst, int letterbox_size, cv::Size& border_dims);
#endif