#pragma once

#include <memory>
#include <tuple>
#include <pjsua2.hpp>
#include "Common/NoCopyMove.hpp"
#include "Common/Log.hpp"

namespace eg::ad3
{
	inline constexpr int k_pj_log_level = 0;
	inline constexpr unsigned int k_endpoint_port_no = 5060;
	inline constexpr auto k_pj_null_device = "null device";
	inline constexpr int k_max_calls = PJSUA_MAX_CALLS;

	class MyLogWriter : public pj::LogWriter
	{
	public:
		void write(const pj::LogEntry& entry) noexcept override
		{
			LOG_II("PJ:: {}", entry.msg.c_str());
		}
	};

	class ServicePJEndpoint final :
		public eg::sys::NoCopyMove
	{
	public:
		pj::Endpoint ep;
		std::vector<pj::TransportId> transport_ids;

		~ServicePJEndpoint();
		static void init();
		static void shutdown();
		static ServicePJEndpoint& instance();

		pj::AudioMedia& get_mic();
		pj::AudioMedia& get_speaker();

		std::pair<int, int> get_in_out_count();

	private:
		ServicePJEndpoint();
		int in_, out_;

		inline static std::unique_ptr<ServicePJEndpoint> instance_;
	};
}