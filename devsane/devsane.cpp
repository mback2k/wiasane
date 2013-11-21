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

#include "devsane.h"

#include <shellapi.h>

#include "devsane_driver.h"
#include "devsane_device.h"

HINSTANCE g_hModuleInstance = NULL; // instance of this Application (used for loading from a resource)

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	PTSTR lpszType, lpszMode, lpszFile;
	HANDLE hHeap;
	HWND hwnd;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	g_hModuleInstance = hInstance;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return ERROR_NOT_ENOUGH_MEMORY;

	if (__argc > 3) {
		lpszType = __targv[1];
		lpszMode = __targv[2];
		lpszFile = __targv[3];

		if (__argc > 4)
			hwnd = (HWND) _tstoi(__targv[4]);
		else
			hwnd = NULL;

		if (!lpszType || !lpszMode || !lpszFile)
			return ERROR_INVALID_COMMAND_LINE;

		if (_tcsicmp(lpszType, TEXT("driver")) == 0) {
			if (_tcsicmp(lpszMode, TEXT("install")) == 0)
				return DriverInstall(hHeap, lpszFile, nCmdShow);
			else if (_tcsicmp(lpszMode, TEXT("uninstall")) == 0)
				return DriverUninstall(hHeap, lpszFile, nCmdShow);
			else
				return ERROR_INVALID_PARAMETER;

		} else if (_tcsicmp(lpszType, TEXT("device")) == 0) {
			if (_tcsicmp(lpszMode, TEXT("install")) == 0)
				return DeviceInstall(hHeap, lpszFile, hwnd, nCmdShow);
			else if (_tcsicmp(lpszMode, TEXT("uninstall")) == 0)
				return DeviceUninstall(hHeap, lpszFile, hwnd, nCmdShow);
			else
				return ERROR_INVALID_PARAMETER;

		} else
			return ERROR_INVALID_PARAMETER;
	} else
		return ERROR_INVALID_COMMAND_LINE;
}

HINSTANCE GetModuleInstance()
{
	return g_hModuleInstance;
}
