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

namespace eg::net
{
	struct PJCallAMP : public pj::AudioMediaPlayer
	{
		std::function<void()> play_complete;

		void onEof2() override
		{
			if (play_complete)
			{
				play_complete();
			}
		}
	};

	class PJCall : public pj::Call, public std::enable_shared_from_this<PJCall>
	{
	public:
		using RemoveHandler = std::function<void(std::shared_ptr<PJCall>)>;
		template <typename T>
		static std::shared_ptr<PJCall> create(PJAccount& account, std::chrono::seconds timeout_secs, nlohmann::json j, RemoveHandler remove_handler)
		{
			return std::shared_ptr<PJCall>(new T(account, timeout_secs, std::move(j), std::move(remove_handler)));
		}

		PJCall(PJAccount& account, std::chrono::seconds timeout_secs, nlohmann::json call_request, RemoveHandler remove_handler);
		virtual ~PJCall();

		PJCall() = delete;
		PJCall(const PJCall&) = delete;
		PJCall(PJCall&&) = delete;
		PJCall& operator=(const PJCall&) = delete;
		PJCall& operator=(PJCall&&) = delete;

		void wait_until_state_is_disconnected();
		void hangup_call();
		std::optional<pj::AudioMedia> get_active_media(const pj::CallInfo&);

	protected:

		pjsip_inv_state last_call_state_;
		pjsip_inv_state last_call_state_before_disconnected_;

		nlohmann::json call_request_;
		RemoveHandler remove_handler_;
		std::chrono::seconds timeout_secs_;

		void onCallState(pj::OnCallStateParam&) override;
		void onCallMediaState(pj::OnCallMediaStateParam&) override;

		virtual void on_call_state_disconnected_();
		virtual void on_call_media_state_on_confirmed_(pj::AudioMedia&);

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

		static void transfer_call_id_(size_t session_id, size_t call_id, pjsip_inv_state before, pjsip_inv_state after);
	};
}
