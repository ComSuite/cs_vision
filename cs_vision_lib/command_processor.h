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
#include <variant>
#include <list>
#include "JsonWrapper.h"
#include "JsonWriter.h"
#include "ReadOnlyValues.h"

namespace cs
{
	class error_codes 
	{
	public:
		static const int __ERROR_NO_ERROR = 0;
		static const int __ERROR_INTERNAL_ERROR = 1;
		static const int __ERROR_NOT_ENOUGH_MEMORY = 2;
		static const int __ERROR_INVALID_SETTINGS_VERSION = 3;
		static const int __ERROR_INVALID_COMMAND_VERSION = 4;
		static const int __ERROR_BAD_COMMAND_ID = 5;
		static const int __ERROR_CAN_NOT_RESTORE_FROM_BACKUP = 6;
	};

	class item_value 
	{
	public:
		std::string path = "";
		json_value value;
	};

	class command_processor : public cs::JsonWrapper
	{
	public:
		static const int command_version_high = 1;
		static const int command_version_low = 4;

		static const int COMMAND_ID_NO = 0;
		static const int COMMAND_ID_RESTART = 1; //restart service
		static const int COMMAND_ID_REBOOT = 2; //reboot device
		static const int COMMAND_ID_SET_SETTINGS = 3; //set parameters for some cameras
		static const int COMMAND_ID_GET_SETTINGS = 4; //return all configuration 
		static const int COMMAND_ID_SET_CONFIGURATION = 5; //set all configuration
		static const int COMMAND_ID_REMOVE = 6; //remove config for one or more cameras
		static const int COMMAND_ID_GET_BACKUPS_LIST = 7; //return list of saved bacup files
		static const int COMMAND_ID_RESTORE_FROM_BACKUP = 8; //restore config for buckup. if no params - from last backup
		static const int COMMAND_ID_PING = 9; //ping
		static const int COMMAND_ID_SET_LLM_PROMPT = 10; //set LLM prompt for VLLM & LLM detectors
		static const int COMMAND_ID_MQTT_REQUEST = 11; //set LLM prompt for VLLM & LLM detectors

		virtual ~command_processor();
		bool execute(cs::JsonWriter* root, cs::ReadOnlyValues* checker = nullptr, const char* settings_file_path = nullptr, bool is_backup = false);
		bool hide_secrets(cs::JsonWriter* root, std::list<std::string>& secrets, std::string& json);

		item_value* get_item_value(const char* path);

		int command_id = COMMAND_ID_NO;
		std::string device_id = "";
		uint64_t req_id = 0;
		uint64_t timestamp = 0;
	protected:
		int parse(rapidjson::Document& root) override;
	private:
		std::list<item_value*> values;
	};
}

