#include "stdafx.h"
#include "coisane_prop.h"

#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

#include "resource.h"
#include "coisane_util.h"

DWORD AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
	PCOISANE_Property_Page_Data pPropertyPageData;
	SP_ADDPROPERTYPAGE_DATA addPropertyPageData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	HANDLE hHeap;
	BOOL res;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return ERROR_OUTOFMEMORY;

	ZeroMemory(&addPropertyPageData, sizeof(addPropertyPageData));
	addPropertyPageData.ClassInstallHeader.cbSize = sizeof(addPropertyPageData.ClassInstallHeader);
	addPropertyPageData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData), NULL);
	if (!res)
		return GetLastError();

	if (addPropertyPageData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
		return NO_ERROR;

	pPropertyPageData = (PCOISANE_Property_Page_Data) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(COISANE_Property_Page_Data));
	if (!pPropertyPageData)
		return ERROR_OUTOFMEMORY;

	pPropertyPageData->DeviceInfoSet = DeviceInfoSet;
	pPropertyPageData->DeviceInfoData = DeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcPropertyPageAdvanced;
	propSheetPage.pfnCallback = &PropSheetPageProcPropertyPageAdvanced;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);
	propSheetPage.lParam = (LPARAM) pPropertyPageData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage) {
		HeapFree(hHeap, 0, pPropertyPageData);
		return GetLastError();
	}
	
	addPropertyPageData.DynamicPages[addPropertyPageData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcPropertyPageAdvanced(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	Trace(TEXT("DialogProcPropertyPageAdvanced(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			break;
	}

	return FALSE;
}

UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	UINT ret;

	Trace(TEXT("PropSheetPageProcPropertyPageAdvanced(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);

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
			if (HeapFree(GetProcessHeap(), 0, (LPVOID) ppsp->lParam))
				ppsp->lParam = NULL;
			break;
	}

	return ret;
}
