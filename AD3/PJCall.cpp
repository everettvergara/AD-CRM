#include "PJCall.h"

#include <stdexcept>
#include "ServicePJEndpoint.h"
#include "PJHelper.h"
#include "Common/Log.hpp"
#include "ServicePJWavPool.h"

namespace eg::ad3
{
	PJCall::PJCall(PJAccount& account) :
		pj::Call(*dynamic_cast<pj::Account*>(&account)),
		last_call_state(PJSIP_INV_STATE_NULL),
		hangup_requested(false)
	{
		timeout_thread_ = std::thread([this]
			{
				register_current_thread_in_pj("PJCall:TimeoutThread");

				std::unique_lock lock(mutex_);
				cv_.wait_for(lock, std::chrono::seconds(k_pj_call_timeout_secs), [this]
					{
						if (last_call_state == PJSIP_INV_STATE_NULL)
						{
							return false;
						}

						return true;
					});
				lock.unlock();

				if (last_call_state == PJSIP_INV_STATE_CALLING or
					last_call_state == PJSIP_INV_STATE_EARLY or
					last_call_state == PJSIP_INV_STATE_CONNECTING)
				{
					this->hangup_call();
				}
			});
	}

	PJCall::~PJCall()
	{
		//stop_local_ringback();
		timeout_thread_.join();
		wait_until_state_is_disconnected();
	}

	void PJCall::wait_until_state_is_disconnected()
	{
		// No Ctrl-C waiting
		// Just plain PJSIP_INV_STATE_DISCONNECTED
		// All states eventually fall to PJSIP_INV_STATE_DISCONNECTED

		std::unique_lock lock(mutex_);
		cv_.wait(lock, [this]
			{
				return last_call_state == PJSIP_INV_STATE_DISCONNECTED;
			});
	}

	void PJCall::hangup_call()
	{
		hangup_requested = true;
		{
			std::lock_guard lock(mutex_);
			if (last_call_state == PJSIP_INV_STATE_DISCONNECTED)
			{
				return;
			}
		}

		register_current_thread_in_pj("PJCall:HangupCall");
		this->hangup(pj::CallOpParam(true));
	}

	std::optional<pj::AudioMedia> PJCall::get_active_media(const pj::CallInfo& info)
	{
		for (auto& media : info.media)
		{
			if (media.type == PJMEDIA_TYPE_AUDIO and media.status == PJSUA_CALL_MEDIA_ACTIVE)
			{
				if (auto am = this->getAudioMedia(media.index); am.getPortId() >= 0)
				{
					return am;
				}
			}
		}

		return {};
	}

	void PJCall::onCallState(pj::OnCallStateParam&)
	{
		const auto info = this->getInfo();

		{
			std::lock_guard lock(mutex_);
			last_call_state = info.state;
		}

		if (last_call_state == PJSIP_INV_STATE_DISCONNECTED)
		{
			on_call_state_disconnected();

			// Notify:
			//	- timeout_thread_
			//	- wait_until_state_is_disconnected
			cv_.notify_all();
		}

		if (last_call_state == PJSIP_INV_STATE_CONFIRMED)
		{
			// Intended to notify:
			//	- timeout_thread_

			cv_.notify_all();
		}

		on_user_call_state_changed(info);
	}

	void PJCall::onCallMediaState(pj::OnCallMediaStateParam&)
	{
		if (last_call_state == PJSIP_INV_STATE_CONFIRMED)
		{
			const auto info = this->getInfo();
			if (auto active_media = get_active_media(info);
				active_media.has_value())
			{
				on_call_media_state_on_confirmed(active_media.value());
				return;
			}

			// If active media cannot be obtained, hangup the call
			hangup_call();
		}
	}

	// The last event to be called in onCallState
	void PJCall::on_user_call_state_changed(const pj::CallInfo& info)
	{
		// Update status of whatever resources
	}

	void PJCall::on_call_state_disconnected()
	{
		// Clean-up resources
	}

	void PJCall::on_call_media_state_on_confirmed(pj::AudioMedia&)
	{
		// Init resources
	}
}