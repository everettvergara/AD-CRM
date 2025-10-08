#pragma once

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "WChildFrame.h"
#include "PJCallManualDial.hpp"
#include "WDialerCommon.hpp"

namespace eg::ad3
{
	//constexpr auto k_db_driver = "{SQL Server}";
	//constexpr auto k_db_server = "WIN-0BL4BGRJARA";
	//constexpr auto k_db_database = "wmc";
	//constexpr auto k_db_user = "sa";
	//constexpr auto k_db_password = "Kerberos2014!";
	//constexpr auto k_collector_id = "6";

	class WDialer :
		public WChildFrame
	{
	public:

		WDialer(wxMDIParentFrame* parent, const char* title, int account_ix);

		//void on_close(wxCloseEvent& event)
		//{
		//	if (current_call_ >= 0)
		//	{
		//		wxMessageBox("Call is ongoing. Cancel / Stop the call first.", this->GetTitle());
		//		event.Veto();
		//		return;
		//	}

		//	// normal close
		//	Destroy();  // or event.Skip() if you want the default handler
		//}

	protected:

		wxTreeCtrl* tree_;

		wxCheckBox* filter_is_auto_;
		wxChoice* filter_client_;
		wxChoice* filter_campaign_;
		wxChoice* filter_prio_;
		wxChoice* filter_status_;
		wxTextCtrl* filter_to_call_count_;

		wxTextCtrl* id_;
		wxTextCtrl* ucode_;
		wxTextCtrl* mobile_;
		wxTextCtrl* name_;
		wxTextCtrl* status_;
		wxTextCtrl* time_of_call_;
		wxTextCtrl* time_call_ended_;
		wxTextCtrl* file_recording_;
		wxTextCtrl* remarks_;
		wxButton* new_button_;
		wxButton* call_button_;
		wxButton* call_again_button_;
		wxButton* playback_button_;
		wxButton* cm_button_;

		wxButton* stop_button_;
		wxButton* save_button_;
		wxButton* cancel_button_;

		int current_call_;
		int account_ix_;

		DialerFilter filter_;
		DialerData data_;

		void on_init_filter_controls_();
		void on_init_input_controls_();
		void on_init_tree_();
		void on_init_buttons_();

		void call_proper_(const std::string& validated_name);
		void on_call_manual_();
		void on_call_auto_();

		void on_call_(wxCommandEvent&);
		void on_call_again_(wxCommandEvent&);
		void on_new_(wxCommandEvent&);
		void on_stop_(wxCommandEvent&);
		void on_save_(wxCommandEvent&);
		void on_close_(wxCommandEvent&);
		void on_win_close_(wxCloseEvent& event);
		void on_playback_(wxCommandEvent&);
		void on_cm_(wxCommandEvent&);

		void on_call_state_changed_(pjsip_inv_state state, pj::CallInfo info, bool hangup_requested);

		void save_();
		void update_components_state_();
		void populate_master_();
		void update_components_from_data_();
		[[nodiscard]] const char* update_data_from_components_();

		void register_node_elements_(const wxTreeItemId& node_id, const std::string& path);
		void add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& file);

		static [[nodiscard]] std::string generate_wav_filename(const std::string& validated_mobile, const std::string& validated_name);
		[[nodiscard]] std::tuple<std::string, std::string, std::string, std::string> generate_meta_folder();
	};
}