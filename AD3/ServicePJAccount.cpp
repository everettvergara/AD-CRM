#include "ServicePJAccount.h"
#include <cassert>
#include "Common/Log.hpp"
#include "ConfigSettings.hpp"
#include "ServicePJEndpoint.h"
namespace eg::ad3
{
	ServicePJAccount::ServicePJAccount() :
		accounts([]() -> std::vector<std::unique_ptr<PJAccount>>
			{
				std::vector<std::unique_ptr<PJAccount>> accounts;

				const auto& config = ConfigSettings::instance();
				const auto& ep = ServicePJEndpoint::instance();

				if (config.sip_accounts.size() < ep.transport_ids.size())
				{
					LOG_XX("ServicePJAccount::ServicePJAccount: Provide more SIP accounts to match the number of EP.");
					throw std::runtime_error("Provide more SIP accounts to match the number of EP.");
				}

				for (size_t i = 0; auto tid : ep.transport_ids)
				{
					const auto& account = config.sip_accounts.at(i);
					LOG_II("ServicePJAccount::ServicePJAccount: Adding account for SIP ID {} on transport ID {}", account.sip_id, tid);
					accounts.emplace_back(std::make_unique<PJAccount>(account.sip_id, account.sip_password, tid));

					++i;
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
			//instance_->account.shutdown();
		}
	}
}