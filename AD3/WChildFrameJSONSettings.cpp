#include "WChildFrameJSONSettings.h"

namespace eg::ad3
{
	WChildFrameJSONSettings::WChildFrameJSONSettings
	(
		wxMDIParentFrame* parent,
		const wxString& title,
		const wxPoint& pos,
		const wxSize& size,
		long style
	) :
		WChildFrame(parent, title, pos, size, style)
	{
	}
}