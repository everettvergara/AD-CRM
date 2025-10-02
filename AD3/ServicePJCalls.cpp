#include "ServicePJCalls.h"
#include <cassert>
#include <format>
#include "Common/Log.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"
#include "PJHelper.h"

namespace eg::ad3
{
	void ServicePJCalls::init()
	{
		LOG_II("ServicePJCalls::init:");

		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServicePJCalls>(new ServicePJCalls);
	}

	ServicePJCalls& ServicePJCalls::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void ServicePJCalls::shutdown()
	{
		if (instance_ not_eq nullptr)
		{
			//instance_->account.shutdown();
		}
	}

	int ServicePJCalls::make_call(std::function<void(pjsip_inv_state, pj::CallInfo info)> fn, const std::string& wav_filename, const std::string& mobile)
	{
		register_current_thread_in_pj("make_call");

		std::lock_guard lock(call_mutex_);

		auto current_call = std::make_shared<PJCallManualDial>(
			ServicePJAccount::instance().account,
			std::move(fn),
			wav_filename);

		current_call->makeCall(std::format("sip:{}@{}", mobile, ConfigSettings::instance().server_ip), []
			{
				pj::CallOpParam p(true);
				p.opt.audioCount = 1;
				p.opt.videoCount = 0;
				return p;
			}());

		auto id = current_call->getId();

		calls_[id] = current_call;

		return id;
	}

	void ServicePJCalls::hangup_all_calls_except(int except_call)
	{
		//register_current_thread_in_pj("hangup_except");
		LOG_II("ServicePJCalls::hangup_all_calls EXCEPT {}:", except_call);

		std::lock_guard lock(call_mutex_);

		for (const auto& [id, call] : calls_)
		{
			call->stop_ringing();
		}

		for (const auto& [id, call] : calls_)
		{
			if (id not_eq except_call)
			{
				call->hangup_call();
			}
		}

		for (const auto& [id, call] : calls_)
		{
			if (id not_eq except_call)
			{
				call->wait_until_state_is_disconnected();
			}
		}

		for (auto it = calls_.begin(); it != calls_.end();)
		{
			if (it->first != except_call)
			{
				it = calls_.erase(it);
				break;
			}
			else
			{
				++it;
			}
		}
	}

	void ServicePJCalls::hangup_all_calls()
	{
		LOG_II("ServicePJCalls::hangup_all_calls:");

		std::lock_guard lock(call_mutex_);

		for (const auto& [_, call] : calls_)
		{
			call->stop_ringing();
			call->hangup_call();
		}
		for (const auto& [_, call] : calls_)
		{
			call->wait_until_state_is_disconnected();
		}
		calls_.clear();
	}

	void ServicePJCalls::hangup_and_remove_call(int call_id)
	{
		LOG_II("ServicePJCalls::hangup_and_remove_call: call_id={}", call_id);
		std::lock_guard lock(call_mutex_);
		if (not calls_.contains(call_id))
		{
			return;
		}

		auto& call = calls_.at(call_id);
		call->stop_ringing();
		call->hangup_call();
		call->wait_until_state_is_disconnected();
		calls_.erase(call_id);
	}

	void ServicePJCalls::remove_call(int call_id)
	{
		LOG_II("ServicePJCalls::remove_call: call_id={}", call_id);
		std::lock_guard lock(call_mutex_);
		if (not calls_.contains(call_id))
		{
			return;
		}

		calls_.erase(call_id);
	}
}