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

#include "CSScriptLib.h"

#ifdef __WITH_SCRIPT_LANG__

#include <map>
#include <string>
#include "std_utils.h"

using namespace cs;
using namespace std;

void CSScriptLib::init(chaiscript::ChaiScript* chai, struct mosquitto* mosq)
{
	if (chai == nullptr)
		return;

	this->chai = chai;
	mqtt.mqtt_init(mosq);

	chai->add(chaiscript::fun(&CSScriptLib::get_cv_lib), "get_cv_lib");
	chai->add(chaiscript::fun(&CSScriptLib::get_mqtt_lib), "get_mqtt_lib");
	chai->add(chaiscript::fun(&CSScriptLib::get_gpio_lib), "get_gpio_lib");
	chai->add(chaiscript::fun(&CSScriptLib::is_null), "is_null");
	chai->add(chaiscript::fun(&CSScriptLib::get_time_stamp), "get_time_stamp");

	chai->add(chaiscript::fun(&mqtt_lib::mqtt_send), "mqtt_send");
	chai->add(chaiscript::fun(&mqtt_lib::mqtt_subscribe), "mqtt_subscribe");
	chai->add(chaiscript::fun(&mqtt_lib::mqtt_unsubscribe), "mqtt_unsubscribe");


	chai->add(chaiscript::fun(&gpio_lib::set_gpio), "set_gpio");
	chai->add(chaiscript::fun(&gpio_lib::get_gpio), "get_gpio");

	chai->add(chaiscript::fun(&cv_lib::draw_text), "draw_text");
	chai->add(chaiscript::fun(&cv_lib::mat_get_cols), "mat_get_cols");
	chai->add(chaiscript::fun(&cv_lib::mat_get_rows), "mat_get_rows");
}

uint64_t CSScriptLib::get_time_stamp()
{
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

////////////////////////////////////////////////////////////////////////////////
//MQTT
void mqtt_lib::mqtt_init(struct mosquitto* mosq)
{
	mqtt = new MQTTClient(mosq);
}

void mqtt_lib::mqtt_send(const std::string& topic, const std::string& payload)
{
	if (mqtt == nullptr)
		return;

	mqtt->send(topic.c_str(), payload.c_str());
}

void mqtt_lib::mqtt_subscribe(const std::string& topic, const std::string& callback)
{
	if (mqtt == nullptr)
		return;
}

void mqtt_lib::mqtt_unsubscribe(const std::string& topic)
{
	if (mqtt == nullptr)
		return;
}

////////////////////////////////////////////////////////////////////////////////
//GPIO
void gpio_lib::set_gpio()
{

}

int gpio_lib::get_gpio()
{
	return 0;
}

#endif


////////////////////////////////////////////////////////////////////////////////
//OpenCV
#ifdef __HAS_CUDA__
void cv_lib::draw_text(cv::cuda::GpuMat* src, const std::string& text, int x, int y, int size, int r, int g, int b, int weight)
#else
void cv_lib::draw_text(cv::Mat* img, std::string& text, int x, int y, int size, int r, int g, int b, int weight)
#endif
{
#ifdef __HAS_CUDA__
	if (src == nullptr)
		return;

	cv::Mat img;
	src->download(img);
#else
	if (img == nullptr)
		return;
#endif

	cv::Point text_position(x, y);
	int font_size = size;
	cv::Scalar font_Color(r, g, b);
	putText(img, text, text_position, cv::FONT_HERSHEY_COMPLEX, font_size, font_Color, weight);

#ifdef __HAS_CUDA__
	src->upload(img);
#endif
}

#ifdef __HAS_CUDA__
int cv_lib::mat_get_cols(cv::cuda::GpuMat* src)
#else
int cv_lib::mat_get_cols(cv::Mat* src)
#endif
{
	if (src == nullptr)
		return 0;

	return src->cols;
}

#ifdef __HAS_CUDA__
int cv_lib::mat_get_rows(cv::cuda::GpuMat* src)
#else
int cv_lib::mat_get_rows(cv::Mat* src)
#endif
{
	if (src == nullptr)
		return 0;

	return src->rows;
}

