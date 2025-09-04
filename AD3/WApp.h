#pragma once

#include <wx/wx.h>

namespace eg::ad3
{
	class WApp :
		public wxApp
	{
	public:
		bool OnInit() override = 0;
	};
}