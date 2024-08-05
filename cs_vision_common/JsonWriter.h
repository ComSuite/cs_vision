/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2024, Alexander Epstine (a@epstine.com)
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

#include <vector>
#include <string>
#include <variant>
#include <list>
#include <fstream>
#include <iostream>
#ifdef __LINUX__
#include <syslog.h>
#endif
#include "JsonWrapper.h"
#include "../rapidjson/rapidjson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/istreamwrapper.h"
#include "../rapidjson/ostreamwrapper.h"
#include "../rapidjson/pointer.h"

namespace cs
{
	class json_path
	{
	public:
		std::list<std::string> path;

		void parse(const char* path);
	};

	typedef std::variant<int, std::string, float, double, bool> json_value;

	class JsonWriter : public JsonWrapper
	{
	public:
		virtual int set_json_value(json_path& path, json_value value);
		virtual int set_json_value(const char* path, json_value value);

		rapidjson::Document* get_root() { return &root; };
		std::string get_string()
		{
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			root.Accept(writer);
			return buffer.GetString();
		};
	protected:
		virtual int parse(rapidjson::Document& root) 
		{ 
			this->root.CopyFrom(root, this->root.GetAllocator());
			return 1; 
		};

		int get_json_element_index(std::string& name);
		//rapidjson::GenericObject<false, rapidjson::Value>* find_json_object(rapidjson::Document& root, json_path& path);
		rapidjson::Document* find_json_object(rapidjson::Document* root, json_path& path);
	private:
		rapidjson::Document root;
	};
}


