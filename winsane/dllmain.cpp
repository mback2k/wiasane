#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	WSADATA wsaData;

	switch (ul_reason_for_call)
	{
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
