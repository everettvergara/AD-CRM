#include "ServicePJAccount.h"
#include <cassert>
#include "Common/Log.hpp"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
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
			instance_->account.shutdown();
		}
	}
}