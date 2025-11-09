#pragma once

#include "PJCall.h"
#include <filesystem>
//#include <pjmedia/wav_port.h>
#include "Common/Log.hpp"
#include "ServicePJEndpoint.h"
#include "ServicePJWavPool.h"
#include "PJHelper.h"

namespace eg::ad3
{
	class RoboPlayer : public pj::AudioMediaPlayer
	{
	public:

		std::function<void()> on_done_;

		void onEof2() override
		{
			if (on_done_)
			{
				on_done_();
			}
		}
	};

	class PJCallRoboDial final :
		public PJCall
	{
	public:
		PJCallRoboDial(PJAccount& account, std::function<void(pjsip_inv_state, pj::CallInfo info, bool)> fn, const std::string& recorder_wav_filename, const std::string& player_wav_filename) :
			PJCall(account),
			on_status_change_callback_(std::move(fn)),
			is_ringing(false),
			recorder_wav_filename_(recorder_wav_filename),
			player_wav_filename_(player_wav_filename)
		{
			std::filesystem::path file(recorder_wav_filename_);
			if (file.has_parent_path())
			{
				auto parent = file.parent_path();
				if (not std::filesystem::exists(parent))
				{
					create_directories(std::filesystem::path(recorder_wav_filename_).parent_path());
				}
			}
		}

		~PJCallRoboDial()
		{
			if (is_ringing)
			{
				ServicePJWavPool::instance().stop(this->getId());
			}
		}

		void stop_ringing()
		{
			ServicePJWavPool::instance().stop(this->getId());
			is_ringing = false;
		}

	protected:
		bool is_ringing;
		void on_user_call_state_changed(const pj::CallInfo& info) override
		{
			//LOG_II("PJCallRoboDial::on_user_call_state_changed: BEFORE");
			on_status_change_callback_(last_call_state, info /* Copy */, this->hangup_requested);
			//LOG_II("PJCallRoboDial::on_user_call_state_changed: AFTER");

			if (not is_ringing and last_call_state == PJSIP_INV_STATE_EARLY)
			{
				//LOG_II("PJCallRoboDial::on_user_call_state_changed: EARLY");
				//play_local_ringback();
				ServicePJWavPool::instance().play_local_ringback(this->getId());
				is_ringing = true;
			}
			else if (is_ringing and (
				last_call_state == PJSIP_INV_STATE_CONNECTING or
				last_call_state == PJSIP_INV_STATE_CONFIRMED or
				last_call_state == PJSIP_INV_STATE_DISCONNECTED
				))
			{
				//LOG_II("PJCallRoboDial::on_user_call_state_changed: CONNECTING/CONFIRMED/DISCONN");
				stop_ringing();
			}
			//LOG_II("PJCallRoboDial::on_user_call_state_changed: END");
		}

		void on_call_state_disconnected() override
		{
			//LOG_II("PJCallRoboDial::on_user_call_state_changed: DISCONNECTED");
		}

		void on_call_media_state_on_confirmed(pj::AudioMedia& media) override
		{
			// 0.0 Create the effin recorder
			recorder_.createRecorder(recorder_wav_filename_);

			// 1. Call audio -> recorder
			media.startTransmit(recorder_);

			// 2. Player -> Call audio
			player_.createPlayer(player_wav_filename_, PJMEDIA_FILE_NO_LOOP);
			player_.startTransmit(media);

			// 4. Player -> Recorder
			player_.startTransmit(recorder_);

			player_.on_done_ = [this]()
				{
					this->hangup_call();
				};
		}

	private:
		pj::AudioMediaRecorder recorder_;
		RoboPlayer player_;

		std::function<void(pjsip_inv_state, pj::CallInfo, bool) > on_status_change_callback_;
		std::string recorder_wav_filename_;
		std::string player_wav_filename_;
	};
}