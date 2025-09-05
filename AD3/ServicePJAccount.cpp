#include "ServicePJAccount.h"
#include <cassert>
#include "Log.hpp"
#include "ConfigSIP.hpp"
#include "Global.hpp"

namespace eg::net
{
	ServicePJAccount::ServicePJAccount() :
		accounts_([]
			{
				std::unordered_map<std::string, PJAccount> accounts;

				const auto& config = ConfigSIP::instance();
				for (const auto& [key, value] : config.configs)
				{
					accounts.try_emplace(key, &key);
				}

				return accounts;
			}()),
		accounts_it_(accounts_.begin())
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
		LOG_II("ServicePJAccount::shutdown:");
		if (instance_ not_eq nullptr)
		{
			for (auto& [_, account] : instance_->accounts_)
			{
				account.shutdown();
			}
		}
	}

	PJAccount& ServicePJAccount::auto_account()
	{
		assert(accounts_.size() >= 1);

		auto t = accounts_it_;

		++accounts_it_;

		if (accounts_it_ == accounts_.end())
		{
			accounts_it_ = accounts_.begin();
		}

		return t->second;
	}

	PJAccount& ServicePJAccount::account(const std::string& code)
	{
		assert(accounts_.contains(code));

		return accounts_.at(code);
	}
}