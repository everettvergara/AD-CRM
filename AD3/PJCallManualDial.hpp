#pragma once

#include "PJCall.h"
#include <filesystem>
#include "Common/Log.hpp"
#include "ServicePJEndpoint.h"

namespace eg::ad3
{
	constexpr auto k_ringing_filename = "ringing.wav";

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
			// Create recording folder if not exists
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
			//PJSIP_INV_STATE_NULL,           /**< Before INVITE is sent or received  */
			//PJSIP_INV_STATE_CALLING,        /**< After INVITE is sent               */
			//PJSIP_INV_STATE_INCOMING,       /**< After INVITE is received.          */
			//PJSIP_INV_STATE_EARLY,          /**< After response with To tag.        */
			//PJSIP_INV_STATE_CONNECTING,     /**< After 2xx is sent/received.        */
			//PJSIP_INV_STATE_CONFIRMED,      /**< After ACK is sent/received.        */
			//PJSIP_INV_STATE_DISCONNECTED,   /**< Session is terminated.             */

			if (last_call_state == PJSIP_INV_STATE_EARLY)
			{
				// Play ringing
				const auto info = this->getInfo();
				auto t = get_active_media(info);
				if (t.has_value())
				{
					auto& ep = ServicePJEndpoint::instance();
					t->startTransmit(ep.get_speaker());
					is_ringing = true;
				}
			}
			else if
				(
					last_call_state == PJSIP_INV_STATE_CONNECTING or
					last_call_state == PJSIP_INV_STATE_CONFIRMED or
					last_call_state == PJSIP_INV_STATE_DISCONNECTED)
			{
				if (is_ringing)
				{
					const auto info = this->getInfo();
					auto t = get_active_media(info);
					if (t.has_value())
					{
						auto& ep = ServicePJEndpoint::instance();
						t->stopTransmit(ep.get_speaker());
					}
					is_ringing = false;
				}
			}
		}

		void on_call_state_disconnected() override
		{
			//LOG_II("DISCONNECTED_")/*;*/
		}

		void on_call_media_state_on_confirmed(pj::AudioMedia& media) override
		{
			//LOG_II("Call connected, setting up audio media and recording to {}", wav_filename_);
			recorder_.createRecorder(wav_filename_);

			// 1. Call audio -> Speaker (so YOU hear them)
			//LOG_II("1_");

			media.startTransmit(
				pj::Endpoint::instance().audDevManager().getPlaybackDevMedia()
			);

			// 2. Mic -> Call audio (so THEY hear you)
			//LOG_II("2_");

			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(media);

			// 3. Optional: Call audio -> Recorder
			//LOG_II("3_");

			media.startTransmit(recorder_);

			// 4. Optional: Mic -> Recorder (to capture your voice too)
			//LOG_II("5_");

			pj::Endpoint::instance().audDevManager()
				.getCaptureDevMedia().startTransmit(recorder_);

			//LOG_II("6_");
		}

	private:
		pj::AudioMediaRecorder recorder_;
		std::function<void(pjsip_inv_state)> on_status_change_callback_;
		std::string wav_filename_;
	};
}