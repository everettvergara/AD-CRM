#pragma once

#include <string>
#include "Common/Config.hpp"

namespace eg::ad3
{
	constexpr auto k_settings_filename = "config/settings.json";

	struct ConfigSettings final :
		public sys::Config<ConfigSettings>
	{
		std::string server_ip;
		std::string server_port;
		std::string sip_id;
		std::string sip_password;
		size_t concurrent_calls;

		size_t max_registration_attempts;
		unsigned int server_timeout_secs;
		unsigned int server_keep_alive_secs;
		std::string server_keep_alive_data;
		bool register_on_add;
		bool mock;

		std::string db_driver;
		std::string db_server;
		std::string db_database;
		std::string db_user;
		std::string db_password;
		std::string fycrm_path;

		nlohmann::json to_json() const override
		{
			return nlohmann::json
			{
				{"server_ip", server_ip},
				{"server_port", server_port},
				{"sip_id", sip_id},
				{"sip_password", sip_password},
				{"concurrent_calls", concurrent_calls},
				{"max_registration_attempts", max_registration_attempts},
				{"server_timeout_secs", server_timeout_secs},
				{"server_keep_alive_secs", server_keep_alive_secs},
				{"server_keep_alive_data", server_keep_alive_data},
				{"register_on_add", register_on_add},
				{"mock", mock},
				{"db_driver", db_driver},
				{"db_server", db_server},
				{"db_database", db_database},
				{"db_user", db_user},
				{"db_password", db_password},
				{"fycrm_path", fycrm_path}
			};
		}

		ConfigSettings(const nlohmann::json& data) :
			server_ip(data.value("server_ip", "127.0.0.1")),
			server_port(data.value("server_port", "5060")),
			sip_id(data.value("sip_id", "1234")),
			sip_password(data.value("sip_password", "0000")),
			concurrent_calls(data.value("concurrent_calls", 1ull)),
			max_registration_attempts(data.value("max_registration_attemps", 3ull)),
			server_timeout_secs(data.value("server_timeout_secs", 10u)),
			server_keep_alive_secs(data.value("server_timeout_secs", 10u)),
			server_keep_alive_data(data.value("server_keep_alive_data", R"(\r\n)")),
			register_on_add(data.value("register_on_add", false)),
			mock(data.value("mock", true)),
			db_driver(data.value("db_driver", "{SQL Server}")),
			db_server(data.value("db_server", "WIN-0BL4BGRJARA")),
			db_database(data.value("db_database", "wmc")),
			db_user(data.value("db_user", "sa")),
			db_password(data.value("db_password", "Kerberos2014!")),
			fycrm_path(data.value("fycrm_path", "c:/fy-crm/fydesigns.exe"))
		{
		}
	};
}