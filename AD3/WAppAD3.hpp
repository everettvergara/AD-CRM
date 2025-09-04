#pragma once

#include "WApp.h"
#include "WMDIParentFrameAD3.hpp"

namespace eg::ad3
{
	class WAppAD3 final :
		public WApp
	{
	public:

		bool OnInit() override
		{
			wxInitAllImageHandlers();

			auto frame = new WMDIParentFrameAD3;

			frame->init("AD3.0 for FY-CRM", wxDefaultPosition, wxSize(1024, 768));

			return true;
		}
	};
}