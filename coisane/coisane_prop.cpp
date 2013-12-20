/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
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

#include "coisane_prop.h"

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "dllmain.h"
#include "resource.h"
#include "coisane_util.h"

DWORD AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData)
{
	SP_ADDPROPERTYPAGE_DATA addPropertyPageData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	PCOISANE_Data privateData;
	HANDLE hActCtx, hHeap;
	HINSTANCE hInstance;
	BOOL res;

	hActCtx = GetActivationContext();
	hInstance = GetModuleInstance();
	if (!hInstance)
		return ERROR_OUTOFMEMORY;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return ERROR_OUTOFMEMORY;

	ZeroMemory(&addPropertyPageData, sizeof(addPropertyPageData));
	addPropertyPageData.ClassInstallHeader.cbSize = sizeof(addPropertyPageData.ClassInstallHeader);
	addPropertyPageData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(hDeviceInfoSet, pDeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData), NULL);
	if (!res)
		return GetLastError();

	if (addPropertyPageData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
		return NO_ERROR;

	privateData = (PCOISANE_Data) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(COISANE_Data));
	if (!privateData)
		return ERROR_OUTOFMEMORY;

	privateData->hHeap = hHeap;
	privateData->hDeviceInfoSet = hDeviceInfoSet;
	privateData->pDeviceInfoData = pDeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK;
	propSheetPage.hActCtx = hActCtx;
	propSheetPage.hInstance = hInstance;
	propSheetPage.pfnDlgProc = &DialogProcPropertyPageAdvanced;
	propSheetPage.pfnCallback = &PropSheetPageProcPropertyPageAdvanced;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);
	propSheetPage.lParam = (LPARAM) privateData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	addPropertyPageData.DynamicPages[addPropertyPageData.NumDynamicPages++] = hPropSheetPage;
	res = SetupDiSetClassInstallParams(hDeviceInfoSet, pDeviceInfoData, &addPropertyPageData.ClassInstallHeader, sizeof(addPropertyPageData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcPropertyPageAdvanced(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LPPROPSHEETPAGE lpPropSheetPage;
	PCOISANE_Data privateData;

	UNREFERENCED_PARAMETER(wParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			lpPropSheetPage = (LPPROPSHEETPAGE) lParam;
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			InitPropertyPageAdvanced(hwndDlg, privateData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			switch (((LPNMHDR) lParam)->code) {
				case PSN_APPLY:
					Trace(TEXT("PSN_APPLY"));
					if (!ExitPropertyPageAdvanced(hwndDlg, privateData)) {
						MessageBox(hwndDlg, TEXT("Unable to select the specified scanner!"), TEXT("Error"), MB_OK | MB_ICONERROR);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_RESET:
					Trace(TEXT("PSN_RESET"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					break;
			}
			break;
	}

	return FALSE;
}

UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Data privateData;
	UINT ret;

	UNREFERENCED_PARAMETER(hwnd);

	ret = 0;

	if (ppsp && ppsp->lParam) {
		privateData = (PCOISANE_Data) ppsp->lParam;
	} else {
		privateData = NULL;
	}

	switch (uMsg) {
		case PSPCB_ADDREF:
			Trace(TEXT("PSPCB_ADDREF"));
			if (privateData) {
				privateData->uiReferences++;
			}
			break;

		case PSPCB_CREATE:
			Trace(TEXT("PSPCB_CREATE"));
			if (privateData) {
				ret = 1;
			}
			break;

		case PSPCB_RELEASE:
			Trace(TEXT("PSPCB_RELEASE"));
			if (privateData) {
				privateData->uiReferences--;

				if (privateData->uiReferences == 0) {
					if (privateData->lpHost)
						free(privateData->lpHost);
					if (privateData->lpName)
						free(privateData->lpName);
					if (privateData->lpUsername)
						free(privateData->lpUsername);
					if (privateData->lpPassword)
						free(privateData->lpPassword);

					HeapFree(privateData->hHeap, 0, privateData);
					ppsp->lParam = NULL;
				}
			}
			break;
	}

	return ret;
}

BOOL InitPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	WINSANE_Session *session;
	WINSANE_Device *device;
	int devices, i;
	HWND hwnd;
	BOOL res;

	QueryDeviceData(privateData);

	if (!privateData->lpHost)
		privateData->lpHost = _tcsdup(TEXT("localhost"));

	if (!privateData->usPort)
		privateData->usPort = WINSANE_DEFAULT_PORT;

	session = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
	if (session) {
		if (session->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER);
			devices = session->GetDevices();
			for (i = 0; i < devices; i++) {
				device = session->GetDevice(i);
				if (device) {
					SendMessageA(hwnd, CB_ADDSTRING, (WPARAM) 0, (LPARAM) device->GetName());
				}
			}
			res = session->Exit() == SANE_STATUS_GOOD;
		} else {
			res = FALSE;
		}
		delete session;
	} else {
		res = FALSE;
	}

	if (privateData->lpName)
		SendDlgItemMessage(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) privateData->lpName);

	if (privateData->lpUsername)
		SetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, privateData->lpUsername);

	if (privateData->lpPassword)
		SetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, privateData->lpPassword);

	return res;
}

BOOL ExitPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	SP_DEVINSTALL_PARAMS devInstallParams;
	WINSANE_Session *session;
	WINSANE_Device *device;
	int devices;
	LPTSTR lpName;
	LPTSTR lpUsername;
	LPTSTR lpPassword;
	BOOL res;

	lpName = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
	if (lpName) {
		res = GetDlgItemText(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, lpName, MAX_PATH);
		if (res) {
			if (privateData->lpName) {
				free(privateData->lpName);
			}
			privateData->lpName = _tcsdup(lpName);
		}
		HeapFree(privateData->hHeap, 0, lpName);
	} else {
		res = FALSE;
	}

	if (res) {
		lpUsername = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
		if (lpUsername) {
			if (GetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, lpUsername, MAX_PATH)) {
				if (privateData->lpUsername) {
					free(privateData->lpUsername);
				}
				privateData->lpUsername = _tcsdup(lpUsername);
			}
			HeapFree(privateData->hHeap, 0, lpUsername);
		}

		lpPassword = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
		if (lpPassword) {
			if (GetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, lpPassword, MAX_PATH)) {
				if (privateData->lpPassword) {
					free(privateData->lpPassword);
				}
				privateData->lpPassword = _tcsdup(lpPassword);
			}
			HeapFree(privateData->hHeap, 0, lpPassword);
		}

		session = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
		if (session) {
			if (session->Init(NULL, NULL) == SANE_STATUS_GOOD) {
				devices = session->GetDevices();
				if (devices > 0) {
					device = session->GetDevice(privateData->lpName);
					if (device) {
						UpdateDeviceInfo(privateData, device);
						UpdateDeviceData(privateData, device);

						ZeroMemory(&devInstallParams, sizeof(SP_DEVINSTALL_PARAMS));
						devInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
						res = SetupDiGetDeviceInstallParams(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, &devInstallParams);
						if (res) {
							devInstallParams.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;
							res = SetupDiSetDeviceInstallParams(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, &devInstallParams);
						}
					} else {
						res = FALSE;
					}
				} else {
					res = FALSE;
				}
				if (session->Exit() != SANE_STATUS_GOOD) {
					res = FALSE;
				}
			} else {
				res = FALSE;
			}
			delete session;
		} else {
			res = FALSE;
		}
	}

	return res;
}
