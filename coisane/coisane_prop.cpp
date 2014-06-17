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

#include "coisane_prop.h"

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "dllmain.h"
#include "resource.h"
#include "strutil.h"
#include "strutil_dbg.h"
#include "strutil_mem.h"
#include "strutil_res.h"
#include "coisane_util.h"


static PCOISANE_Data g_pPropertyPageData = NULL; // global instance of the COISANE data with device information

DWORD WINAPI AddPropertyPageAdvanced(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData)
{
	SP_ADDPROPERTYPAGE_DATA addPropertyPageData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	PCOISANE_Data pData;
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

	pData = (PCOISANE_Data) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(COISANE_Data));
	if (!pData)
		return ERROR_OUTOFMEMORY;

	pData->hHeap = hHeap;
	pData->hInstance = hInstance;
	pData->hDeviceInfoSet = hDeviceInfoSet;
	pData->pDeviceInfoData = pDeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK;
	propSheetPage.hActCtx = hActCtx;
	propSheetPage.hInstance = hInstance;
	propSheetPage.pfnDlgProc = &DialogProcPropertyPageAdvanced;
	propSheetPage.pfnCallback = &PropSheetPageProcPropertyPageAdvanced;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);
	propSheetPage.lParam = (LPARAM) pData;

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
	PCOISANE_Data pData;

	UNREFERENCED_PARAMETER(wParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			lpPropSheetPage = (LPPROPSHEETPAGE) lParam;
			if (!lpPropSheetPage)
				break;

			pData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!pData)
				break;

			pData->uiReferences++;
			pData->hwndDlg = hwndDlg;

			InitPropertyPageAdvanced(hwndDlg, pData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (!lpPropSheetPage)
				break;

			pData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!pData)
				break;

			switch (((LPNMHDR) lParam)->code) {
				case PSN_SETACTIVE:
					Trace(TEXT("PSN_SETACTIVE"));
					pData->hwndPropDlg = ((LPNMHDR) lParam)->hwndFrom;
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					pData->hwndPropDlg = NULL;
					break;

				case PSN_APPLY:
					Trace(TEXT("PSN_APPLY"));
					if (!ExitPropertyPageAdvanced(hwndDlg, pData)) {
						MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
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

		case WM_COMMAND:
			Trace(TEXT("WM_COMMAND"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (!lpPropSheetPage)
				break;

			pData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!pData)
				break;

			switch (HIWORD(wParam)) {
				case BN_CLICKED:
					Trace(TEXT("BN_CLICKED"));
					return DialogProcPropertyPageAdvancedBtnClicked(hwndDlg, LOWORD(wParam), pData);
					break;
			}
			break;

		case WM_DESTROY:
			Trace(TEXT("WM_DESTROY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (!lpPropSheetPage)
				break;

			pData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!pData)
				break;

			pData->uiReferences--;
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProcPropertyPageAdvancedBtnClicked(_In_ HWND hwndDlg, _In_ UINT hwndDlgItem, _Inout_ PCOISANE_Data pData)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PWINSANE_Params oParams;

	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		g_pPropertyPageData = pData;
		if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(pData->lpName);
				if (oDevice) {
					if (oDevice->Open() == SANE_STATUS_GOOD) {
						switch (hwndDlgItem) {
							case IDC_PROPERTIES_BUTTON_CHECK:
								if (oDevice->GetParams(&oParams) == SANE_STATUS_GOOD) {
									delete oParams;
									MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_CHECK_SUCCESSFUL, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONINFORMATION | MB_OK);
								} else {
									MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_CHECK_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
								}
								break;

							case IDC_PROPERTIES_BUTTON_RESET:
								if (oDevice->Cancel() == SANE_STATUS_GOOD) {
									MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_RESET_SUCCESSFUL, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONINFORMATION | MB_OK);
								} else {
									MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_RESET_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
								}
								break;
						}
						oDevice->Close();
					} else {
						MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
					}
				} else {
					MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
				}
			} else {
				MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
			}
			oSession->Exit();
		} else {
			MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
		}
		g_pPropertyPageData = NULL;
		delete oSession;
	} else {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
	}

	return FALSE;
}


UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Data pData;
	UINT ret;

	UNREFERENCED_PARAMETER(hwnd);

	ret = 0;

	if (ppsp && ppsp->lParam) {
		pData = (PCOISANE_Data) ppsp->lParam;
	} else {
		pData = NULL;
	}

	switch (uMsg) {
		case PSPCB_ADDREF:
			Trace(TEXT("PSPCB_ADDREF"));
			if (pData) {
				pData->uiReferences++;
			}
			break;

		case PSPCB_CREATE:
			Trace(TEXT("PSPCB_CREATE"));
			if (pData) {
				ret = 1;
			}
			break;

		case PSPCB_RELEASE:
			Trace(TEXT("PSPCB_RELEASE"));
			if (pData) {
				pData->uiReferences--;

				if (pData->uiReferences == 0) {
					if (pData->lpHost)
						HeapSafeFree(pData->hHeap, 0, pData->lpHost);
					if (pData->lpName)
						HeapSafeFree(pData->hHeap, 0, pData->lpName);
					if (pData->lpUsername)
						HeapSafeFree(pData->hHeap, 0, pData->lpUsername);
					if (pData->lpPassword)
						HeapSafeFree(pData->hHeap, 0, pData->lpPassword);

					HeapSafeFree(pData->hHeap, 0, pData);
					ppsp->lParam = NULL;
				}
			}
			break;
	}

	return ret;
}


