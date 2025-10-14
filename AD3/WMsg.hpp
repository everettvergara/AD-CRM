#pragma once

#include <string>
#include <vector>
#include <condition_variable>
#include <thread>
#include <format>
#include "Common/ServiceCtrlC.h"
#include "WChildFrame.h"
#include "ServiceMsg.h"

namespace eg::ad3
{
	class WMsg :
		public WChildFrame
	{
	public:
		WMsg(wxMDIParentFrame* parent) :
			WChildFrame
			(
				WChildProp
				{
					.parent = parent,
					.title = "Notification Messages",
					.pos = wxDefaultPosition,
					.size = wxSize(560, 700),
					.style = (wxDEFAULT_FRAME_STYLE & ~(wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU)) | wxSTAY_ON_TOP,
					.form_columns = 2,
					.has_tree = false
				}
			),
			has_update_(false),
			signal_exit_(false)

		{
			lts_.emplace_back(register_text_msg("title1", "msg1", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title2", "msg2", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title3", "msg3", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title4", "msg4", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title5", "msg5", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title6", "msg6", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title7", "msg7", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title8", "msg8", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title9", "msg9", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title10", "msg10", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title11", "msg11", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title12", "msg12", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title13", "msg13", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title14", "msg14", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title15", "msg15", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title16", "msg16", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title17", "msg17", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title18", "msg18", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title19", "msg19", "", "", wxDefaultPosition, wxDefaultSize));
			lts_.emplace_back(register_text_msg("title20", "msg20", "", "", wxDefaultPosition, wxDefaultSize));

			ServiceMsg::instance().reg_monitor("wmsg", &cv_, [this]
				{
					std::lock_guard lock(mutex_);
					has_update_ = true;
				});

			//update();

			updater_thread_ = std::thread([this]
				{
					do
					{
						std::unique_lock lock(mutex_);
						cv_.wait(lock, [&]
							{
								return signal_exit_ or has_update_;
							});

						if (signal_exit_)
						{
							break;
						}

						this->update();
						has_update_ = false;
					} while (true);
				});

			//std::this_thread::sleep_for(std::chrono::seconds(2));
			//ServiceMsg::instance().log("Hello Whatever", "This is a sample message .....", eg::ad3::ServiceData::Type::INFO);

			Show(true);
		}

		~WMsg()
		{
			//LOG_II("WMsg::~WMsg");

			{
				std::lock_guard lock(mutex_);
				signal_exit_ = true;
			}

			cv_.notify_all();

			ServiceMsg::instance().unreg_monitor("wmsg");
			updater_thread_.join();
			//LOG_II("WMsg::~WMsg() Existing Service...");
		}

		void update()
		{
			auto msgs = ServiceMsg::instance().get_msgs();

			for (size_t i = 0; auto& msg : msgs)
			{
				if (i < lts_.size())
				{
					auto tp = std::chrono::zoned_time{ std::chrono::current_zone(), msg.tp };
					lts_.at(i).label->SetLabel(msg.title);
					lts_.at(i).text->SetLabel(std::format(" {:%H:%M}: {}", tp, msg.msg));

					switch (msg.type)
					{
					case ServiceData::Type::ERR:
						lts_.at(i).label->SetForegroundColour(*wxRED);
						lts_.at(i).text->SetForegroundColour(*wxRED);
						break;
					case ServiceData::Type::INFO:
						lts_.at(i).label->SetForegroundColour(*wxBLUE);
						lts_.at(i).text->SetForegroundColour(*wxBLUE);
						break;
					case ServiceData::Type::WARNING:
						lts_.at(i).label->SetForegroundColour(*wxYELLOW);
						lts_.at(i).text->SetForegroundColour(*wxYELLOW);
						break;
					default:
						lts_.at(i).label->SetForegroundColour(*wxBLACK);
						lts_.at(i).text->SetForegroundColour(*wxBLACK);
						break;
					}
				}
				else
				{
					break;
				}

				++i;
			}

			panel->Refresh(true);
			panel->Update();
		}

	private:

		std::mutex mutex_;
		std::condition_variable cv_;
		std::vector<LabelText> lts_;
		std::thread updater_thread_;
		volatile bool has_update_;
		volatile bool signal_exit_;
	};
}