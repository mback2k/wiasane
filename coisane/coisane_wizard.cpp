#include "stdafx.h"
#include "coisane_wizard.h"

#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

#include "winsane.h"

#include "resource.h"
#include "coisane_util.h"

DWORD NewDeviceWizardFinishInstallServer(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
	PCOISANE_Wizard_Page_Data pWizardPageData;
	SP_NEWDEVICEWIZARD_DATA newDeviceWizardData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	BOOL res;

	ZeroMemory(&newDeviceWizardData, sizeof(newDeviceWizardData));
	newDeviceWizardData.ClassInstallHeader.cbSize = sizeof(newDeviceWizardData.ClassInstallHeader);
	newDeviceWizardData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData), NULL);
	if (!res)
		return GetLastError();

	if (newDeviceWizardData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
		return NO_ERROR;

	pWizardPageData = new COISANE_Wizard_Page_Data;
	if (!pWizardPageData)
		return ERROR_OUTOFMEMORY;

	ZeroMemory(pWizardPageData, sizeof(COISANE_Wizard_Page_Data));
	pWizardPageData->DeviceInfoSet = DeviceInfoSet;
	pWizardPageData->DeviceInfoData = DeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageServer;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPageServer;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SERVER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_SUBTITLE);
	propSheetPage.lParam = (LPARAM) pWizardPageData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage) {
		delete pWizardPageData;
		return GetLastError();
	}
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

