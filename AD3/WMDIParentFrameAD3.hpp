#pragma once

#include "WMDIParentFrame.h"

#include "WConfigSettings.hpp"
#include "WDialer.h"
#include "WRDialer.h"
#include "resource.h"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"
#include "WMsg.hpp"

namespace eg::ad3
{
	constexpr const auto k_msg_window_id = "wmsg";

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
			register_sub_menu(call_menu, next_menu_id(), "Dialer - A", &WMDIParentFrameAD3::on_call_dialer1, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - B", &WMDIParentFrameAD3::on_call_dialer2, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - C", &WMDIParentFrameAD3::on_call_dialer3, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - D", &WMDIParentFrameAD3::on_call_dialer4, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - E", &WMDIParentFrameAD3::on_call_dialer5, this);
			register_sub_menu(call_menu, next_menu_id(), "Dialer - F", &WMDIParentFrameAD3::on_call_dialer6, this);

			menu->Append(file_menu, "&File");
			menu->Append(call_menu, "&Call");
			if (ConfigSettings::instance().is_admin)
			{
				auto* admin_menu = new wxMenu();
				register_sub_menu(admin_menu, next_menu_id(), "Recordings", &WMDIParentFrameAD3::on_admin_recordings, this);

				auto* robo_menu = new wxMenu();
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 1", &WMDIParentFrameAD3::on_robo_dialer1, this);
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 2", &WMDIParentFrameAD3::on_robo_dialer2, this);
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 3", &WMDIParentFrameAD3::on_robo_dialer3, this);
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 4", &WMDIParentFrameAD3::on_robo_dialer4, this);
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 5", &WMDIParentFrameAD3::on_robo_dialer5, this);
				register_sub_menu(robo_menu, next_menu_id(), "Robo - 6", &WMDIParentFrameAD3::on_robo_dialer6, this);

				menu->Append(robo_menu, "&Robo");
				menu->Append(admin_menu, "&Admin");
			}
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

		void on_admin_recordings(wxCommandEvent&)
		{
			static constexpr auto id = "recordings";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 5 % ConfigSettings::instance().max_ep();
				auto child = new WDialer(this, "Recordings Browser", ix, true);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer1(wxCommandEvent&)
		{
			static constexpr auto id = "dialer1";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 0 % ConfigSettings::instance().max_ep();
				LOG_II("Account index: 0: {}", ix);
				auto child = new WDialer(this, "Dialer - A", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer2(wxCommandEvent&)
		{
			static constexpr auto id = "dialer2";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 1 % ConfigSettings::instance().max_ep();
				//LOG_II("Account index: 1: {}", ix);

				auto child = new WDialer(this, "Dialer - B", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer3(wxCommandEvent&)
		{
			static constexpr auto id = "dialer3";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 2 % ConfigSettings::instance().max_ep();
				//LOG_II("Account index: 2: {}", ix);

				auto child = new WDialer(this, "Dialer - C", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer4(wxCommandEvent&)
		{
			static constexpr auto id = "dialer4";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 3 % ConfigSettings::instance().max_ep();

				auto child = new WDialer(this, "Dialer - D", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer5(wxCommandEvent&)
		{
			static constexpr auto id = "dialer5";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 4 % ConfigSettings::instance().max_ep();
				auto child = new WDialer(this, "Dialer - E", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_call_dialer6(wxCommandEvent&)
		{
			static constexpr auto id = "dialer6";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 5 % ConfigSettings::instance().max_ep();

				auto child = new WDialer(this, "Dialer - F", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer1(wxCommandEvent&)
		{
			static constexpr auto id = "robo1";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 0 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 1", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer2(wxCommandEvent&)
		{
			static constexpr auto id = "robo2";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 1 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 2", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer3(wxCommandEvent&)
		{
			static constexpr auto id = "robo3";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 2 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 3", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer4(wxCommandEvent&)
		{
			static constexpr auto id = "robo4";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 3 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 4", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer5(wxCommandEvent&)
		{
			static constexpr auto id = "robo5";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 4 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 5", ix);
				child_ids[id] = child;

				return;
			}
		}

		void on_robo_dialer6(wxCommandEvent&)
		{
			static constexpr auto id = "robo6";

			if (not is_child_active(k_msg_window_id))
			{
				auto child = new WMsg(this);
				child_ids[k_msg_window_id] = child;
			}

			if (not is_child_active(id))
			{
				auto ix = 5 % ConfigSettings::instance().max_ep();

				auto child = new WRDialer(this, "Robo - 6", ix);
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