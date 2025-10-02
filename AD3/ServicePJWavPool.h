#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <pjsua2.hpp>
#include "Common/NoCopyMove.hpp"
#include "Common/Log.hpp"
#include "Common/ServiceCtrlC.h"

namespace eg::ad3
{
	class ServicePJWavPool final :
		public eg::sys::NoCopyMove
	{
	public:

		static void init();
		static ServicePJWavPool& instance();
		static void shutdown();

		~ServicePJWavPool()
		{
			if (pool_ not_eq nullptr)
			{
				pj_pool_release(pool_);
				pool_ = nullptr;
			}
		}

		bool play_local_ringback(int id)
		{
			std::lock_guard lock(wav_mutex_);
			if (playing_.contains(id))
			{
				return false;
			}

			if (not playing_.empty())
			{
				playing_.insert(id);

				return true;
			}

			playing_.insert(id);

			if (const pj_status_t status = pjmedia_tonegen_create(pool_, 8000, 1, 160, 16, PJMEDIA_TONEGEN_LOOP, &tone_port_);
				status not_eq PJ_SUCCESS)
			{
				return false;
			}

			pjmedia_tone_desc tones[1];
			tones[0].freq1 = 440;
			tones[0].freq2 = 480;
			tones[0].on_msec = 2000;
			tones[0].off_msec = 4000;
			tones[0].volume = PJMEDIA_TONEGEN_VOLUME;

			pjmedia_tonegen_play(tone_port_, 1, tones, PJMEDIA_TONEGEN_LOOP);

			pjsua_conf_add_port(pool_, tone_port_, &wav_port_);
			pjsua_conf_connect(wav_port_, 0);

			return true;
		}

		void stop(int id)
		{
			std::lock_guard lock(wav_mutex_);
			if (not playing_.contains(id))
			{
				return;
			}
			playing_.erase(id);

			if (not playing_.empty())
			{
				return;
			}

			if (on_complete_ not_eq nullptr)
			{
				on_complete_();
				on_complete_ = nullptr;
			}

			pjsua_conf_disconnect(wav_port_, 0);
			pjsua_conf_remove_port(wav_port_);
			pjmedia_tonegen_stop(tone_port_);
			pjmedia_port_destroy(tone_port_);
			wav_port_ = PJSUA_INVALID_ID;
			tone_port_ = nullptr;

			if (play_thread_.joinable())
			{
				play_thread_.join();
				play_thread_ = std::thread();
			}
		}

	private:
		pj_pool_t* pool_;
		pjsua_conf_port_id wav_port_;
		pjmedia_port* tone_port_;

		std::unordered_set<int> playing_;
		std::function<void()> on_complete_;
		std::thread play_thread_;

		std::mutex wav_mutex_;
		ServicePJWavPool() :
			pool_(pjsua_pool_create("wavpool", 512, 512)),
			wav_port_(PJSUA_INVALID_ID),
			tone_port_{},
			on_complete_(nullptr)
		{
		}

		inline static std::unique_ptr<ServicePJWavPool> instance_;
	};
}