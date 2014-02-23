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

#include "coisane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "dllmain.h"
#include "resource.h"
#include "strutil.h"


VOID Trace(_In_ LPCTSTR pszFormat, ...)
{
#ifdef _DEBUG
	PTSTR lpMsg, lpOut;
	va_list argList;
	HANDLE hHeap;
	size_t cbLen;
	HRESULT hr;

	hHeap = GetProcessHeap();
	if (hHeap) {
		va_start(argList, pszFormat);
		hr = StringCchAVPrintf(hHeap, &lpMsg, &cbLen, pszFormat, argList);
		va_end(argList);
		if (SUCCEEDED(hr)) {
			hr = StringCchAPrintf(hHeap, &lpOut, &cbLen, TEXT("coisane: %s\r\n"), lpMsg);
			if (SUCCEEDED(hr)) {
				OutputDebugString(lpOut);
				HeapFree(hHeap, 0, lpOut);
			}
			HeapFree(hHeap, 0, lpMsg);
		}
	}
#else
	UNREFERENCED_PARAMETER(pszFormat);
#endif
}


HINF OpenInfFile(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _Out_opt_ PUINT ErrorLine)
{
	SP_DRVINFO_DATA DriverInfoData;
	SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
	HINF FileHandle;

	if (NULL != ErrorLine)
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


DWORD UpdateInstallDeviceFlags(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags)
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

	return NO_ERROR;
}

DWORD ChangeDeviceState(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD StateChange, _In_ DWORD Scope)
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

	return NO_ERROR;
}

DWORD UpdateDeviceInfo(_In_ PCOISANE_Data privateData, _In_ PWINSANE_Device device)
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

	ret = NO_ERROR;

	name = device->GetName();
	type = device->GetType();
	model = device->GetModel();
	vendor = device->GetVendor();

	hDeviceKey = SetupDiOpenDevRegKey(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_SET_VALUE);

	if (vendor && model) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs"), vendor, model);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) lpStr, (DWORD) cbLen);
			if (hDeviceKey != INVALID_HANDLE_VALUE)
				RegSetValueEx(hDeviceKey, TEXT("FriendlyName"), 0, REG_SZ, (LPBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (vendor && model && type && name) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs %hs (%hs)"), vendor, model, type, name);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (vendor) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs"), vendor);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_MFG, (PBYTE) (PBYTE) lpStr, (DWORD) cbLen);
			if (hDeviceKey != INVALID_HANDLE_VALUE)
				RegSetValueEx(hDeviceKey, TEXT("Vendor"), 0, REG_SZ, (LPBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				ret = GetLastError();
		}
	}

	if (hDeviceKey != INVALID_HANDLE_VALUE)
		RegCloseKey(hDeviceKey);

	return ret;
}


DWORD QueryDeviceData(_In_ PCOISANE_Data privateData)
{
	HKEY hDeviceKey, hDeviceDataKey;
	DWORD cbData, dwType, dwPort;
	PTSTR lpString;
	LONG res;

	hDeviceKey = SetupDiOpenDevRegKey(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_QUERY_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		res = RegQueryValueEx(hDeviceDataKey, TEXT("Port"), NULL, &dwType, NULL, &cbData);
		if (res == ERROR_SUCCESS && dwType == REG_DWORD && cbData == sizeof(DWORD)) {
			res = RegQueryValueEx(hDeviceDataKey, TEXT("Port"), NULL, &dwType, (LPBYTE) &dwPort, &cbData);
			if (res == ERROR_SUCCESS) {
				privateData->usPort = (USHORT) dwPort;
			}
		}

		res = RegQueryValueEx(hDeviceDataKey, TEXT("Host"), NULL, &dwType, NULL, &cbData);
		if (res == ERROR_SUCCESS && dwType == REG_SZ) {
			lpString = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, cbData);
			if (lpString) {
				res = RegQueryValueEx(hDeviceDataKey, TEXT("Host"), NULL, &dwType, (LPBYTE) lpString, &cbData);
				if (res == ERROR_SUCCESS) {
					if (privateData->lpHost) {
						HeapFree(privateData->hHeap, 0, privateData->lpHost);
					}
					privateData->lpHost = lpString;
				} else {
					HeapFree(privateData->hHeap, 0, lpString);
				}
			}
		}

		res = RegQueryValueEx(hDeviceDataKey, TEXT("Name"), NULL, &dwType, NULL, &cbData);
		if (res == ERROR_SUCCESS && dwType == REG_SZ) {
			lpString = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, cbData);
			if (lpString) {
				res = RegQueryValueEx(hDeviceDataKey, TEXT("Name"), NULL, &dwType, (LPBYTE) lpString, &cbData);
				if (res == ERROR_SUCCESS) {
					if (privateData->lpName) {
						HeapFree(privateData->hHeap, 0, privateData->lpName);
					}
					privateData->lpName = lpString;
				} else {
					HeapFree(privateData->hHeap, 0, lpString);
				}
			}
		}

		res = RegQueryValueEx(hDeviceDataKey, TEXT("Username"), NULL, &dwType, NULL, &cbData);
		if (res == ERROR_SUCCESS && dwType == REG_SZ) {
			lpString = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, cbData);
			if (lpString) {
				res = RegQueryValueEx(hDeviceDataKey, TEXT("Username"), NULL, &dwType, (LPBYTE) lpString, &cbData);
				if (res == ERROR_SUCCESS) {
					if (privateData->lpUsername) {
						HeapFree(privateData->hHeap, 0, privateData->lpUsername);
					}
					privateData->lpUsername = lpString;
				} else {
					HeapFree(privateData->hHeap, 0, lpString);
				}
			}
		}

		res = RegQueryValueEx(hDeviceDataKey, TEXT("Password"), NULL, &dwType, NULL, &cbData);
		if (res == ERROR_SUCCESS && dwType == REG_SZ) {
			lpString = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, cbData);
			if (lpString) {
				res = RegQueryValueEx(hDeviceDataKey, TEXT("Password"), NULL, &dwType, (LPBYTE) lpString, &cbData);
				if (res == ERROR_SUCCESS) {
					if (privateData->lpPassword) {
						HeapFree(privateData->hHeap, 0, privateData->lpPassword);
					}
					privateData->lpPassword = lpString;
				} else {
					HeapFree(privateData->hHeap, 0, lpString);
				}
			}
		}

		RegCloseKey(hDeviceDataKey);
	}

	RegCloseKey(hDeviceKey);

	return NO_ERROR;
}

