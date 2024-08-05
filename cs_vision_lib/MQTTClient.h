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

#include <list>
#include "../rapidjson/document.h"
#include "MQTTWrapper.h"
#include "IObjectDetector.h"

namespace cs
{
	class MQTTClient : public cs::MQTTWrapper
	{
	public:
		MQTTClient();
		MQTTClient(struct mosquitto* mosq);

		void send_detection(const char* camera_id, const char* topic, std::list<DetectionItem*> detections);
		void send_command_response(int command_id, const char* device_id, const char* topic, uint64_t req_id, int error_code, const char* error_string);
	private:
		void _send(const char* topic, rapidjson::Document& root);
	};
}

