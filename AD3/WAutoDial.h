#pragma once

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "WChildFrame.h"
#include "PJCallManualDial.hpp"

namespace eg::ad3
{
	constexpr auto k_auto_folder = "auto";

	struct CMData
	{
		std::string ucode;
		std::string mobile;
		std::string cm_id;
		std::string name;
		std::string status;
		std::string time_of_call;
		std::string time_call_ended;
		std::string remarks;
		bool saved = false;
		bool valid = false;
	};

	struct StatusSeriesDD
	{
		size_t status_id, min_id, max_id, next_id;
		std::string ucode;
	};

	struct PrioDD
	{
		size_t prio_id;
		std::vector<StatusSeriesDD> status_series;
	};

	struct CampaignDD
	{
		size_t campaign_id;
		std::vector<PrioDD> prios;
	};

	struct ClientDD
	{
		size_t client_id;
		std::vector<CampaignDD> campaigns;
	};

	struct CollectorDD
	{
		std::vector<ClientDD> clients;
	};

	class WAutoDial :
		public WChildFrame
	{
	public:

		enum class RecordState
		{
			Stopped, Calling
		};

		WAutoDial(wxMDIParentFrame* parent);

	private:

		wxTreeCtrl* tree_;
		wxChoice* cm_client_;
		wxChoice* cm_client_campaign_;
		wxChoice* cm_prio_;
		wxChoice* cm_status_;
		wxTextCtrl* ucode_;
		wxTextCtrl* min_;
		wxTextCtrl* max_;
		wxTextCtrl* next_;
		wxTextCtrl* to_call_count_;
		wxButton* auto_dial_;

		ClientDD* selected_client_;
		CampaignDD* selected_campaign_;
		PrioDD* selected_prio_;
		StatusSeriesDD* selected_status_;

		wxTextCtrl* mobile_;
		wxTextCtrl* cm_id_;
		wxTextCtrl* name_;
		wxTextCtrl* status_;
		wxTextCtrl* time_of_call_;
		wxTextCtrl* time_call_ended_;
		wxCheckBox* saved_;
		wxTextCtrl* remarks_;

		wxButton* stop_button_;
		wxButton* cancel_button_;

		CMData last_cm_data_;

		std::shared_ptr<PJCallManualDial> current_call_;
		std::condition_variable call_cv_;
		std::mutex call_mutex_;

		std::unordered_map<size_t, std::string> client_master_;
		std::unordered_map<size_t, std::string> campaign_master_;
		std::unordered_map<size_t, std::string> prio_master_;
		std::unordered_map<size_t, std::string> client_status_master_;
		CollectorDD collector_dd_;

		std::thread auto_dial_thread_;
		volatile bool is_calling_ = false;

		void on_init_tree_();
		void on_init_buttons_();
		void on_init_dropdown_controls_();
		void on_init_input_controls_();

		void on_stop_(wxCommandEvent&);
		void on_save_();
		void on_call_();
		void on_cancel_(wxCommandEvent&);
		void on_call_state_changed_(pjsip_inv_state state);

		void on_download_(wxCommandEvent&);
		void on_auto_dial_(wxCommandEvent&);

		void set_components_state_(RecordState state);
		void register_node_elements_(const wxTreeItemId& node_id, const std::string& path);
		void add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& ucode, const std::string& file);

		bool set_next_();

		[[nodiscard]] std::string get_validated_mobile_();
		[[nodiscard]] std::string get_validated_name_();
	};
}