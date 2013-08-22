#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	WSADATA wsaData;

	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			WSAStartup(MAKEWORD(2,2), &wsaData);
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			WSACleanup();
			break;
	}

	return TRUE;
}
