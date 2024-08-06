#pragma once

#include "JsonWrapper.h"
#include <map>
#include <string>

namespace cs
{
	class aliases : public cs::JsonWrapper
	{
	public:
		void set_id(const char* id) 
		{
			this->id = id;
		}
		const std::string get_alias(const std::string& camera_id, const std::string& topic, const std::string& name, const std::string& def_val);
	protected:
		int parse(rapidjson::Document& root) override;
	private:
		std::string id = "";
		std::map<std::tuple<std::string, std::string>, std::string> data;
	};
}

