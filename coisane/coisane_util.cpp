#include "stdafx.h"
#include "coisane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <tchar.h>
#include <strsafe.h>

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
	UNREFERENCED_PARAMETER(format);
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

VOID UpdateDeviceInfo(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ WINSANE_Device *device)
{
	SANE_String_Const name, type, model, vendor;
	HANDLE hHeap;
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;

	if (!device)
		return;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return;

	name = device->GetName();
	type = device->GetType();
	model = device->GetModel();
	vendor = device->GetVendor();

	if (vendor && model) {
		hr = StringCbAPrintf(hHeap, &lpStr, &cbLen, TEXT("%hs %hs"), vendor, model);
		if (SUCCEEDED(hr)) {
			SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(hHeap, 0, lpStr);
		}
	}

	if (vendor && model && type && name) {
		hr = StringCbAPrintf(hHeap, &lpStr, &cbLen, TEXT("%hs %hs %hs (%hs)"), vendor, model, type, name);
		if (SUCCEEDED(hr)) {
			SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(hHeap, 0, lpStr);
		}
	}

	if (vendor) {
		hr = StringCbAPrintf(hHeap, &lpStr, &cbLen, TEXT("%hs"), vendor);
		if (SUCCEEDED(hr)) {
			SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_MFG, (PBYTE) (PBYTE) lpStr, (DWORD) cbLen);
			HeapFree(hHeap, 0, lpStr);
		}
	}
}
