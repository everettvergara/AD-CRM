#pragma once

#include <memory>
#include <mutex>
#include <list>
#include <string>
#include <chrono>
#include <unordered_map>
#include <condition_variable>
#include <functional>
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	constexpr size_t k_service_msg_max = 10;

	struct ServiceData
	{
		std::string title;
		std::string msg;
		enum struct Type { INFO, WARNING, ERR, CRITICAL } type;
		std::chrono::time_point<std::chrono::system_clock> tp;

		ServiceData(std::string title, std::string msg, ServiceData::Type type);
	};

	class ServiceMsg :
		public eg::sys::NoCopyMove

	{
	public:
		static void init();
		static void shutdown();
		static ServiceMsg& instance();

		void log(std::string title, std::string msg, ServiceData::Type type);
		void reg_monitor(const char* registry, std::condition_variable*, std::function<void()> update_fn);
		void unreg_monitor(const char* registry);
		std::list<ServiceData> get_msgs();

	private:

		inline static std::unique_ptr<ServiceMsg> instance_;
		std::mutex msg_mutex_;
		std::list<ServiceData> msgs_;
		std::unordered_map<const char*, std::condition_variable*> msgs_cv_;
		std::unordered_map<const char*, std::function<void()>> msgs_fn_;

		ServiceMsg() = default;
	};
}