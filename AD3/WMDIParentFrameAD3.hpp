#pragma once

#include "WMDIParentFrame.h"

#include "WConfigSettings.hpp"
#include "WManualDial.h"

namespace eg::ad3
{
	class WMDIParentFrameAD3 final :
		public WMDIParentFrame
	{
	protected:

		virtual void on_init_menu(wxMenuBar* menu)
		{
			auto* file_menu = new wxMenu();
			register_sub_menu(file_menu, next_menu_id(), "Settings", &WMDIParentFrameAD3::on_file_settings, this);
			file_menu->AppendSeparator();
			register_sub_menu(file_menu, next_menu_id(), "Close", &WMDIParentFrameAD3::on_file_close, this);

			auto* call_menu = new wxMenu();
			register_sub_menu(call_menu, next_menu_id(), "Manual dial", &WMDIParentFrameAD3::on_call_manual_dial, this);
			register_sub_menu(call_menu, next_menu_id(), "Auto dial", &WMDIParentFrameAD3::on_file_close, this);
			register_sub_menu(call_menu, next_menu_id(), "Receive progressive calls", &WMDIParentFrameAD3::on_file_close, this);

			menu->Append(file_menu, "&File");
			menu->Append(call_menu, "&Call");
		}

		void on_file_settings(wxCommandEvent&)
		{
			static constexpr auto id = "settings";
			if (not is_child_active(id))
			{
				child_ids[id] = new WConfigSettings(this);
				return;
			}
		}

		void on_call_manual_dial(wxCommandEvent&)
		{
			static constexpr auto id = "manual_dial";
			if (not is_child_active(id))
			{
				child_ids[id] = new WManualDial(this);
				return;
			}
		}
		void on_file_close(wxCommandEvent&)
		{
			Close(true);
		}
	};
}