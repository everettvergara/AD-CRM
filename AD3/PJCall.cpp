#include "PJCall.h"

#include <stdexcept>
#include "ServicePJEndpoint.h"
#include "PJHelper.h"
#include "ConfigSIP.hpp"
#include "ServiceCtrlC.h"
#include "ServiceCallManager.h"

namespace eg::net
{
	PJCall::PJCall(PJAccount& account, std::chrono::seconds timeout_secs, nlohmann::json call_request, RemoveHandler remove_handler) :
		pj::Call(*dynamic_cast<pj::Account*>(&account)),
		last_call_state_(PJSIP_INV_STATE_NULL),
		last_call_state_before_disconnected_(PJSIP_INV_STATE_NULL),
		call_request_(std::move(call_request)),
		remove_handler_(std::move(remove_handler)),
		timeout_secs_(timeout_secs)
	{
		timeout_thread_ = std::thread([this]
			{
				register_current_thread_in_pj("PJCall:TimeoutThread");

				const auto& config = sys::Config<ConfigSIP>::instance();

				std::unique_lock lock(mutex_);
				cv_.wait_for(lock, timeout_secs_, [this]
					{
						if (last_call_state_ == PJSIP_INV_STATE_NULL)
						{
							return false;
						}

						return true;
					});
				lock.unlock();

				if (last_call_state_ == PJSIP_INV_STATE_CALLING or
					last_call_state_ == PJSIP_INV_STATE_EARLY or
					last_call_state_ == PJSIP_INV_STATE_CONNECTING)
				{
					this->hangup_call();
					//LOG_DD("PJCall::PJCall: \033[32m{} \033[0m Timeout detected. Hangging up.", call_request_.at("mobile").get_ref<const std::string&>());
					//this->hangup(pj::CallOpParam(true));
				}
			});
	}

	PJCall::~PJCall()
	{
		LOG_DD("PJCall::~PJCall: \033[32m{}\033[0m", call_request_.at("mobile").get_ref<const std::string&>());

		timeout_thread_.join();
		wait_until_state_is_disconnected();

		auto session_id = call_request_.at("session_id").get<size_t>();
		auto call_id = call_request_.at("id").get<size_t>();

		transfer_call_id_(session_id, call_id, last_call_state_before_disconnected_, last_call_state_before_disconnected_);
	}

	void PJCall::wait_until_state_is_disconnected()
	{
		// No Ctrl-C waiting
		// Just plain PJSIP_INV_STATE_DISCONNECTED
		// All states eventually fall to PJSIP_INV_STATE_DISCONNECTED

		std::unique_lock lock(mutex_);
		cv_.wait(lock, [this]
			{
				return last_call_state_ == PJSIP_INV_STATE_DISCONNECTED;
			});
	}

	void PJCall::hangup_call()
	{
		// Expected to be called in another thread
		// Call this if user presses Ctrl-C
		LOG_DD("PJCall::hangup_call: \033[32m{}\033[0m {} {}", call_request_.at("mobile").get_ref<const std::string&>(), this->getId(), (int)last_call_state_);

		{
			std::lock_guard lock(mutex_);
			if (last_call_state_ == PJSIP_INV_STATE_DISCONNECTED)
			{
				return;
			}
		}

		register_current_thread_in_pj("PJCall:HangupCall");
		this->hangup(pj::CallOpParam(true));
	}

	std::optional<pj::AudioMedia> PJCall::get_active_media(const pj::CallInfo& info)
	{
		for (auto& media : info.media)
		{
			if (media.type == PJMEDIA_TYPE_AUDIO and media.status == PJSUA_CALL_MEDIA_ACTIVE)
			{
				if (auto am = this->getAudioMedia(media.index); am.getPortId() >= 0)
				{
					return am;
				}
			}
		}

		return {};
	}

	void PJCall::onCallState(pj::OnCallStateParam&)
	{
		const auto info = this->getInfo();

		{
			std::lock_guard lock(mutex_);
			last_call_state_ = info.state;

			//if (last_call_state_ not_eq PJSIP_INV_STATE_DISCONNECTED)
			//{
			auto session_id = call_request_.at("session_id").get<size_t>();
			auto call_id = call_request_.at("id").get<size_t>();

			transfer_call_id_(session_id, call_id, last_call_state_before_disconnected_, last_call_state_);

			last_call_state_before_disconnected_ = last_call_state_;
			//}
		}

		LOG_DD("PJCall::onCallState: \033[31m{}\033[0m - \033[32m{}\033[0m", info.stateText, call_request_.at("mobile").get_ref<const std::string&>());

		if (info.state == PJSIP_INV_STATE_DISCONNECTED)
		{
			on_call_state_disconnected_();
		}

		if (last_call_state_ == PJSIP_INV_STATE_CONFIRMED)
		{
			cv_.notify_all();
		}

		else if (last_call_state_ == PJSIP_INV_STATE_DISCONNECTED)
		{
			cv_.notify_all();
			remove_handler_(shared_from_this());
		}
	}

	void PJCall::onCallMediaState(pj::OnCallMediaStateParam&)
	{
		//LOG_TT("PJCall::onCallMediaState: \033[32m{}\033[0m", call_request_.at("mobile").get_ref<const std::string&>());

		const auto info = this->getInfo();

		if (info.state == PJSIP_INV_STATE_CONFIRMED)
		{
			if (auto active_media = get_active_media(info);
				active_media.has_value())
			{
				on_call_media_state_on_confirmed_(active_media.value());
				return;
			}

			LOG_INSTANCE;
			LOG_CC("PJCall::onCallMediaState: Invalid media detected.");

			hangup_call();
		}
	}

	void PJCall::on_call_state_disconnected_()
	{
		// Clean-up resources
		//register_current_thread_in_pj("PJCall:onCallMediaState");
	}

	void PJCall::on_call_media_state_on_confirmed_(pj::AudioMedia&)
	{
		// Init resources
	}

	void PJCall::transfer_call_id_(size_t session_id, size_t call_id, pjsip_inv_state before, pjsip_inv_state after)
	{
		const char* from = [before]() -> const char*
			{
				switch (before)
				{
				case PJSIP_INV_STATE_NULL:
					return k_call_manager_tried_folder;

				case PJSIP_INV_STATE_CALLING:
					return k_call_manager_calling_folder;

				case PJSIP_INV_STATE_EARLY:
					return k_call_manager_early_folder;

				case PJSIP_INV_STATE_CONNECTING:
					return k_call_manager_connecting_folder;

				default:
					return nullptr;
				}
			}();

		const char* to = [after]() -> const char*
			{
				switch (after)
				{
				case PJSIP_INV_STATE_NULL:
					return k_call_manager_others_folder;

				case PJSIP_INV_STATE_CALLING:
					return k_call_manager_calling_folder;

				case PJSIP_INV_STATE_EARLY:
					return k_call_manager_early_folder;

				case PJSIP_INV_STATE_CONNECTING:
					return k_call_manager_connecting_folder;

				case PJSIP_INV_STATE_CONFIRMED:
					return k_call_manager_confirmed_folder;

				default:
					return nullptr;
				}
			}();

		if (from and to and from not_eq to)
		{
			ServiceCallManager::transfer_call_id(session_id, from, to, call_id);
		}
	}
}