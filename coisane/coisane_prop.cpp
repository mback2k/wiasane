#include "stdafx.h"
#include "coisane_prop.h"

#include "resource.h"
#include "coisane_util.h"

DWORD AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
	SP_ADDPROPERTYPAGE_DATA addPropertyPageData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	BOOL res;

	ZeroMemory(&addPropertyPageData, sizeof(addPropertyPageData));
	addPropertyPageData.ClassInstallHeader.cbSize = sizeof(addPropertyPageData.ClassInstallHeader);
	addPropertyPageData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData), NULL);
	if (!res)
		return GetLastError();

	if (addPropertyPageData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
		return NO_ERROR;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcPropertyPageAdvanced;
	propSheetPage.pfnCallback = &PropSheetPageProcPropertyPageAdvanced;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	addPropertyPageData.DynamicPages[addPropertyPageData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcPropertyPageAdvanced(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	Trace(TEXT("DialogProcPropertyPageAdvanced(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	return FALSE;
}

UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	Trace(TEXT("PropSheetPageProcPropertyPageAdvanced(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);

	return TRUE;
}
