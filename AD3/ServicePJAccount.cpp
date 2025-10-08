#include "ServicePJAccount.h"
#include <cassert>
#include "Common/Log.hpp"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
	ServicePJAccount::ServicePJAccount() :
		accounts([]() -> std::vector<std::unique_ptr<PJAccount>>
			{
				std::vector<std::unique_ptr<PJAccount>> accounts;

				const auto& config = ConfigSettings::instance();

				if (config.sip_id not_eq "NA")
				{
					LOG_II("ServicePJAccount::ServicePJAccount: Adding account for SIP ID {}", config.sip_id);
					accounts.emplace_back(std::make_unique<PJAccount>(config.sip_id, config.sip_password));
				}

				if (config.sip_id2 not_eq "NA")
				{
					LOG_II("ServicePJAccount::ServicePJAccount: Adding account for SIP ID {}", config.sip_id2);
					accounts.emplace_back(std::make_unique<PJAccount>(config.sip_id2, config.sip_password2));
				}

				return accounts;
			}())
	{
	}

	void ServicePJAccount::init()
	{
		LOG_II("ServicePJAccount::init:");

		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServicePJAccount>(new ServicePJAccount);
	}

	ServicePJAccount& ServicePJAccount::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void ServicePJAccount::shutdown()
	{
		if (instance_ not_eq nullptr)
		{
			for (auto& account : instance_->accounts)
			{
				account->shutdown();
			}
		}
	}
}