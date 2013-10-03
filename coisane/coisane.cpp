#include "stdafx.h"
#include "coisane.h"

#include "coisane_util.h"
#include "coisane_prop.h"
#include "coisane_wizard.h"

//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1999.
//
//  File:       COINST.C
//
//  Contents:   co-installer hook.
//
//  Notes:      For a complete description of CoInstallers, please see the
//                 Microsoft Windows 2000 DDK Documentation
//
//
// Revision History:
//              Added FriendlyName interface
//
//              Modified to create a friendlyname in the first callback to
//              to install device rather than in post-processing.
//                  - July 28, 2000
//
//----------------------------------------------------------------------------

//+---------------------------------------------------------------------------
//
//  Function:   CoInstaller
//
//  Purpose:    Responds to co-installer messages
//
//  Arguments:
//      InstallFunction   [in]
//      DeviceInfoSet     [in]
//      DeviceInfoData    [in]
//      Context           [inout]
//
//  Returns:    NO_ERROR, ERROR_DI_POSTPROCESSING_REQUIRED, or an error code.
//
DWORD CALLBACK CoInstaller(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL _Inout_ PCOINSTALLER_CONTEXT_DATA Context)
{
	TCHAR FriendlyName[MAX_PATH];
	INFCONTEXT InfContext;
	HINF InfFile;
	HRESULT hr;
	DWORD ret;
	BOOL res;
	size_t len;

	ret = NO_ERROR;

	switch (InstallFunction) {
		case DIF_INSTALLDEVICE:
			if (!Context->PostProcessing) {
				Trace(TEXT("DIF_INSTALLDEVICE"));

				//
				// We wil create here a friendly name for this device
				// based on it's serial number.
				// The bus driver returns the serial No. in the UINumber.
				// field of the device capabiliities structure.
				// So let us get that first .
				//

				hr = StringCbPrintf(FriendlyName, sizeof(FriendlyName), TEXT("Friendly Name Test"));
				if (SUCCEEDED(hr)) {
					hr = StringCbLength(FriendlyName, sizeof(FriendlyName), &len);
					if (SUCCEEDED(hr)) {
						res = SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_FRIENDLYNAME, (BYTE *)FriendlyName, (DWORD)len);
						if (res) {
							Trace(TEXT("SetupDiSetDeviceRegistryProperty: %s"), FriendlyName);
						} else {
							Trace(TEXT("SetupDiSetDeviceRegistryProperty failed!"));
						}
					} else {
						Trace(TEXT("StringCbLength failed!"));
					}
				} else {
					Trace(TEXT("StringCbPrintf failed!"));
				}

				// You can use PrivateData to pass Data needed for PostProcessing
				// Context->PrivateData = Something;

				ret = ERROR_DI_POSTPROCESSING_REQUIRED; //Set for PostProcessing
			} else { // post processing
				//
				// Sample code to show how you can process a custom section
				// in your INF file.
				//

				Trace(TEXT("DIF_INSTALLDEVICE PostProcessing"));

				InfFile = OpenInfFile(DeviceInfoSet, DeviceInfoData, NULL);
				if (InfFile == INVALID_HANDLE_VALUE)
					return GetLastError();

				res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Resolutions"), &InfContext);
				if (res) {
					Trace(TEXT("DIF_INSTALLDEVICE, do something here!"));
				}
			}
			break;

		case DIF_REMOVE:
			Trace(TEXT("DIF_REMOVE"));
			break;

		case DIF_SELECTDEVICE:
			Trace(TEXT("DIF_SELECTDEVICE"));
			break;

		case DIF_ASSIGNRESOURCES:
			Trace(TEXT("DIF_ASSIGNRESOURCES"));
			break;

		case DIF_PROPERTIES:
			Trace(TEXT("DIF_PROPERTIES"));
			break;

		case DIF_FIRSTTIMESETUP:
			Trace(TEXT("DIF_FIRSTTIMESETUP"));
			break;

		case DIF_FOUNDDEVICE:
			Trace(TEXT("DIF_FOUNDDEVICE"));
			break;

		case DIF_SELECTCLASSDRIVERS:
			Trace(TEXT("DIF_SELECTCLASSDRIVERS"));
			break;

		case DIF_VALIDATECLASSDRIVERS:
			Trace(TEXT("DIF_VALIDATECLASSDRIVERS"));
			break;

		case DIF_INSTALLCLASSDRIVERS:
			Trace(TEXT("DIF_INSTALLCLASSDRIVERS"));
			break;

		case DIF_CALCDISKSPACE:
			Trace(TEXT("DIF_CALCDISKSPACE"));
			break;

		case DIF_DESTROYPRIVATEDATA:
			Trace(TEXT("DIF_DESTROYPRIVATEDATA"));
			break;

		case DIF_VALIDATEDRIVER:
			Trace(TEXT("DIF_VALIDATEDRIVER"));
			break;

		case DIF_MOVEDEVICE:
			Trace(TEXT("DIF_MOVEDEVICE"));
			break;

		case DIF_DETECT:
			Trace(TEXT("DIF_DETECT"));
			break;

		case DIF_INSTALLWIZARD:
			Trace(TEXT("DIF_INSTALLWIZARD"));
			break;

		case DIF_DESTROYWIZARDDATA:
			Trace(TEXT("DIF_DESTROYWIZARDDATA"));
			break;

		case DIF_PROPERTYCHANGE:
			Trace(TEXT("DIF_PROPERTYCHANGE"));
			break;

		case DIF_ENABLECLASS:
			Trace(TEXT("DIF_ENABLECLASS"));
			break;

		case DIF_DETECTVERIFY:
			Trace(TEXT("DIF_DETECTVERIFY"));
			break;

		case DIF_INSTALLDEVICEFILES:
			Trace(TEXT("DIF_INSTALLDEVICEFILES"));
			break;

		case DIF_ALLOW_INSTALL:
			Trace(TEXT("DIF_ALLOW_INSTALL"));
			break;

		case DIF_SELECTBESTCOMPATDRV:
			Trace(TEXT("DIF_SELECTBESTCOMPATDRV"));
			break;

		case DIF_REGISTERDEVICE:
			Trace(TEXT("DIF_REGISTERDEVICE"));
			break;

		case DIF_NEWDEVICEWIZARD_PRESELECT:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_PRESELECT"));
			break;

		case DIF_NEWDEVICEWIZARD_SELECT:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_SELECT"));
			break;

		case DIF_NEWDEVICEWIZARD_PREANALYZE:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_PREANALYZE"));
			break;

		case DIF_NEWDEVICEWIZARD_POSTANALYZE:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_POSTANALYZE"));
			break;

		case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_FINISHINSTALL"));
			ret = NewDeviceWizardFinishInstall(InstallFunction, DeviceInfoSet, DeviceInfoData);
			break;

		case DIF_INSTALLINTERFACES:
			Trace(TEXT("DIF_INSTALLINTERFACES"));
			break;

		case DIF_DETECTCANCEL:
			Trace(TEXT("DIF_DETECTCANCEL"));
			break;

		case DIF_REGISTER_COINSTALLERS:
			Trace(TEXT("DIF_REGISTER_COINSTALLERS"));
			break;

		case DIF_ADDPROPERTYPAGE_ADVANCED:
			Trace(TEXT("DIF_ADDPROPERTYPAGE_ADVANCED"));
			ret = AddPropertyPageAdvanced(InstallFunction, DeviceInfoSet, DeviceInfoData);
			break;

		case DIF_ADDPROPERTYPAGE_BASIC:
			Trace(TEXT("DIF_ADDPROPERTYPAGE_BASIC"));
			break;

		case DIF_TROUBLESHOOTER:
			Trace(TEXT("DIF_TROUBLESHOOTER"));
			break;

		case DIF_POWERMESSAGEWAKE:
			Trace(TEXT("DIF_POWERMESSAGEWAKE"));
			break;

		default:
			Trace(TEXT("Unknown install function call"));
			break;
	}

	return ret;
}
