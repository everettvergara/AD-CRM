#pragma once

#include <memory>
#include <vector>
#include <pjsua2.hpp>
#include "Common/NoCopyMove.hpp"

namespace eg::ad3
{
	inline constexpr int k_pj_log_level = 0;
	inline constexpr unsigned int k_endpoint_port_no = 5060;
	inline constexpr auto k_pj_null_device = "null device";
	inline constexpr int k_max_calls = PJSUA_MAX_CALLS;

	class ServicePJEndpoint final :
		public eg::sys::NoCopyMove
	{
	public:
		pj::Endpoint ep;
		std::vector<pj::TransportId> transport_ids;

		static void init();
		static void shutdown();
		static ServicePJEndpoint& instance();

		pj::AudioMedia& get_mic();
		pj::AudioMedia& get_speaker();

	private:
		ServicePJEndpoint();

		inline static std::unique_ptr<ServicePJEndpoint> instance_;
	};
}