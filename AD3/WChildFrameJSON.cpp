#include "WChildFrameJSON.h"
#include <fstream>

namespace eg::ad3
{
	WChildFrameJSON::WChildFrameJSON(const WChildProp& p, const std::string& json_filename) :
		WChildFrame(p),
		json_path(json_filename)
	{
	}

	void WChildFrameJSON::init()
	{
		// Ensure path exists
		if (json_path.has_parent_path())
		{
			const auto parent_path = json_path.parent_path();
			if (not std::filesystem::exists(parent_path))
			{
				if (not std::filesystem::create_directories(parent_path))
				{
					throw std::runtime_error("Failed to create directory: " + parent_path.string());
				}
			}
		}

		if (not std::filesystem::exists(json_path))
		{
			on_new(data_);
			save();
		}
		else
		{
			load();
		}

		paint();
	}

	void WChildFrameJSON::save()
	{
		std::ofstream file(json_path);
		file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

		file << data_.dump(4);
		file.close();
	}

	void WChildFrameJSON::load()
	{
		std::ifstream file(json_path);
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		data_ = nlohmann::ordered_json::parse(file);
	}

	void WChildFrameJSON::paint()
	{
		for (auto& [key, object] : data_.items())
		{
			assert(object.is_object());
			assert(object.contains("type"));

			const auto& type = object.at("type").get_ref<const std::string&>();

			if (type == "text_input")
			{
				assert(object.contains("label"));
				assert(object.contains("value"));

				const auto& label = object.at("label").get_ref<const std::string&>();
				const auto& value = object.at("value").get_ref<const std::string&>();

				register_text_input(key, label, value);
				continue;
			}

			if (type == "button")
			{
				assert(object.contains("label"));
				assert(object.contains("action"));

				const auto& label = object.at("label").get_ref<const std::string&>();
				const auto& action = object.at("action").get_ref<const std::string&>();
				if (action == "01")
				{
					register_button(label)->Bind(wxEVT_BUTTON, &WChildFrameJSON::on_button_01, this);
				}
				else if (action == "02")
				{
					register_button(label)->Bind(wxEVT_BUTTON, &WChildFrameJSON::on_button_02, this);
				}
				else if (action == "03")
				{
					register_button(label)->Bind(wxEVT_BUTTON, &WChildFrameJSON::on_button_03, this);
				}
				else if (action == "04")
				{
					register_button(label)->Bind(wxEVT_BUTTON, &WChildFrameJSON::on_button_04, this);
				}
				else if (action == "05")
				{
					register_button(label)->Bind(wxEVT_BUTTON, &WChildFrameJSON::on_button_05, this);
				}

				continue;
			}

			throw std::runtime_error("Unknown control type: " + type);
		}
	}
}