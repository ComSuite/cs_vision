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

#include "JsonWriter.h"
#include <iostream>

using namespace cs;
using namespace rapidjson;
using namespace std;

void json_path::parse(const char* path)
{
	this->path.clear();
	string str = path;
	size_t pos = 0;
	size_t prev = 0;
	while ((pos = str.find("/", prev)) != string::npos) {
		if (pos > prev) {
			this->path.push_back(str.substr(prev, pos - prev));
		}
		prev = pos + 1;
	}

	if (prev < str.length()) {
		this->path.push_back(str.substr(prev, string::npos));
	}
}

//******************************************************
int JsonWriter::set_json_value(const char* path, json_value value)
{
	json_path jpath;
	jpath.parse(path);

	return set_json_value(jpath, value);
}

int JsonWriter::get_json_element_index(string& name)
{
	size_t i = name.find_last_of('[');
	size_t j = name.find_last_of(']');
	if (i == string::npos || j == string::npos || i >= j)
		return -1;	

	string ind = name.substr(i + 1, j - i - 1);
	int index = -1;
	try {
		index = std::stoi(ind);
	}
	catch (exception e) {
		return -1;
	}

	name.erase(i, j - i + 1);

	return index;
}

rapidjson::Document* JsonWriter::find_json_object(rapidjson::Document* root, json_path& path)
{
	if (path.path.size() == 0 || root == nullptr) {
		return root;
	}

	string name = path.path.front();
	Document* val = root;

	auto index = get_json_element_index(name);
	path.path.pop_front();

	if (val->HasMember(name.c_str())) {
		auto obj = val->GetObject();
		val = (Document*)&(obj[name.c_str()]);

		if (val->IsArray() && index >= 0) {
			auto items = val->GetArray();
			val = (Document*)&items[index];
		}

		if (path.path.size() == 1) {
			return val;
		}

		return find_json_object(val, path);
	}

	return nullptr;
}

int JsonWriter::set_json_value(json_path& path, json_value value)
{
	if (path.path.size() == 0) {
		return -1;
	}

	Document* item = find_json_object(&root, path);
	if (item != nullptr && path.path.size() == 1) {
		rapidjson::Value::MemberIterator val = item->FindMember(path.path.front().c_str());
		if (val == item->MemberEnd())
			return 0;

		if (std::holds_alternative<int>(value)) {
			val->value.SetInt(std::get<int>(value));
		}
		else if (std::holds_alternative<std::string>(value)) {
			val->value.SetString(std::get<std::string>(value).c_str(), root.GetAllocator());
		}
		else if (std::holds_alternative<float>(value)) {
			val->value.SetFloat(std::get<float>(value));
		}
		else if (std::holds_alternative<double>(value)) {
			val->value.SetDouble(std::get<double>(value));
		}
		else if (std::holds_alternative<bool>(value)) {
			val->value.SetBool(std::get<bool>(value));
		}
	}

	return 0;
}