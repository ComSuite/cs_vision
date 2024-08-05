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

#include "CSScript.h"
#include "std_utils.h"
#include <opencv2/opencv.hpp>

#ifdef __WITH_SCRIPT_LANG__

using namespace cs;

bool CSScript::init(struct mosquitto* mosq)
{
	if (chai != nullptr)
		return true;

	chai = new chaiscript::ChaiScript();
	if (chai != nullptr) {
		lib = new CSScriptLib();

		chai->add(chaiscript::user_type<DetectionItem>(), "DetectionItem");
		chai->add(chaiscript::user_type<cv::Mat>(), "cvMat");
		chai->add(chaiscript::user_type<cv::cuda::GpuMat>(), "cvGpuMat");
		chai->add(chaiscript::user_type<CSScriptLib>(), "CSScriptLib");

		chai->add_global(chaiscript::var(&param), "detections");
		chai->add_global(chaiscript::var(lib), "cslibrary");

		chai->add(chaiscript::fun(&script_detection_param::get_detections_count), "get_detections_count");
		chai->add(chaiscript::fun(&script_detection_param::get_detection_item), "get_detection_item");
		chai->add(chaiscript::fun(&script_detection_param::get_image), "get_image");

		chai->add(chaiscript::fun(&DetectionItem::get_id), "get_id");
		chai->add(chaiscript::fun(&DetectionItem::get_neural_network_id), "get_neural_network_id");
		chai->add(chaiscript::fun(&DetectionItem::get_class_id), "get_class_id");
		chai->add(chaiscript::fun(&DetectionItem::get_score), "get_score");

		chai->add(chaiscript::fun(&DetectionItem::set_is_send_result), "set_is_send_result");
		chai->add(chaiscript::fun(&DetectionItem::set_color), "set_color");

		if (lib != nullptr) {
			lib->init(chai, mosq);
		}

		return true;
	}

	return false;
}

#ifdef __HAS_CUDA__
bool CSScript::execute_script(const char* filename, std::list<DetectionItem*>* detections, cv::cuda::GpuMat* image)
#else
bool CSScript::execute_script(const char* filename, std::list<DetectionItem*>* detections, cv::Mat* image)
#endif
{
	if (chai == nullptr)
		return false;

	if (scripts.find(filename) == scripts.end()) {
		scripts[filename] = read_str_file(filename);
	}

	execute(scripts[filename].c_str(), detections, image);

	return true;
}

#ifdef __HAS_CUDA__
bool CSScript::execute(const char* expr, std::list<cs::DetectionItem*>* detections, cv::cuda::GpuMat* image)
#else
bool CSScript::execute(const char* expr, std::list<cs::DetectionItem*>* detections, cv::Mat* image)
#endif
{
	if (chai == nullptr)
		return false;

	try {
		param.detections = detections;
		param.image = image;

		chai->eval(expr);
	}
	catch (const chaiscript::exception::eval_error& e) {
		std::cout << "Error executing script: " << expr << std::endl << e.pretty_print() << std::endl;
		return false;
	}

	return true;
}

#endif