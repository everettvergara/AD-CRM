#include "WDialer.h"
#include <fstream>
#include <format>
#include <tuple>
#include <chrono>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <miniaudio.h>

#include <nanodbc/nanodbc.h>
#include "Common/StringHelper.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
	WDialer::WDialer(wxMDIParentFrame* parent) :
		WChildFrame(
			WChildProp
			{
				.parent = parent,
				.title = "Auto / Manual Dialer",
				.pos = wxDefaultPosition,
				.size = wxSize(800, 800),
				.style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX),
				.form_columns = 2,
				.has_tree = true
			}),
		tree_(nullptr),
		id_(nullptr),
		ucode_(nullptr),
		mobile_(nullptr),
		status_(nullptr),
		new_button_(nullptr),
		call_button_(nullptr),
		call_again_button_(nullptr),
		stop_button_(nullptr),
		save_button_(nullptr),
		cancel_button_(nullptr),
		data_()

	{
		on_init_filter_controls_();
		on_init_input_controls_();
		on_init_tree_();
		on_init_buttons_();
		update_components_state_();

		Show(true);
	}

	void WDialer::update_components_from_data_()
	{
		if (not filter_.is_manual)
		{
			filter_to_call_count_->SetValue(std::to_string(filter_.selected_status->to_call_count()));
		}

		id_->SetValue(data_.id);
		ucode_->SetValue(data_.ucode);
		mobile_->SetValue(data_.mobile);
		name_->SetValue(data_.name);
		status_->SetValue(data_.status);
		time_of_call_->SetValue(data_.time_of_call);
		time_call_ended_->SetValue(data_.time_call_ended);
		remarks_->SetValue(data_.remarks);

		if (data_.has_confirmed_status())
		{
			file_recording_->SetValue(data_.file_recording);
		}
		else
		{
			file_recording_->SetValue("");
		}
	}

	void WDialer::update_components_state_()
	{
		switch (data_.state)
		{
		case DialerState::New:

			if (filter_.is_manual)
			{
				filter_is_manual_->Enable();
				filter_client_->Disable();
				filter_campaign_->Disable();
				filter_prio_->Disable();
				filter_status_->Disable();

				mobile_->Enable();
				name_->Enable();
				remarks_->Enable();
			}
			else
			{
				filter_is_manual_->Enable();
				filter_client_->Enable();
				filter_campaign_->Enable();
				filter_prio_->Enable();
				filter_status_->Enable();

				mobile_->Disable();
				name_->Disable();
				remarks_->Disable();
			}

			id_->Disable();
			ucode_->Disable();
			tree_->Enable();

			playback_button_->Disable();
			new_button_->Disable();
			call_button_->Enable();
			call_again_button_->Disable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Enable();
			break;

		case DialerState::Calling:

			filter_is_manual_->Disable();
			filter_client_->Disable();
			filter_campaign_->Disable();
			filter_prio_->Disable();
			filter_status_->Disable();

			id_->Disable();
			ucode_->Disable();
			mobile_->Disable();
			name_->Disable();
			remarks_->Enable();
			tree_->Disable();

			playback_button_->Disable();
			new_button_->Disable();
			call_button_->Disable();
			call_again_button_->Disable();
			stop_button_->Enable();
			save_button_->Disable();
			cancel_button_->Disable();
			break;

		case DialerState::Stopping:

			filter_is_manual_->Disable();
			filter_client_->Disable();
			filter_campaign_->Disable();
			filter_prio_->Disable();
			filter_status_->Disable();

			id_->Disable();
			ucode_->Disable();
			mobile_->Disable();
			name_->Disable();
			remarks_->Enable();
			tree_->Disable();

			playback_button_->Disable();
			new_button_->Disable();
			call_button_->Disable();
			call_again_button_->Disable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Disable();

		case DialerState::JustEnded:

			if (filter_.is_manual)
			{
				filter_is_manual_->Enable();
				filter_client_->Disable();
				filter_campaign_->Disable();
				filter_prio_->Disable();
				filter_status_->Disable();
				remarks_->Enable();
			}
			else
			{
				filter_is_manual_->Enable();
				filter_client_->Enable();
				filter_campaign_->Enable();
				filter_prio_->Enable();
				filter_status_->Enable();
				remarks_->Disable();
			}

			id_->Disable();
			ucode_->Disable();
			mobile_->Disable();
			name_->Disable();

			tree_->Enable();

			playback_button_->Enable(data_.has_confirmed_status());
			new_button_->Enable();
			call_button_->Disable();
			call_again_button_->Enable();
			stop_button_->Disable();
			save_button_->Enable();
			cancel_button_->Enable();
			break;

		case DialerState::PlayingWav:

			filter_is_manual_->Disable();
			filter_client_->Disable();
			filter_campaign_->Disable();
			filter_prio_->Disable();
			filter_status_->Disable();

			id_->Disable();
			ucode_->Disable();
			mobile_->Disable();
			name_->Disable();
			remarks_->Disable();
			tree_->Disable();

			playback_button_->Disable();
			new_button_->Disable();
			call_button_->Disable();
			call_again_button_->Disable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Disable();

		case DialerState::Saved:

			if (filter_.is_manual)
			{
				filter_is_manual_->Enable();
				filter_client_->Disable();
				filter_campaign_->Disable();
				filter_prio_->Disable();
				filter_status_->Disable();
			}
			else
			{
				filter_is_manual_->Enable();
				filter_client_->Enable();
				filter_campaign_->Enable();
				filter_prio_->Enable();
				filter_status_->Enable();
			}

			id_->Disable();
			ucode_->Disable();
			mobile_->Disable();
			name_->Disable();
			remarks_->Disable();
			tree_->Enable();

			playback_button_->Enable(data_.has_confirmed_status());
			new_button_->Enable();

			if (filter_.is_manual)
			{
				call_button_->Disable();
			}
			else
			{
				call_button_->Enable();
			}

			call_again_button_->Enable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Enable();
			break;
		}
	}

	void WDialer::on_init_tree_()
	{
		if (not std::filesystem::exists(k_calls_folder))
		{
			if (not std::filesystem::create_directory(k_calls_folder))
			{
				throw std::runtime_error("Could not create the calls folder.");
			}
		}

		tree_ = register_tree("history", 400);

		tree_->Bind(wxEVT_TREE_SEL_CHANGED, [this](wxTreeEvent& e)
			{
				const auto item_id = e.GetItem();

				auto meta = static_cast<DirMeta*>(tree_->GetItemData(e.GetItem()));
				if (not meta->is_file and not meta->visited)
				{
					meta->visited = true;
					if (const auto children_count = tree_->GetChildrenCount(item_id); children_count == 0)
					{
						this->register_node_elements_(item_id, meta->path);

						if (tree_->GetChildrenCount(item_id) > 0)
						{
							tree_->Expand(item_id);
						}
					}
				}

				else if (meta->is_file)
				{
					data_.from_json([&meta]
						{
							std::ifstream file(meta->path);
							return nlohmann::json::parse(file);;
						}());

					update_components_from_data_();
					update_components_state_();
				}
			});

		const auto root_id = tree_->AddRoot(k_calls_folder, -1, -1, new DirMeta(k_calls_folder));

		register_node_elements_(root_id, k_calls_folder);
	}

	void WDialer::on_init_buttons_()
	{
		new_button_ = register_button("New", wxID_ANY);
		new_button_->Bind(wxEVT_BUTTON, &WDialer::on_new_, this);
		call_button_ = register_button("Call", wxID_ANY);
		call_button_->Bind(wxEVT_BUTTON, &WDialer::on_call_, this);
		call_again_button_ = register_button("Call Again", wxID_ANY);
		call_again_button_->Bind(wxEVT_BUTTON, &WDialer::on_call_again_, this);
		stop_button_ = register_button("Stop", wxID_ANY);
		stop_button_->Bind(wxEVT_BUTTON, &WDialer::on_stop_, this);
		stop_button_->Disable();
		save_button_ = register_button("Save", wxID_ANY);
		save_button_->Bind(wxEVT_BUTTON, &WDialer::on_save_, this);
		cancel_button_ = register_button("Cancel", wxID_ANY);
		cancel_button_->Bind(wxEVT_BUTTON, &WDialer::on_close_, this);
	}

	void WDialer::on_init_filter_controls_()
	{
		// Filters
		filter_is_manual_ = register_checkbox("is_manual", "Manual dialer?:", true);
		filter_is_manual_->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e)
			{
				filter_.is_manual = e.IsChecked();

				if (not filter_.is_manual)
				{
					populate_master_();
				}

				update_components_state_();
			});

		filter_client_ = register_dropdown("filter_client", "Select Client:");
		filter_client_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto client_id = reinterpret_cast<size_t>(ptr);

				auto fclient = std::find_if(filter_.clients.begin(), filter_.clients.end(), [client_id](const ClientDD& a) -> bool
					{
						return a.client_id == client_id;
					});

				filter_.selected_client = &(*fclient);

				filter_campaign_->Clear();
				filter_prio_->Clear();
				filter_status_->Clear();

				filter_.selected_campaign = nullptr;
				filter_.selected_prio = nullptr;
				filter_.selected_status = nullptr;

				for (const auto& [campaign_id, _] : fclient->campaigns)
				{
					filter_campaign_->Append(filter_.campaign_master.at(campaign_id), reinterpret_cast<void*>(campaign_id));
				}

				data_.clear();
			});

		filter_campaign_ = register_dropdown("filter_client_campaign", "Select Campaign:");
		filter_campaign_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto campaign_id = reinterpret_cast<size_t>(ptr);

				auto fcampaign = std::find_if(filter_.selected_client->campaigns.begin(), filter_.selected_client->campaigns.end(), [campaign_id](const CampaignDD& a) -> bool
					{
						return a.campaign_id == campaign_id;
					});

				filter_.selected_campaign = &(*fcampaign);

				filter_prio_->Clear();
				filter_status_->Clear();
				filter_.selected_prio = nullptr;
				filter_.selected_status = nullptr;

				for (const auto& [prio_id, _] : fcampaign->prios)
				{
					filter_prio_->Append(filter_.prio_master.at(prio_id), reinterpret_cast<void*>(prio_id));
				}

				data_.clear();
			});

		filter_prio_ = register_dropdown("filter_prio", "Select Prio:");
		filter_prio_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto prio_id = reinterpret_cast<size_t>(ptr);
				auto fprio = std::find_if(filter_.selected_campaign->prios.begin(), filter_.selected_campaign->prios.end(), [prio_id](const PrioDD& a) -> bool
					{
						return a.prio_id == prio_id;
					});

				filter_.selected_prio = &(*fprio);

				filter_status_->Clear();
				filter_.selected_status = nullptr;

				for (const auto& [status_id, _, __, ___, ____] : fprio->status_series)
				{
					filter_status_->Append(filter_.status_master.at(status_id), reinterpret_cast<void*>(status_id));
				}

				data_.clear();
			});

		filter_status_ = register_dropdown("filter_status", "Select Status:");
		filter_status_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto status_id = reinterpret_cast<size_t>(ptr);
				auto fstatus = std::find_if(filter_.selected_prio->status_series.begin(), filter_.selected_prio->status_series.end(), [status_id](const StatusSeriesDD& a) -> bool
					{
						return a.status_id == status_id;
					});

				filter_.selected_status = &(*fstatus);

				filter_to_call_count_->SetValue(std::to_string(filter_.selected_status->to_call_count()));

				data_.clear();
				data_.ucode = filter_.selected_status->ucode;

				update_components_from_data_();
			});

		filter_to_call_count_ = register_text_input("to_call_count", "To Call Count:", "0", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	}

	void WDialer::on_init_input_controls_()
	{
		// Fields
		register_text("Call details", 300);
		id_ = register_text_input("id", "CM ID:", data_.id);
		ucode_ = register_text_input("ucode", "Code:", data_.ucode);
		register_text("Please input mobile in the following format: 0XXXYYYZZZZ i.e. 09177101995.", 300);
		mobile_ = register_text_input("mobile", "Mobile to dial:", "");
		mobile_->SetHint("09177101995");
		name_ = register_text_input("name", "Name:", "");
		name_->SetHint("Juan dela Cruz");
		status_ = register_text_input("status", "Last Status:", "PJSIP_INV_STATE_NULL", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_of_call_ = register_text_input("time_of_call", "Time of Call:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_call_ended_ = register_text_input("time_call_ended", "Time Call Ended:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		remarks_ = register_text_input_multi("remarks", "Remarks:", 5);
		remarks_->SetHint("Enter details of your conversation here...");
		file_recording_ = register_text_input("wav_recording", "Playback file:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		playback_button_ = register_button_field("Play");
		playback_button_->Bind(wxEVT_BUTTON, &WDialer::on_playback_, this);
	}

	void WDialer::register_node_elements_(const wxTreeItemId& node_id, const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			const auto p = entry.path();
			tree_->AppendItem(node_id, p.filename().string(), -1, -1, new DirMeta(p.string(), std::filesystem::is_regular_file(p)));
		}
	}

	void WDialer::on_call_state_changed_(pjsip_inv_state state)
	{
		wxTheApp->CallAfter([this, state]
			{
				switch (state)
				{
				case PJSIP_INV_STATE_CALLING:
					data_.status = "PJSIP_INV_STATE_CALLING";
					break;

				case PJSIP_INV_STATE_INCOMING:
					data_.status = "PJSIP_INV_STATE_INCOMING";
					break;

				case PJSIP_INV_STATE_EARLY:
					data_.status = "PJSIP_INV_STATE_EARLY";
					break;

				case PJSIP_INV_STATE_CONNECTING:
					data_.status = "PJSIP_INV_STATE_CONNECTING";
					break;

				case PJSIP_INV_STATE_CONFIRMED:
					data_.status = "PJSIP_INV_STATE_CONFIRMED";
					break;

				case PJSIP_INV_STATE_DISCONNECTED:
				{
					auto should_stop_auto = (data_.state == DialerState::Stopping);

					{
						std::lock_guard lock(call_mutex_);;
						current_call_.reset();
						data_.time_call_ended = eg::string::datetime_to_formatted_string();
						data_.state = DialerState::JustEnded;
						time_call_ended_->SetValue(data_.time_call_ended);

						if (data_.has_confirmed_status())
						{
							file_recording_->SetValue(data_.file_recording);
						}
					}

					if (filter_.is_manual)
					{
						update_components_state_();
						wxMessageBox("Call ended", "Info", wxOK | wxICON_INFORMATION);
					}
					else
					{
						save_();

						// If user did not stop the auto dialer...
						if (not should_stop_auto)
						{
							// If the last call was answered...
							// wait for the user to update the contract master first.
							if (data_.has_confirmed_status())
							{
								wxMessageBox("Press Call once you are done updating the contract master.", "Info", wxOK | wxICON_INFORMATION);
							}

							// Otherwise proceed to the next call.
							else
							{
								on_call_auto_();
							}
						}

						update_components_state_();
					}

					break;
				}

				default:
					break;
				}

				status_->SetValue(data_.status);
			});
	}

	void WDialer::on_call_(wxCommandEvent&)
	{
		if (filter_.is_manual)
		{
			on_call_manual_();
		}
		else
		{
			on_call_auto_();
		}
	}

	void WDialer::on_call_auto_()
	{
		if (filter_.selected_status->to_call_count() == 0)
		{
			wxMessageBox("No more records to call for this option.", "Info", wxOK | wxICON_INFORMATION, this);
			return;
		}

		const auto retrieve_sql = std::format("select a.cm_id, a.contact_name, a.mobile, a.remarks from vw_ad_lr_priority_new as a where next_id = {}", filter_.selected_status->next_id);

		auto db_conn = std::format("Driver={};Server={};Database={};UID={};PWD={};", k_db_driver, k_db_server, k_db_database, k_db_user, k_db_password);
		nanodbc::connection conn(NANODBC_TEXT(db_conn));

		if (not conn.connected())
		{
			wxMessageBox(std::format("Could not connect to database: {}", db_conn), "Database Connection", wxOK | wxICON_ERROR, this);
			return;
		}

		nanodbc::result results = nanodbc::execute(
			conn,
			NANODBC_TEXT(retrieve_sql));

		data_.clear();
		if (results.next())
		{
			data_.id = results.get<std::string>(0);
			data_.name = results.get<std::string>(1);
			data_.mobile = results.get<std::string>(2);
			data_.remarks = results.get<std::string>(3);
			data_.ucode = filter_.selected_status->ucode;
			auto next_id = filter_.selected_status->next_id;
			nanodbc::statement stmt(conn);
			auto update_sql = std::format("UPDATE tb_ad_cache_priority_series SET next_id = {} WHERE {} between min_id and max_id", next_id + 1, next_id);
			prepare(stmt, NANODBC_TEXT(update_sql));

			if (auto result = execute(stmt);
				result.affected_rows() == 0)
			{
				conn.disconnect();
				wxMessageBox("Could not update the next_id", "Database Error", wxOK | wxICON_ERROR, this);
				return;
			}

			filter_.selected_status->next_id = next_id + 1;
			update_components_from_data_();
		}
		else
		{
			wxMessageBox("No record retrieved.", "Database Error", wxOK | wxICON_ERROR, this);
			return;
		}

		const auto validated_name = DialerData::trimmed_name(data_.name);

		call_proper_(validated_name);
	}

	void WDialer::on_call_manual_()
	{
		if (auto err = update_data_from_components_(); err not_eq nullptr)
		{
			wxMessageBox(err, "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		const auto validated_name = DialerData::trimmed_name(data_.name);
		if (validated_name.empty())
		{
			wxMessageBox("Invalid name.", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		data_.ucode = "NA";
		ucode_->SetValue(data_.ucode);

		call_proper_(validated_name);
	}

	void WDialer::call_proper_(const std::string& validated_name)
	{
		{
			data_.file_recording = generate_wav_filename(data_.mobile, validated_name);
			std::lock_guard lock(call_mutex_);

			current_call_ = std::make_shared<PJCallManualDial>(
				ServicePJAccount::instance().account,
				std::bind(&WDialer::on_call_state_changed_, this, std::placeholders::_1),
				data_.file_recording);

			current_call_->makeCall(std::format("sip:{}@{}", data_.mobile, ConfigSettings::instance().server_ip), []
				{
					pj::CallOpParam p(true);
					p.opt.audioCount = 1;
					p.opt.videoCount = 0;
					return p;
				}());
		}

		data_.time_of_call = eg::string::datetime_to_formatted_string();
		time_of_call_->SetValue(data_.time_of_call);
		data_.state = DialerState::Calling;

		update_components_state_();
	}

	void WDialer::on_call_again_(wxCommandEvent& e)
	{
		if (data_.state == DialerState::JustEnded)
		{
			if (wxMessageBox("Save this record before proceeding with the call?", "Confirmation", wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION) == wxYES)
			{
				save_();
			}
		}

		data_.state = DialerState::New;
		data_.status = "PJSIP_INV_STATE_NULL";
		data_.time_of_call.clear();
		data_.time_call_ended.clear();
		data_.file_recording.clear();
		time_of_call_->SetValue("");
		time_call_ended_->SetValue("");
		file_recording_->SetValue("");
		//update_components_state_();

		on_call_manual_();
	}

	void WDialer::on_new_(wxCommandEvent&)
	{
		data_.clear();

		update_components_from_data_();
		update_components_state_();
	}

	void WDialer::on_stop_(wxCommandEvent&)
	{
		{
			std::lock_guard lock(call_mutex_);
			current_call_->hangup_call();
		}

		data_.state = DialerState::Stopping;

		update_components_state_();
	}

	const char* WDialer::update_data_from_components_()
	{
		const auto validated_mobile = DialerData::validated_mobile(mobile_->GetValue().ToStdString());
		if (validated_mobile.empty())
		{
			return "Invalid mobile number format!";
		}

		const auto name = name_->GetValue().ToStdString();
		const auto validated_name = DialerData::trimmed_name(name);
		if (validated_name.empty())
		{
			return "Name is invalid.";
		}

		data_.mobile = validated_mobile;
		data_.name = name;
		data_.status = status_->GetValue().ToStdString();
		data_.time_of_call = time_of_call_->GetValue().ToStdString();
		data_.time_call_ended = time_call_ended_->GetValue().ToStdString();
		data_.remarks = remarks_->GetValue().ToStdString();

		return nullptr;
	}

	void WDialer::on_save_(wxCommandEvent&)
	{
		// Validation

		if (const auto err = update_data_from_components_(); err not_eq nullptr)
		{
			wxMessageBox(err, "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		if (data_.time_call_ended.empty())
		{
			wxMessageBox("Cannot save a record that has not been called.", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		save_();

		update_components_state_();

		wxMessageBox("Call record saved", "Info", wxOK | wxICON_INFORMATION, this);
	}

	void WDialer::save_()
	{
		const auto [meta_folder, yyyy, mm, dd] = generate_meta_folder();
		if (const auto path = std::filesystem::path(meta_folder);
			not std::filesystem::exists(path))
		{
			if (not std::filesystem::create_directories(path))
			{
				wxMessageBox("Could not create the directory to save the file.", "Info", wxOK | wxICON_INFORMATION, this);
				return;
			}
		}

		const auto filename = std::format("{}/{}_{}_{}.json", meta_folder, data_.mobile, DialerData::trimmed_name(data_.name), std::time(nullptr));

		data_.save(filename);

		add_item_expand_(yyyy, mm, dd, std::filesystem::path(filename).filename().string());

		update_components_state_();
	}

	std::string WDialer::generate_wav_filename(const std::string& validated_mobile, const std::string& validated_name)
	{
		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		return std::format("{}/{}_{}_{}.wav", k_record_folder, validated_mobile, validated_name, t);
	}

	std::tuple<std::string, std::string, std::string, std::string> WDialer::generate_meta_folder()
	{
		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		auto yyyy = std::format("{:04}", tm.tm_year + 1900);
		auto mm = std::format("{:02}", tm.tm_mon + 1);
		auto dd = std::format("{:02}", tm.tm_mday);
		return { std::format("{}/{}/{}/{}/{}", k_calls_folder, yyyy, mm, dd, (data_.ucode.empty() ? "NA" : data_.ucode)), yyyy, mm, dd };
	}

	void WDialer::add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& file)
	{
		auto item_exists = [this](wxTreeItemId& parent_id, const wxString& text) -> wxTreeItemId
			{
				wxTreeItemIdValue cookie;
				auto child = tree_->GetFirstChild(parent_id, cookie);
				while (child.IsOk())
				{
					if (tree_->GetItemText(child) == text)
					{
						return child;
					}

					child = tree_->GetNextChild(parent_id, cookie);
				}

				return parent_id;
			};

		auto root_id = tree_->GetRootItem();

		auto fyyyy = item_exists(root_id, yyyy);
		if (fyyyy == root_id)
		{
			fyyyy = tree_->AppendItem(root_id, yyyy, -1, -1, new DirMeta(std::format("{}/{}", k_calls_folder, yyyy)));
		}

		auto fmm = item_exists(fyyyy, mm);
		if (fmm == fyyyy)
		{
			fmm = tree_->AppendItem(fyyyy, mm, -1, -1, new DirMeta(std::format("{}/{}/{}", k_calls_folder, yyyy, mm)));
		}

		auto fdd = item_exists(fmm, dd);
		if (fdd == fmm)
		{
			fdd = tree_->AppendItem(fmm, dd, -1, -1, new DirMeta(std::format("{}/{}/{}/{}", k_calls_folder, yyyy, mm, dd)));
		}

		auto tucode = (data_.ucode.empty() ? std::string("NA") : data_.ucode);
		auto ucode = item_exists(fdd, tucode);
		if (ucode == fdd)
		{
			ucode = tree_->AppendItem(fdd, tucode, -1, -1, new DirMeta(std::format("{}/{}/{}/{}/{}", k_calls_folder, yyyy, mm, dd, tucode)));
		}

		if (auto item_data = dynamic_cast<DirMeta*>(tree_->GetItemData(ucode)); not item_data->visited)
		{
			register_node_elements_(ucode, std::format("{}/{}/{}/{}/{}", k_calls_folder, yyyy, mm, dd, tucode));
			auto file_id = item_exists(ucode, file);
			tree_->SelectItem(file_id);
			item_data->visited = true;
		}
		else
		{
			auto file_id = tree_->AppendItem(ucode, file, -1, -1, new DirMeta(std::format("{}/{}/{}/{}/{}/{}", k_calls_folder, yyyy, mm, dd, tucode, file), true));
			tree_->SelectItem(file_id);
		}

		tree_->Expand(root_id);
		tree_->Expand(fyyyy);
		tree_->Expand(fmm);
		tree_->Expand(fdd);
		tree_->Expand(ucode);
	}

	void WDialer::on_close_(wxCommandEvent&)
	{
		if (data_.state == DialerState::JustEnded)
		{
			if (wxMessageBox("Do you want to save record before closing the window?", "Confirmation", wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION) == wxYES)
			{
				save_();
			}
		}

		Close(true);
	}

	void WDialer::on_playback_(wxCommandEvent&)
	{
		//wxMessageBox("Test", "Error");
		auto orig_state = data_.state;
		data_.state = DialerState::PlayingWav;
		wxTheApp->CallAfter([this]
			{
				update_components_state_();
			});

		ma_engine engine;
		if (ma_engine_init(NULL, &engine) not_eq MA_SUCCESS)
		{
			data_.state = orig_state;
			update_components_state_();

			wxMessageBox("Cannot initialize MA engine", "Error");
			return;
		}

		ma_sound sound;
		if (ma_sound_init_from_file(&engine, data_.file_recording.c_str(),
			MA_SOUND_FLAG_DECODE,
			NULL, NULL, &sound) not_eq MA_SUCCESS)
		{
			ma_engine_uninit(&engine);
			data_.state = orig_state;
			update_components_state_();
			wxMessageBox("Cannot initialize MA sounds", "Error");
			return;
		}

		ma_sound_start(&sound);

		while (ma_sound_is_playing(&sound))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		ma_sound_uninit(&sound);
		ma_engine_uninit(&engine);

		data_.state = orig_state;
		update_components_state_();

		wxMessageBox("Recording played successfully.", "Information");
	}

	void WDialer::populate_master_()
	{
		if (filter_.client_master.empty())
		{
			auto db_conn = std::format("Driver={};Server={};Database={};UID={};PWD={};", k_db_driver, k_db_server, k_db_database, k_db_user, k_db_password);
			nanodbc::connection conn(NANODBC_TEXT(db_conn));

			if (not conn.connected())
			{
				wxMessageBox(std::format("Could not connect to database: {}", db_conn), "Database Connection", wxOK | wxICON_ERROR, this);
				Close(true);
				return;
			}

			auto sql = std::format("select	a.collector_id, a.client_id, a.client_name, a.client_campaign_id, a.client_campaign_name, a.prio_type_id, a.prio_type, a.wmc_status_id, a.wmc_status_name, a.name, a.min_id, a.max_id, a.next_id, a.ucode "
				"from	vw_ad_cache_priority_series as a where a.collector_id = {}", 6);
			nanodbc::result results = nanodbc::execute(
				conn,
				NANODBC_TEXT(sql));

			auto last_client_id = 0;
			auto last_client_campaign_id = 0;
			auto last_prio_type_id = 0;
			auto last_client_status_id = 0;

			while (results.next())
			{
				// Save to Master file:

				if (const auto client_id = results.get<size_t>(1);
					client_id not_eq last_client_id)
				{
					filter_.client_master.try_emplace(client_id, results.get<std::string>(2));
					last_client_id = client_id;
					filter_.clients.emplace_back(client_id);
				}

				if (const auto client_campaign_id = results.get<size_t>(3);
					client_campaign_id not_eq last_client_campaign_id)
				{
					filter_.campaign_master.try_emplace(client_campaign_id, results.get<std::string>(4));
					last_client_campaign_id = client_campaign_id;
					filter_.clients.back().campaigns.emplace_back(client_campaign_id);
				}

				if (const auto prio_type_id = results.get<size_t>(5);
					prio_type_id not_eq last_client_campaign_id)
				{
					filter_.prio_master.try_emplace(prio_type_id, results.get<std::string>(6));
					last_prio_type_id = prio_type_id;

					filter_.clients.back().campaigns.back().prios.emplace_back(prio_type_id);
				}

				last_client_status_id = results.get<size_t>(7);
				filter_.status_master.try_emplace(last_client_status_id, results.get<std::string>(8));

				//status, min, max, next
				filter_.clients.back().campaigns.back().prios.back().status_series.emplace_back(last_client_status_id, results.get<size_t>(10), results.get<size_t>(11), results.get<size_t>(12), results.get<std::string>(13));
			}

			for (const auto& [id, name] : filter_.client_master)
			{
				filter_client_->Append(name, reinterpret_cast<void*>(id));
			}
		}
	}
}