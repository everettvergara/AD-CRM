#pragma once

#include <unordered_map>
#include <wx/wx.h>

namespace eg::ad3
{
	class WMDIParentFrame :
		public wxMDIParentFrame
	{
	public:

		WMDIParentFrame() = default;
		WMDIParentFrame(const WMDIParentFrame&) = delete;
		WMDIParentFrame& operator=(const WMDIParentFrame&) = delete;
		WMDIParentFrame(WMDIParentFrame&&) = delete;
		WMDIParentFrame& operator=(WMDIParentFrame&&) = delete;
		virtual ~WMDIParentFrame() = default;

		void init(const wxString& title, wxPoint pos, wxSize size, long style = wxDEFAULT_FRAME_STYLE);

	protected:
		std::unordered_map<const char*, wxWeakRef<wxMDIChildFrame>> child_ids;

		[[nodiscard]] static int next_menu_id();
		[[nodiscard]] bool is_child_active(const char* id) const;

		virtual void on_init_menu(wxMenuBar*) {}

		template<typename Class, typename EventArg>
		void register_sub_menu(wxMenu* menu, int id, const char* menu_name, void (Class::* function)(EventArg&), Class* handler)
		{
			menu->Append(id, menu_name);
			Bind(wxEVT_MENU, function, handler, id);
		}

		template<typename F>
		void register_sub_menu(wxMenu* menu, int id, const char* menu_name, F&& function)
		{
			menu->Append(id, menu_name);
			Bind(wxEVT_MENU, std::forward<F>(function), id);
		}
	};
}