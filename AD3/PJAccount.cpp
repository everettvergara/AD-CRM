#include "PJAccount.h"

#include <format>
#include "Common/Log.hpp"
#include "Common/ServiceCtrlC.h"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
	PJAccount::PJAccount(const std::string& sip_id, const std::string& password) :
		is_registered(false),
		failed_registration_ctr_(0)
	{
		LOG_INSTANCE;

		LOG_I("PJAccount::PJAccount: Creating PJ Account");
		this->create([this, &sip_id, &password]
			{
				pj::AccountConfig account_config{};
				const auto& settings = ConfigSettings::instance();

				account_config.idUri = std::format(k_pj_sip_user_server_format, sip_id, settings.server_ip);
				account_config.sipConfig.authCreds.emplace_back("digest", "*", sip_id, 0, password);

				account_config.regConfig.timeoutSec = settings.server_timeout_secs;
				account_config.regConfig.registrarUri = std::format(k_pj_sip_server_port_no_format, settings.server_ip, settings.server_port);
				account_config.regConfig.registerOnAdd = settings.register_on_add;
				account_config.natConfig.udpKaIntervalSec = settings.server_keep_alive_secs;
				account_config.natConfig.udpKaData = settings.server_keep_alive_data;

				return account_config;
			}());

		wait_until_registered_or_signal_exit();
	}

	void PJAccount::wait_until_registered_or_signal_exit()
	{
		LOG_II("PJAccount::wait_until_registered:");
		if (ConfigSettings::instance().mock)
		{
			LOG_II("PJAccount::wait_until_registered: Mock mode detected.");
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
		//LOG_I("PJAccount::onRegState:  ({}) {}", static_cast<int>(prm.status), prm.reason);

		is_registered = (prm.code == PJSIP_SC_OK);

		if (is_registered)
		{
			//LOG_I("account::onRegState: Account registration successful.");
			failed_registration_ctr_ = 0;
			cv_.notify_one();
		}
		else
		{
			++failed_registration_ctr_;
			LOG_X("account::onRegState: Registration failed: {}", static_cast<int>(prm.code));

			if (failed_registration_ctr_ == ConfigSettings::instance().max_registration_attempts)
			{
				LOG_X("Max registration failed. Signaling exit.");

				sys::ServiceCtrlC::instance().signal_exit();
				cv_.notify_one();
			}
		}
	}

	void PJAccount::onIncomingCall(pj::OnIncomingCallParam& prm)
	{
		//LOG_INSTANCE;
		//LOG_I("PJAccount::onIncomingCall:");
	}

	void PJAccount::onIncomingSubscribe(pj::OnIncomingSubscribeParam&)
	{
		//LOG_INSTANCE;
		//LOG_I("PJAccount::onIncomingSubscribe:");
	}
}