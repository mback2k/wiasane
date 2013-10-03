#ifndef COISANE_WIZARD_H
#define COISANE_WIZARD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <setupapi.h>

DWORD NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData);

#endif
