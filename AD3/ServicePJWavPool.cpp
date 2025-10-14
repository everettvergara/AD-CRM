#include "ServicePJWavPool.h"
#include <cassert>
#include <format>
#include "Common/Log.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"
#include "PJHelper.h"

namespace eg::ad3
{
	void ServicePJWavPool::init()
	{
		LOG_II("ServicePJWavPool::init:");

		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServicePJWavPool>(new ServicePJWavPool);
	}

	ServicePJWavPool& ServicePJWavPool::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void ServicePJWavPool::shutdown()
	{
		if (instance_ not_eq nullptr)
		{
			//instance_->account.shutdown();
			if (instance_->pool_ not_eq nullptr)
			{
				pj_pool_release(instance_->pool_);
				instance_->pool_ = nullptr;
			}
		}
	}
}