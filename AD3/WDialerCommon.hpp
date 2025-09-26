#pragma once

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "WChildFrame.h"
#include "PJCallManualDial.hpp"

namespace eg::ad3
{
	constexpr auto k_calls_folder = "calls";
	constexpr auto k_record_folder = "record";

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

	enum class DialerState
	{
		New, Calling, Stopping, JustEnded, Saved, PlayingWav
	};

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

		size_t to_call_count() const
		{
			return max_id - next_id + 1;
		}
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

	struct DialerFilter
	{
		bool is_auto = false;
		size_t client_id;
		size_t campaign_id;
		size_t prio_id;
		size_t status_id;

		std::unordered_map<size_t, std::string> client_master;
		std::unordered_map<size_t, std::string> campaign_master;
		std::unordered_map<size_t, std::string> prio_master;
		std::unordered_map<size_t, std::string> status_master;

		std::vector<ClientDD> clients;

		ClientDD* selected_client;
		CampaignDD* selected_campaign;
		PrioDD* selected_prio;
		StatusSeriesDD* selected_status;
	};

	struct DialerData
	{
		size_t id;
		std::string ucode;
		std::string mobile;
		std::string name;
		std::string status = "PJSIP_INV_STATE_NULL";
		std::string time_of_call;
		std::string time_call_ended;
		std::string remarks;
		std::string file_recording;
		size_t collector_id;
		size_t uploader_contact_id;
		time_t call_confirmed;
		time_t call_ended;

		DialerState state = DialerState::New;

		bool has_call_attempt() const
		{
			return time_of_call.empty();
		}

		bool has_confirmed_status() const
		{
			return status == "PJSIP_INV_STATE_CONFIRMED";
		}

		void save(const std::string& filename)
		{
			state = DialerState::Saved;

			std::ofstream file(filename);
			file << to_json().dump(4);
		}

		void clear()
		{
			id = 0;
			//ucode.clear();
			mobile.clear();
			name.clear();
			time_of_call.clear();
			time_call_ended.clear();
			remarks.clear();
			file_recording.clear();
			collector_id = 0;
			uploader_contact_id = 0;
			status = "PJSIP_INV_STATE_NULL";
			call_confirmed = 0;
			call_ended = 0;

			state = DialerState::New;
		}

		void from_json(const nlohmann::json& data)
		{
			id = data.value("id", 0);
			ucode = data.value("ucode", "");
			mobile = data.value("mobile", "");
			name = data.value("name", "");
			status = data.value("status", "");
			time_of_call = data.value("time_of_call", "");
			time_call_ended = data.value("time_call_ended", "");
			remarks = data.value("remarks", "");
			file_recording = data.value("file_recording", "");
			collector_id = data.value("collector_id", 0);
			uploader_contact_id = data.value("uploader_contact_id", 0);

			state = DialerState::Saved;
		}

		static [[nodiscard]] std::string validated_mobile(const std::string& m)
		{
			std::string validated_mobile;
			for (const auto ch : m)
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

		static [[nodiscard]] std::string trimmed_name(const std::string& n)
		{
			if (n.empty())
			{
				return {};
			}

			std::string sanitized;
			for (const auto ch : n)
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

		nlohmann::json to_json() const
		{
			nlohmann::json data;

			data["id"] = id;
			data["ucode"] = ucode;
			data["mobile"] = mobile;
			data["name"] = name;
			data["status"] = status;
			data["time_of_call"] = time_of_call;
			data["time_call_ended"] = time_call_ended;
			data["remarks"] = remarks;
			data["file_recording"] = file_recording;
			data["collector_id"] = collector_id;
			data["uploader_contact_id"] = uploader_contact_id;

			return data;
		}
	};
}