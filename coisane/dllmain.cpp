#include "dllmain.h"

#include <commctrl.h>

HINSTANCE g_hModuleInstance = NULL; // instance of this CoInstaller (used for loading from a resource)
HANDLE g_hActivationContext = NULL; // global ActivationContext handle
BOOL g_bCommonControls = FALSE;	// global CommonControls initialization status

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX initCommonControlsEx;
	ACTCTX actCtx;

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
			if (!g_hActivationContext) {
				ZeroMemory(&actCtx, sizeof(ACTCTX));
				actCtx.cbSize = sizeof(ACTCTX);
				actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
				actCtx.hModule = hModule;
				actCtx.lpResourceName = TEXT("MANIFEST");
				g_hActivationContext = CreateActCtx(&actCtx);
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

HANDLE GetActivationContext()
{
	return g_hActivationContext;
}
