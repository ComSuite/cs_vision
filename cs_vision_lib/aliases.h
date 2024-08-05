#pragma once

#include "JsonWrapper.h"
#include <map>
#include <string>

namespace cs
{
	class aliases : public cs::JsonWrapper
	{
	public:
		std::string get_alias(const std::string& camera_id, const std::string& topic, const std::string& name, const std::string& def_val);
	protected:
		std::map<std::string, std::map<std::tuple<std::string, std::string>, std::string>> data;

		int parse(rapidjson::Document& root) override;
	};
}

