#include "dllmain.h"

#include <commctrl.h>

HINSTANCE g_hModuleInstance = NULL; // instance of this CoInstaller (used for loading from a resource)
BOOL g_bCommonControls = FALSE;	// global CommonControls initialization status

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX initCommonControlsEx;

	UNREFERENCED_PARAMETER(lpReserved);

	g_hModuleInstance = (HINSTANCE) hModule;

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			if (!g_bCommonControls) {
				ZeroMemory(&initCommonControlsEx, sizeof(INITCOMMONCONTROLSEX));
				initCommonControlsEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
				initCommonControlsEx.dwICC = ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
				g_bCommonControls = InitCommonControlsEx(&initCommonControlsEx);
			}
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

HINSTANCE GetModuleInstance()
{
	return g_hModuleInstance;
}
