#ifndef COISANE_UTIL_H
#define COISANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>

#include "coisane.h"
#include "winsane.h"

VOID Trace(_In_ LPCTSTR pszFormat, ...);
HINF OpenInfFile(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _Out_opt_ PUINT ErrorLine);
DWORD ChangeDeviceState(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD StateChange, _In_ DWORD Scope);
DWORD UpdateDeviceInfo(_Inout_ PCOISANE_Data privateData, _In_ PWINSANE_Device device);

#endif
