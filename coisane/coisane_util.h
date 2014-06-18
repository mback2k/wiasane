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
 ***************************************************************************/

#ifndef COISANE_UTIL_H
#define COISANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>

#include "coisane.h"
#include "winsane.h"

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI GetDlgItemAText(_In_ HANDLE hHeap, _In_ HWND hDlg, _In_ int nIDDlgItem, _Outptr_result_nullonfailure_ LPTSTR *plpszText, _Out_opt_ size_t *pcchLength);

_Success_(return != INVALID_HANDLE_VALUE)
HINF WINAPI OpenInfFile(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _Out_opt_ PUINT puiErrorLine);

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateInstallDeviceFlags(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags);
_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateInstallDeviceFlagsEx(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags, _In_ DWORD dwFlagsEx);

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI ChangeDeviceState(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD StateChange, _In_ DWORD Scope);
_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateDeviceInfo(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device oDevice);

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI QueryDeviceData(_In_ PCOISANE_Data pData);
_Success_(return == ERROR_SUCCESS)
DWORD WINAPI UpdateDeviceData(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device oDevice);

_Success_(return == ERROR_SUCCESS)
DWORD WINAPI CreateResolutionList(_In_ PCOISANE_Data pData, _In_ PWINSANE_Device oDevice, _Outptr_result_nullonfailure_ LPTSTR *plpszResolutions, _Out_opt_ size_t *pcbResolutions);

#endif
