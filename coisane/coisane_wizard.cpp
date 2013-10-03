#include "stdafx.h"
#include "coisane_wizard.h"

#include "resource.h"
#include "coisane_util.h"

DWORD NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
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
	propSheetPage.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PAGE_SERVER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_PAGE_SERVER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_PAGE_SERVER_HEADER_SUBTITLE);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}
