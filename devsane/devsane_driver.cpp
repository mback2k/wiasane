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

#include "devsane_driver.h"

#include <setupapi.h>
#include <difxapi.h>

#include "winsane-util_str.h"
#include "winsane-util_mem.h"
#include "winsane-util_dbg.h"
#include "devsane_util.h"


DWORD DriverInstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ int nCmdShow)
{
	INSTALLERINFO installerInfo;
	DWORD dwFlags;
	BOOL needReboot;
	LPVOID lpData;
	DWORD res;

	Trace(TEXT("DriverInstall(%s, %d)"), lpInfPath, nCmdShow);

	res = CreateInstallInfo(hHeap, &installerInfo, &lpData);
	if (res == ERROR_SUCCESS) {
		if (nCmdShow == SW_HIDE)
			dwFlags = DRIVER_PACKAGE_SILENT;
		else
			dwFlags = 0;

		res = DriverPackageInstall(lpInfPath, dwFlags, &installerInfo, &needReboot);
		if (res != ERROR_SUCCESS && res != ERROR_NO_SUCH_DEVINST)
			Trace(TEXT("DriverPackageInstall failed: %08X"), res);

		if (lpData)
			HeapSafeFree(hHeap, 0, lpData);
	}

	return res;
}

DWORD DriverUninstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ int nCmdShow)
{
	INSTALLERINFO installerInfo;
	LPTSTR lpDsInfPath;
	DWORD cbDsInfPath, dwFlags;
	BOOL needReboot;
	LPVOID lpData;
	DWORD res;

	Trace(TEXT("DriverUninstall(%s, %d)"), lpInfPath, nCmdShow);

	cbDsInfPath = 0;
	res = DriverPackageGetPath(lpInfPath, NULL, &cbDsInfPath);
	if (res == ERROR_INSUFFICIENT_BUFFER) {
		lpDsInfPath = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDsInfPath * sizeof(TCHAR));
		if (lpDsInfPath) {
			res = DriverPackageGetPath(lpInfPath, lpDsInfPath, &cbDsInfPath);
			if (res == ERROR_SUCCESS) {
				Trace(TEXT("DsInfPath: %s"), lpDsInfPath);

				res = CreateInstallInfo(hHeap, &installerInfo, &lpData);
				if (res == ERROR_SUCCESS) {
					if (nCmdShow == SW_HIDE)
						dwFlags = DRIVER_PACKAGE_SILENT;
					else
						dwFlags = 0;

					res = DriverPackageUninstall(lpDsInfPath, dwFlags, &installerInfo, &needReboot);
					if (res != ERROR_SUCCESS)
						Trace(TEXT("DriverPackageUninstall failed: %08X"), res);

					if (lpData)
						HeapSafeFree(hHeap, 0, lpData);
				}
			} else
				Trace(TEXT("DriverPackageGetPath 2 failed: %08X"), res);

			HeapSafeFree(hHeap, 0, lpDsInfPath);
		} else
			Trace(TEXT("HeapAlloc failed"));
	} else
		Trace(TEXT("DriverPackageGetPath 1 failed: %08X"), res);

	return res;
}
