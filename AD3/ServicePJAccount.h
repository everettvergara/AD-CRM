#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <pjsua2.hpp>
#include "PJAccount.h"
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	class ServicePJAccount final :
		public eg::sys::NoCopyMove
	{
	public:
		PJAccount account;

		static void init();
		static ServicePJAccount& instance();
		static void shutdown();

	private:

		ServicePJAccount() = default;

		inline static std::unique_ptr<ServicePJAccount> instance_;
	};
}