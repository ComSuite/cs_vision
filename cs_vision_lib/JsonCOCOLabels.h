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

#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <variant>
#include <string>
#include <chrono>
#include "JsonWrapper.h"

class JsonCOCOLabels : public cs::JsonWrapper
{
public:
	int parse(const char* json) { return JsonWrapper::parse(json); };

	void get(std::list<std::string>& labels)
	{
		labels = this->labels;
	}

	void get(std::vector<std::string>& labels)
	{
		for (auto& l : this->labels) {
			labels.push_back(l);
		}
	}
protected:
	virtual int parse(rapidjson::Document& root) override
	{
		int ret = 1;

		try {
			if (root.IsNull())
				return 0;

			if (!root.IsArray())
				return 0;

			auto names = root.GetArray();
			for (auto& name : names) {
				std::string lbl = json_get_string(name, "name", "");
				if (lbl.size() > 0) {
					labels.push_back(lbl);
				}
			}
		}
		catch (...) {
			ret = 0;
		}

		return ret;
	}
private:
	std::list<std::string> labels;
};