DWORD NewDeviceWizardFinishInstallScanner(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
	PCOISANE_Wizard_Page_Data pWizardPageData;
	SP_NEWDEVICEWIZARD_DATA newDeviceWizardData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	BOOL res;

	ZeroMemory(&newDeviceWizardData, sizeof(newDeviceWizardData));
	newDeviceWizardData.ClassInstallHeader.cbSize = sizeof(newDeviceWizardData.ClassInstallHeader);
	newDeviceWizardData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData), NULL);
	if (!res)
		return GetLastError();

	if (newDeviceWizardData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
		return NO_ERROR;

	pWizardPageData = new COISANE_Wizard_Page_Data;
	if (!pWizardPageData)
		return ERROR_OUTOFMEMORY;

	ZeroMemory(pWizardPageData, sizeof(COISANE_Wizard_Page_Data));
	pWizardPageData->DeviceInfoSet = DeviceInfoSet;
	pWizardPageData->DeviceInfoData = DeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageScanner;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPageScanner;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SCANNER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_SUBTITLE);
	propSheetPage.lParam = (LPARAM) pWizardPageData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage) {
		delete pWizardPageData;
		return GetLastError();
	}
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	PCOISANE_Wizard_Page_Data pWizardPageData;
	LPPROPSHEETPAGE lpPropSheetPage;

	Trace(TEXT("DialogProcWizardPageServer(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			lpPropSheetPage = (LPPROPSHEETPAGE) lParam;
			pWizardPageData = (PCOISANE_Wizard_Page_Data) lpPropSheetPage->lParam;

			InitWizardPageServer(hwndDlg, pWizardPageData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			pWizardPageData = (PCOISANE_Wizard_Page_Data) lpPropSheetPage->lParam;

			switch (((NMHDR*) lParam)->code) {
				case PSN_SETACTIVE:
					Trace(TEXT("PSN_SETACTIVE"));
					PropSheet_SetWizButtons(((LPNMHDR) lParam)->hwndFrom, PSWIZB_NEXT);
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (!NextWizardPageServer(hwndDlg, pWizardPageData)) {
						MessageBox(hwndDlg, TEXT("Unable to connect to the specified server!"), TEXT("Error"), MB_OK | MB_ICONERROR);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;
			}
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	Trace(TEXT("DialogProcWizardPageScanner(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			switch (((LPNMHDR) lParam)->code) {
				case PSN_SETACTIVE:
					Trace(TEXT("PSN_SETACTIVE"));
					PropSheet_SetWizButtons(((LPNMHDR) lParam)->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;
			}
			break;
	}

	return FALSE;
}

UINT CALLBACK PropSheetPageProcWizardPageServer(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Wizard_Page_Data pWizardPageData;
	UINT ret;

	Trace(TEXT("PropSheetPageProcWizardPageServer(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);
	
	ret = 0;

	switch (uMsg) {
		case PSPCB_ADDREF:
			Trace(TEXT("PSPCB_ADDREF"));
			break;

		case PSPCB_CREATE:
			Trace(TEXT("PSPCB_CREATE"));
			ret = 1;
			break;

		case PSPCB_RELEASE:
			Trace(TEXT("PSPCB_RELEASE"));
			pWizardPageData = (PCOISANE_Wizard_Page_Data) ppsp->lParam;
			delete pWizardPageData;
			ppsp->lParam = NULL;
			break;
	}

	return ret;
}

UINT CALLBACK PropSheetPageProcWizardPageScanner(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Wizard_Page_Data pWizardPageData;
	UINT ret;

	Trace(TEXT("PropSheetPageProcWizardPageScanner(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);

	ret = 0;

	switch (uMsg) {
		case PSPCB_ADDREF:
			Trace(TEXT("PSPCB_ADDREF"));
			break;

		case PSPCB_CREATE:
			Trace(TEXT("PSPCB_CREATE"));
			ret = 1;
			break;

		case PSPCB_RELEASE:
			Trace(TEXT("PSPCB_RELEASE"));
			pWizardPageData = (PCOISANE_Wizard_Page_Data) ppsp->lParam;
			delete pWizardPageData;
			ppsp->lParam = NULL;
			break;
	}

	return ret;
}

BOOL InitWizardPageServer(HWND hwndDlg, PCOISANE_Wizard_Page_Data pWizardPageData)
{
	INFCONTEXT InfContext;
	HINF InfFile;
	PTCHAR strField;
	INT intField;
	DWORD size;
	BOOL res;

	InfFile = OpenInfFile(pWizardPageData->DeviceInfoSet, pWizardPageData->DeviceInfoData, NULL);
	if (InfFile != INVALID_HANDLE_VALUE) {
		res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Host"), &InfContext);
		if (res) {
			res = SetupGetStringField(&InfContext, 1, NULL, 0, &size);
			if (res) {
				strField = new TCHAR[size];
				if (strField) {
					res = SetupGetStringField(&InfContext, 1, strField, size, NULL);
					if (res) {
						res = SetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, strField);
					}
					delete strField;
				} else
					res = FALSE;
			}
		}

		if (res) {
			res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Port"), &InfContext);
			if (res) {
				res = SetupGetIntField(&InfContext, 1, &intField);
				if (res) {
					res = SetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, intField, FALSE);
				}
			}
		}

		SetupCloseInfFile(InfFile);
	} else
		res = FALSE;

	return res;
}

BOOL NextWizardPageServer(HWND hwndDlg, PCOISANE_Wizard_Page_Data pWizardPageData)
{
	WINSANE_Session *session;
	BOOL res;

	if (pWizardPageData->Host) {
		delete pWizardPageData->Host;
	}

	pWizardPageData->Host = new TCHAR[256];
	if (pWizardPageData->Host) {
		res = GetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, pWizardPageData->Host, 256);
	} else {
		res = FALSE;
	}
	if (!res) {
		pWizardPageData->Host = _tcsdup(TEXT("localhost"));
	}

	pWizardPageData->Port = (USHORT) GetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, &res, FALSE);
	if (!res) {
		pWizardPageData->Port = WINSANE_DEFAULT_PORT;
	}

	res = FALSE;

	session = WINSANE_Session::Remote(pWizardPageData->Host, pWizardPageData->Port);
	if (session) {
		if (session->Init(NULL, NULL)) {
			res = session->Exit();
		}
		delete session;
	}

	return res;
}
