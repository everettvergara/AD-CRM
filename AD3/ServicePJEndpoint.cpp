#include "ServicePJEndpoint.h"

#include <cassert>
#include "ConfigSettings.hpp"
#include "Common/Log.hpp"
//#include "pjmedia/config.h"
//#include "pjmedia/endpoint.h"

namespace eg::ad3
{
	ServicePJEndpoint::ServicePJEndpoint() :
		in_(0),
		out_(0)
	{
		LOG_II("ServicePJEndpoint::ServicePJEndpoint:");
		static MyLogWriter logger;

		pj_log_set_level(k_pj_log_level);
		ep.libCreate();
		ep.libInit([]()
			{
				pj::EpConfig ep_config{};
				ep_config.logConfig.level = k_pj_log_level;
				ep_config.logConfig.consoleLevel = k_pj_log_level;
				ep_config.uaConfig.maxCalls = k_max_calls;
				ep_config.uaConfig.threadCnt = k_max_calls;
				ep_config.medConfig.threadCnt = k_max_calls;
				ep_config.medConfig.maxMediaPorts = k_max_calls * 8;

				ep_config.logConfig.writer = &logger;

				return ep_config;
			}());

		const auto [in, out] = [this]
			{
				int in = 0;
				int out = 0;

				const auto& adm = ep.audDevManager();
				//auto devs = adm.enumDev();
				//for (auto& d : devs)
				//	std::cout << d.name << std::endl;

				LOG_II("Checking dev count: {}", adm.getDevCount());

				pjmedia_aud_param param;
				for (unsigned count = adm.getDevCount(), i = 0; i < count; ++i)
				{
					const auto& info = adm.getDevInfo(i);

					LOG_II("Dev found: {} {} ", i, info.name);

					if (info.name == k_pj_null_device)
					{
						continue;
					}
					LOG_II("Opening: {} {} ", i, info.name);
					pj_status_t rc = pjmedia_aud_dev_default_param(i, &param);  // actually open/close device
					if (rc != PJ_SUCCESS)
					{
						LOG_II("Cannot open {} {}", i, info.name);
						//PJ_LOG(1, ("AudCheck", "Device %u FAILED (status=%d)", i, rc));
						throw std::runtime_error("Cannot access io device.");
					}

					LOG_II("Creating stream: {} {} ", i, info.name);
					pjmedia_aud_stream* stream = nullptr;
					auto status = pjmedia_aud_stream_create(&param, nullptr, nullptr, nullptr, &stream);
					if (status != PJ_SUCCESS)
					{
						LOG_II("Cannot stream create {} {}", i, info.name);
						throw std::runtime_error("Cannot stream io device.");
					}

					LOG_II("Destroying stream: {} {} ", i, info.name);
					pjmedia_aud_stream_destroy(stream);

					in += info.inputCount;
					out += info.outputCount;
				}

				return std::make_pair(in, out);
			} ();

		if (in == 0 or out == 0)
		{
			LOG_XX("ServicePJEndpoint::ServicePJEndpoint: No input or output devices found: in{} out{}.", in, out);
			ep.audDevManager().setNullDev();
		}

		in_ = in;
		out_ = out;

		const auto& settings = ConfigSettings::instance();
		transport_ids.reserve(settings.max_ep());

		for (size_t i = 0; i < settings.max_ep(); ++i)
		{
			transport_ids.emplace_back
			(
				ep.transportCreate(PJSIP_TRANSPORT_UDP, [i]
					{
						pj::TransportConfig tp_config;
						tp_config.port = k_endpoint_port_no + i * 2;
						return tp_config;
					}())
			);

			LOG_II("ServicePJEndpoint::ServicePJEndpoint: Transport Created {}", k_endpoint_port_no + i * 2);
		}

		ep.libStart();
		LOG_II("ServicePJEndpoint::ServicePJEndpoint: Lib Started");
	}

	ServicePJEndpoint::~ServicePJEndpoint()
	{
		shutdown();
	}

	void ServicePJEndpoint::init()
	{
		LOG_II("ServicePJEndpoint::init:");
		assert(instance_ == nullptr);

		instance_ = std::unique_ptr<ServicePJEndpoint>(new ServicePJEndpoint);
	}

	void ServicePJEndpoint::shutdown()
	{
		LOG_II("ServicePJEndpoint::shutdown:");
		if (instance_ not_eq nullptr)
		{
			instance_->ep.libDestroy();
		}
	}

	ServicePJEndpoint& ServicePJEndpoint::instance()
	{
		assert(instance_ not_eq nullptr);

		return *instance_.get();
	}

	pj::AudioMedia& ServicePJEndpoint::get_mic()
	{
		return ep.audDevManager().getCaptureDevMedia();
	}

	pj::AudioMedia& ServicePJEndpoint::get_speaker()
	{
		return ep.audDevManager().getPlaybackDevMedia();
	}

	std::pair<int, int> ServicePJEndpoint::get_in_out_count()
	{
		return { in_, out_ };
	}
}