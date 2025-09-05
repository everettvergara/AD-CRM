#pragma once

#include "WChildFrame.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace eg::ad3
{
	class WChildFrameJSON :
		public WChildFrame
	{
	public:

		WChildFrameJSON(const WChildProp& p, const std::string& json_filename);

	protected:

		std::filesystem::path json_path;

		void init();
		virtual void on_new(nlohmann::ordered_json& data) {};

		void save();
		void load();
		void paint();

	public:
		virtual void on_button_01(wxCommandEvent& evt) {}
		virtual void on_button_02(wxCommandEvent& evt) {}
		virtual void on_button_03(wxCommandEvent& evt) {}
		virtual void on_button_04(wxCommandEvent& evt) {}
		virtual void on_button_05(wxCommandEvent& evt) {}

	private:

		nlohmann::ordered_json data_;
	};
}