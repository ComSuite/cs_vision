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

#ifdef __WITH_SCRIPT_LANG__

#include <map>
#include <opencv2/opencv.hpp>
#include "../rapidjson/document.h"
#include "chaiscript/chaiscript.hpp"
#include "IObjectDetector.h"
#include "CSScriptLib.h"

namespace cs 
{
	struct script_detection_param
	{
		int get_detections_count()
		{
			if (detections != nullptr)
				return detections->size();

			return 0;
		}

		DetectionItem* get_detection_item(int index)
		{
			if (index >= detections->size())
				return nullptr;

			std::list<DetectionItem*>::iterator it = detections->begin();
			advance(it, index);

			return *it;
		}

#ifdef __HAS_CUDA__
		cv::cuda::GpuMat* get_image() { return image; }
#else
		cv::Mat* get_image() { return image; };
#endif


		std::list<DetectionItem*>* detections = nullptr;
#ifdef __HAS_CUDA__
		cv::cuda::GpuMat* image = nullptr;
#else
		cv::Mat* image = nullptr;
#endif
	};

	class CSScript
	{
	public:
		virtual ~CSScript()
		{
			if (chai != nullptr)
				delete chai;

			if (lib != nullptr)
				delete lib;

			scripts.clear();
		}

		bool init(struct mosquitto* mosq);
#ifdef __HAS_CUDA__
		bool execute_script(const char* filename, std::list<cs::DetectionItem*>* detections, cv::cuda::GpuMat* image);
		bool execute(const char* expr, std::list<cs::DetectionItem*>* detections, cv::cuda::GpuMat* image);
#else
		bool execute_script(const char* filename, std::list<cs::DetectionItem*>* detections, cv::Mat* image);
		bool execute(const char* expr, std::list<cs::DetectionItem*>* detections, cv::Mat* image);
#endif
	private:
		chaiscript::ChaiScript* chai = nullptr;
		chaiscript::ModulePtr module = nullptr;
		script_detection_param param;
		CSScriptLib* lib = nullptr;

		std::map<std::string, std::string> scripts;
	};
}

#endif