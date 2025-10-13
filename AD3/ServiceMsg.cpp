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

	void SerivceMsg::init()
	{
		LOG_II("SerivceMsg::init:");
		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<SerivceMsg>(new SerivceMsg);
	}

	void SerivceMsg::shutdown()
	{
		LOG_II("SerivceMsg::shutdown:");
	}

	SerivceMsg& SerivceMsg::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	void SerivceMsg::log(std::string title, std::string msg, ServiceData::Type type)
	{
		std::lock_guard lock(msg_mutex_);
		msgs_.emplace_front(std::move(title), std::move(msg), type);

		// Notify monitors
		for (auto& [_, cv] : msgs_cv_)
		{
			cv->notify_all();
		}
	}

	void SerivceMsg::reg_monitor(const char* registry, std::condition_variable* cv)
	{
		std::lock_guard lock(msg_mutex_);
		assert(not msgs_cv_.contains(registry));
		msgs_cv_[registry] = cv;
	}

	void SerivceMsg::unreg_monitor(const char* registry)
	{
		std::lock_guard lock(msg_mutex_);
		assert(msgs_cv_.contains(registry));
		msgs_cv_.erase(registry);
	}
}