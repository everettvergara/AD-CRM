#pragma once

#include <string>
#include <vector>
#include "Common/Config.hpp"

namespace eg::ad3
{
	constexpr auto k_settings_filename = "config/settings.json";

	struct sip_account
	{
		std::string sip_id;
		std::string sip_password;
	};

	struct ConfigSettings final :
		public sys::Config<ConfigSettings>
	{
		std::string server_ip;
		std::string server_port;
		std::vector<sip_account> sip_accounts;
		size_t concurrent_calls;
		size_t max_calls;

		size_t max_registration_attempts;
		unsigned int server_timeout_secs;
		unsigned int server_keep_alive_secs;
		std::string server_keep_alive_data;
		bool register_on_add;
		bool mock;
		bool is_admin;
		bool number_masking;
		std::string db_driver;
		std::string db_server;
		std::string db_database;
		std::string db_user;
		std::string db_password;
		std::string fycrm_path;
		std::string playback_central_path;

		std::string ftp_server;
		std::string ftp_port;
		std::string ftp_user;
		std::string ftp_password;
		std::string ftp_root_folder;

		size_t redial = 0;

		std::string sip_accounts_str() const
		{
			auto sip_accounts = nlohmann::json::array();
			for (const auto& account : this->sip_accounts)
			{
				sip_accounts.push_back(
					{
						{"sip_id", account.sip_id},
						{"sip_password", account.sip_password}
					});
			}

			return sip_accounts.dump();
		}

		size_t max_ep() const
		{
			if (concurrent_calls == 0)
			{
				return 1;
			}
			//PJSUA_MAX_CALLS

			return  max_calls / concurrent_calls;
		}

		nlohmann::json to_json() const override
		{
			auto sip_accounts = nlohmann::json::array();
			for (const auto& account : this->sip_accounts)
			{
				sip_accounts.push_back(
					{
						{"sip_id", account.sip_id},
						{"sip_password", account.sip_password}
					});
			}

			return nlohmann::json
			{
				{"server_ip", server_ip},
				{"server_port", server_port},
				{"is_admin", is_admin},
				{"number_masking", number_masking},
				{"sip_accounts", sip_accounts},
				{"concurrent_calls", concurrent_calls},
				{"max_calls", max_calls},
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
				{"fycrm_path", fycrm_path},
				{"redial", redial},
				{"playback_central_path", playback_central_path},
				{"ftp_server", ftp_server},
				{"ftp_port", ftp_port},
				{"ftp_user", ftp_user},
				{"ftp_password", ftp_password},
				{"ftp_root_folder", ftp_root_folder}
			};
		}

		ConfigSettings(const nlohmann::json& data) :
			server_ip(data.value("server_ip", "127.0.0.1")),
			server_port(data.value("server_port", "5060")),
			sip_accounts([&data]()
				{
					std::vector<sip_account> accounts;
					if (data.contains("sip_accounts") and data["sip_accounts"].is_array())
					{
						for (const auto& item : data["sip_accounts"])
						{
							accounts.push_back(
								{
									item.value("sip_id", "1234"),
									item.value("sip_password", "0000")
								});
						}
					}
					else
					{
						accounts.push_back(
							{
								data.value("sip_id", "1234"),
								data.value("sip_password", "0000")
							});
					}
					return accounts;
				}()),
			concurrent_calls(data.value("concurrent_calls", 2ull)),
			max_calls(data.value("max_calls", 4ull)),
			max_registration_attempts(data.value("max_registration_attemps", 3ull)),
			server_timeout_secs(data.value("server_timeout_secs", 10u)),
			server_keep_alive_secs(data.value("server_timeout_secs", 10u)),
			server_keep_alive_data(data.value("server_keep_alive_data", R"(\r\n)")),
			register_on_add(data.value("register_on_add", false)),
			mock(data.value("mock", true)),
			is_admin(data.value("is_admin", false)),
			number_masking(data.value("number_masking", false)),
			db_driver(data.value("db_driver", "{SQL Server Native Client 11.0}")),
			db_server(data.value("db_server", "WIN-0BL4BGRJARA")),
			db_database(data.value("db_database", "wmc")),
			db_user(data.value("db_user", "sa")),
			db_password(data.value("db_password", "Kerberos2014!")),
			fycrm_path(data.value("fycrm_path", "c:/fy-crm/crm_dialer.exe")),
			redial(data.value("redial", 1)),
			playback_central_path(data.value("playback_central_path", "")),
			ftp_server(data.value("ftp_server", "")),
			ftp_port(data.value("ftp_port", "21")),
			ftp_user(data.value("ftp_user", "")),
			ftp_password(data.value("ftp_password", "")),
			ftp_root_folder(data.value("ftp_root_folder", "ad3/Answered"))
		{
		}
	};
}