#pragma once

#include <string>
#include <map>
#include <variant>
#include <list>
#include "JsonWrapper.h"
#include "types.h"

namespace cs
{
	class dynamic_settings : protected cs::JsonWrapper
	{
	public:
		template<typename T>
		T get(const std::string& key, T default_value = T()) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<T>(it->second)) {
					return std::get<T>(it->second);
				}
			}

			return default_value;
		}

		double get_double(const std::string& key, double default_value = 0.0f) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<double>(it->second)) {
					return std::get<double>(it->second);
				}
			}

			return default_value;
		}

		float get_float(const std::string& key, float default_value = 0.0f) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<float>(it->second)) {
					return std::get<float>(it->second);
				}
			}

			return default_value;
		}

		int get_int(const std::string& key, int default_value = 0) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<int>(it->second)) {
					return std::get<int>(it->second);
				}
			}

			return default_value;
		}

		std::string get_string(const std::string& key, const std::string& default_value = "") const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<std::string>(it->second)) {
					return std::get<std::string>(it->second);
				}
			}

			return default_value;
		}

		bool get_bool(const std::string& key, bool default_value = false) const
		{
			auto it = settings.find(key);
			if (it != settings.end()) {
				if (std::holds_alternative<bool>(it->second)) {
					return std::get<bool>(it->second);
				}
			}

			return default_value;
		}

		int parse(rapidjson::Value& root);
	private:
		std::map<std::string, std::variant<int, std::string, float, double, bool>> settings;
	};
}
