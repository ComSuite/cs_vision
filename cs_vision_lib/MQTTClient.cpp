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

#include "MQTTClient.h"

#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "command_processor.h"

using namespace std;
using namespace rapidjson;
using namespace cs;

MQTTClient::MQTTClient()
{
}

MQTTClient::MQTTClient(struct mosquitto* mosq) : MQTTWrapper(mosq)
{
}

void MQTTClient::prepare_root(Document& root)
{

}

void MQTTClient::send_detection(const char* camera_id, const char* topic, std::list<DetectionItem*> detections, aliases* field_aliases)
{
	if (field_aliases == nullptr)
		return send_detection(camera_id, topic, detections);

	Document root;
	Value objects(kArrayType);

	root.SetObject();
	Document::AllocatorType& allocator = root.GetAllocator();

	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	root.AddMember("ts", ms.count(), root.GetAllocator());
	root.AddMember("cam_id", Value().SetString(camera_id, strlen(camera_id)), allocator);

	for (auto& it : detections) {
		if (it->is_send_result) {
			Document object;
			object.SetObject();
			
			std::string name = field_aliases->get_alias(camera_id, topic, "id", "id");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), (int)it->id, allocator);
			name = field_aliases->get_alias(camera_id, topic, "kind", "kind");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), (int)it->kind, allocator);
			name = field_aliases->get_alias(camera_id, topic, "cls", "cls");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->class_id, allocator);
			name = field_aliases->get_alias(camera_id, topic, "nn_id", "nn_id");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->neural_network_id, allocator);
			name = field_aliases->get_alias(camera_id, topic, "label", "label");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), Value().SetString(it->label.c_str(), it->label.length()), allocator);
			name = field_aliases->get_alias(camera_id, topic, "conf", "conf");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->score, allocator);
			name = field_aliases->get_alias(camera_id, topic, "predecessor_object_id", "predecessor_object_id");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->predecessor_id, allocator);
			name = field_aliases->get_alias(camera_id, topic, "predecessor_detector_id", "predecessor_detector_id");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->predecessor_detector_id, allocator);
			name = field_aliases->get_alias(camera_id, topic, "predecessor_class", "predecessor_class");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), it->predecessor_class, allocator);

			float x0 = it->box.x + it->original_x;
			float y0 = it->box.y + it->original_y;
			float x1 = x0 + it->box.width;
			float y1 = y0 + it->box.height;

			if (it->mapping_rule == DetectionItem::RESULTS_MAPPING_RULE_NORM && it->frame_w != 0 && it->frame_h != 0) {
				x0 = x0 / it->frame_w;
				y0 = y0 / it->frame_h;
				x1 = x1 / it->frame_w;
				y1 = y1 / it->frame_h;
			}

			name = field_aliases->get_alias(camera_id, topic, "x0", "x0");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), x0, allocator);
			name = field_aliases->get_alias(camera_id, topic, "y0", "y0");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), y0, allocator);
			name = field_aliases->get_alias(camera_id, topic, "x1", "x1");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), x1, allocator);
			name = field_aliases->get_alias(camera_id, topic, "y1", "y1");
			object.AddMember(Value().SetString(name.c_str(), name.length(), allocator), y1, allocator);

			objects.PushBack(object, allocator);
		}
	}

	root.AddMember("detections", objects, root.GetAllocator());

	_send(topic, root);
}

void MQTTClient::send_detection(const char* camera_id, const char* topic, list<DetectionItem*> detections)
{
	if (topic == NULL || strlen(topic) == 0)
		return;

	Document root;
	Value objects(kArrayType);

	root.SetObject();
	Document::AllocatorType& allocator = root.GetAllocator();

	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	root.AddMember("ts", ms.count(), root.GetAllocator());
	root.AddMember("cam_id", Value().SetString(camera_id, strlen(camera_id)), allocator);

	for (auto& it : detections) {
		if (it->is_send_result) {
			Document object;
			object.SetObject();

			object.AddMember("id", (int)it->id, allocator);
			object.AddMember("kind", (int)it->kind, allocator);
			object.AddMember("cls", it->class_id, allocator);
			object.AddMember("nn_id", it->neural_network_id, allocator);
			object.AddMember("label", Value().SetString(it->label.c_str(), it->label.length()), allocator);
			object.AddMember("conf", it->score, allocator);
			object.AddMember("predecessor_object_id", it->predecessor_id, allocator);
			object.AddMember("predecessor_detector_id", it->predecessor_detector_id, allocator);
			object.AddMember("predecessor_class", it->predecessor_class, allocator);

			float x0 = it->box.x + it->original_x;
			float y0 = it->box.y + it->original_y;
			float x1 = x0 + it->box.width;
			float y1 = y0 + it->box.height;

			if (it->mapping_rule == DetectionItem::RESULTS_MAPPING_RULE_NORM && it->frame_w != 0 && it->frame_h != 0) {
				x0 = x0 / it->frame_w;
				y0 = y0 / it->frame_h;
				x1 = x1 / it->frame_w;
				y1 = y1 / it->frame_h;
			}

			object.AddMember("x0", x0, allocator);
			object.AddMember("y0", y0, allocator);
			object.AddMember("x1", x1, allocator);
			object.AddMember("y1", y1, allocator);

			objects.PushBack(object, allocator);
		}
	}

	root.AddMember("detections", objects, root.GetAllocator());

	_send(topic, root);
}

void MQTTClient::send_command_response(int command_id, const char* device_id, const char* topic, uint64_t req_id, int error_code, const char* error_string)
{
	if (topic == NULL || strlen(topic) == 0)
		return;

	Document root;
	root.SetObject();
	Document::AllocatorType& allocator = root.GetAllocator();

	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	root.AddMember("ts", ms.count(), root.GetAllocator());
	root.AddMember("req_id", req_id, root.GetAllocator());
	root.AddMember("command_version_high", command_processor::command_version_high, root.GetAllocator());
	root.AddMember("command_version_low", command_processor::command_version_low, root.GetAllocator());

	Document response;
	response.SetObject();

	response.AddMember("command_id", command_id, allocator);
	response.AddMember("device_id", Value().SetString(device_id, strlen(device_id)), allocator);
	response.AddMember("error_code", error_code, allocator);
	response.AddMember("error_string", Value().SetString(error_string, strlen(error_string)), allocator);

	root.AddMember("response", response, root.GetAllocator());

	_send(topic, root);
}

void MQTTClient::_send(const char* topic, rapidjson::Document& root)
{
	StringBuffer buffer;
	buffer.Clear();

	Writer<StringBuffer> writer(buffer);
	root.Accept(writer);

	const char* payload = buffer.GetString();
	if (payload == NULL || strlen(payload) == 0)
		return;

	MQTTWrapper::send(topic, payload);
}