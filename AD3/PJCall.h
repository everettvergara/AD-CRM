#pragma once

#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <optional>
#include <pjsua2.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <chrono>

#include "PJAccount.h"
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	constexpr auto k_pj_call_timeout_secs = 30;

	class PJCall :
		public pj::Call,
		public sys::NoCopyMove
	{
	public:

		PJCall(PJAccount& account);
		virtual ~PJCall();

		void wait_until_state_is_disconnected();
		void hangup_call();

		std::optional<pj::AudioMedia> get_active_media(const pj::CallInfo&);

	protected:

		volatile pjsip_inv_state last_call_state;

		void onCallState(pj::OnCallStateParam&) override;
		void onCallMediaState(pj::OnCallMediaStateParam&) override;

		virtual void on_user_call_state_changed();
		virtual void on_call_state_disconnected();
		virtual void on_call_media_state_on_confirmed(pj::AudioMedia&);

		void play_local_ringback();
		void stop_local_ringback();

	private:

		//PJSIP_INV_STATE_NULL,           /**< Before INVITE is sent or received  */
		//PJSIP_INV_STATE_CALLING,        /**< After INVITE is sent               */
		//PJSIP_INV_STATE_INCOMING,       /**< After INVITE is received.          */
		//PJSIP_INV_STATE_EARLY,          /**< After response with To tag.        */
		//PJSIP_INV_STATE_CONNECTING,     /**< After 2xx is sent/received.        */
		//PJSIP_INV_STATE_CONFIRMED,      /**< After ACK is sent/received.        */
		//PJSIP_INV_STATE_DISCONNECTED,   /**< Session is terminated.             */

		std::thread timeout_thread_;
		std::mutex mutex_;
		std::condition_variable cv_;
		pjsua_conf_port_id ring_tone_port_;
	};
}
