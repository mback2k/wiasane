#ifndef COISANE_PROP_H
#define COISANE_PROP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <setupapi.h>

DWORD AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData);

#endif
