#pragma once

#include <string>
#include "WChildFrame.h"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
	class WConfigSettings :
		public WChildFrame
	{
	public:
		WConfigSettings(wxMDIParentFrame* parent) :
			WChildFrame
			(
				WChildProp
				{
					.parent = parent,
					.title = "AD3.0 Settings",
					.pos = wxDefaultPosition,
					.size = wxSize(400, 900),
					.style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX),
					.form_columns = 2,
					.has_tree = false
				}
			)

		{
			const auto& settings = ConfigSettings::instance();

			register_text_input("sip_server_ip", "SIP Server IP:", settings.server_ip);
			register_text_input("sip_server_port", "Port:", settings.server_port);
			register_text_input("sip_id", "SIP ID:", settings.sip_id);
			register_text_input("sip_password", "SIP Password:", settings.sip_password);
			register_text_input("sip_id2", "SIP ID2:", settings.sip_id2);
			register_text_input("sip_password2", "SIP Password2:", settings.sip_password2);
			register_text_input("concurrent_calls", "Concurrent calls:", std::to_string(settings.concurrent_calls));

			register_text_input("max_registration_attempts", "Max account registration attempts:", std::to_string(settings.max_registration_attempts));
			register_text_input("server_timeout_secs", "Server timeout (secs):", std::to_string(settings.server_timeout_secs));
			register_text_input("server_keep_alive_secs", "Keep alive (secs):", std::to_string(settings.server_keep_alive_secs));
			register_text_input("server_keep_alive_data", "Keep alive data:", settings.server_keep_alive_data);
			register_checkbox("register_on_add", "Register on add:", settings.register_on_add);
			register_checkbox("mock", "Mock mode:", settings.mock);

			register_text_input("db_driver", "DB Driver:", settings.db_driver);
			register_text_input("db_server", "DB Server:", settings.db_server);
			register_text_input("db_database", "DB Database:", settings.db_database);
			register_text_input("db_user", "DB User:", settings.db_user);
			register_text_input("db_password", "DB Password:", settings.db_password, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
			register_text_input("fycrm_path", "CRM Path:", settings.fycrm_path);

			register_button("Save", wxID_OK)->Bind(wxEVT_BUTTON, &WConfigSettings::on_ok_, this);
			register_button("Cancel", wxID_CANCEL)->Bind(wxEVT_BUTTON, &WConfigSettings::on_cancel_, this);

			Show(true);
		}

	private:

		const char* set_settings()
		{
			auto server_ip = get<wxTextCtrl>("sip_server_ip")->GetValue();
			auto server_port = get<wxTextCtrl>("sip_server_port")->GetValue();
			auto sip_id = get<wxTextCtrl>("sip_id")->GetValue();
			auto sip_password = get<wxTextCtrl>("sip_password")->GetValue();
			auto sip_id2 = get<wxTextCtrl>("sip_id2")->GetValue();
			auto sip_password2 = get<wxTextCtrl>("sip_password2")->GetValue();
			auto concurrent_calls = get<wxTextCtrl>("concurrent_calls")->GetValue();

			auto max_registration_attempts = get<wxTextCtrl>("max_registration_attempts")->GetValue();
			auto server_timeout_secs = get<wxTextCtrl>("server_timeout_secs")->GetValue();
			auto server_keep_alive_secs = get<wxTextCtrl>("server_keep_alive_secs")->GetValue();
			auto server_keep_alive_data = get<wxTextCtrl>("server_keep_alive_data")->GetValue();
			auto register_on_add = get<wxCheckBox>("register_on_add")->GetValue();
			auto mock = get<wxCheckBox>("mock")->GetValue();

			// DB
			auto db_driver = get<wxTextCtrl>("db_driver")->GetValue();
			auto db_server = get<wxTextCtrl>("db_server")->GetValue();
			auto db_database = get<wxTextCtrl>("db_database")->GetValue();
			auto db_user = get<wxTextCtrl>("db_user")->GetValue();
			auto db_password = get<wxTextCtrl>("db_password")->GetValue();

			auto fycrm_path = get<wxTextCtrl>("fycrm_path")->GetValue();

			if (db_driver.IsEmpty()) return "DB Driver is empty";
			if (db_server.IsEmpty()) return "DB Server is empty";
			if (db_database.IsEmpty()) return "DB Database is empty";
			if (db_user.IsEmpty()) return "DB User is empty";
			if (db_password.IsEmpty()) return "DB Password is empty";

			if (server_ip.IsEmpty()) return "Server IP is empty";
			if (server_port.IsEmpty()) return "Port is empty";
			if (sip_id.IsEmpty()) return "SIP ID is empty";
			if (sip_password.IsEmpty()) return "Password is empty";

			if (sip_id2.IsEmpty()) return "SIP ID2 is empty";
			if (sip_password2.IsEmpty()) return "Password2 is empty";

			if (concurrent_calls.IsEmpty()) return "Concurrent calls is empty";
			if (max_registration_attempts.IsEmpty()) return "Max registration attempts is empty";
			if (server_timeout_secs.IsEmpty()) return "Server timeout is empty";
			if (server_keep_alive_secs.IsEmpty()) return "Keep alive secs is empty";
			if (server_keep_alive_data.IsEmpty()) return "Keep alive data is empty";

			if (fycrm_path.IsEmpty()) return "CRM Path is empty";

			auto& settings = ConfigSettings::instance();
			settings.server_ip = server_ip.ToStdString();
			settings.server_port = server_port.ToStdString();
			settings.sip_id = sip_id.ToStdString();
			settings.sip_password = sip_password.ToStdString();
			settings.sip_id2 = sip_id2.ToStdString();
			settings.sip_password2 = sip_password2.ToStdString();

			settings.concurrent_calls = std::stoull(concurrent_calls.ToStdString());
			settings.max_registration_attempts = std::stoull(max_registration_attempts.ToStdString());
			settings.server_timeout_secs = std::stoul(server_timeout_secs.ToStdString());
			settings.server_keep_alive_secs = std::stoul(server_keep_alive_secs.ToStdString());
			settings.server_keep_alive_data = server_keep_alive_data.ToStdString();
			settings.register_on_add = register_on_add;
			settings.mock = mock;

			settings.db_driver = db_driver.ToStdString();
			settings.db_server = db_server.ToStdString();
			settings.db_database = db_database.ToStdString();
			settings.db_user = db_user.ToStdString();
			settings.db_password = db_password.ToStdString();

			settings.fycrm_path = fycrm_path.ToStdString();

			return nullptr;
		}

		void on_ok_(wxCommandEvent&)
		{
			if (auto error = set_settings(); error)
			{
				wxMessageBox(error, "Info", wxOK | wxICON_INFORMATION, this);
				return;
			}

			if (auto& settings = ConfigSettings::instance();
				settings.save(k_settings_filename).empty())
			{
				wxMessageBox("Error saving the settings", "Info", wxOK | wxICON_INFORMATION, this);
				return;
			}

			wxMessageBox("Settings saved", "Info", wxOK | wxICON_INFORMATION, this);
		}

		void on_cancel_(wxCommandEvent&)
		{
			Close(true);
		}
	};
}