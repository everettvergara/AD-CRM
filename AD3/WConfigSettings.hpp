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
					.size = wxSize(300, 300),
					.style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX),
					.form_columns = 2,
				}
				)

		{
			const auto& settings = ConfigSettings::instance();

			register_text_input("sip_server_ip", "SIP Server IP:", settings.server_ip);
			register_text_input("sip_server_port", "Port:", settings.server_port);
			register_text_input("sip_id", "SIP ID:", settings.sip_id);
			register_text_input("sip_password", "SIP Password:", settings.sip_password);
			register_text_input("concurrent_calls", "Concurrent calls:", std::to_string(settings.concurrent_calls));

			register_button("Save", wxID_OK)->Bind(wxEVT_BUTTON, &WConfigSettings::on_ok_, this);
			register_button("Cancel", wxID_CANCEL)->Bind(wxEVT_BUTTON, &WConfigSettings::on_cancel_, this);

			Show(true);
		}

	private:

		const char* set_settings()
		{
			auto server_ip = get_text_input("sip_server_ip")->GetValue();
			auto server_port = get_text_input("sip_server_port")->GetValue();
			auto sip_id = get_text_input("sip_id")->GetValue();
			auto sip_password = get_text_input("sip_password")->GetValue();
			auto concurrent_calls = get_text_input("concurrent_calls")->GetValue();

			if (server_ip.IsEmpty()) return "Server IP is empty";
			if (server_port.IsEmpty()) return "Port is empty";
			if (sip_id.IsEmpty()) return "SIP ID is empty";
			if (sip_password.IsEmpty()) return "Password is empty";
			if (concurrent_calls.IsEmpty()) return "Concurrent calls is empty";

			auto& settings = ConfigSettings::instance();
			settings.server_ip = server_ip.ToStdString();
			settings.server_port = server_port.ToStdString();
			settings.sip_id = sip_id.ToStdString();
			settings.sip_password = sip_password.ToStdString();
			settings.concurrent_calls = std::stoull(concurrent_calls.ToStdString());

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