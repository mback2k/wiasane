#include "stdafx.h"

#include <winsock2.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	WSADATA wsaData;

	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			if (!g_bStarted)
				g_bStarted = WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if (g_bStarted)
				g_bStarted = WSACleanup() != 0;
			break;
	}

	return TRUE;
}
