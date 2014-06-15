/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this software distribution.
 *
 * You may opt to use, copy, modify, and distribute this software for any
 * purpose with or without fee, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either expressed or implied.
 *
 ***************************************************************************/

#include "dllmain.h"

#include <winsock2.h>

static BOOL g_bWinsockStarted = FALSE; // global Winsock initialization status

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	WSADATA wsaData;

	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			if (!g_bWinsockStarted)
				g_bWinsockStarted = WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if (g_bWinsockStarted)
				g_bWinsockStarted = WSACleanup() != 0;
			break;
	}

	return TRUE;
}

BOOL IsWinsockStarted()
{
	return g_bWinsockStarted;
}
