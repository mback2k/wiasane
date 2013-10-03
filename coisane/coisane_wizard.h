#ifndef COISANE_WIZARD_H
#define COISANE_WIZARD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <setupapi.h>

typedef struct _COISANE_Wizard_Page_Data {
	HDEVINFO DeviceInfoSet;
	PSP_DEVINFO_DATA DeviceInfoData;
} COISANE_Wizard_Page_Data, *PCOISANE_Wizard_Page_Data;


DWORD NewDeviceWizardFinishInstallServer(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData);
DWORD NewDeviceWizardFinishInstallScanner(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData);

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

UINT CALLBACK PropSheetPageProcWizardPageServer(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp);
UINT CALLBACK PropSheetPageProcWizardPageScanner(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp);

#endif
