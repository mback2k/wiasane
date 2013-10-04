#include "stdafx.h"
#include "coisane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <strsafe.h>


VOID Trace(_In_ LPCTSTR format, ...)
{
#ifdef _DEBUG
	TCHAR input[1024], output[2048];
	va_list arglist;
	HRESULT hr;

	va_start(arglist, format);
	hr = StringCchVPrintf(input, sizeof(input) / sizeof(TCHAR), format, arglist);
	va_end(arglist);

	if (SUCCEEDED(hr)) {
		hr = StringCchPrintf(output, sizeof(output) / sizeof(TCHAR), TEXT("coisane: %s\r\n"), input);

		if (SUCCEEDED(hr)) {
			OutputDebugString(output);
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
	TCHAR StringBuf[MAX_PATH];
	HRESULT hr;
	size_t len;

	name = device->GetName();
	type = device->GetType();
	model = device->GetModel();
	vendor = device->GetVendor();

	if (vendor && model) {
		hr = StringCbPrintf(StringBuf, sizeof(StringBuf), TEXT("%hs %hs"), vendor, model);
		if (SUCCEEDED(hr)) {
			hr = StringCbLength(StringBuf, sizeof(StringBuf), &len);
			if (SUCCEEDED(hr)) {
				SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) StringBuf, (DWORD) len);
			}
		}
	}

	if (vendor && model && type && name) {
		hr = StringCbPrintf(StringBuf, sizeof(StringBuf), TEXT("%hs %hs %hs (%hs)"), vendor, model, type, name);
		if (SUCCEEDED(hr)) {
			hr = StringCbLength(StringBuf, sizeof(StringBuf), &len);
			if (SUCCEEDED(hr)) {
				SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) StringBuf, (DWORD) len);
			}
		}
	}

	if (vendor) {
		hr = StringCbPrintf(StringBuf, sizeof(StringBuf), TEXT("%hs"), vendor);
		if (SUCCEEDED(hr)) {
			hr = StringCbLength(StringBuf, sizeof(StringBuf), &len);
			if (SUCCEEDED(hr)) {
				SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, pDeviceInfoData, SPDRP_MFG, (PBYTE) StringBuf, (DWORD) len);
			}
		}
	}
}
