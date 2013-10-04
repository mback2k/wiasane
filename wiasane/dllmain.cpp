#include "dllmain.h"

HINSTANCE g_hModuleInstance = NULL; // instance of this MicroDriver (used for loading from a resource)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(dwReason);
	UNREFERENCED_PARAMETER(lpReserved);

	g_hModuleInstance = (HINSTANCE) hModule;

	return TRUE;
}

HINSTANCE GetModuleInstance()
{
	return g_hModuleInstance;
}
