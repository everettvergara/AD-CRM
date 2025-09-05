#pragma once

#include <string>
#include <condition_variable>
#include <mutex>

#include <pjsua2.hpp>
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	inline constexpr auto k_pj_sip_user_server_format = R"(sip:{}@{})";
	inline constexpr auto k_pj_sip_server_port_no_format = R"(sip:{}:{})";
	inline constexpr auto k_pj_sip_mobile_no_server_format = R"(sip:0{}@{})";

	class PJAccount final :
		public pj::Account, public eg::sys::NoCopyMove
	{
	public:
		bool is_registered;

		PJAccount();

		void wait_until_registered_or_signal_exit();
		void onRegState(pj::OnRegStateParam& prm);
		void onIncomingCall(pj::OnIncomingCallParam& prm);
		void onIncomingSubscribe(pj::OnIncomingSubscribeParam&);

	private:

		size_t failed_registration_ctr_;

		std::condition_variable cv_;
		std::mutex mutex_;
	};
}