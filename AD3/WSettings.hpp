#pragma once

#include "WChildFrameJSONSettings.h"

namespace eg::ad3
{
	class WSettings :
		public WChildFrameJSONSettings
	{
	public:
		WSettings
		(
			wxMDIParentFrame* parent,
			const wxString& title,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0
		) :
			WChildFrameJSONSettings(parent, title, pos, size, style)
		{
		}
	};
}