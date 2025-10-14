#pragma once

#include <string>
#include <wx/wx.h>
#include <wx/treectrl.h>

namespace eg::ad3
{
	inline constexpr int k_gap_h = 10;
	inline constexpr int k_gap_v = 10;
	inline constexpr int k_border = 5;

	struct WChildProp
	{
		wxMDIParentFrame* parent;
		wxString title;
		wxPoint pos = wxDefaultPosition;
		wxSize size = wxDefaultSize;
		long style = wxDEFAULT_FRAME_STYLE;
		int form_columns = 2;
		bool has_tree = false;
	};

	struct LabelText
	{
		wxStaticText* label;
		wxStaticText* text;
	};

	class WChildFrame :
		public wxMDIChildFrame
	{
	public:

		WChildFrame(const WChildProp&);

	protected:

		wxPanel* panel;
		wxBoxSizer* sizer_header_buttons;
		wxBoxSizer* sizer_tree_fields;
		wxBoxSizer* sizer_tree;
		wxFlexGridSizer* sizer_fields;
		wxBoxSizer* sizer_buttons;

		template<typename T>
		T* get(const std::string& code)
		{
			assert(controls_.contains(code));
			return dynamic_cast<T*>(controls_.at(code));
		}

		LabelText register_text_msg(const std::string& code1, const std::string& code2, const wxString& label, const wxString& def = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxStaticText* register_text(const wxString& text, int wrap, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxTreeCtrl* register_tree(const std::string& code, int width, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_DEFAULT_STYLE | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
		wxChoice* register_dropdown(const std::string& code, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxTextCtrl* register_text_input(const std::string& code, const wxString& label, const wxString& def = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxTextCtrl* register_text_input_multi(const std::string& code, const wxString& label, int lines = 5, const wxString& def = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxCheckBox* register_checkbox(const std::string& code, const wxString& label, bool def = false, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		wxButton* register_button_field(const wxString& label, int id = wxID_ANY);
		wxButton* register_button(const wxString& label, int id = wxID_ANY);
	private:
		std::unordered_map<std::string, wxWindow*> controls_;
	};
}