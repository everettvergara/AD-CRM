#include "WAutoDial.h"
#include <fstream>
#include <format>
#include <unordered_map>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <nanodbc/nanodbc.h>
#include "Common/Log.hpp"
#include "Common/StringHelper.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"
#include "WManualDial.h"
#include "PJHelper.h"

namespace eg::ad3
{
	constexpr auto k_db_driver = "{SQL Server}";
	constexpr auto k_db_server = "WIN-0BL4BGRJARA"; // 192.168.254.120;
	constexpr auto k_db_database = "wmc";
	constexpr auto k_db_user = "sa";
	constexpr auto k_db_password = "Kerberos2014!";

	WAutoDial::WAutoDial(wxMDIParentFrame* parent) :
		WChildFrame
		(
			WChildProp
			{
				.parent = parent,
				.title = "Auto Dial",
				.pos = wxDefaultPosition,
				.size = wxSize(800, 800),
				.style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX),
				.form_columns = 2,
				.has_tree = true
			}
		),
		tree_(nullptr),

		cm_client_(nullptr),
		cm_client_campaign_(nullptr),
		cm_prio_(nullptr),
		cm_status_(nullptr),
		selected_client_(nullptr),
		selected_campaign_(nullptr),
		selected_prio_(nullptr),
		selected_status_(nullptr),
		mobile_(nullptr),
		status_(nullptr),
		stop_button_(nullptr),
		cancel_button_(nullptr)

	{
		on_init_dropdown_controls_();
		on_init_input_controls_();
		on_init_tree_();
		on_init_buttons_();
		set_components_state_(RecordState::Stopped);

		Show(true);
	}

	void WAutoDial::set_components_state_(WAutoDial::RecordState state)
	{
		switch (state)
		{
		case RecordState::Stopped:
			tree_->Enable();
			stop_button_->Disable();
			cancel_button_->Enable();
			break;

		case RecordState::Calling:
			tree_->Disable();
			stop_button_->Enable();
			cancel_button_->Disable();
			break;
		}
	}

	void WAutoDial::on_init_tree_()
	{
		if (not std::filesystem::exists(k_auto_folder))
		{
			if (not std::filesystem::create_directory(k_auto_folder))
			{
				throw std::runtime_error("Could not create the auto folder.");
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
					std::ifstream file(meta->path);

					auto data = nlohmann::json::parse(file);

					cm_id_->SetValue(data.at("cm_id").get_ref<const std::string&>());
					mobile_->SetValue(data.at("mobile").get_ref<const std::string&>());
					name_->SetValue(data.at("name").get_ref<const std::string&>());
					time_of_call_->SetValue(data.at("time_of_call").get_ref<const std::string&>());
					time_call_ended_->SetValue(data.at("time_call_ended").get_ref<const std::string&>());
					remarks_->SetValue(data.at("remarks").get_ref<const std::string&>());
					saved_->SetValue(true);
					set_components_state_(RecordState::Stopped);
				}
			});

		const auto root_id = tree_->AddRoot(k_auto_folder, -1, -1, new DirMeta(k_auto_folder));

		register_node_elements_(root_id, k_auto_folder);
	}

	void WAutoDial::on_init_buttons_()
	{
		stop_button_ = register_button("Stop", wxID_ANY);
		stop_button_->Bind(wxEVT_BUTTON, &WAutoDial::on_stop_, this);
		stop_button_->Disable();
		cancel_button_ = register_button("Cancel", wxID_ANY);
		cancel_button_->Bind(wxEVT_BUTTON, &WAutoDial::on_cancel_, this);

		set_components_state_(RecordState::Stopped);
	}

	void WAutoDial::on_init_dropdown_controls_()
	{
		auto db_conn = std::format("Driver={};Server={};Database={};UID={};PWD={};", k_db_driver, k_db_server, k_db_database, k_db_user, k_db_password);
		nanodbc::connection conn(NANODBC_TEXT(db_conn));

		if (not conn.connected())
		{
			wxMessageBox(std::format("Could not connect to database: {}", db_conn), "Database Connection", wxOK | wxICON_ERROR, this);
			Close(true);
			return;
		}

		nanodbc::result results = nanodbc::execute(
			conn,
			NANODBC_TEXT(
				"select	a.collector_id, a.client_id, a.client_name, a.client_campaign_id, a.client_campaign_name, a.prio_type_id, a.prio_type, a.wmc_status_id, a.wmc_status_name, a.name, a.min_id, a.max_id, a.next_id, a.ucode "
				"from	vw_ad_cache_priority_series as a where a.collector_id = 6"));
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
				client_master_.try_emplace(client_id, results.get<std::string>(2));
				last_client_id = client_id;
				collector_dd_.clients.emplace_back(client_id);
			}

			if (const auto client_campaign_id = results.get<size_t>(3);
				client_campaign_id not_eq last_client_campaign_id)
			{
				campaign_master_.try_emplace(client_campaign_id, results.get<std::string>(4));
				last_client_campaign_id = client_campaign_id;
				collector_dd_.clients.back().campaigns.emplace_back(client_campaign_id);
			}

			if (const auto prio_type_id = results.get<size_t>(5);
				prio_type_id not_eq last_client_campaign_id)
			{
				prio_master_.try_emplace(prio_type_id, results.get<std::string>(6));
				last_prio_type_id = prio_type_id;

				collector_dd_.clients.back().campaigns.back().prios.emplace_back(prio_type_id);
			}

			last_client_status_id = results.get<size_t>(7);
			client_status_master_.try_emplace(last_client_status_id, results.get<std::string>(8));

			// status, min, max, next
			collector_dd_.clients.back().campaigns.back().prios.back().status_series.emplace_back(last_client_status_id, results.get<size_t>(10), results.get<size_t>(11), results.get<size_t>(12), results.get<std::string>(13));
		}

		// Populate the Client;
		cm_client_ = register_dropdown("client", "Client:");
		cm_client_campaign_ = register_dropdown("campaign", "Campaign:");
		cm_prio_ = register_dropdown("prio", "Prio:");
		cm_status_ = register_dropdown("status", "Status:");
		min_ = register_text_input("min_id", "Min ID:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		max_ = register_text_input("max_id", "Max ID:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		next_ = register_text_input("next_id", "Next ID:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		ucode_ = register_text_input("ucode", "Code:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		to_call_count_ = register_text_input("to_call_count", "To Call Count:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		//download_ = register_button_field("Download");
		auto_dial_ = register_button_field("Auto Dial");

		for (const auto& [id, name] : client_master_)
		{
			cm_client_->Append(name, reinterpret_cast<void*>(id));
		}

		cm_client_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto client_id = reinterpret_cast<size_t>(ptr);

				auto fclient = std::find_if(collector_dd_.clients.begin(), collector_dd_.clients.end(), [client_id](const ClientDD& a) -> bool
					{
						return a.client_id == client_id;
					});

				selected_client_ = &(*fclient);

				cm_client_campaign_->Clear();
				for (const auto& [campaign_id, _] : fclient->campaigns)
				{
					cm_client_campaign_->Append(campaign_master_.at(campaign_id), reinterpret_cast<void*>(campaign_id));
				}
				cm_prio_->Clear();
				cm_status_->Clear();

				selected_campaign_ = nullptr;
				selected_prio_ = nullptr;
				selected_status_ = nullptr;
				//download_->Disable();
				auto_dial_->Disable();

				min_->ChangeValue("");
				max_->ChangeValue("");
				next_->ChangeValue("");
				to_call_count_->ChangeValue("");
				ucode_->ChangeValue("");
			});

		cm_client_campaign_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto campaign_id = reinterpret_cast<size_t>(ptr);

				auto fcampaign = std::find_if(selected_client_->campaigns.begin(), selected_client_->campaigns.end(), [campaign_id](const CampaignDD& a) -> bool
					{
						return a.campaign_id == campaign_id;
					});

				selected_campaign_ = &(*fcampaign);

				cm_prio_->Clear();
				for (const auto& [prio_id, _] : fcampaign->prios)
				{
					cm_prio_->Append(prio_master_.at(prio_id), reinterpret_cast<void*>(prio_id));
				}

				cm_status_->Clear();

				selected_prio_ = nullptr;
				selected_status_ = nullptr;

				min_->ChangeValue("");
				max_->ChangeValue("");
				next_->ChangeValue("");
				to_call_count_->ChangeValue("");
				ucode_->ChangeValue("");

				auto_dial_->Disable();
			});

		cm_prio_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto prio_id = reinterpret_cast<size_t>(ptr);
				auto fprio = std::find_if(selected_campaign_->prios.begin(), selected_campaign_->prios.end(), [prio_id](const PrioDD& a) -> bool
					{
						return a.prio_id == prio_id;
					});

				selected_prio_ = &(*fprio);
				cm_status_->Clear();
				for (const auto& [status_id, _, __, ___, ____] : fprio->status_series)
				{
					cm_status_->Append(client_status_master_.at(status_id), reinterpret_cast<void*>(status_id));
				}
				selected_status_ = nullptr;

				min_->ChangeValue("");
				max_->ChangeValue("");
				next_->ChangeValue("");
				to_call_count_->ChangeValue("");
				ucode_->ChangeValue("");

				auto_dial_->Disable();
			});

		cm_status_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e)
			{
				void* ptr = e.GetClientData();
				auto status_id = reinterpret_cast<size_t>(ptr);
				auto fstatus = std::find_if(selected_prio_->status_series.begin(), selected_prio_->status_series.end(), [status_id](const StatusSeriesDD& a) -> bool
					{
						return a.status_id == status_id;
					});
				selected_status_ = &(*fstatus);

				min_->ChangeValue(std::to_string(selected_status_->min_id));
				max_->ChangeValue(std::to_string(selected_status_->max_id));
				next_->ChangeValue(std::to_string(selected_status_->next_id));
				to_call_count_->ChangeValue(std::to_string(selected_status_->max_id - selected_status_->next_id + 1));
				ucode_->ChangeValue(selected_status_->ucode);

				auto_dial_->Enable();
			});

		//download_->Bind(wxEVT_BUTTON, &WAutoDial::on_download_, this);
		auto_dial_->Bind(wxEVT_BUTTON, &WAutoDial::on_auto_dial_, this);
		auto_dial_->Disable();
	}

	void WAutoDial::on_init_input_controls_()
	{
		mobile_ = register_text_input("mobile", "Mobile to dial:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		cm_id_ = register_text_input("cm_id", "CM ID:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		name_ = register_text_input("name", "Name:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		status_ = register_text_input("status", "Last Status:", "PJSIP_INV_STATE_NULL", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_of_call_ = register_text_input("time_of_call", "Time of Call:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_call_ended_ = register_text_input("time_call_ended", "Time Call Ended:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		saved_ = register_checkbox("saved", "Saved:", false, wxDefaultPosition, wxDefaultSize);
		saved_->Disable();
		saved_->SetValue(false);
		remarks_ = register_text_input_multi("remarks", "Remarks:", 10, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	}

	void WAutoDial::register_node_elements_(const wxTreeItemId& node_id, const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			const auto p = entry.path();
			tree_->AppendItem(node_id, p.filename().string(), -1, -1, new DirMeta(p.string(), std::filesystem::is_regular_file(p)));
		}
	}

	void WAutoDial::on_call_state_changed_(pjsip_inv_state state)
	{
		wxTheApp->CallAfter([this, state]() {
			switch (state)
			{
			case PJSIP_INV_STATE_CALLING:
				LOG_II("Call State: PJSIP_INV_STATE_CALLING");
				status_->SetValue("PJSIP_INV_STATE_CALLING");
				break;

			case PJSIP_INV_STATE_INCOMING:
				LOG_II("Call State: PJSIP_INV_STATE_INCOMING");
				status_->SetValue("PJSIP_INV_STATE_INCOMING");
				break;

			case PJSIP_INV_STATE_EARLY:
				LOG_II("Call State: PJSIP_INV_STATE_EARLY");
				status_->SetValue("PJSIP_INV_STATE_EARLY");
				break;

			case PJSIP_INV_STATE_CONNECTING:
				LOG_II("Call State: PJSIP_INV_STATE_CONNECTING");
				status_->SetValue("PJSIP_INV_STATE_CONNECTING");
				break;

			case PJSIP_INV_STATE_CONFIRMED:
				LOG_II("Call State: PJSIP_INV_STATE_CONFIRMED");
				status_->SetValue("PJSIP_INV_STATE_CONFIRMED");
				break;

			case PJSIP_INV_STATE_DISCONNECTED:
			{
				LOG_II("Call State: PJSIP_INV_STATE_DISCONNECTED");
				{
					std::lock_guard lock(call_mutex_);

					if (current_call_ not_eq nullptr)
					{
						LOG_II("bEFORE RESET");

						current_call_.reset();
						stop_button_->Disable();
						cancel_button_->Enable();

						last_cm_data_.time_call_ended = eg::string::datetime_to_formatted_string();
						time_call_ended_->SetValue(eg::string::datetime_to_formatted_string());
					}
				}
				LOG_II("Call ended");
				call_cv_.notify_all();

				break;
			}

			default:
				break;
			}
			});
	}

	void WAutoDial::on_call_()
	{
		{
			std::lock_guard lock(call_mutex_);
			if (current_call_ == nullptr)
			{
				current_call_ = std::make_shared<PJCallManualDial>(ServicePJAccount::instance().account, std::bind(&WAutoDial::on_call_state_changed_, this, std::placeholders::_1));
				current_call_->makeCall(std::format("sip:{}@{}", last_cm_data_.mobile, ConfigSettings::instance().server_ip), []
					{
						pj::CallOpParam p(true);
						p.opt.audioCount = 1;
						p.opt.videoCount = 0;
						return p;
					}());
			}
		}

		last_cm_data_.time_of_call = eg::string::datetime_to_formatted_string();

		wxTheApp->CallAfter([this, time_of_call = last_cm_data_.time_of_call]()
			{
				time_of_call_->SetValue(time_of_call);
			});
	}

	void WAutoDial::on_stop_(wxCommandEvent&)
	{
		{
			std::lock_guard lock(call_mutex_);
			is_calling_ = false;
			if (current_call_ not_eq nullptr)
			{
				current_call_->hangup_call();
			}
		}

		//call_cv_.notify_all();

		auto_dial_thread_.join();

		wxMessageBox("Stopped", "Info", wxOK | wxICON_INFORMATION, this);
	}

	void WAutoDial::on_save_()
	{
		if (not last_cm_data_.valid)
		{
			return;
		}

		nlohmann::json j;

		auto name = get_validated_name_();
		j["mobile"] = last_cm_data_.mobile;
		j["cm_id"] = last_cm_data_.cm_id;
		j["name"] = last_cm_data_.name;
		j["status"] = last_cm_data_.status;
		j["time_of_call"] = last_cm_data_.time_of_call;
		j["time_call_ended"] = last_cm_data_.time_call_ended;
		j["remarks"] = last_cm_data_.remarks;

		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		const auto filename = std::format("{}/{:04}/{:02}/{:02}/{}/{}_{}_{}.json", k_auto_folder, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, last_cm_data_.ucode, last_cm_data_.mobile, name, t);

		if (const auto path = std::filesystem::path(filename).parent_path();
			not std::filesystem::exists(path))
		{
			std::filesystem::create_directories(path);
		}

		std::ofstream file(filename);
		file << j.dump(4);
		file.close();

		add_item_expand_(std::to_string(tm.tm_year + 1900), std::format("{:02}", tm.tm_mon + 1), std::format("{:02}", tm.tm_mday), last_cm_data_.ucode,
			std::filesystem::path(filename).filename().string());
	}

	void WAutoDial::add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& ucode, const std::string& file)
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
			fyyyy = tree_->AppendItem(root_id, yyyy, -1, -1, new DirMeta(std::format("{}/{}", k_auto_folder, yyyy)));
		}
		auto fmm = item_exists(fyyyy, mm);
		if (fmm == fyyyy)
		{
			fmm = tree_->AppendItem(fyyyy, mm, -1, -1, new DirMeta(std::format("{}/{}/{}", k_auto_folder, yyyy, mm)));
		}
		auto fdd = item_exists(fmm, dd);
		if (fdd == fmm)
		{
			fdd = tree_->AppendItem(fmm, dd, -1, -1, new DirMeta(std::format("{}/{}/{}/{}", k_auto_folder, yyyy, mm, dd)));
		}

		auto fucode = item_exists(fdd, ucode);
		if (fucode == fdd)
		{
			fucode = tree_->AppendItem(fdd, dd, -1, -1, new DirMeta(std::format("{}/{}/{}/{}/{}", k_auto_folder, yyyy, mm, dd, ucode)));
		}

		if (auto item_data = dynamic_cast<DirMeta*>(tree_->GetItemData(fdd)); not item_data->visited)
		{
			register_node_elements_(fucode, std::format("{}/{}/{}/{}/{}", k_auto_folder, yyyy, mm, dd, ucode));
			auto file_id = item_exists(fucode, file);
			tree_->SelectItem(file_id);
			item_data->visited = true;
		}
		else
		{
			auto file_id = tree_->AppendItem(fucode, file, -1, -1, new DirMeta(std::format("{}/{}/{}/{}/{}/{}", k_auto_folder, yyyy, mm, dd, ucode, file), true));
			tree_->SelectItem(file_id);
		}

		tree_->Expand(root_id);
		tree_->Expand(fyyyy);
		tree_->Expand(fmm);
		tree_->Expand(fdd);
		tree_->Expand(fucode);
	}

	void WAutoDial::on_cancel_(wxCommandEvent&)
	{
		Close(true);
	}

	void WAutoDial::on_download_(wxCommandEvent&)
	{
	}

	void WAutoDial::on_auto_dial_(wxCommandEvent& e)
	{
		auto_dial_thread_ = std::thread
		([this]
			{
				//LOG_II("started_thread");
				ad3::register_current_thread_in_pj("ad_thread");

				is_calling_ = true;
				do
				{
					//LOG_II("set_next_");

					if (not set_next_())
					{
						break;
					}

					if (is_calling_)
					{
						//LOG_II("on_call_");

						on_call_();

						std::unique_lock lock(call_mutex_);
						call_cv_.wait(lock, [this]
							{
								//LOG_II("awaken");

								return current_call_ == nullptr or not is_calling_;
							});

						lock.unlock();
						//LOG_II("saving....");

						on_save_();
						//LOG_II("done saving....");
					}
				} while (is_calling_);

				//LOG_II("exited thread....");
			});

		set_components_state_(RecordState::Calling);
	}

	bool WAutoDial::set_next_()
	{
		// Get next_id from db
		auto next_id = selected_status_->next_id;
		auto max_id = selected_status_->max_id;
		auto retrieve_sql = std::format("select a.cm_id, a.contact_name, a.mobile, a.remarks from vw_ad_lr_priority_new as a where next_id = {}", next_id);

		auto db_conn = std::format("Driver={};Server={};Database={};UID={};PWD={};", k_db_driver, k_db_server, k_db_database, k_db_user, k_db_password);
		nanodbc::connection conn(NANODBC_TEXT(db_conn));

		if (not conn.connected())
		{
			wxMessageBox(std::format("Could not connect to database: {}", db_conn), "Database Connection", wxOK | wxICON_ERROR, this);
			//Close(true);
			return false;
		}

		nanodbc::result results = nanodbc::execute(
			conn,
			NANODBC_TEXT(retrieve_sql));

		if (results.next())
		{
			last_cm_data_.cm_id = std::to_string(results.get<size_t>(0));
			last_cm_data_.name = results.get<std::string>(1);
			last_cm_data_.mobile = results.get<std::string>(2);
			last_cm_data_.remarks = results.get<std::string>(3);
			last_cm_data_.valid = true;
		}
		else
		{
			last_cm_data_.cm_id = "";
			last_cm_data_.name = "";
			last_cm_data_.mobile = "";
			last_cm_data_.remarks = "";
			last_cm_data_.valid = false;
		}

		last_cm_data_.ucode = selected_status_->ucode;
		last_cm_data_.status = "";
		last_cm_data_.time_of_call = "";
		last_cm_data_.time_call_ended = "";
		last_cm_data_.saved = false;

		nanodbc::statement stmt(conn);
		auto update_sql = std::format("UPDATE tb_ad_cache_priority_series SET next_id = {} WHERE {} between min_id and max_id", next_id + 1, next_id);
		prepare(stmt, NANODBC_TEXT(update_sql));

		if (auto result = execute(stmt);
			result.affected_rows() == 0)
		{
			conn.disconnect();
			wxMessageBox("Could not update the next_id", "Database Error", wxOK | wxICON_ERROR, this);
			return false;
		}

		selected_status_->next_id = next_id + 1;
		wxTheApp->CallAfter([this, max_id = max_id, next_id = selected_status_->next_id, last_cm_data = last_cm_data_]()
			{
				next_->ChangeValue(std::to_string(next_id));
				to_call_count_->ChangeValue(std::to_string(max_id - next_id + 1));

				cm_id_->ChangeValue(last_cm_data.cm_id);
				name_->ChangeValue(last_cm_data.name);
				mobile_->ChangeValue(last_cm_data.mobile);
				remarks_->ChangeValue(last_cm_data.remarks);
				status_->ChangeValue(last_cm_data.status);
				time_call_ended_->ChangeValue(last_cm_data.time_call_ended);
				time_of_call_->ChangeValue(last_cm_data.time_of_call);
				saved_->SetValue(false);
			});

		return 	last_cm_data_.valid;
	}

	std::string WAutoDial::get_validated_name_()
	{
		const auto name = last_cm_data_.name;
		if (name.empty())
		{
			return {};
		}

		std::string sanitized;
		for (const auto ch : name)
		{
			if (std::isalnum(ch))
			{
				sanitized += ch;
			}
			else
			{
				sanitized += '_';
			}
		}

		return sanitized;
	}
}