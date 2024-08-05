#include "aliases.h"

using namespace std;
using namespace cs;

std::string aliases::get_alias(const std::string& camera_id, const std::string& topic, const std::string& name, const std::string& def_val)
{
	if (data.find(camera_id) != data.end()) {
		if (data.find(camera_id)->second.find(std::tuple(topic, name)) != data.find(camera_id)->second.end())
			return data.find(camera_id)->second.find(std::tuple(topic, name))->second;
	}

	return "";
}

int aliases::parse(rapidjson::Document& root)
{
	try {
		if (root.IsNull())
			return 0;

		if (root.HasMember("aliases")) {
			if (root["aliases"].IsArray()) {
				auto _aliases = root["aliases"].GetArray();
				for (auto& _alias : _aliases) {
					string camera_id = json_get_string(_alias, "camera_id", "");
					string topic = json_get_string(_alias, "topic", "");
					string name = json_get_string(_alias, "name", "");
					string alias = json_get_string(_alias, "alias", "");

					if (camera_id.length() > 0) {
						data[camera_id][std::tuple(topic, name)] = alias;
					}
				}
			}
		}


		return 1;
	}
	catch (...) {
		return 0;
	}
}

