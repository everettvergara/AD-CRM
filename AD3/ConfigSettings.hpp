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
		size_t concurrent_calls = 1;

		nlohmann::json to_json() const override
		{
			return nlohmann::json
			{
				{"server_ip", server_ip},
				{"server_port", server_port},
				{"sip_id", sip_id},
				{"sip_password", sip_password},
				{"concurrent_calls", concurrent_calls}
			};
		}

		ConfigSettings(const nlohmann::json& data) :
			server_ip(data.value("server_ip", "127.0.0.1")),
			server_port(data.value("server_port", "5060")),
			sip_id(data.value("sip_id", "1234")),
			sip_password(data.value("sip_password", "0000")),
			concurrent_calls(data.value("concurrent_calls", 1))
		{
		}
	};
}