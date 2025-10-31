#pragma once

#include <string>
#include <memory>

#define CURL_STATICLIB 1
#include "curl/curl.h"

#include "Common/Log.hpp"

namespace eg::ftp
{
	class ServiceFTP
	{
	public:
		static void init()
		{
			LOG_II("ServiceFTP::init:");
			assert(instance_ == nullptr);

			instance_ = std::unique_ptr<ServiceFTP>(new ServiceFTP);
		}

		static void shutdown()
		{
			LOG_II("ServiceFTP::shutdown:");

			if (instance_ not_eq nullptr)
			{
			}
		}

		static ServiceFTP& instance()
		{
			assert(instance_ not_eq nullptr);

			return *instance_.get();
		}

		static size_t null_sink(void*, size_t s, size_t n, void*)
		{
			return s * n;
		}

		static bool is_dir_exists(const std::string& url, const std::string& user, const std::string& pwd)
		{
			CURL* curl = curl_easy_init();

			if (not curl)
			{
				return false;
			}

			const auto userpwd = user + ":" + pwd;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, null_sink);

			CURLcode res = curl_easy_perform(curl);

			long ftp_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ftp_code);
			curl_easy_cleanup(curl);

			return (res == CURLE_OK);
		}

		static bool mkdir(std::string remote_fullpath, const std::string& userpwd)
		{
			CURL* curl = curl_easy_init();
			if (not curl)
			{
				LOG_XX("curl_easy_init failed");
				return false;
			}

			FILE* dummy = tmpfile();
			if (not dummy)
			{
				curl_easy_cleanup(curl);
				LOG_XX("tmpfile failed");
				return false;
			}

			if (remote_fullpath.back() == '/' or remote_fullpath.back() == '\\')
			{
				remote_fullpath.pop_back();
			}
			remote_fullpath += "/.tmp";

			curl_easy_setopt(curl, CURLOPT_URL, remote_fullpath.c_str());
			curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(curl, CURLOPT_READDATA, dummy);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE, 0L);
			curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, null_sink);

			curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);

			CURLcode res = curl_easy_perform(curl);
			fclose(dummy);
			curl_easy_cleanup(curl);

			if (res not_eq CURLE_OK)
			{
				LOG_XX("Failed to create dirs: {}", curl_easy_strerror(res));
				return false;
			}

			return true;
		}

		static bool upload
		(
			const std::string& local_file,
			const std::string& remote_fullpath,
			const std::string& userpwd
		)
		{
			LOG_II("FTP: local:{} remote:{}", local_file, remote_fullpath);

			CURL* curl = curl_easy_init();
			if (not curl)
			{
				//std::cerr << "curl_easy_init() failed\n";
				LOG_XX("curl_easy_init() failed");
				return false;
			}

			FILE* fp = std::fopen(local_file.c_str(), "rb");
			if (not fp)
			{
				std::cerr << "Cannot open local file: " << local_file << "\n";
				curl_easy_cleanup(curl);
				return false;
			}

			curl_easy_setopt(curl, CURLOPT_URL, remote_fullpath.c_str());
			curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(curl, CURLOPT_READDATA, fp);
			curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);
			curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L);
			curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);

			CURLcode res = curl_easy_perform(curl);

			std::fclose(fp);
			curl_easy_cleanup(curl);

			if (res not_eq CURLE_OK)
			{
				LOG_XX("Upload failed: {}", curl_easy_strerror(res));
				return false;
			}

			return true;
		}

		~ServiceFTP()
		{
			curl_global_cleanup();
		}

	private:
		inline static std::unique_ptr<ServiceFTP> instance_;

		ServiceFTP()
		{
			if (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_FAILED_INIT)
			{
				throw std::runtime_error("Could not initialize cURL");
			}
		}
	};
}