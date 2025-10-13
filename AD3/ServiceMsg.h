#pragma once

#include <memory>
#include <mutex>
#include <list>
#include <string>
#include <chrono>
#include <unordered_map>
#include <condition_variable>
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	constexpr size_t k_service_msg_max = 5;

	struct ServiceData
	{
		std::string title;
		std::string msg;
		enum struct Type { INFO, WARNING, ERROR, CRITICAL } type;
		std::chrono::time_point<std::chrono::system_clock> tp;

		ServiceData(std::string title, std::string msg, ServiceData::Type type);
	};

	class SerivceMsg :
		public eg::sys::NoCopyMove

	{
		static void init();
		static void shutdown();
		static SerivceMsg& instance();

		void log(std::string title, std::string msg, ServiceData::Type type);
		void reg_monitor(const char* registry, std::condition_variable*);
		void unreg_monitor(const char* registry);

	private:

		inline static std::unique_ptr<SerivceMsg> instance_;
		std::mutex msg_mutex_;
		std::list<ServiceData> msgs_;
		std::unordered_map<const char*, std::condition_variable*> msgs_cv_;

		SerivceMsg() = default;
	};
}