#include "stdafx.h"
#include "wiasane-coi_util.h"

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
//  Function:   OpenInfFile
//
//  Purpose:    Will open the handle to the INF file being installed
//
//  Arguments:
//      DeviceInfoSet     [in]
//      DeviceInfoData    [in]
//      ErrorLine         [out] // Optional, See SetupOpenInfFile
//
//  Returns:    HINF Handle of INF file
//
HINF OpenInfFile(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, _Out_opt_ PUINT ErrorLine)
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    DWORD Status;
    HINF FileHandle;

    if (NULL != ErrorLine)
        *ErrorLine = 0;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver( DeviceInfoSet,
        DeviceInfoData,
        &DriverInfoData))
    {
        DbgOut("Fail: SetupDiGetSelectedDriver");
        return INVALID_HANDLE_VALUE;
    }

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
        DeviceInfoData,
        &DriverInfoData,
        &DriverInfoDetailData,
        sizeof(SP_DRVINFO_DETAIL_DATA),
        NULL))
    {
        if ((Status = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
        {
            // We don't need the extended information.  Ignore.
        }
        else
        {
            DbgOut("Fail: SetupDiGetDriverInfoDetail");
            return INVALID_HANDLE_VALUE;
        }
    }

    if (INVALID_HANDLE_VALUE == (FileHandle = SetupOpenInfFile(
        DriverInfoDetailData.InfFileName,
        NULL,
        INF_STYLE_WIN4,
        ErrorLine)))
    {
        DbgOut("Fail: SetupOpenInfFile");
    }
    return FileHandle;
}
