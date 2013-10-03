#include "stdafx.h"
#include "coisane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <strsafe.h>


VOID Trace(_In_ LPCTSTR format, ...)
{
#ifdef _DEBUG
    TCHAR buffer[1024];
    va_list arglist;

	va_start(arglist, format);
    StringCchVPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), format, arglist);
    va_end(arglist);

    OutputDebugString(buffer);
    OutputDebugString(TEXT("\r\n"));
#else
    UNREFERENCED_PARAMETER(format);
#endif
}


HINF OpenInfFile(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, _Out_opt_ PUINT ErrorLine)
{
	SP_DRVINFO_DATA DriverInfoData;
	SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
	HINF FileHandle;

	if (NULL != ErrorLine)
		*ErrorLine = 0;

	DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
	if (!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
		Trace(TEXT("Fail: SetupDiGetSelectedDriver"));
		return INVALID_HANDLE_VALUE;
	}

	DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
	if (!SetupDiGetDriverInfoDetail(DeviceInfoSet, DeviceInfoData, &DriverInfoData,	&DriverInfoDetailData, sizeof(SP_DRVINFO_DETAIL_DATA), NULL)) {
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
