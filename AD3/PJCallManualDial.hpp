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
	class PJCallManualDial final :
		public PJCall
	{
	public:
		PJCallManualDial(PJAccount& account, std::function<void(pjsip_inv_state, pj::CallInfo info)> fn, const std::string& wav_filename) :
			PJCall(account),
			on_status_change_callback_(std::move(fn)),
			wav_filename_(wav_filename),
			is_ringing(false)
		{
			std::filesystem::path file(wav_filename_);
			if (file.has_parent_path())
			{
				auto parent = file.parent_path();
				if (not std::filesystem::exists(parent))
				{
					create_directories(std::filesystem::path(wav_filename_).parent_path());
				}
			}
		}

		~PJCallManualDial()
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
			//LOG_II("PJCallManualDial::on_user_call_state_changed: BEFORE");
			on_status_change_callback_(last_call_state, info /* Copy */);
			//LOG_II("PJCallManualDial::on_user_call_state_changed: AFTER");

			if (not is_ringing and last_call_state == PJSIP_INV_STATE_EARLY)
			{
				//LOG_II("PJCallManualDial::on_user_call_state_changed: EARLY");
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
				//LOG_II("PJCallManualDial::on_user_call_state_changed: CONNECTING/CONFIRMED/DISCONN");
				stop_ringing();
			}
			//LOG_II("PJCallManualDial::on_user_call_state_changed: END");
		}

		void on_call_state_disconnected() override
		{
			//LOG_II("PJCallManualDial::on_user_call_state_changed: DISCONNECTED");
		}

		void on_call_media_state_on_confirmed(pj::AudioMedia& media) override
		{
			//LOG_II("PJCallManualDial::on_call_media_state_on_confirmed: {}", wav_filename_);

			recorder_.createRecorder(wav_filename_);

			// 1. Call audio -> Speaker (so YOU hear them)
			//LOG_II("PJCallManualDial::on_call_media_state_on_confirmed: 2");

			media.startTransmit(
				pj::Endpoint::instance().audDevManager().getPlaybackDevMedia()
			);

			// 2. Mic -> Call audio (so THEY hear you)
			//LOG_II("PJCallManualDial::on_call_media_state_on_confirmed: 3");

			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(media);

			// 3. Optional: Call audio -> Recorder
			//LOG_II("PJCallManualDial::on_call_media_state_on_confirmed: 4");

			media.startTransmit(recorder_);

			// 4. Optional: Mic -> Recorder (to capture your voice too)
			//LOG_II("PJCallManualDial::on_call_media_state_on_confirmed: 5");

			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(recorder_);
		}

	private:
		pj::AudioMediaRecorder recorder_;
		std::function<void(pjsip_inv_state, pj::CallInfo) > on_status_change_callback_;
		std::string wav_filename_;
	};
}