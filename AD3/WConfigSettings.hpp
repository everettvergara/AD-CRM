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
					.size = wxSize(400, 450),
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
			register_text_input("concurrent_calls", "Concurrent calls:", std::to_string(settings.concurrent_calls));

			register_text_input("max_registration_attempts", "Max account registration attempts:", std::to_string(settings.max_registration_attempts));
			register_text_input("server_timeout_secs", "Server timeout (secs):", std::to_string(settings.server_timeout_secs));
			register_text_input("server_keep_alive_secs", "Keep alive (secs):", std::to_string(settings.server_keep_alive_secs));
			register_text_input("server_keep_alive_data", "Keep alive data:", settings.server_keep_alive_data);
			register_checkbox("register_on_add", "Concurrent calls:", settings.register_on_add);
			register_checkbox("mock", "Mock mode:", settings.mock);

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
			auto concurrent_calls = get<wxTextCtrl>("concurrent_calls")->GetValue();

			auto max_registration_attempts = get<wxTextCtrl>("max_registration_attempts")->GetValue();
			auto server_timeout_secs = get<wxTextCtrl>("server_timeout_secs")->GetValue();
			auto server_keep_alive_secs = get<wxTextCtrl>("server_keep_alive_secs")->GetValue();
			auto server_keep_alive_data = get<wxTextCtrl>("server_keep_alive_data")->GetValue();
			auto register_on_add = get<wxCheckBox>("register_on_add")->GetValue();
			auto mock = get<wxCheckBox>("mock")->GetValue();

			if (server_ip.IsEmpty()) return "Server IP is empty";
			if (server_port.IsEmpty()) return "Port is empty";
			if (sip_id.IsEmpty()) return "SIP ID is empty";
			if (sip_password.IsEmpty()) return "Password is empty";
			if (concurrent_calls.IsEmpty()) return "Concurrent calls is empty";
			if (max_registration_attempts.IsEmpty()) return "Max registration attempts is empty";
			if (server_timeout_secs.IsEmpty()) return "Server timeout is empty";
			if (server_keep_alive_secs.IsEmpty()) return "Keep alive secs is empty";
			if (server_keep_alive_data.IsEmpty()) return "Keep alive data is empty";

			auto& settings = ConfigSettings::instance();
			settings.server_ip = server_ip.ToStdString();
			settings.server_port = server_port.ToStdString();
			settings.sip_id = sip_id.ToStdString();
			settings.sip_password = sip_password.ToStdString();
			settings.concurrent_calls = std::stoull(concurrent_calls.ToStdString());
			settings.max_registration_attempts = std::stoull(max_registration_attempts.ToStdString());
			settings.server_timeout_secs = std::stoul(server_timeout_secs.ToStdString());
			settings.server_keep_alive_secs = std::stoul(server_keep_alive_secs.ToStdString());
			settings.server_keep_alive_data = server_keep_alive_data.ToStdString();
			settings.register_on_add = register_on_add;
			settings.mock = mock;

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