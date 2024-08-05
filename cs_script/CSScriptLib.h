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
#include "MQTTClient.h"
#include "../rapidjson/document.h"
#ifdef __WITH_SCRIPT_LANG__
#include "chaiscript/chaiscript.hpp"
#endif

#ifdef __WITH_SCRIPT_LANG__
namespace cs
{
	class mqtt_lib
	{
	public:
		virtual ~mqtt_lib()
		{
			if (mqtt != nullptr)
				delete mqtt;
		}

		void mqtt_init(struct mosquitto* mosq);
		void mqtt_send(const std::string& topic, const std::string& payload);
		void mqtt_subscribe(const std::string& topic, const std::string& callback);
		void mqtt_unsubscribe(const std::string& topic);
	private:
		MQTTClient* mqtt = nullptr;
	};
	
	class gpio_lib
	{
	public:
		void set_gpio();
		int get_gpio();
	};

	class cv_lib
	{
	public:
#ifdef __HAS_CUDA__
		void draw_text(cv::cuda::GpuMat* src, const std::string& text, int x, int y, int size, int r, int g, int b, int weight);
		int mat_get_cols(cv::cuda::GpuMat* src);
		int mat_get_rows(cv::cuda::GpuMat* src);
#else
		void draw_text(cv::Mat* img, const std::string& text, int x, int y, int size, int r, int g, int b, int weight);
		int mat_get_cols(cv::Mat* src);
		int mat_get_rows(cv::Mat* src);
#endif
	};

	class CSScriptLib
	{
	public:
		CSScriptLib() {}
		CSScriptLib(CSScriptLib* lib) {}
		virtual ~CSScriptLib()
		{

		}

		void init(chaiscript::ChaiScript* chai, struct mosquitto* mosq);
		cv_lib* get_cv_lib() { return &cv; }
		mqtt_lib* get_mqtt_lib() { return &mqtt; };
		gpio_lib* get_gpio_lib() { return &gpio; };

		bool is_null(const void* ptr) { return ptr == nullptr; }
		uint64_t get_time_stamp();
	private:
		chaiscript::ChaiScript* chai = nullptr;
		mqtt_lib mqtt;
		gpio_lib gpio;
		cv_lib cv;
	};
}
#endif
