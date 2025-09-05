#include "PJAccount.h"

#include <format>
#include "ConfigSIP.hpp"

#include "ServiceCtrlC.h"

#include "Log.hpp"
#include "Config.hpp"

namespace eg::net
{
	PJAccount::PJAccount(const std::string* code) :
		account_code(code),
		is_registered(false),
		failed_registration_ctr_(0)
	{
		LOG_INSTANCE;
		LOG_I("PJAccount::PJAccount: {}", *account_code);

		this->create([this]
			{
				pj::AccountConfig account_config{};
				const auto& sip_config = sys::Config<ConfigSIP>::instance().configs.at(*account_code);

				account_config.idUri = std::format(k_pj_sip_user_server_format, sip_config.server_user, sip_config.server_name);
				account_config.regConfig.timeoutSec = sip_config.server_timeout_sec;
				account_config.regConfig.registrarUri = std::format(k_pj_sip_server_port_no_format, sip_config.server_name, sip_config.server_port_no);
				account_config.regConfig.registerOnAdd = sip_config.account_register_on_add;
				account_config.sipConfig.authCreds.emplace_back("digest", "*", sip_config.server_user, 0, sip_config.server_password);
				account_config.natConfig.udpKaIntervalSec = sip_config.server_keep_alive_sec;
				account_config.natConfig.udpKaData = sip_config.server_keep_alive_data;
				//account_config.callConfig.prackUse = PJSUA_100REL_MANDATORY; // Support 100rel
				return account_config;
			}());

		//if ()
		wait_until_registered_or_signal_exit();
	}

	PJAccount::~PJAccount()
	{
		this->shutdown();
	}

	void PJAccount::wait_until_registered_or_signal_exit()
	{
		LOG_INSTANCE;
		LOG_I("PJAccount::wait_until_registered:");

		if (std::get<bool>(eg::global::get("mock")))
		{
			return;
		}

		auto& ctrlc = sys::ServiceCtrlC::instance();

		ctrlc.register_cv("pjaccount_register_wait", &cv_);

		std::unique_lock lock(mutex_);
		cv_.wait(lock, [this, &ctrlc]
			{
				return is_registered or ctrlc.is_signal_exit();
			});

		ctrlc.unregister_cv("pjaccount_register_wait");
	}

	void PJAccount::onRegState(pj::OnRegStateParam& prm)
	{
		LOG_INSTANCE;
		LOG_I("PJAccount::onRegState:  ({}) {}", static_cast<int>(prm.status), prm.reason);

		is_registered = (prm.code == PJSIP_SC_OK);

		if (is_registered)
		{
			LOG_I("account::onRegState: Account registration successful.");

			failed_registration_ctr_ = 0;
			cv_.notify_one();
		}
		else
		{
			++failed_registration_ctr_;
			LOG_X("account::onRegState: Registration failed: {}", static_cast<int>(prm.code));

			if (failed_registration_ctr_ == sys::Config<ConfigSIP>::instance().configs.at(*account_code).server_max_registration_attempt)
			{
				LOG_X("Max registration failed. Signaling exit.");

				sys::ServiceCtrlC::instance().signal_exit();
				cv_.notify_one();
			}
		}
	}

	void PJAccount::onIncomingCall(pj::OnIncomingCallParam& prm)
	{
		LOG_INSTANCE;
		LOG_I("PJAccount::onIncomingCall:");
	}

	void PJAccount::onIncomingSubscribe(pj::OnIncomingSubscribeParam&)
	{
		LOG_INSTANCE;
		LOG_I("PJAccount::onIncomingSubscribe:");
	}
}