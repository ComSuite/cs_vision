#include "aliases.h"

using namespace std;
using namespace cs;

const std::string aliases::get_alias(const std::string& camera_id, const std::string& topic, const std::string& name, const std::string& def_val)
{
	if (data.find(std::tuple(topic, name)) != data.end()) {
		return data.find(std::tuple(topic, name))->second;
	}

	return def_val;
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
					if (this->id.compare(json_get_string(_alias, "camera_id", "")) == 0) {
						string topic = json_get_string(_alias, "topic", "");
						string name = json_get_string(_alias, "name", "");
						string alias = json_get_string(_alias, "alias", "");

						data[std::tuple(topic, name)] = alias;
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

