#pragma once

#include "PJCall.h"
#include <filesystem>
#include "Common/Log.hpp"
#include "ServicePJEndpoint.h"
#include <pjmedia/wav_port.h>

namespace eg::ad3
{
	class PJCallManualDial final :
		public PJCall
	{
	public:
		PJCallManualDial(PJAccount& account, std::function<void(pjsip_inv_state)> fn, const std::string& wav_filename) :
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

	protected:
		bool is_ringing;
		void on_user_call_state_changed() override
		{
			on_status_change_callback_(last_call_state);

			if (last_call_state == PJSIP_INV_STATE_EARLY)
			{
				play_local_ringback();
				is_ringing = true;
			}
			else if (is_ringing and (
				last_call_state == PJSIP_INV_STATE_CONNECTING or
				last_call_state == PJSIP_INV_STATE_CONFIRMED or
				last_call_state == PJSIP_INV_STATE_DISCONNECTED
				))
			{
				stop_local_ringback();
				is_ringing = false;
			}
		}

		void on_call_state_disconnected() override
		{
		}

		void on_call_media_state_on_confirmed(pj::AudioMedia& media) override
		{
			recorder_.createRecorder(wav_filename_);

			// 1. Call audio -> Speaker (so YOU hear them)
			media.startTransmit(
				pj::Endpoint::instance().audDevManager().getPlaybackDevMedia()
			);

			// 2. Mic -> Call audio (so THEY hear you)
			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(media);

			// 3. Optional: Call audio -> Recorder
			media.startTransmit(recorder_);

			// 4. Optional: Mic -> Recorder (to capture your voice too)
			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(recorder_);
		}

	private:
		pj::AudioMediaRecorder recorder_;
		std::function<void(pjsip_inv_state)> on_status_change_callback_;
		std::string wav_filename_;
	};
}