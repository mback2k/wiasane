/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2014, Marc Hoersken, <info@marc-hoersken.de>
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

#include "coisane_util.h"

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "dllmain.h"
#include "resource.h"
#include "strutil.h"
#include "strutil_reg.h"
#include "strutil_mem.h"
#include "strutil_dbg.h"

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI GetDlgItemAText(_In_ HANDLE hHeap, _In_ HWND hDlg, _In_ int nIDDlgItem, _Outptr_result_nullonfailure_ LPTSTR *plpszText, _Out_opt_ size_t *pcchLength)
{
	HWND hwndDlgItem;
	LPTSTR lpszText;
	size_t cbLength;
	int iLength;

	if (!plpszText)
		return ERROR_INVALID_PARAMETER;

	*plpszText = NULL;
	if (pcchLength)
		*pcchLength = 0;

	hwndDlgItem = GetDlgItem(hDlg, nIDDlgItem);
	if (!hwndDlgItem)
		return GetLastError();

	iLength = GetWindowTextLength(hwndDlgItem);
	if (iLength < 0)
		return ERROR_INVALID_DATA;

	if ((iLength+1) <= iLength)
		return ERROR_INVALID_DATA;

	iLength += 1;
	cbLength = iLength * sizeof(TCHAR);

	lpszText = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbLength);
	if (!lpszText)
		return ERROR_OUTOFMEMORY;

	iLength = GetDlgItemText(hDlg, nIDDlgItem, lpszText, iLength);

	*plpszText = lpszText;
	if (pcchLength)
		*pcchLength = iLength;

	return ERROR_SUCCESS;
}

_Success_(return != INVALID_HANDLE_VALUE)
HINF WINAPI OpenInfFile(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _Out_opt_ PUINT ErrorLine)
{
	SP_DRVINFO_DATA DriverInfoData;
	SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
	HINF FileHandle;

	if (ErrorLine)
		*ErrorLine = 0;

	DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
	if (!SetupDiGetSelectedDriver(hDeviceInfoSet, pDeviceInfoData, &DriverInfoData)) {
		Trace(TEXT("Fail: SetupDiGetSelectedDriver"));
		return INVALID_HANDLE_VALUE;
	}

	DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	if (!SetupDiGetDriverInfoDetail(hDeviceInfoSet, pDeviceInfoData, &DriverInfoData, &DriverInfoDetailData, sizeof(SP_DRVINFO_DETAIL_DATA), NULL)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			Trace(TEXT("Fail: SetupDiGetDriverInfoDetail"));
			return INVALID_HANDLE_VALUE;
		}
	}

	FileHandle = SetupOpenInfFile(DriverInfoDetailData.InfFileName, NULL, INF_STYLE_WIN4, ErrorLine);
	if (FileHandle == INVALID_HANDLE_VALUE) {
		Trace(TEXT("Fail: SetupOpenInfFile"));
	}

	return FileHandle;
}


DWORD WINAPI UpdateInstallDeviceFlags(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags)
{
	SP_DEVINSTALL_PARAMS devInstallParams;
	BOOL res;

	ZeroMemory(&devInstallParams, sizeof(devInstallParams));
	devInstallParams.cbSize = sizeof(devInstallParams);
	res = SetupDiGetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	devInstallParams.Flags |= dwFlags;
	res = SetupDiSetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	return ERROR_SUCCESS;
}

DWORD WINAPI ChangeDeviceState(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD StateChange, _In_ DWORD Scope)
{
	SP_PROPCHANGE_PARAMS propChangeParams;
	BOOL res;

	ZeroMemory(&propChangeParams, sizeof(propChangeParams));
	propChangeParams.ClassInstallHeader.cbSize = sizeof(propChangeParams.ClassInstallHeader);
	propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	propChangeParams.StateChange = StateChange;
	propChangeParams.Scope = Scope;
	propChangeParams.HwProfile = 0;

	res = SetupDiSetClassInstallParams(hDeviceInfoSet, pDeviceInfoData, &propChangeParams.ClassInstallHeader, sizeof(propChangeParams));
	if (!res)
		return GetLastError();

	res = SetupDiChangeState(hDeviceInfoSet, pDeviceInfoData);
	if (!res)
		return GetLastError();

	return ERROR_SUCCESS;
}

