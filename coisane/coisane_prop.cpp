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
#include "strutil_res.h"
#include "coisane_util.h"


static PCOISANE_Data g_pPropertyPageData = NULL; // global instance of the COISANE data with device information

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
	privateData->hInstance = hInstance;
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
			if (!lpPropSheetPage)
				break;

			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!privateData)
				break;

			switch (((LPNMHDR) lParam)->code) {
				case PSN_APPLY:
					Trace(TEXT("PSN_APPLY"));
					if (!ExitPropertyPageAdvanced(hwndDlg, privateData)) {
						MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
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

			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;
			if (!privateData)
				break;

			switch (HIWORD(wParam)) {
				case BN_CLICKED:
					Trace(TEXT("BN_CLICKED"));
					return DialogProcPropertyPageAdvancedBtnClicked(hwndDlg, LOWORD(wParam), privateData);
					break;
			}
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProcPropertyPageAdvancedBtnClicked(_In_ HWND hwndDlg, _In_ UINT hwndDlgItem, _Inout_ PCOISANE_Data privateData)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PWINSANE_Params oParams;

	oSession = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
	if (oSession) {
		g_pPropertyPageData = privateData;
		if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(privateData->lpName);
				if (oDevice) {
					if (oDevice->Open() == SANE_STATUS_GOOD) {
						switch (hwndDlgItem) {
							case IDC_PROPERTIES_BUTTON_CHECK:
								if (oDevice->GetParams(&oParams) == SANE_STATUS_GOOD) {
									delete oParams;
									MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_CHECK_SUCCESSFUL, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONINFORMATION | MB_OK);
								} else {
									MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_CHECK_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
								}
								break;

							case IDC_PROPERTIES_BUTTON_RESET:
								if (oDevice->Cancel() == SANE_STATUS_GOOD) {
									MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_RESET_SUCCESSFUL, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONINFORMATION | MB_OK);
								} else {
									MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_PROPERTIES_SCANNER_RESET_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
								}
								break;
						}
						oDevice->Close();
					} else {
						MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
					}
				} else {
					MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
				}
			} else {
				MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
			}
			oSession->Exit();
		} else {
			MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
		}
		g_pPropertyPageData = NULL;
		delete oSession;
	} else {
		MessageBoxR(privateData->hHeap, privateData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONEXCLAMATION | MB_OK);
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
						HeapFree(privateData->hHeap, 0, privateData->lpHost);
					if (privateData->lpName)
						HeapFree(privateData->hHeap, 0, privateData->lpName);
					if (privateData->lpUsername)
						HeapFree(privateData->hHeap, 0, privateData->lpUsername);
					if (privateData->lpPassword)
						HeapFree(privateData->hHeap, 0, privateData->lpPassword);

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
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	LONG index;
	HWND hwnd;
	BOOL res;

	QueryDeviceData(privateData);

	if (!privateData->lpHost)
		privateData->lpHost = StringAClone(privateData->hHeap, TEXT("localhost"));

	if (!privateData->usPort)
		privateData->usPort = WINSANE_DEFAULT_PORT;

	oSession = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
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
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	LPTSTR lpName;
	LPTSTR lpUsername;
	LPTSTR lpPassword;
	BOOL res;

	lpName = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
	if (lpName) {
		res = GetDlgItemText(hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, lpName, MAX_PATH);
		if (res) {
			if (privateData->lpName) {
				HeapFree(privateData->hHeap, 0, privateData->lpName);
			}
			privateData->lpName = lpName;
		} else {
			HeapFree(privateData->hHeap, 0, lpName);
		}
	} else {
		res = FALSE;
	}

	if (res) {
		lpUsername = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
		if (lpUsername) {
			if (GetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, lpUsername, MAX_PATH)) {
				if (privateData->lpUsername) {
					HeapFree(privateData->hHeap, 0, privateData->lpUsername);
				}
				privateData->lpUsername = lpUsername;
			} else {
				HeapFree(privateData->hHeap, 0, lpUsername);
			}
		}

		lpPassword = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
		if (lpPassword) {
			if (GetDlgItemText(hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, lpPassword, MAX_PATH)) {
				if (privateData->lpPassword) {
					HeapFree(privateData->hHeap, 0, privateData->lpPassword);
				}
				privateData->lpPassword = lpPassword;
			} else {
				HeapFree(privateData->hHeap, 0, lpPassword);
			}
		}

		oSession = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
		if (oSession) {
			g_pPropertyPageData = privateData;
			if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
				if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
					oDevice = oSession->GetDevice(privateData->lpName);
					if (oDevice) {
						UpdateDeviceInfo(privateData, oDevice);
						UpdateDeviceData(privateData, oDevice);

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
				if (oSession->Exit() != SANE_STATUS_GOOD) {
					res = FALSE;
				}
			} else {
				res = FALSE;
			}
			g_pPropertyPageData = NULL;
			delete oSession;
		} else {
			res = FALSE;
		}
	}

	return res;
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

	lpUsername = StringToA(hHeap, g_pPropertyPageData->lpUsername);
	if (lpUsername) {
		lpPassword = StringToA(hHeap, g_pPropertyPageData->lpPassword);
		if (lpPassword) {
			strcpy_s(username, SANE_MAX_USERNAME_LEN, lpUsername);
			strcpy_s(password, SANE_MAX_PASSWORD_LEN, lpPassword);
			HeapFree(hHeap, 0, lpPassword);
		}
		HeapFree(hHeap, 0, lpUsername);
	}

	Trace(TEXT("Username: %hs (%d)"), username, strlen(username));
	Trace(TEXT("Password: ******** (%d)"), strlen(password));
}
