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

#include "JsonWrapper.h"
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <sstream>
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/istreamwrapper.h"
#include "../rapidjson/ostreamwrapper.h"
#include "std_utils.h"

using namespace std;
using namespace rapidjson;
using namespace cs;

bool JsonWrapper::is_file_exists(const char* filename)
{
	return ::is_file_exists(filename);
}

int JsonWrapper::load(const char* filename, string& json)
{
	if (filename == NULL)
		return 0;

	if (!is_file_exists(filename))
		return 0;

	file_path = filename;

	int ret = 1;

	try {
		ifstream ifs(filename);
		stringstream buffer;
		buffer << ifs.rdbuf();
		json = buffer.str();
	}
	catch (...) {
		ret = 0;
	}

	return ret;
}

int JsonWrapper::load(const char* filename)
{
	string json = "";
	int ret = load(filename, json);
	if (ret){
		ret = parse(json.c_str());
		if (ret) {
			cout << "Data successfully loaded from file: " << filename << endl;
		}
	}

	return ret;
}

int JsonWrapper::load()
{
	return load(file_path.c_str());
}

int JsonWrapper::parse(const char* json)
{
	int ret = 1;
	Document root;

	try {
		if (!root.Parse(json).HasParseError()) {
			ret = parse(root);
		}
		else {
			cout << "Error JSON file. Check up the JSON format." << endl;
			ret = 0;
		}
	}
	catch (...) {
		ret = 0;
	}

	return ret;
}

bool JsonWrapper::get_json_array(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, Value& val)
{

//	if (get_json_document(doc, topic, val)) {
//		if (val.IsArray())
//			return true;
//	}

	return false;
}

bool JsonWrapper::get_json_document(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, GenericObject<false, rapidjson::Value>& val)
{
//	if (doc.HasMember(topic)) {
//		if (doc[topic].IsObject()) {
//			val = doc[topic].GetObject();
//			return true;
//		}
//	}

	return false;
}

int JsonWrapper::json_get_int(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, int defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsInt())
			return doc[topic].GetInt();
	}

	return defval;
}

uint64_t JsonWrapper::json_get_uint64(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, int defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsUint64())
			return doc[topic].GetUint64();
	}

	return defval;
}

bool JsonWrapper::json_get_bool(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, bool defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsBool())
			return doc[topic].GetBool();
	}

	return defval;
}

std::variant<int, std::string, float, double, bool> JsonWrapper::json_get_variant(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, std::variant<int, std::string, float, double, bool>& defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsInt())
			defval = doc[topic].GetInt();
		else if (doc[topic].IsString())
			defval = std::string(doc[topic].GetString());
		else if (doc[topic].IsFloat())
			defval = doc[topic].GetFloat();
		else if (doc[topic].IsDouble())
			defval = doc[topic].GetDouble();
		else if (doc[topic].IsBool())
			defval = doc[topic].GetBool();
	}

	return defval;
}

bool JsonWrapper::json_get_array(rapidjson::Value& doc, const char* topic, rapidjson::Value& val)
{
	if (doc.HasMember(topic) && doc[topic].IsArray()) {
		val = doc[topic].GetArray();
		return true;
	}

	return false;
}

double JsonWrapper::json_get_double(rapidjson::Value& doc, const char* topic, double defval)
{
	auto val = doc.GetObject();

	return json_get_double(val, topic, defval);
}

int JsonWrapper::json_get_int(rapidjson::Value& doc, const char* topic, int defval)
{
	auto val = doc.GetObject();

	return json_get_int(val, topic, defval);
}

uint64_t JsonWrapper::json_get_uint64(rapidjson::Value& doc, const char* topic, int defval)
{
	auto val = doc.GetObject();

	return json_get_uint64(val, topic, defval);
}

bool JsonWrapper::json_get_bool(rapidjson::Value& doc, const char* topic, bool defval)
{
	auto val = doc.GetObject();

	return json_get_bool(val, topic, defval);
}

const char* JsonWrapper::json_get_string(rapidjson::Value& doc, const char* topic, const char* defval)
{
	auto val = doc.GetObject();

	return json_get_string(val, topic, defval);
}

double JsonWrapper::json_get_double(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, double defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsDouble() || doc[topic].IsFloat() || doc[topic].IsInt())
			return doc[topic].GetDouble();
	}

	return defval;
}

const char* JsonWrapper::json_get_string(rapidjson::GenericObject<false, rapidjson::Value>& doc, const char* topic, const char* defval)
{
	if (doc.HasMember(topic)) {
		if (doc[topic].IsString())
			return doc[topic].GetString();
	}

	return defval;
}

std::variant<int, std::string, float, double, bool> JsonWrapper::json_get_variant(rapidjson::Value& doc, const char* topic, std::variant<int, std::string, float, double, bool>& defval)
{
	auto val = doc.GetObject();

	if (val.HasMember(topic)) {
		if (val[topic].IsInt())
			defval = val[topic].GetInt();
		else if (val[topic].IsString())
			defval = val[topic].GetString();
		else if (val[topic].IsFloat())
			defval = val[topic].GetFloat();
		else if (val[topic].IsDouble())
			defval = val[topic].GetDouble();
		else if (val[topic].IsBool())
			defval = val[topic].GetBool();
	}

	return defval;
}