DWORD UpdateDeviceData(_In_ PCOISANE_Data privateData, _In_ PWINSANE_Device device)
{
	HKEY hDeviceKey, hDeviceDataKey;
	DWORD cbData, dwPort;
	LPTSTR lpResolutions;
	size_t cbResolutions, cbLength;
	HRESULT hr;
	LONG res;

	if (!device)
		return ERROR_INVALID_PARAMETER;

	hDeviceKey = SetupDiOpenDevRegKey(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	if (device->Open() == SANE_STATUS_GOOD) {
		cbResolutions = CreateResolutionList(privateData, device, &lpResolutions);

		device->Close();
	} else {
		lpResolutions = NULL;
		cbResolutions = 0;
	}

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_SET_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		if (privateData->usPort) {
			dwPort = (DWORD) privateData->usPort;
			RegSetValueEx(hDeviceDataKey, TEXT("Port"), 0, REG_DWORD, (LPBYTE) &dwPort, sizeof(DWORD));
		}

		if (privateData->lpHost) {
			hr = StringCbLength(privateData->lpHost, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Host"), 0, REG_SZ, (LPBYTE) privateData->lpHost, cbData);
			}
		}

		if (privateData->lpName) {
			hr = StringCbLength(privateData->lpName, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Name"), 0, REG_SZ, (LPBYTE) privateData->lpName, cbData);
			}
		}

		if (privateData->lpUsername) {
			hr = StringCbLength(privateData->lpUsername, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Username"), 0, REG_SZ, (LPBYTE) privateData->lpUsername, cbData);
			}
		}

		if (privateData->lpPassword) {
			hr = StringCbLength(privateData->lpPassword, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				RegSetValueEx(hDeviceDataKey, TEXT("Password"), 0, REG_SZ, (LPBYTE) privateData->lpPassword, cbData);
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
		HeapFree(privateData->hHeap, 0, lpResolutions);

	return NO_ERROR;
}


size_t CreateResolutionList(_In_ PCOISANE_Data privateData, _In_ PWINSANE_Device device, _Outptr_result_maybenull_ LPTSTR *ppszResolutions)
{
	PWINSANE_Option resolution;
	PSANE_Word pWordList;
	LPTSTR lpResolutions;
	size_t cbResolutions;
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
						StringCbAPrintf(privateData->hHeap, ppszResolutions, &cbResolutions, TEXT("%d"), pWordList[1]);
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = *ppszResolutions;
							StringCbAPrintf(privateData->hHeap, ppszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, pWordList[index]);
							HeapFree(privateData->hHeap, 0, lpResolutions);
						}
						break;

					case SANE_TYPE_FIXED:
						StringCbAPrintf(privateData->hHeap, ppszResolutions, &cbResolutions, TEXT("%d"), SANE_UNFIX(pWordList[1]));
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = *ppszResolutions;
							StringCbAPrintf(privateData->hHeap, ppszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, SANE_UNFIX(pWordList[index]));
							HeapFree(privateData->hHeap, 0, lpResolutions);
						}
						break;
				}
			}
		}
	}

	return cbResolutions;
}
