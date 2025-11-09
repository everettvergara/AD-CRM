#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <pjsua2.hpp>
#include "PJCallRoboDial.hpp"
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	class ServicePJRoboCalls final :
		public eg::sys::NoCopyMove
	{
	public:

		std::unordered_map<int, std::shared_ptr<PJCallRoboDial>> calls_;
		std::optional<int> active_call_id_;

		~ServicePJRoboCalls();

		static void init();
		static ServicePJRoboCalls& instance();
		static void shutdown();

		bool play_wav(const std::string& wav_filename);
		int  make_call(std::function<void(pjsip_inv_state, pj::CallInfo info, bool)> fn, const std::string& record_wav_filename, const std::string& player_wav_filename, const std::string& mobile, size_t account_ix);
		//void hangup_all_calls_except(int except_call);
		void hangup_all_calls();
		void hangup_and_remove_call(int call_id);
		void remove_call(int call_id);

	private:

		std::mutex call_mutex_;
		ServicePJRoboCalls() = default;

		inline static std::unique_ptr<ServicePJRoboCalls> instance_;
	};
}