#include "WChildFrame.h"

namespace eg::ad3
{
	WChildFrame::WChildFrame
	(
		wxMDIParentFrame* parent,
		const wxString& title,
		const wxPoint& pos,
		const wxSize& size,
		long style
	) :
		wxMDIChildFrame(parent, wxID_ANY, title, pos, size, style)
	{
	}
}