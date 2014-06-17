/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2014, Marc Hoersken, <info@marc-hoersken.de>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this software distribution.
 *
 * You may opt to use, copy, modify, and distribute this software for any
 * purpose with or without fee, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either expressed or implied.
 *
 * This file was inspired by the example source code provided within the
 * Toaster Sample Driver package and CoInstaller project which could be
 * found at the following download location (last accessed 2013-11-21):
 * http://code.msdn.microsoft.com/windowshardware/Toaster-7d256224
 *
 ***************************************************************************/

#include "coisane.h"

#include "coisane_util.h"
#include "coisane_prop.h"
#include "coisane_wizard.h"
#include "strutil_dbg.h"


DWORD CALLBACK CoInstaller(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, OPTIONAL _Inout_ PCOINSTALLER_CONTEXT_DATA Context)
{
	DWORD ret;

	UNREFERENCED_PARAMETER(Context);

	ret = NO_ERROR;

	switch (InstallFunction) {
		case DIF_INSTALLDEVICE:
			Trace(TEXT("DIF_INSTALLDEVICE"));
			ret = UpdateInstallDeviceFlags(hDeviceInfoSet, pDeviceInfoData, DI_INSTALLDISABLED);
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
			ret = NewDeviceWizardFinishInstall(InstallFunction, hDeviceInfoSet, pDeviceInfoData);
			break;

		case DIF_INSTALLINTERFACES:
			Trace(TEXT("DIF_INSTALLINTERFACES"));
			ret = UpdateInstallDeviceFlags(hDeviceInfoSet, pDeviceInfoData, DI_INSTALLDISABLED);
			break;

		case DIF_DETECTCANCEL:
			Trace(TEXT("DIF_DETECTCANCEL"));
			break;

		case DIF_REGISTER_COINSTALLERS:
			Trace(TEXT("DIF_REGISTER_COINSTALLERS"));
			break;

		case DIF_ADDPROPERTYPAGE_ADVANCED:
			Trace(TEXT("DIF_ADDPROPERTYPAGE_ADVANCED"));
			ret = AddPropertyPageAdvanced(InstallFunction, hDeviceInfoSet, pDeviceInfoData);
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
