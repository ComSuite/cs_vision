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

#include <string>
#include <opencv2/opencv.hpp>

namespace cs 
{
	class SampleImageWriter
	{
	public:
		SampleImageWriter() {};

		SampleImageWriter(std::string prefix, std::string ext) {
			init();
			this->prefix = prefix;
			this->ext = ext;
		};

		void init() {
			ind = 0;
		};

		void save_sample(cv::Mat& src, cv::Mat& dst) {
			std::string fname = prefix + std::to_string(ind);
			cv::imwrite(fname + ext, src);
			cv::imwrite(fname + "a" + ext, dst);

			ind++;
		};
	private:
		int ind = 0;
		std::string prefix = "si_";
		std::string ext = ".jpg";
	};
}