BOOL WINAPI InitPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	HINSTANCE hInst;
	HANDLE hIcon;
	LONG index;
	HWND hwnd;
	BOOL res;

	hInst = GetStiCiInstance();
	if (hInst) {
		hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_ICON);
		if (hwnd) {
			hIcon = LoadImage(hInst, MAKEINTRESOURCE(1000), IMAGE_ICON, 32, 32, LR_SHARED);
			if (hIcon) {
				res = PostMessage(hwnd, STM_SETICON, (WPARAM) hIcon, (LPARAM) 0);
			} else {
				res = FALSE;
			}
		} else {
			res = FALSE;
		}
	} else {
		res = FALSE;
	}

	QueryDeviceData(pData);

	if (!pData->lpHost)
		pData->lpHost = StringDup(pData->hHeap, TEXT("localhost"));

	if (!pData->usPort)
		pData->usPort = WINSANE_DEFAULT_PORT;

	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		if (oSession->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER);
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				SendMessageA(hwnd, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
				for (index = 0; index < oSession->GetDevices(); index++) {
					oDevice = oSession->GetDevice(index);
					if (oDevice) {
						SendMessageA(hwnd, CB_ADDSTRING, (WPARAM) 0, (LPARAM) oDevice->GetName());
					}
				}
			}
			res = oSession->Exit() == SANE_STATUS_GOOD;
		} else {
			res = FALSE;
		}
		delete oSession;
	} else {
		res = FALSE;
	}

	if (pData->lpName)
		SendDlgItemMessage(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) pData->lpName);

	if (pData->lpUsername)
		SetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, pData->lpUsername);

	if (pData->lpPassword)
		SetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, pData->lpPassword);

	return res;
}

BOOL WINAPI ExitPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	SP_DEVINSTALL_PARAMS devInstallParams;
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	LPTSTR lpName, lpUsername, lpPassword;
	DWORD res;

	lpName = NULL;
	lpUsername = NULL;
	lpPassword = NULL;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, &lpName, NULL);
	if (res != ERROR_SUCCESS) {
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, &lpUsername, NULL);
	if (res != ERROR_SUCCESS) {
		HeapSafeFree(pData->hHeap, 0, lpName);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, &lpPassword, NULL);
	if (res != ERROR_SUCCESS) {
		HeapSafeFree(pData->hHeap, 0, lpUsername);
		HeapSafeFree(pData->hHeap, 0, lpName);
		return FALSE;
	}

	if (pData->lpName)
		HeapSafeFree(pData->hHeap, 0, pData->lpName);
	if (pData->lpUsername)
		HeapSafeFree(pData->hHeap, 0, pData->lpUsername);
	if (pData->lpPassword)
		HeapSafeFree(pData->hHeap, 0, pData->lpPassword);

	pData->lpName = lpName;
	pData->lpUsername = lpUsername;
	pData->lpPassword = lpPassword;

	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		g_pPropertyPageData = pData;
		if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(pData->lpName);
				if (oDevice) {
					UpdateDeviceInfo(pData, oDevice);
					UpdateDeviceData(pData, oDevice);

					if (oSession->Exit() == SANE_STATUS_GOOD) {
						ZeroMemory(&devInstallParams, sizeof(SP_DEVINSTALL_PARAMS));
						devInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
						res = SetupDiGetDeviceInstallParams(pData->hDeviceInfoSet, pData->pDeviceInfoData, &devInstallParams);
						if (res) {
							devInstallParams.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;
							res = SetupDiSetDeviceInstallParams(pData->hDeviceInfoSet, pData->pDeviceInfoData, &devInstallParams);
							if (res) {
								g_pPropertyPageData = NULL;
								delete oSession;
								return TRUE;
							}
						}
					}
				}
			}
		}
		g_pPropertyPageData = NULL;
		delete oSession;
	}

	return FALSE;
}


WINSANE_API_CALLBACK PropertyPageAuthCallback(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password)
{
	LPSTR lpUsername, lpPassword;
	HANDLE hHeap;

	if (!g_pPropertyPageData || !resource || !strlen(resource) || !username || !password)
		return;

	Trace(TEXT("------ PropertyPageAuthCallback(resource='%hs') ------"), resource);

	hHeap = GetProcessHeap();
	if (!hHeap)
		return;

	lpUsername = StringConvToA(hHeap, g_pPropertyPageData->lpUsername);
	if (lpUsername) {
		lpPassword = StringConvToA(hHeap, g_pPropertyPageData->lpPassword);
		if (lpPassword) {
			strcpy_s(username, SANE_MAX_USERNAME_LEN, lpUsername);
			strcpy_s(password, SANE_MAX_PASSWORD_LEN, lpPassword);
			HeapSafeFree(hHeap, 0, lpPassword);
		}
		HeapSafeFree(hHeap, 0, lpUsername);
	}

	Trace(TEXT("Username: %hs (%d)"), username, strlen(username));
	Trace(TEXT("Password: ******** (%d)"), strlen(password));
}
