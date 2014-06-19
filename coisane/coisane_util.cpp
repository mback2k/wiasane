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
HINF WINAPI OpenInfFile(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _Out_opt_ PUINT puiErrorLine)
{
	SP_DRVINFO_DETAIL_DATA driverInfoDetailData;
	SP_DRVINFO_DATA driverInfoData;
	HINF hFileHandle;

	if (puiErrorLine)
		*puiErrorLine = 0;

	driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
	if (!SetupDiGetSelectedDriver(hDeviceInfoSet, pDeviceInfoData, &driverInfoData)) {
		Trace(TEXT("Fail: SetupDiGetSelectedDriver"));
		return INVALID_HANDLE_VALUE;
	}

	driverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	if (!SetupDiGetDriverInfoDetail(hDeviceInfoSet, pDeviceInfoData, &driverInfoData, &driverInfoDetailData, sizeof(SP_DRVINFO_DETAIL_DATA), NULL)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			Trace(TEXT("Fail: SetupDiGetDriverInfoDetail"));
			return INVALID_HANDLE_VALUE;
		}
	}

	hFileHandle = SetupOpenInfFile(driverInfoDetailData.InfFileName, NULL, INF_STYLE_WIN4, puiErrorLine);
	if (hFileHandle == INVALID_HANDLE_VALUE) {
		Trace(TEXT("Fail: SetupOpenInfFile"));
		return INVALID_HANDLE_VALUE;
	}

	return hFileHandle;
}


_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateInstallDeviceFlags(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags)
{
	return UpdateInstallDeviceFlagsEx(hDeviceInfoSet, pDeviceInfoData, dwFlags, 0);
}

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateInstallDeviceFlagsEx(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags, _In_ DWORD dwFlagsEx)
{
	SP_DEVINSTALL_PARAMS devInstallParams;
	BOOL res;

	ZeroMemory(&devInstallParams, sizeof(devInstallParams));
	devInstallParams.cbSize = sizeof(devInstallParams);
	res = SetupDiGetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	devInstallParams.Flags |= dwFlags;
	devInstallParams.FlagsEx |= dwFlagsEx;
	res = SetupDiSetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	return ERROR_SUCCESS;
}


