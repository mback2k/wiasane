#include "stdafx.h"
#include "coisane_wizard.h"

#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

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
	Trace(TEXT("DialogProcWizardPageServer(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
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
