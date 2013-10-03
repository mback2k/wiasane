#ifndef COISANE_UTIL_H
#define COISANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

VOID Trace(_In_ LPCTSTR format, ...);
HINF OpenInfFile(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, _Out_opt_ PUINT ErrorLine);
DWORD ChangeDeviceState(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, _In_ DWORD StateChange, _In_ DWORD Scope);

#endif
