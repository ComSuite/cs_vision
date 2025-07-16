/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief       JSON wrapper
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

#include <cstddef>
#include <string>
#include <variant>
#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"

namespace cs
{
	class JsonWrapper
	{
	public:
		virtual ~JsonWrapper() {};

		bool is_file_exists(const char* filename);
		int load(const char* filename, std::string& json);
		int load(const char* filename);
		int load();

		int parse(const char* json);
		const char* get_file_path() { return file_path.c_str(); };

		bool get_json_array(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, rapidjson::Value& val);
		bool get_json_document(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, rapidjson::GenericObject<false, rapidjson::Value>& val);
		int json_get_int(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, int defval);
		uint64_t json_get_uint64(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, int defval);
		bool json_get_bool(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, bool defval);
		double json_get_double(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, double defval);
		const char* json_get_string(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, const char* defval);
		std::variant<int, std::string, float, double, bool> json_get_variant(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, std::variant<int, std::string, float, double, bool>& defval);

		bool json_get_array(rapidjson::Value& doc, const char* topic, rapidjson::Value& val);
		int json_get_int(rapidjson::Value& doc, const char* topic, int defval);
		uint64_t json_get_uint64(rapidjson::Value& doc, const char* topic, int defval);
		bool json_get_bool(rapidjson::Value& doc, const char* topic, bool defval);
		double json_get_double(rapidjson::Value& doc, const char* topic, double defval);
		const char* json_get_string(rapidjson::Value& doc, const char* topic, const char* defval);
		std::variant<int, std::string, float, double, bool> json_get_variant(rapidjson::Value& doc, const char* topic, std::variant<int, std::string, float, double, bool>& defval);
	protected:
		std::string file_path = "";

		virtual int parse(rapidjson::Document& root) { return 0; };
	};
}
