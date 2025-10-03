#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <pjsua2.hpp>
#include "PJCallManualDial.hpp"
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	class ServicePJCalls final :
		public eg::sys::NoCopyMove
	{
	public:

		std::unordered_map<int, std::shared_ptr<PJCallManualDial>> calls_;
		std::optional<int> active_call_id_;

		static void init();
		static ServicePJCalls& instance();
		static void shutdown();

		bool play_wav(const std::string& wav_filename);
		int  make_call(std::function<void(pjsip_inv_state, pj::CallInfo info, bool)> fn, const std::string& wav_filename, const std::string& mobile);
		void hangup_all_calls_except(int except_call);
		void hangup_all_calls();
		void hangup_and_remove_call(int call_id);
		void remove_call(int call_id);

	private:

		std::mutex call_mutex_;
		ServicePJCalls() = default;

		inline static std::unique_ptr<ServicePJCalls> instance_;
	};
}