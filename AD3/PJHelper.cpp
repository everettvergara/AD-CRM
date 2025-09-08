#include "PJHelper.h"

#include <stdexcept>
#include <pjsua2.hpp>

namespace eg::ad3
{
	void register_current_thread_in_pj(const char* thread_name)
	{
		thread_local pj_thread_desc desc;
		thread_local pj_thread_t* thread = nullptr;

		if (not pj_thread_is_registered())
		{
			pj_bzero(desc, sizeof(desc));
			pj_status_t rc = pj_thread_register(thread_name, desc, &thread);
			if (rc not_eq PJ_SUCCESS)
			{
				throw std::runtime_error("Could not register thread");
			}
		}
	}
}