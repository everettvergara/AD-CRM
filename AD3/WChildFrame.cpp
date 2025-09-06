#include "WChildFrame.h"

namespace eg::ad3
{
	WChildFrame::WChildFrame(const WChildProp& p) :
		wxMDIChildFrame(p.parent, wxID_ANY, p.title, p.pos, p.size, p.style),
		panel(new wxPanel(this)),
		sizer_header_buttons(new wxBoxSizer(wxVERTICAL)),
		sizer_tree_fields(new wxBoxSizer(wxHORIZONTAL)),
		sizer_tree(p.has_tree ? new wxBoxSizer(wxVERTICAL) : nullptr),
		sizer_fields(new wxFlexGridSizer(0, p.form_columns, k_gap_v, k_gap_h)),
		sizer_buttons(new wxBoxSizer(wxHORIZONTAL))
	{
		assert(p.form_columns > 0 and p.form_columns % 2 == 0);

		// Make input fields growable
		for (int i = 1; i < p.form_columns; i += 2)
		{
			sizer_fields->AddGrowableCol(i);
		}

		// Add spacer
		sizer_buttons->AddStretchSpacer(1);

		// Fix the sizers
		if (p.has_tree)
		{
			sizer_tree_fields->Add(sizer_tree, 0, wxEXPAND | wxALL, k_border);
		}

		sizer_tree_fields->Add(sizer_fields, 1, wxEXPAND | wxALL, k_border);

		sizer_header_buttons->Add(sizer_tree_fields, 1, wxEXPAND | wxALL, k_border);
		sizer_header_buttons->Add(sizer_buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, k_border);

		panel->SetSizer(sizer_header_buttons);
	}

	wxTextCtrl* WChildFrame::register_text_input(const std::string& code, const wxString& label, const wxString& def, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, label);
		auto* input_control = new wxTextCtrl(panel, wxID_ANY, def, wxDefaultPosition, wxDefaultSize, style);

		sizer_fields->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_fields->Add(input_control, 1, wxEXPAND);

		controls_[code] = input_control;
		return input_control;
	}

	wxTextCtrl* WChildFrame::register_text_input_multi(const std::string& code, const wxString& label, int lines, const wxString& def, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, label);
		auto* input_control = new wxTextCtrl(panel, wxID_ANY, def, wxDefaultPosition, wxDefaultSize, style | wxTE_MULTILINE | wxTE_WORDWRAP);

		wxClientDC dc(input_control);
		dc.SetFont(input_control->GetFont());
		const int line_height = dc.GetCharHeight();
		const int height = line_height * lines + k_border;

		input_control->SetMinSize(wxSize(-1, height));
		input_control->SetMaxSize(wxSize(-1, height));

		sizer_fields->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_fields->Add(input_control, 1, wxEXPAND);

		controls_[code] = input_control;
		return input_control;
	}
	wxStaticText* WChildFrame::register_text(const wxString& text, int wrap, const wxPoint& pos, const wxSize& size, long style)
	{
		//assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, "");
		auto* text_control = new wxStaticText(panel, wxID_ANY, text, pos, size, style);

		text_control->Wrap(wrap);

		sizer_fields->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_fields->Add(text_control, 1, wxEXPAND);

		return text_control;
	}

	wxTreeCtrl* WChildFrame::register_tree(const std::string& code, int width, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));
		assert(sizer_tree not_eq nullptr);

		auto* tree_control = new wxTreeCtrl(panel, wxID_ANY, pos, size, style);

		tree_control->SetMinSize(wxSize(width, -1));
		tree_control->SetMaxSize(wxSize(width, -1));

		sizer_tree->Add(tree_control, 1, wxEXPAND);

		controls_[code] = tree_control;
		return tree_control;
	}

	wxCheckBox* WChildFrame::register_checkbox(const std::string& code, const wxString& label, bool def, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, label);
		auto* input_control = new wxCheckBox(panel, wxID_ANY, "");

		input_control->SetValue(def);

		sizer_fields->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_fields->Add(input_control, 1, wxEXPAND);

		controls_[code] = input_control;
		return input_control;
	}

	wxButton* WChildFrame::register_button(const wxString& label, int id)
	{
		auto* button = new wxButton(panel, wxID_ANY, label);
		sizer_buttons->Add(button, 0, wxALL, k_border);
		return button;
	}
}