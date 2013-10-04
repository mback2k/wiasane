#ifndef COISANE_PROP_H
#define COISANE_PROP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>

#include "coisane.h"


DWORD AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData);

INT_PTR CALLBACK DialogProcPropertyPageAdvanced(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp);

#endif
