#include "WChildFrame.h"

namespace eg::ad3
{
	WChildFrame::WChildFrame(const WChildProp& p) :
		wxMDIChildFrame(p.parent, wxID_ANY, p.title, p.pos, p.size, p.style),
		panel(new wxPanel(this)),
		sizer_header_buttons(new wxBoxSizer(wxVERTICAL)),
		sizer_header(new wxFlexGridSizer(0, p.form_columns, k_gap_v, k_gap_h)),
		sizer_buttons(new wxBoxSizer(wxHORIZONTAL))
	{
		assert(p.form_columns > 0 and p.form_columns % 2 == 0);

		// Make input fields growable
		for (int i = 1; i < p.form_columns; i += 2)
		{
			sizer_header->AddGrowableCol(i);
		}

		// Add spacer
		sizer_buttons->AddStretchSpacer(1);

		// Fix the sizers
		sizer_header_buttons->Add(sizer_header, 1, wxEXPAND | wxALL, k_border);
		sizer_header_buttons->Add(sizer_buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, k_border);

		panel->SetSizer(sizer_header_buttons);
	}

	wxTextCtrl* WChildFrame::register_text_input(const std::string& code, const wxString& label, const wxString& def, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, label);
		auto* input_control = new wxTextCtrl(panel, wxID_ANY, def, wxDefaultPosition, wxDefaultSize, style);

		sizer_header->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_header->Add(input_control, 1, wxEXPAND);

		controls_[code] = input_control;
		return input_control;
	}

	wxCheckBox* WChildFrame::register_checkbox(const std::string& code, const wxString& label, bool def, const wxPoint& pos, const wxSize& size, long style)
	{
		assert(not controls_.contains(code));

		auto* label_control = new wxStaticText(panel, wxID_ANY, label);
		auto* input_control = new wxCheckBox(panel, wxID_ANY, "");

		input_control->SetValue(def);

		sizer_header->Add(label_control, 0, wxALIGN_CENTER_VERTICAL);
		sizer_header->Add(input_control, 1, wxEXPAND);

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