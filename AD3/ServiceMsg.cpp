#include "ServiceMsg.h"
#include "Common/Log.hpp"

namespace eg::ad3
{
	ServiceData::ServiceData(std::string title, std::string msg, ServiceData::Type type) :
		title(std::move(title)),
		msg(std::move(msg)),
		type(type),
		tp(std::chrono::system_clock::now())
	{
	}

	void ServiceMsg::init()
	{
		LOG_II("ServiceMsg::init:");
		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServiceMsg>(new ServiceMsg);
	}

	void ServiceMsg::shutdown()
	{
		LOG_II("ServiceMsg::shutdown:");

		if (instance_ not_eq nullptr)
		{
			std::lock_guard lock(instance_->msg_mutex_);
			instance_->msgs_cv_.clear();
		}
	}

	ServiceMsg& ServiceMsg::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void ServiceMsg::log(std::string title, std::string msg, ServiceData::Type type)
	{
		std::lock_guard lock(msg_mutex_);
		msgs_.emplace_front(std::move(title), std::move(msg), type);

		if (msgs_.size() >= k_service_msg_max)
		{
			msgs_.pop_back();
		}

		// Notify monitors
		for (auto& [_, fn] : msgs_fn_)
		{
			fn();
		}

		for (auto& [_, cv] : msgs_cv_)
		{
			cv->notify_all();
		}
	}

	void ServiceMsg::reg_monitor(const char* registry, std::condition_variable* cv, std::function<void()> update_fn)
	{
		std::lock_guard lock(msg_mutex_);
		assert(not msgs_cv_.contains(registry));
		msgs_cv_[registry] = cv;
		msgs_fn_[registry] = std::move(update_fn);
	}

	void ServiceMsg::unreg_monitor(const char* registry)
	{
		std::lock_guard lock(msg_mutex_);
		assert(msgs_cv_.contains(registry));
		msgs_cv_.erase(registry);
	}
	std::list<ServiceData> ServiceMsg::get_msgs()
	{
		std::lock_guard lock(msg_mutex_);

		// Copy
		return msgs_;
	}
}