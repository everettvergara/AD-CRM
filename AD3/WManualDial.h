#pragma once

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include "WChildFrame.h"
#include "PJCallManualDial.hpp"

namespace eg::ad3
{
	constexpr auto k_manual_folder = "manual";

	struct DirMeta :
		public wxTreeItemData
	{
		std::string path;
		bool is_file;
		bool visited;

		DirMeta(std::string p, bool isf = false, bool v = false) :
			path(std::move(p)), is_file(isf), visited(v)
		{
		}
	};

	class WManualDial :
		public WChildFrame
	{
	public:

		enum class RecordState
		{
			New, Calling, JustEnded, Saved
		};

		WManualDial(wxMDIParentFrame* parent);

	private:

		wxTreeCtrl* tree_;
		wxTextCtrl* mobile_;
		wxTextCtrl* name_;
		wxTextCtrl* status_;
		wxTextCtrl* time_of_call_;
		wxTextCtrl* time_call_ended_;
		wxTextCtrl* remarks_;
		wxButton* new_button_;
		wxButton* call_button_;
		wxButton* call_again_button_;

		wxButton* stop_button_;
		wxButton* save_button_;
		wxButton* cancel_button_;

		std::shared_ptr<PJCallManualDial> current_call_;
		std::mutex call_mutex_;

		void on_init_tree_();
		void on_init_buttons_();
		void on_init_input_controls_();
		void on_call_(wxCommandEvent&);
		void on_call_again_(wxCommandEvent&);
		void on_new_(wxCommandEvent&);
		void on_stop_(wxCommandEvent&);
		void on_save_(wxCommandEvent&);
		void on_cancel_(wxCommandEvent&);
		void on_call_state_changed_(pjsip_inv_state state);

		void set_components_state_(RecordState state);
		void register_node_elements_(const wxTreeItemId& node_id, const std::string& path);
		void add_item_expand_(const std::string& yyyy, const std::string& mm, const std::string& dd, const std::string& file);
		[[nodiscard]] std::string get_validated_mobile_();
		[[nodiscard]] std::string get_validated_name_();
	};
}