_Success_(return == ERROR_SUCCESS)
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

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateDeviceInfo(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device oDevice)
{
	SANE_String_Const name, type, model, vendor;
	HKEY hDeviceKey;
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;
	DWORD ret;
	BOOL res;

	if (!oDevice)
		return ERROR_INVALID_PARAMETER;

	ret = ERROR_SUCCESS;

	name = oDevice->GetName();
	type = oDevice->GetType();
	model = oDevice->GetModel();
	vendor = oDevice->GetVendor();

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


_Success_(return == ERROR_SUCCESS)
DWORD WINAPI QueryDeviceData(_In_ PCOISANE_Data pData)
{
	LPTSTR lpHost, lpName, lpUsername, lpPassword;
	HKEY hDeviceKey, hDeviceDataKey;
	DWORD dwPort, ret;
	LONG res;

	if (!pData)
		return ERROR_INVALID_PARAMETER;

	ret = ERROR_SUCCESS;

	hDeviceKey = SetupDiOpenDevRegKey(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_QUERY_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		dwPort = WINSANE_DEFAULT_PORT;
		lpHost = NULL;
		lpName = NULL;
		lpUsername = NULL;
		lpPassword = NULL;

		res = ReadRegistryLong(pData->hHeap, hDeviceDataKey, TEXT("Port"), &dwPort);
		if (res == ERROR_SUCCESS) {
			pData->usPort = (USHORT) dwPort;
		} else
			ret = res;

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Host"), &lpHost, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpHost) {
				HeapSafeFree(pData->hHeap, 0, pData->lpHost);
			}
			pData->lpHost = lpHost;
		} else
			ret = res;

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Name"), &lpName, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpName) {
				HeapSafeFree(pData->hHeap, 0, pData->lpName);
			}
			pData->lpName = lpName;
		} else
			ret = res;

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Username"), &lpUsername, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpUsername) {
				HeapSafeFree(pData->hHeap, 0, pData->lpUsername);
			}
			pData->lpUsername = lpUsername;
		} else
			ret = res;

		res = ReadRegistryString(pData->hHeap, hDeviceDataKey, TEXT("Password"), &lpPassword, NULL);
		if (res == ERROR_SUCCESS) {
			if (pData->lpPassword) {
				HeapSafeFree(pData->hHeap, 0, pData->lpPassword);
			}
			pData->lpPassword = lpPassword;
		} else
			ret = res;

		RegCloseKey(hDeviceDataKey);
	} else
		ret = res;

	RegCloseKey(hDeviceKey);

	return ret;
}

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateDeviceData(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device oDevice)
{
	HKEY hDeviceKey, hDeviceDataKey;
	size_t cbResolutions, cbLength;
	LPTSTR lpResolutions;
	DWORD cbData, dwPort;
	HRESULT hr;
	DWORD ret;
	LONG res;

	if (!pData || !oDevice)
		return ERROR_INVALID_PARAMETER;

	ret = ERROR_SUCCESS;

	hDeviceKey = SetupDiOpenDevRegKey(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	if (oDevice->Open() != SANE_STATUS_GOOD)
		return ERROR_INVALID_ACCESS;

	ret = CreateResolutionList(pData->hHeap, oDevice, &lpResolutions, &cbResolutions);

	oDevice->Close();

	if (ret != ERROR_SUCCESS)
		return ret;

	res = RegOpenKeyEx(hDeviceKey, TEXT("DeviceData"), 0, KEY_SET_VALUE, &hDeviceDataKey);
	if (res == ERROR_SUCCESS) {
		if (pData->usPort) {
			dwPort = (DWORD) pData->usPort;
			res = RegSetValueEx(hDeviceDataKey, TEXT("Port"), 0, REG_DWORD, (LPBYTE) &dwPort, sizeof(DWORD));
			if (res != ERROR_SUCCESS)
				ret = res;
		} else
			ret = ERROR_INVALID_DATA;

		if (pData->lpHost) {
			hr = StringCbLength(pData->lpHost, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				res = RegSetValueEx(hDeviceDataKey, TEXT("Host"), 0, REG_SZ, (LPBYTE) pData->lpHost, cbData);
				if (res != ERROR_SUCCESS)
					ret = res;
			}
		} else
			ret = ERROR_INVALID_DATA;

		if (pData->lpName) {
			hr = StringCbLength(pData->lpName, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				res = RegSetValueEx(hDeviceDataKey, TEXT("Name"), 0, REG_SZ, (LPBYTE) pData->lpName, cbData);
				if (res != ERROR_SUCCESS)
					ret = res;
			}
		} else
			ret = ERROR_INVALID_DATA;

		if (pData->lpUsername) {
			hr = StringCbLength(pData->lpUsername, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				res = RegSetValueEx(hDeviceDataKey, TEXT("Username"), 0, REG_SZ, (LPBYTE) pData->lpUsername, cbData);
				if (res != ERROR_SUCCESS)
					ret = res;
			}
		} else
			ret = ERROR_INVALID_DATA;

		if (pData->lpPassword) {
			hr = StringCbLength(pData->lpPassword, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbLength);
			if (SUCCEEDED(hr)) {
				cbData = (DWORD) cbLength + sizeof(TCHAR);
				res = RegSetValueEx(hDeviceDataKey, TEXT("Password"), 0, REG_SZ, (LPBYTE) pData->lpPassword, cbData);
				if (res != ERROR_SUCCESS)
					ret = res;
			}
		} else
			ret = ERROR_INVALID_DATA;

		if (lpResolutions) {
			cbData = (DWORD) cbResolutions;
			res = RegSetValueEx(hDeviceDataKey, TEXT("Resolutions"), 0, REG_SZ, (LPBYTE) lpResolutions, cbData);
			if (res != ERROR_SUCCESS)
				ret = res;
		}

		RegCloseKey(hDeviceDataKey);
	} else
		ret = res;

	RegCloseKey(hDeviceKey);

	if (lpResolutions)
		HeapSafeFree(pData->hHeap, 0, lpResolutions);

	return ret;
}


_Success_(return == ERROR_SUCCESS)
DWORD WINAPI CreateResolutionList(_In_ HANDLE hHeap, _In_ PWINSANE_Device oDevice, _Outptr_result_maybenull_ LPTSTR *plpszResolutions, _Out_opt_ size_t *pcbResolutions)
{
	LPTSTR lpResolutions, lpszResolutions;
	PWINSANE_Option oResolution;
	PSANE_Range pRangeSpec;
	PSANE_Word pWordList;
	size_t cbResolutions;
	SANE_Word quant;
	HRESULT hr;
	int index;

	if (!plpszResolutions)
		return ERROR_INVALID_PARAMETER;

	*plpszResolutions = NULL;
	if (pcbResolutions)
		*pcbResolutions = 0;

	lpszResolutions = NULL;
	cbResolutions = 0;

	if (!oDevice)
		return ERROR_INVALID_PARAMETER;

	if (oDevice->FetchOptions() != SANE_STATUS_GOOD)
		return ERROR_INVALID_DATA;

	oResolution = oDevice->GetOption("resolution");
	if (!oResolution)
		return ERROR_NOT_SUPPORTED;

	switch (oResolution->GetConstraintType()) {
		case SANE_CONSTRAINT_RANGE:
			pRangeSpec = oResolution->GetConstraintRange();
			if (pRangeSpec->quant)
				quant = pRangeSpec->quant;
			else if (pRangeSpec->min)
				quant = pRangeSpec->min;
			else
				return ERROR_SUCCESS;
			switch (oResolution->GetType()) {
				case SANE_TYPE_INT:
					hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%d"), pRangeSpec->min);
					if (FAILED(hr))
						return ERROR_OUTOFMEMORY;
					for (index = 2; (quant*index) > pRangeSpec->min && (quant*index) <= pRangeSpec->max; index++) {
						lpResolutions = lpszResolutions;
						if (index <= 12)
							hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, quant*index);
						else
							hr = S_FALSE;
						HeapSafeFree(hHeap, 0, lpResolutions);
						lpResolutions = NULL;
						if (FAILED(hr))
							return ERROR_OUTOFMEMORY;
						else if (hr == S_FALSE)
							return ERROR_SUCCESS;
					}
					break;

				case SANE_TYPE_FIXED:
					hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%.0f"), SANE_UNFIX(pRangeSpec->min));
					if (FAILED(hr))
						return ERROR_OUTOFMEMORY;
					for (index = 2; (quant*index) > pRangeSpec->min && (quant*index) <= pRangeSpec->max; index++) {
						lpResolutions = lpszResolutions;
						if (index <= 12)
							hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%s, %.0f"), lpResolutions, SANE_UNFIX(quant*index));
						else
							hr = S_FALSE;
						HeapSafeFree(hHeap, 0, lpResolutions);
						lpResolutions = NULL;
						if (FAILED(hr))
							return ERROR_OUTOFMEMORY;
						else if (hr == S_FALSE)
							return ERROR_SUCCESS;
					}
					break;

				default:
					return ERROR_NOT_SUPPORTED;
					break;
			}
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			pWordList = oResolution->GetConstraintWordList();
			if (pWordList && pWordList[0] > 0) {
				switch (oResolution->GetType()) {
					case SANE_TYPE_INT:
						hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%d"), pWordList[1]);
						if (FAILED(hr))
							return ERROR_OUTOFMEMORY;
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = lpszResolutions;
							hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%s, %d"), lpResolutions, pWordList[index]);
							HeapSafeFree(hHeap, 0, lpResolutions);
							lpResolutions = NULL;
							if (FAILED(hr))
								return ERROR_OUTOFMEMORY;
						}
						break;

					case SANE_TYPE_FIXED:
						hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%.0f"), SANE_UNFIX(pWordList[1]));
						if (FAILED(hr))
							return ERROR_OUTOFMEMORY;
						for (index = 2; index <= pWordList[0]; index++) {
							lpResolutions = lpszResolutions;
							hr = StringCbAPrintf(hHeap, &lpszResolutions, &cbResolutions, TEXT("%s, %.0f"), lpResolutions, SANE_UNFIX(pWordList[index]));
							HeapSafeFree(hHeap, 0, lpResolutions);
							lpResolutions = NULL;
							if (FAILED(hr))
								return ERROR_OUTOFMEMORY;
						}
						break;

					default:
						return ERROR_NOT_SUPPORTED;
						break;
				}
			}
			break;

		default:
			return ERROR_SUCCESS;
			break;
	}

	*plpszResolutions = lpszResolutions;
	if (pcbResolutions)
		*pcbResolutions = cbResolutions;

	return ERROR_SUCCESS;
}
