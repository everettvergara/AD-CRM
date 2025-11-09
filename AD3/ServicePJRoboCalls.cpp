#include "ServicePJRoboCalls.h"
#include <cassert>
#include <format>
#include "Common/Log.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"
#include "PJHelper.h"

namespace eg::ad3
{
	ServicePJRoboCalls::~ServicePJRoboCalls()
	{
		shutdown();
	}

	void ServicePJRoboCalls::init()
	{
		LOG_II("ServicePJRoboCalls::init:");

		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServicePJRoboCalls>(new ServicePJRoboCalls);
	}

	ServicePJRoboCalls& ServicePJRoboCalls::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void ServicePJRoboCalls::shutdown()
	{
		if (instance_ not_eq nullptr)
		{
			//instance_->account.shutdown();
		}
	}

	bool ServicePJRoboCalls::play_wav(const std::string& wav_filename)
	{
		//LOG_II("playwav {}", wav_filename);
		std::lock_guard lock(call_mutex_);
		if (not calls_.empty())
		{
			return false;
		}

		ServicePJWavPool::instance().play_wav(wav_filename);

		return true;
	}

	int ServicePJRoboCalls::make_call(std::function<void(pjsip_inv_state, pj::CallInfo info, bool)> fn, const std::string& record_wav_filename, const std::string& player_wav_filename, const std::string& mobile, size_t account_ix)
	{
		LOG_II("makecall-1 {}", mobile);

		register_current_thread_in_pj("make_call");
		LOG_II("makecall-2 {}", mobile);

		std::lock_guard lock(call_mutex_);

		LOG_II("makecall-3 {}", mobile);

		if (active_call_id_.has_value())
		{
			return -1;
		}

		LOG_II("makecall-4 {}", mobile);

		auto current_call = std::make_shared<PJCallRoboDial>(
			*ServicePJAccount::instance().accounts.at(account_ix),
			std::move(fn),
			record_wav_filename,
			player_wav_filename);

		LOG_II("makecall-5 {}", mobile);

		current_call->makeCall(std::format("sip:{}@{}", mobile, ConfigSettings::instance().server_ip), []
			{
				pj::CallOpParam p(true);
				p.opt.audioCount = 1;
				p.opt.videoCount = 0;
				return p;
			}());

		LOG_II("makecall-6 {}", mobile);

		auto id = current_call->getId();

		LOG_II("makecall-7 {}", mobile);

		calls_[id] = current_call;

		LOG_II("makecall-8 {}", mobile);

		return id;
	}

	void ServicePJRoboCalls::hangup_all_calls()
	{
		LOG_II("ServicePJRoboCalls::hangup_all_calls:");

		std::lock_guard lock(call_mutex_);

		for (const auto& [_, call] : calls_)
		{
			call->stop_ringing();
			call->hangup_call(true);
		}
		for (const auto& [_, call] : calls_)
		{
			call->wait_until_state_is_disconnected();
		}
		calls_.clear();
		active_call_id_.reset();
	}

	void ServicePJRoboCalls::hangup_and_remove_call(int call_id)
	{
		LOG_II("ServicePJRoboCalls::hangup_and_remove_call: call_id={}", call_id);
		std::lock_guard lock(call_mutex_);
		if (not calls_.contains(call_id))
		{
			return;
		}

		auto& call = calls_.at(call_id);
		call->stop_ringing();
		call->hangup_call(true);
		call->wait_until_state_is_disconnected();
		calls_.erase(call_id);
		if (active_call_id_.has_value() and active_call_id_.value() == call_id)
		{
			active_call_id_.reset();
		}
	}

	void ServicePJRoboCalls::remove_call(int call_id)
	{
		LOG_II("ServicePJRoboCalls::remove_call: call_id={}", call_id);
		std::lock_guard lock(call_mutex_);
		if (not calls_.contains(call_id))
		{
			return;
		}

		calls_.erase(call_id);
		if (active_call_id_.has_value() and active_call_id_.value() == call_id)
		{
			active_call_id_.reset();
		}
	}
}