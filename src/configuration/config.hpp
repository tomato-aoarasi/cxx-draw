/*
 * @File	  : config.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/05 21:14
 * @Introduce : 配置类(解析yaml)
*/

#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "yaml-cpp/yaml.h"
#include "fmt/format.h"
#include <limits>
#include <snowflake.hpp>
#include <sqlite_modern_cpp.h>
#include <Poco/Net/MediaType.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/NetException.h>
#include "ImageMagick-6/Magick++.h"
#include "configuration/define.hpp"

#ifndef CONFIG_HPP
#define CONFIG_HPP  

constexpr const int64_t TWEPOCH{ -1L };
using namespace std::string_literals;
using namespace std::chrono_literals;
using snowflake_t = snowflake<TWEPOCH, std::mutex>;

namespace std {
	using fmt::format;
	using fmt::format_error;
	using fmt::formatter;
}

namespace Config {
	inline YAML::Node config_yaml{ YAML::LoadFile("./config.yaml") };
};

namespace Global {
	inline std::string resource_path{ Config::config_yaml["server"]["resource-path"].as<std::string>()};
	inline std::string authorization_code{ "" };
	inline bool is_auth{ false };
	inline self::define::UuidInfo uuid_info;
	inline snowflake_t snowflake_uuid;
	inline self::define::ExecutableFileInfo execute_info;
	inline std::filesystem::path svg_temp_path { Config::config_yaml["temporary"]["svg-path"].as<std::string>()};
	inline std::filesystem::path lua_directory{ Config::config_yaml["lua"]["directory"].as<std::string>()};
};

namespace self::DB {
	inline sqlite::database LocalDB{ sqlite::database(Config::config_yaml["db"]["sqlite"]["local"].as<std::string>()) };
};

namespace Config {
	inline const int initialization_status{ []() {
		// UUID生成器配置
		{
			Global::uuid_info.uuid_namespace = Config::config_yaml["server"]["generator"]["uuid"]["namespace"].as<std::string>();
			Global::uuid_info.snowflake_twepoch = Config::config_yaml["server"]["generator"]["uuid"]["twepoch"].as<int64_t>();
			Global::uuid_info.snowflake_workerid = Config::config_yaml["server"]["generator"]["uuid"]["workerid"].as<int64_t>();
			Global::uuid_info.snowflake_datacenterid = Config::config_yaml["server"]["generator"]["uuid"]["datacenterid"].as<int64_t>();
		};

		// snowflake
		{
			snowflake_t::twepoch = Global::uuid_info.snowflake_twepoch;
			Global::snowflake_uuid.init(Global::uuid_info.snowflake_workerid, Global::uuid_info.snowflake_datacenterid);
		}
		// execute
		{
			char result[PATH_MAX];
			ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
			std::string path = std::string(result, (count > 0) ? count : 0);
			size_t found = path.find_last_of("/\\");
			std::string exe_dir = path.substr(0, found);
			std::string exe_name = path.substr(found + 1);

			Global::execute_info.path = path;
			Global::execute_info.directory = exe_dir;
			Global::execute_info.executable_file_name = exe_name;
		}

		// ImageMagick init
		{
			Magick::InitializeMagick(Global::execute_info.path.data());
		}

		// svg临时文件夹的创建
		{
			if (!std::filesystem::exists(Global::svg_temp_path)) {  // 如果目录不存在，则创建
				std::filesystem::create_directory(Global::svg_temp_path);
			}
		}
		// lua脚本集目录创建
		{
			if (!std::filesystem::exists(Global::lua_directory)) {  // 如果目录不存在，则创建
				std::filesystem::create_directory(Global::lua_directory);
			}
		}

		// auth
		{
			auto auth{ Config::config_yaml["server"]["authorization-code"] };
			if (auth) {
				Global::authorization_code = auth.as<std::string>();
				Global::is_auth = true;
			}
		}
		return 0;
	}() };
}

#endif