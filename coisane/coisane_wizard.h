#ifndef COISANE_WIZARD_H
#define COISANE_WIZARD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <setupapi.h>

#include "coisane.h"


DWORD NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData);

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

UINT CALLBACK PropSheetPageProcWizardPage(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp);

BOOL InitWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data PrivateData);
BOOL NextWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data PrivateData);

BOOL InitWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data PrivateData);
BOOL NextWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data PrivateData);

#endif
