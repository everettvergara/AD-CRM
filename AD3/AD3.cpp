#include <wx/wx.h>

#include "Common/AppRegister.hpp"
#include "Common/ComInitializer.hpp"
#include "Common/Log.hpp"
#include "ConfigSettings.hpp"

#include "WAppAD3.hpp"

static constexpr auto k_app_name = "AD3";

wxIMPLEMENT_APP_NO_MAIN(eg::ad3::WAppAD3);

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
	try
	{
		eg::sys::AppRegister app_register(k_app_name);
		eg::sys::ComInitializer com_initializer(COINIT_APARTMENTTHREADED);

		eg::sys::Log::init(k_app_name);
		eg::sys::Config<eg::ad3::ConfigSettings>::init(eg::ad3::k_settings_filename);

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

		return EXIT_SUCCESS;
	}

	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	catch (...)
	{
		MessageBoxA(nullptr, "Fatal error: unknown exception", "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
}