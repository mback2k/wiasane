#include "coisane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "coisane_str.h"


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
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;
	BOOL res;

	if (!device)
		return ERROR_INVALID_PARAMETER;

	name = device->GetName();
	type = device->GetType();
	model = device->GetModel();
	vendor = device->GetVendor();

	if (vendor && model) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs"), vendor, model);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				return GetLastError();
		}
	}

	if (vendor && model && type && name) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs %hs %hs (%hs)"), vendor, model, type, name);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				return GetLastError();
		}
	}

	if (vendor) {
		hr = StringCbAPrintf(privateData->hHeap, &lpStr, &cbLen, TEXT("%hs"), vendor);
		if (SUCCEEDED(hr)) {
			res = SetupDiSetDeviceRegistryProperty(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_MFG, (PBYTE) (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(privateData->hHeap, 0, lpStr);
			if (!res)
				return GetLastError();
		}
	}

	return NO_ERROR;
}


DWORD UpdateDeviceData(_In_ PCOISANE_Data privateData, _In_ PWINSANE_Device device)
{
	HKEY hDeviceKey, hDeviceDataKey;
	DWORD cbData, dwPort;
	LPTSTR lpResolutions;
	size_t cbResolutions;
	LONG res;

	if (!device)
		return ERROR_INVALID_PARAMETER;

	hDeviceKey = SetupDiOpenDevRegKey(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ENUMERATE_SUB_KEYS);
	if (hDeviceKey == INVALID_HANDLE_VALUE)
		return GetLastError();

	if (device->Open()) {
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
			cbData = (DWORD) _msize(privateData->lpHost);
			RegSetValueEx(hDeviceDataKey, TEXT("Host"), 0, REG_SZ, (LPBYTE) privateData->lpHost, cbData);
		}

		if (privateData->lpName) {
			cbData = (DWORD) _msize(privateData->lpName);
			RegSetValueEx(hDeviceDataKey, TEXT("Name"), 0, REG_SZ, (LPBYTE) privateData->lpName, cbData);
		}

		if (privateData->lpUsername) {
			cbData = (DWORD) _msize(privateData->lpUsername);
			RegSetValueEx(hDeviceDataKey, TEXT("Username"), 0, REG_SZ, (LPBYTE) privateData->lpUsername, cbData);
		}

		if (privateData->lpPassword) {
			cbData = (DWORD) _msize(privateData->lpPassword);
			RegSetValueEx(hDeviceDataKey, TEXT("Password"), 0, REG_SZ, (LPBYTE) privateData->lpPassword, cbData);
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


size_t CreateResolutionList(_In_ PCOISANE_Data privateData, _In_ PWINSANE_Device device, _Inout_ LPTSTR *ppszResolutions)
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

	if (device->FetchOptions() > 0) {
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
