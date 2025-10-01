#pragma once

#include "WMDIParentFrame.h"

#include "WConfigSettings.hpp"
#include "WDialer.h"
//#include "WDialer2.h"
#include "resource.h"

namespace eg::ad3
{
	class WMDIParentFrameAD3 final :
		public WMDIParentFrame
	{
	public:
		//WMDIParentFrameAD3()
		//{
		//	SetIcon(wxICON(IDI_MAINICON));
		//}
	protected:

		virtual void on_init_menu(wxMenuBar* menu)
		{
			auto* file_menu = new wxMenu();
			register_sub_menu(file_menu, next_menu_id(), "Settings", &WMDIParentFrameAD3::on_file_settings, this);
			file_menu->AppendSeparator();
			register_sub_menu(file_menu, next_menu_id(), "Close", &WMDIParentFrameAD3::on_file_close, this);

			auto* call_menu = new wxMenu();
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 1", &WMDIParentFrameAD3::on_call_dialer1, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 2", &WMDIParentFrameAD3::on_call_dialer2, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 3", &WMDIParentFrameAD3::on_call_dialer3, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 4", &WMDIParentFrameAD3::on_call_dialer4, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 5", &WMDIParentFrameAD3::on_call_dialer5, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - 6", &WMDIParentFrameAD3::on_call_dialer6, this);

			//register_sub_menu(call_menu, next_menu_id(), "Auto dial", &WMDIParentFrameAD3::on_call_auto_dial, this);

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

		void on_call_dialer1(wxCommandEvent&)
		{
			static constexpr auto id = "dialer1";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer2(wxCommandEvent&)
		{
			static constexpr auto id = "dialer2";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer3(wxCommandEvent&)
		{
			static constexpr auto id = "dialer3";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer4(wxCommandEvent&)
		{
			static constexpr auto id = "dialer4";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer5(wxCommandEvent&)
		{
			static constexpr auto id = "dialer5";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer6(wxCommandEvent&)
		{
			static constexpr auto id = "dialer6";
			if (not is_child_active(id))
			{
				auto child = new WDialer(this);
				child_ids[id] = child;

				return;
			}
		}
		void on_file_close(wxCommandEvent&)
		{
			Close(true);
		}
	};
}