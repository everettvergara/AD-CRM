#pragma once

#include "WChildFrame.h"

namespace eg::ad3
{
	class WChildFrameJSONSettings :
		public WChildFrame
	{
	public:

		WChildFrameJSONSettings
		(
			wxMDIParentFrame* parent,
			const wxString& title,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0
		);
	};
}