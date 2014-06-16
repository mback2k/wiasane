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

#ifndef COISANE_WIZARD_H
#define COISANE_WIZARD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>

#include "coisane.h"
#include "winsane.h"


DWORD WINAPI NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData);

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
INT_PTR CALLBACK DialogProcWizardPageProgress(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

UINT CALLBACK PropSheetPageProcWizardPage(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp);

BOOL WINAPI InitWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData);
BOOL WINAPI NextWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData);
DWORD WINAPI ProcWizardPageServer(_In_ LPVOID lpParameter);

BOOL WINAPI InitWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData);
BOOL WINAPI NextWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData);
DWORD WINAPI ProcWizardPageScanner(_In_ LPVOID lpParameter);

BOOL WINAPI InitWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData);

WINSANE_API_CALLBACK WizardPageAuthCallback(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password);

#endif
