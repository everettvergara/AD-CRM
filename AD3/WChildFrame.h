#pragma once

#include <wx/wx.h>

namespace eg::ad3
{
	class WChildFrame :
		public wxMDIChildFrame
	{
	public:

		WChildFrame
		(
			wxMDIParentFrame* parent,
			const wxString& title,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0
		);

	protected:

		std::unordered_map<std::string, wxWindow*> controls_;
	};
}