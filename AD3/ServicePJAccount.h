#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <pjsua2.hpp>
#include "PJAccount.h"
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	class ServicePJAccount final :
		public eg::sys::NoCopyMove
	{
	public:

		std::vector<std::unique_ptr<PJAccount>> accounts;

		//std::mutex call_mutex;

		static void init();
		static ServicePJAccount& instance();
		static void shutdown();

	private:

		ServicePJAccount();

		inline static std::unique_ptr<ServicePJAccount> instance_;
	};
}