#include "stdafx.h"
#include "coisane_wizard.h"

#include "resource.h"
#include "coisane_util.h"

DWORD NewDeviceWizardFinishInstallServer(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
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

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageServer;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPageServer;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SERVER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_SUBTITLE);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

DWORD NewDeviceWizardFinishInstallScanner(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
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

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageScanner;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPageScanner;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SCANNER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_SUBTITLE);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	Trace(TEXT("DialogProcWizardPageServer(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	return FALSE;
}

INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	Trace(TEXT("DialogProcWizardPageScanner(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	return FALSE;
}

UINT CALLBACK PropSheetPageProcWizardPageServer(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	Trace(TEXT("PropSheetPageProcWizardPageServer(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);
	
	return TRUE;
}

UINT CALLBACK PropSheetPageProcWizardPageScanner(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	Trace(TEXT("PropSheetPageProcWizardPageScanner(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);

	return TRUE;
}
