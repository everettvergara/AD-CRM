#pragma once

#include <string>
#include <condition_variable>
#include <mutex>

#include <pjsua2.hpp>

namespace eg::net
{
	inline constexpr auto k_pj_sip_user_server_format = R"(sip:{}@{})";
	inline constexpr auto k_pj_sip_server_port_no_format = R"(sip:{}:{})";
	inline constexpr auto k_pj_sip_mobile_no_server_format = R"(sip:0{}@{})";

	class PJAccount : public pj::Account
	{
	public:
		const std::string* account_code;
		bool is_registered;

		PJAccount(const std::string* code);
		PJAccount(const PJAccount&) = delete;
		PJAccount(PJAccount&&) = delete;
		PJAccount& operator=(const PJAccount&) = delete;
		PJAccount& operator=(PJAccount&&) = delete;
		virtual ~PJAccount() noexcept;

		virtual void wait_until_registered_or_signal_exit();
		virtual void onRegState(pj::OnRegStateParam& prm);
		virtual void onIncomingCall(pj::OnIncomingCallParam& prm);
		virtual void onIncomingSubscribe(pj::OnIncomingSubscribeParam&);

	private:

		unsigned int failed_registration_ctr_;

		std::condition_variable cv_;
		std::mutex mutex_;
	};
}