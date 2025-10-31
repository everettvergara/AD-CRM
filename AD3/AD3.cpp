#include <wx/wx.h>

// App
#include "Common/AppRegister.hpp"
#include "Common/ComInitializer.hpp"
#include "Common/Log.hpp"

// Configurations
#include "ConfigSettings.hpp"

// Services
#include "Common/ServiceCtrlC.h"
#include "ServicePJEndpoint.h"
#include "ServicePJAccount.h"
#include "ServicePJCalls.h"
#include "ServicePJWavPool.h"
#include "ServiceMsg.h"
#include "ServiceFTP.hpp"

// UI/UX
#include "WAppAD3.hpp"

//#define CURL_STATICLIB 1
//#include "curl/curl.h"

static constexpr auto k_app_name = "AD3";

wxIMPLEMENT_APP_NO_MAIN(eg::ad3::WAppAD3);

// todo: REDIAL = 3:
// todo: Add client/campaign/name

void shutdown()
{
	// Can be called multile times safely
	// Can be called in any order safely

	eg::ftp::ServiceFTP::shutdown();
	eg::ad3::ServiceMsg::shutdown();
	eg::ad3::ServicePJWavPool::shutdown();
	eg::ad3::ServicePJCalls::shutdown();
	eg::ad3::ServicePJAccount::shutdown();
	eg::ad3::ServicePJEndpoint::shutdown();
	eg::sys::ServiceCtrlC::shutdown();
}

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
	try
	{
		// App
		eg::sys::AppRegister app_register(k_app_name);
		eg::sys::ComInitializer com_initializer(COINIT_APARTMENTTHREADED);
		eg::sys::Log::init(k_app_name);

		// Configurations
		eg::sys::Config<eg::ad3::ConfigSettings>::init(eg::ad3::k_settings_filename);

		// Services
		eg::sys::ServiceCtrlC::init();
		eg::ad3::ServicePJEndpoint::init();
		eg::ad3::ServicePJAccount::init();
		eg::ad3::ServicePJCalls::init();
		eg::ad3::ServicePJWavPool::init();
		eg::ad3::ServiceMsg::init();
		eg::ftp::ServiceFTP::init();

		wxApp::SetInstance(new eg::ad3::WAppAD3);
		if (not wxEntryStart(hInst))
		{
			return EXIT_FAILURE;
		}

		if (not wxTheApp->CallOnInit())
		{
			wxEntryCleanup();
			return EXIT_FAILURE;
		}

		wxTheApp->OnRun();
		wxTheApp->OnExit();

		wxEntryCleanup();

		shutdown();

		return EXIT_SUCCESS;
	}

	catch (const std::exception& e)
	{
		shutdown();
		MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	catch (...)
	{
		shutdown();
		MessageBoxA(nullptr, "Fatal error: unknown exception", "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
}