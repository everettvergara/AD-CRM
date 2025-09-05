#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <pjsua2.hpp>
#include "PJAccount.h"

namespace eg::net
{
	class ServicePJAccount
	{
	public:

		static void init();
		static ServicePJAccount& instance();
		static void shutdown();

		ServicePJAccount(const ServicePJAccount&) = delete;
		ServicePJAccount(ServicePJAccount&&) noexcept = delete;
		ServicePJAccount& operator=(const ServicePJAccount&) = delete;
		ServicePJAccount& operator=(ServicePJAccount&&) = delete;
		~ServicePJAccount() = default;

		//PJAccount& mock_account();
		PJAccount& auto_account();
		PJAccount& account(const std::string&);

	private:

		ServicePJAccount();

		std::unordered_map<std::string, PJAccount> accounts_;
		std::unordered_map<std::string, PJAccount>::iterator accounts_it_;
		inline static std::unique_ptr<ServicePJAccount> instance_;
	};
}