DWORD WINAPI UpdateDeviceInfo(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device device)
{
	SANE_String_Const name, type, model, vendor;
	HKEY hDeviceKey;
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;
	DWORD ret;
	BOOL res;

	if (!device)
		return ERROR_INVALID_PARAMETER;

	ret = ERROR_SUCCESS;

	name = device->GetName();
	type = device->GetType();
	model = device->GetModel();
	vendor = device->GetVendor();

	hDeviceKey = SetupDiOpenDevRegKey(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_SET_VALUE);

	if (vendor && model) {
		hr = StringCbAPrintf(pData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs"), vendor, model);
		if (SUCCEEDED(hr) && lpStr) {
			res = SetupDiSetDeviceRegistryProperty(pData->hDeviceInfoSet, pData->pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) lpStr, (DWORD) cbLen);
			if (hDeviceKey != INVALID_HANDLE_VALUE)
				RegSetValueEx(hDeviceKey, TEXT("FriendlyName"), 0, REG_SZ, (LPBYTE) lpStr, (DWORD) cbLen);
			HeapSafeFree(pData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (vendor && model && type && name) {
		hr = StringCbAPrintf(pData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs %hs (%hs)"), vendor, model, type, name);
		if (SUCCEEDED(hr) && lpStr) {
			res = SetupDiSetDeviceRegistryProperty(pData->hDeviceInfoSet, pData->pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) lpStr, (DWORD) cbLen);
			HeapSafeFree(pData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (vendor) {
		hr = StringCbAPrintf(pData->hHeap, &lpStr, &cbLen, TEXT("%hs"), vendor);
		if (SUCCEEDED(hr) && lpStr) {
			res = SetupDiSetDeviceRegistryProperty(pData->hDeviceInfoSet, pData->pDeviceInfoData, SPDRP_MFG, (PBYTE) (PBYTE) lpStr, (DWORD) cbLen);
			if (hDeviceKey != INVALID_HANDLE_VALUE)
				RegSetValueEx(hDeviceKey, TEXT("Vendor"), 0, REG_SZ, (LPBYTE) lpStr, (DWORD) cbLen);
			HeapSafeFree(pData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (hDeviceKey != INVALID_HANDLE_VALUE)
		RegCloseKey(hDeviceKey);

	return ret;
}


DWORD WINAPI QueryDeviceData(_In_ PCOISANE_Data pData)
{
	HKEY hDeviceKey, hDeviceDataKey;
	LPTSTR lpszValue;
	DWORD dwValue;
	LONG res;

	hDeviceKey = SetupDiOpenDevRegKey(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_QUERY_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		res = ReadRegistryLong(pData->hHeap, hDeviceDataKey, TEXT("Port"), &dwValue);
		if (res == ERROR_SUCCESS) {
			pData->usPort = (USHORT) dwValue;
		}

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Host"), &lpszValue, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpHost) {
				HeapSafeFree(pData->hHeap, 0, pData->lpHost);
			}
			pData->lpHost = lpszValue;
		}

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Name"), &lpszValue, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpName) {
				HeapSafeFree(pData->hHeap, 0, pData->lpName);
			}
			pData->lpName = lpszValue;
		}

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Username"), &lpszValue, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpUsername) {
				HeapSafeFree(pData->hHeap, 0, pData->lpUsername);
			}
			pData->lpUsername = lpszValue;
		}

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Password"), &lpszValue, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpPassword) {
				HeapSafeFree(pData->hHeap, 0, pData->lpPassword);
			}
			pData->lpPassword = lpszValue;
		}

		RegCloseKey(hDeviceDataKey);
	}

	RegCloseKey(hDeviceKey);

	return ERROR_SUCCESS;
}

DWORD WINAPI UpdateDeviceData(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device device)
{
	HKEY hDeviceKey, hDeviceDataKey;
	DWORD cbData, dwPort;
	LPTSTR lpResolutions;
	size_t cbResolutions, cbLength;
	HRESULT hr;
	LONG res;

	if (!device)
		return ERROR_INVALID_PARAMETER;

	hDeviceKey = SetupDiOpenDevRegKey(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	if (device->Open() == SANE_STATUS_GOOD) {
		cbResolutions = CreateResolutionList(pData, device, &lpResolutions);

		device->Close();
	} else {
		lpResolutions = NULL;
		cbResolutions = 0;
	}

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_SET_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		if (pData->usPort) {
			dwPort = (DWORD) pData->usPort;
			RegSetValueEx(hDeviceDataKey, TEXT("Port"), 0, REG_DWORD, (LPBYTE) &dwPort, sizeof(DWORD));
		}

		if (pData->lpHost) {
			hr = StringCbLength(pData->lpHost, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Host"), 0, REG_SZ, (LPBYTE) pData->lpHost, cbData);
			}
		}

		if (pData->lpName) {
			hr = StringCbLength(pData->lpName, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Name"), 0, REG_SZ, (LPBYTE) pData->lpName, cbData);
			}
		}

		if (pData->lpUsername) {
			hr = StringCbLength(pData->lpUsername, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Username"), 0, REG_SZ, (LPBYTE) pData->lpUsername, cbData);
			}
		}

		if (pData->lpPassword) {
			hr = StringCbLength(pData->lpPassword, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Password"), 0, REG_SZ, (LPBYTE) pData->lpPassword, cbData);
			}
		}

		if (lpResolutions) {
			cbData = (DWORD) cbResolutions;
			RegSetValueEx(hDeviceDataKey, TEXT("Resolutions"), 0, REG_SZ, (LPBYTE) lpResolutions, cbData);
		}

		RegCloseKey(hDeviceDataKey);
	}

	RegCloseKey(hDeviceKey);

	if (lpResolutions)
		HeapSafeFree(pData->hHeap, 0, lpResolutions);

	return ERROR_SUCCESS;
}


size_t WINAPI CreateResolutionList(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device device, _Outptr_result_maybenull_ LPTSTR *ppszResolutions)
{
	PWINSANE_Option resolution;
	PSANE_Word pWordList;
	LPTSTR lpResolutions;
	size_t cbResolutions;
	HRESULT hr;
	int index;

	if (!ppszResolutions)
		return 0;

	*ppszResolutions = NULL;
	cbResolutions = 0;

	if (device->FetchOptions() == SANE_STATUS_GOOD) {
		resolution = device->GetOption("resolution");
		if (resolution && resolution->GetConstraintType() == SANE_CONSTRAINT_WORD_LIST) {
			pWordList = resolution->GetConstraintWordList();
			if (pWordList && pWordList[0] > 0) {
				switch (resolution->GetType()) {
					case SANE_TYPE_INT:
						hr = StringCbAPrintf(pData->hHeap, ppszResolutions, &cbResolutions, TEXT("%d"), pWordList[1]);
						if (FAILED(hr))
							break;
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = *ppszResolutions;
							if (!lpResolutions)
								break;
							StringCbAPrintf(pData->hHeap, ppszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, pWordList[index]);
							HeapSafeFree(pData->hHeap, 0, lpResolutions);
						}
						break;

					case SANE_TYPE_FIXED:
						hr = StringCbAPrintf(pData->hHeap, ppszResolutions, &cbResolutions, TEXT("%d"), SANE_UNFIX(pWordList[1]));
						if (FAILED(hr))
							break;
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = *ppszResolutions;
							if (!lpResolutions)
								break;
							StringCbAPrintf(pData->hHeap, ppszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, SANE_UNFIX(pWordList[index]));
							HeapSafeFree(pData->hHeap, 0, lpResolutions);
						}
						break;
				}
			}
		}
	}

	return cbResolutions;
}
