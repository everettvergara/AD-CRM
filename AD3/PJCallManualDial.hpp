#pragma once

#include "PJCall.h"

namespace eg::ad3
{
	class PJCallManualDial final :
		public PJCall
	{
	public:
		PJCallManualDial(PJAccount& account, std::function<void(pjsip_inv_state)> fn) :
			PJCall(account),
			on_status_change_callback_(std::move(fn))

		{
		}

	protected:
		void on_user_call_state_changed() override
		{
			on_status_change_callback_(last_call_state);
		}

		void on_call_state_disconnected() override
		{
		}

		void on_call_media_state_on_confirmed(pj::AudioMedia& media) override
		{
		}

	private:
		std::function<void(pjsip_inv_state)> on_status_change_callback_;
	};
}