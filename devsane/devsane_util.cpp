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

#include "devsane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "resource.h"
#include "strutil.h"
#include "strutil_mem.h"
#include "strutil_dbg.h"


DWORD CreateInstallInfo(_In_ HANDLE hHeap, _Out_ PINSTALLERINFO pInstallerInfo, _Outptr_result_maybenull_ LPVOID *plpData)
{
	TCHAR szModuleFileName[MAX_PATH + 1];
	LPTSTR lpszSubBlock, lpszValue;
	LPWORD lpwLanguage;
	DWORD dwLen, dwHandle;
	size_t cbSubBlock;
	HMODULE hModule;
	UINT cbSize;
	HRESULT hr;
	BOOL res;

	if (!pInstallerInfo || !plpData)
		return ERROR_INVALID_PARAMETER;

	ZeroMemory(pInstallerInfo, sizeof(INSTALLERINFO));
	*plpData = NULL;

	hModule = GetModuleInstance();
	if (!hModule)
		return ERROR_OUTOFMEMORY;

	dwLen = GetModuleFileName(hModule, szModuleFileName, MAX_PATH);
	if (!dwLen)
		return GetLastError();

	dwLen = GetFileVersionInfoSize(szModuleFileName, &dwHandle);
	if (!dwLen)
		return GetLastError();

	*plpData = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen);
	if (!*plpData)
		return ERROR_OUTOFMEMORY;

	LoadString(hModule, IDS_APPLICATION_ID, (LPWSTR) &pInstallerInfo->pApplicationId, 0);

	res = GetFileVersionInfo(szModuleFileName, dwHandle, dwLen, *plpData);
	if (res) {
		res = VerQueryValue(*plpData, TEXT("\\VarFileInfo\\Translation"), (LPVOID*) &lpwLanguage, &cbSize);
		if (res) {
			Trace(TEXT("Translation: %04x%04x (%d)"), lpwLanguage[0], lpwLanguage[1], cbSize);

			hr = StringCbAPrintf(hHeap, &lpszSubBlock, &cbSubBlock, TEXT("\\StringFileInfo\\%04x%04x\\ProductName"), lpwLanguage[0], lpwLanguage[1]);
			if (SUCCEEDED(hr)) {
				Trace(TEXT("SubBlock: %s (%d)"), lpszSubBlock, cbSubBlock);

				res = VerQueryValue(*plpData, lpszSubBlock, (LPVOID*) &lpszValue, &cbSize);
				if (res) {
					pInstallerInfo->pDisplayName = lpszValue;
					pInstallerInfo->pProductName = lpszValue;
				} else
					Trace(TEXT("VerQueryValue 2 failed: %08X"), GetLastError());

				HeapSafeFree(hHeap, 0, lpszSubBlock);
			} else
				Trace(TEXT("StringCbAPrintf 1 failed: %08X"), hr);

			hr = StringCbAPrintf(hHeap, &lpszSubBlock, &cbSubBlock, TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"), lpwLanguage[0], lpwLanguage[1]);
			if (SUCCEEDED(hr)) {
				Trace(TEXT("SubBlock: %s (%d)"), lpszSubBlock, cbSubBlock);

				res = VerQueryValue(*plpData, lpszSubBlock, (LPVOID*) &lpszValue, &cbSize);
				if (res) {
					pInstallerInfo->pMfgName = lpszValue;
				} else
					Trace(TEXT("VerQueryValue 3 failed: %08X"), GetLastError());

				HeapSafeFree(hHeap, 0, lpszSubBlock);
			} else
				Trace(TEXT("StringCbAPrintf 2 failed: %08X"), hr);
		} else
			Trace(TEXT("VerQueryValue 1 failed: %08X"), GetLastError());
	} else
		Trace(TEXT("GetFileVersionInfo failed: %08X"), GetLastError());

	Trace(TEXT("ApplicationID: %s"), pInstallerInfo->pApplicationId);
	Trace(TEXT("DisplayName:   %s"), pInstallerInfo->pDisplayName);
	Trace(TEXT("ProductName:   %s"), pInstallerInfo->pProductName);
	Trace(TEXT("MfgName:       %s"), pInstallerInfo->pMfgName);

	return ERROR_SUCCESS;
}
