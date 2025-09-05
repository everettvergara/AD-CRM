#include "WMDIParentFrame.h"

namespace eg::ad3
{
	void WMDIParentFrame::init(const wxString& title, wxPoint pos, wxSize size, long style)
	{
		Create(nullptr, wxID_ANY, title, pos, size, style);

		auto menu_bar = new wxMenuBar;

		on_init_menu(menu_bar);
		SetMenuBar(menu_bar);

		Centre();
		Show(true);
	}

	int WMDIParentFrame::next_menu_id()
	{
		static int id = wxID_HIGHEST + 1000;
		return id++;
	}

	bool WMDIParentFrame::is_child_active(const char* id) const
	{
		auto find = child_ids.find(id);
		if (find == child_ids.end())
		{
			return false;
		}
		if (not find->second)
		{
			return false;
		}

		return true;
	}
}