#include "WManualDial.h"
#include <fstream>
#include <format>
#include <nlohmann/json.hpp>
#include "Common/StringHelper.hpp"
#include "ServicePJAccount.h"
#include "ConfigSettings.hpp"

namespace eg::ad3
{
	WManualDial::WManualDial(wxMDIParentFrame* parent) :
		WChildFrame
		(
			WChildProp
			{
				.parent = parent,
				.title = "Manual Dial",
				.pos = wxDefaultPosition,
				.size = wxSize(800, 600),
				.style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX),
				.form_columns = 2,
				.has_tree = true
			}
		),
		tree_(nullptr),
		mobile_(nullptr),
		status_(nullptr),
		new_button_(nullptr),
		call_button_(nullptr),
		call_again_button_(nullptr),
		stop_button_(nullptr),
		save_button_(nullptr),
		cancel_button_(nullptr)

	{
		on_init_input_controls_();
		on_init_tree_();
		on_init_buttons_();
		set_components_state_(RecordState::New);

		Show(true);
	}

	void WManualDial::set_components_state_(WManualDial::RecordState state)
	{
		switch (state)
		{
		case RecordState::New:

			mobile_->Enable();
			name_->Enable();
			remarks_->Enable();

			new_button_->Disable();
			call_button_->Enable();
			call_again_button_->Disable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Enable();
			break;

		case RecordState::Calling:

			mobile_->Disable();
			name_->Disable();
			remarks_->Disable();

			new_button_->Disable();
			call_button_->Disable();
			call_again_button_->Disable();
			stop_button_->Enable();
			save_button_->Disable();
			cancel_button_->Disable();
			break;

		case RecordState::JustEnded:

			mobile_->Disable();
			name_->Enable();
			remarks_->Enable();

			new_button_->Enable();
			call_button_->Disable();
			call_again_button_->Enable();
			stop_button_->Disable();
			save_button_->Enable();
			cancel_button_->Enable();
			break;

		case RecordState::Saved:
			mobile_->Disable();
			name_->Disable();
			remarks_->Disable();

			new_button_->Enable();
			call_button_->Disable();
			call_again_button_->Enable();
			stop_button_->Disable();
			save_button_->Disable();
			cancel_button_->Enable();
			break;
		}
	}

	void WManualDial::on_init_tree_()
	{
		if (not std::filesystem::exists(k_manual_folder))
		{
			if (not std::filesystem::create_directory(k_manual_folder))
			{
				throw std::runtime_error("Could not create the manual folder.");
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
					mobile_->SetValue(data.at("mobile").get_ref<const std::string&>());
					name_->SetValue(data.at("name").get_ref<const std::string&>());
					time_of_call_->SetValue(data.at("time_of_call").get_ref<const std::string&>());
					remarks_->SetValue(data.at("remarks").get_ref<const std::string&>());

					set_components_state_(RecordState::Saved);
				}
			});

		const auto root_id = tree_->AddRoot(k_manual_folder, -1, -1, new DirMeta(k_manual_folder));

		register_node_elements_(root_id, k_manual_folder);
	}

	void WManualDial::on_init_buttons_()
	{
		new_button_ = register_button("New", wxID_ANY);
		new_button_->Bind(wxEVT_BUTTON, &WManualDial::on_new_, this);
		call_button_ = register_button("Call", wxID_ANY);
		call_button_->Bind(wxEVT_BUTTON, &WManualDial::on_call_, this);
		call_again_button_ = register_button("Call Again", wxID_ANY);
		call_again_button_->Bind(wxEVT_BUTTON, &WManualDial::on_call_again_, this);
		stop_button_ = register_button("Stop", wxID_ANY);
		stop_button_->Bind(wxEVT_BUTTON, &WManualDial::on_stop_, this);
		stop_button_->Disable();
		save_button_ = register_button("Save", wxID_ANY);
		save_button_->Bind(wxEVT_BUTTON, &WManualDial::on_save_, this);
		cancel_button_ = register_button("Cancel", wxID_ANY);
		cancel_button_->Bind(wxEVT_BUTTON, &WManualDial::on_cancel_, this);

		set_components_state_(RecordState::New);
	}

	void WManualDial::on_init_input_controls_()
	{
		register_text("Please input mobile in the following format: 0XXXYYYZZZZ i.e. 09177101995.", 300);
		mobile_ = register_text_input("mobile", "Mobile to dial:", "");
		mobile_->SetHint("09177101995");
		name_ = register_text_input("name", "Name:", "");
		name_->SetHint("Juan dela Cruz");
		status_ = register_text_input("status", "Last Status:", "PJSIP_INV_STATE_NULL", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_of_call_ = register_text_input("time_of_call", "Time of Call:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		time_call_ended_ = register_text_input("time_call_ended", "Time Call Ended:", "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		remarks_ = register_text_input_multi("remarks", "Remarks:", 10);
		remarks_->SetHint("Enter details of your conversation here...");
	}

	void WManualDial::register_node_elements_(const wxTreeItemId& node_id, const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			const auto p = entry.path();
			tree_->AppendItem(node_id, p.filename().string(), -1, -1, new DirMeta(p.string(), std::filesystem::is_regular_file(p)));
		}
	}

	void WManualDial::on_call_state_changed_(pjsip_inv_state state)
	{
		wxTheApp->CallAfter([this, state]() {
			switch (state)
			{
			case PJSIP_INV_STATE_CALLING:
				status_->SetValue("PJSIP_INV_STATE_CALLING");
				break;

			case PJSIP_INV_STATE_INCOMING:
				status_->SetValue("PJSIP_INV_STATE_INCOMING");
				break;

			case PJSIP_INV_STATE_EARLY:
				status_->SetValue("PJSIP_INV_STATE_EARLY");
				break;

			case PJSIP_INV_STATE_CONNECTING:
				status_->SetValue("PJSIP_INV_STATE_CONNECTING");
				break;

			case PJSIP_INV_STATE_CONFIRMED:
				status_->SetValue("PJSIP_INV_STATE_CONFIRMED");
				break;

			case PJSIP_INV_STATE_DISCONNECTED:
			{
				std::lock_guard lock(call_mutex_);

				if (current_call_ not_eq nullptr)
				{
					current_call_.reset();
					new_button_->Enable();
					call_button_->Disable();
					stop_button_->Disable();
					save_button_->Enable();
					cancel_button_->Enable();

					time_call_ended_->SetValue(eg::string::datetime_to_formatted_string());

					wxMessageBox("Call ended", "Info", wxOK | wxICON_INFORMATION);
				}

				break;
			}

			default:
				break;
			}
			status_->Refresh();
			status_->Update();
			});
	}

	void WManualDial::on_call_(wxCommandEvent&)
	{
		const auto validated_mobile = get_validated_mobile_();
		if (validated_mobile.empty())
		{
			wxMessageBox("Invalid mobile no.", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		std::lock_guard lock(call_mutex_);
		{
			if (current_call_ == nullptr)
			{
				current_call_ = std::make_shared<PJCallManualDial>(ServicePJAccount::instance().account, std::bind(&WManualDial::on_call_state_changed_, this, std::placeholders::_1));
				current_call_->makeCall(std::format("sip:{}@{}", validated_mobile, ConfigSettings::instance().server_ip), []
					{
						pj::CallOpParam p(true);
						p.opt.audioCount = 1;
						p.opt.videoCount = 0;
						return p;
					}());
			}
		}

		time_of_call_->SetValue(eg::string::datetime_to_formatted_string());

		set_components_state_(RecordState::Calling);
	}

	void WManualDial::on_call_again_(wxCommandEvent& e)
	{
		time_of_call_->SetValue("");

		on_call_(e);
	}

	void WManualDial::on_new_(wxCommandEvent&)
	{
		get<wxTextCtrl>("mobile")->SetValue("");
		get<wxTextCtrl>("name")->SetValue("");
		get<wxTextCtrl>("remarks")->SetValue("");
		status_->SetValue("PJSIP_INV_STATE_NULL");
		time_of_call_->SetValue("");
		time_call_ended_->SetValue("");

		set_components_state_(RecordState::New);
	}

	void WManualDial::on_stop_(wxCommandEvent&)
	{
		std::lock_guard lock(call_mutex_);
		current_call_->hangup_call();
		set_components_state_(RecordState::JustEnded);
	}

	std::string WManualDial::get_validated_mobile_()
	{
		const auto mobile = get<wxTextCtrl>("mobile")->GetValue().Trim();

		std::string validated_mobile;
		for (const auto ch : mobile)
		{
			if (std::isdigit(ch))
			{
				validated_mobile += ch;
			}
		}

		if (validated_mobile.size() < 11)
		{
			return {};
		}

		if (validated_mobile.substr(0, 1) not_eq "0")
		{
			return {};
		}

		return validated_mobile;
	}

	std::string WManualDial::get_validated_name_()
	{
		const auto name = get<wxTextCtrl>("name")->GetValue().Trim();
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

	void WManualDial::on_save_(wxCommandEvent&)
	{
		const auto validated_mobile = get_validated_mobile_();
		if (validated_mobile.empty())
		{
			wxMessageBox("Invalid mobile no.", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		const auto validated_name = get_validated_name_();
		if (validated_name.empty())
		{
			wxMessageBox("Invalid name", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}
		const auto name = get<wxTextCtrl>("name")->GetValue().ToStdString();

		const auto time_of_call = get<wxTextCtrl>("time_of_call")->GetValue();
		if (time_of_call.empty())
		{
			wxMessageBox("Cannot save a record that has not been called.", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		const auto remarks = get<wxTextCtrl>("remarks")->GetValue();
		if (remarks.empty())
		{
			wxMessageBox("Cannot save an empty remarks", "Validation", wxOK | wxICON_INFORMATION, this);
			return;
		}

		nlohmann::json j;
		j["mobile"] = validated_mobile;
		j["name"] = name;
		j["status"] = status_->GetValue().ToStdString();
		j["time_of_call"] = time_of_call.ToStdString();
		j["time_call_ended"] = time_call_ended_->GetValue().ToStdString();
		j["remarks"] = remarks.ToStdString();

		const auto t = std::time(nullptr);
		const auto tm = *std::localtime(&t);
		const auto filename = std::format("{}/{:04}/{:02}/{:02}/{}_{}_{}.json", k_manual_folder, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, validated_mobile, validated_name, t);

		if (const auto path = std::filesystem::path(filename).parent_path();
			not std::filesystem::exists(path))
		{
			if (not std::filesystem::create_directories(path))
			{
				wxMessageBox("Could not create the directory to save the file.", "Info", wxOK | wxICON_INFORMATION, this);
				return;
			}
		}

		std::ofstream file(filename);
		file << j.dump(4);
		file.close();

		add_item_expand_(std::to_string(tm.tm_year + 1900), std::format("{:02}", tm.tm_mon + 1), std::format("{:02}", tm.tm_mday), std::filesystem::path(filename).filename().string());

		set_components_state_(RecordState::Saved);

		wxMessageBox("Call record saved", "Info", wxOK | wxICON_INFORMATION, this);
	}

	void WManualDial::add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& file)
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
			fyyyy = tree_->AppendItem(root_id, yyyy, -1, -1, new DirMeta(std::format("{}/{}", k_manual_folder, yyyy)));
		}
		auto fmm = item_exists(fyyyy, mm);
		if (fmm == fyyyy)
		{
			fmm = tree_->AppendItem(fyyyy, mm, -1, -1, new DirMeta(std::format("{}/{}/{}", k_manual_folder, yyyy, mm)));
		}
		auto fdd = item_exists(fmm, dd);
		if (fdd == fmm)
		{
			fdd = tree_->AppendItem(fmm, dd, -1, -1, new DirMeta(std::format("{}/{}/{}/{}", k_manual_folder, yyyy, mm, dd)));
		}

		if (auto item_data = dynamic_cast<DirMeta*>(tree_->GetItemData(fdd)); not item_data->visited)
		{
			register_node_elements_(fdd, std::format("{}/{}/{}/{}", k_manual_folder, yyyy, mm, dd));
			auto file_id = item_exists(fdd, file);
			tree_->SelectItem(file_id);
			item_data->visited = true;
		}
		else
		{
			auto file_id = tree_->AppendItem(fdd, file, -1, -1, new DirMeta(std::format("{}/{}/{}/{}/{}", k_manual_folder, yyyy, mm, dd, file), true));
			tree_->SelectItem(file_id);
		}

		tree_->Expand(root_id);
		tree_->Expand(fyyyy);
		tree_->Expand(fmm);
		tree_->Expand(fdd);
	}

	void WManualDial::on_cancel_(wxCommandEvent&)
	{
		Close(true);
	}
}