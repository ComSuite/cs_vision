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

#include "command_processor.h"
#include <iostream>
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/istreamwrapper.h"
#include "../rapidjson/ostreamwrapper.h"
#include <fstream>
#include <iostream>
#ifdef __LINUX__
#include <syslog.h>
#endif
#include <std_utils.h>
#include "FileBackup.h"

using namespace std;
using namespace cs;
using namespace rapidjson;

command_processor::~command_processor() 
{
	clear<item_value, std::list>(values);
}

int command_processor::parse(rapidjson::Document& root)
{
	int ret = 1;

	try {
		if (root.IsNull())
			return 0;

		if (json_get_int(root, "command_version_high", 0) != command_version_high || json_get_int(root, "command_version_low", 0) != command_version_low) {
			std::cout << "Invalid command format version. Current is: " << command_version_high << "." << command_version_low << std::endl;
			return 0;
		}

		req_id = json_get_int(root, "req_id", req_id);
		timestamp = json_get_int(root, "timestamp", timestamp);

		if (!root.HasMember("command"))
			return 0;

		if (!root["command"].IsObject())
			return 0;

		auto command = root["command"].GetObject();

		command_id = json_get_int(command, "command_id", command_id);
		device_id = json_get_string(command, "device_id", device_id.c_str());

		if (root.HasMember("payload") && root["payload"].IsObject()) {
			auto payload = root["payload"].GetObject();
			if (payload.HasMember("values") && payload["values"].IsArray()) {
				auto values = payload["values"].GetArray();
				for (auto& v : values) {
					if (v.IsObject()) {
						auto obj = v.GetObject();
						item_value* item = new item_value();
						if (item != nullptr) {
							item->path = json_get_string(obj, "path", item->path.c_str());
							item->value = json_get_variant(obj, "value", item->value);
							this->values.push_back(item);
						}
					}
				}
			}
		}
	}
	catch (...) {
		ret = 0;
	}

	return ret;
}

bool command_processor::execute(cs::JsonWriter* root, ReadOnlyValues* checker, const char* settings_file_path, bool is_backup)
{
	if (root == nullptr)
		return false;

	for (auto& v : values) {
		if (checker == nullptr || checker->check(v->path))
			root->set_json_value(v->path.c_str(), v->value);
	}

	string str = root->get_string();
	if (settings_file_path != nullptr) {
		if (is_backup) {
			FileBackup::create_backup(settings_file_path, FileBackup::default_backup_extension.c_str());
		}

		ofstream ofs(settings_file_path);
		ofs << str;
		ofs.close();
	}

	return true;
}

bool command_processor::hide_secrets(cs::JsonWriter* root, std::list<std::string>& secrets, std::string& json)
{
	if (root == nullptr)
		return false;

	for (auto& secret : secrets) {
		json_value val = std::string("************");
		root->set_json_value(secret.c_str(), val);
	}

	json = root->get_string();

	return true;
}