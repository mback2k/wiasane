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
					pData->hwndDlg = hwndDlg;
					pData->hwndPropDlg = ((LPNMHDR) lParam)->hwndFrom;
					if (ShowPropertyPageAdvanced(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_KILLACTIVE:
				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_KILLACTIVE | PSN_QUERYCANCEL"));
					if (!pData->hThread) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					}
					return TRUE;
					break;

				case PSN_APPLY:
					Trace(TEXT("PSN_APPLY"));
					if (SavePropertyPageAdvanced(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
						pData->hwndPropDlg = NULL;
						pData->hwndDlg = NULL;
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
					}
					return TRUE;
					break;

				case PSN_RESET:
					Trace(TEXT("PSN_RESET"));
					pData->hwndPropDlg = NULL;
					pData->hwndDlg = NULL;
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
				case EN_CHANGE:
				case CBN_SELCHANGE:
					Trace(TEXT("EN_CHANGE | CBN_SELCHANGE"));
					switch (LOWORD(wParam)) {
						case IDC_PROPERTIES_COMBO_SCANNER:
						case IDC_PROPERTIES_EDIT_USERNAME:
						case IDC_PROPERTIES_EDIT_PASSWORD:
							PropSheet_Changed(pData->hwndPropDlg, pData->hwndDlg);
							pData->bPropChanged = TRUE;
							break;
					}
					break;

				case BN_CLICKED:
					Trace(TEXT("BN_CLICKED"));
					switch (LOWORD(wParam)) {
						case IDC_PROPERTIES_BUTTON_CHECK:
						case IDC_PROPERTIES_BUTTON_RESET:
							return DialogProcPropertyPageAdvancedBtnClicked(hwndDlg, LOWORD(wParam), pData);
							break;
					}
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

			FreePropertyPageAdvanced(hwndDlg, pData);
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
	LPTSTR lpName;
	BOOL res;

	lpName = NULL;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, &lpName, NULL);
	if (res != ERROR_SUCCESS) {
		return FALSE;
	}

	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		g_pPropertyPageData = pData;
		if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(lpName);
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
						MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
					}
				} else {
					MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
				}
			} else {
				MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
			}
			oSession->Exit();
		} else {
			MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		}
		g_pPropertyPageData = NULL;
		delete oSession;
	} else {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
	}

	HeapSafeFree(pData->hHeap, 0, lpName);

	return FALSE;
}


UINT CALLBACK PropSheetPageProcPropertyPageAdvanced(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Data pData;
	LONG device;
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

					if (pData->lpNames) {
						for (device = 0; pData->lpNames[device]; device++) {
							HeapSafeFree(pData->hHeap, 0, pData->lpNames[device]);
						}
						HeapSafeFree(pData->hHeap, 0, pData->lpNames);
					}

					HeapSafeFree(pData->hHeap, 0, pData);
					ppsp->lParam = NULL;
				}
			}
			break;
	}

	return ret;
}


VOID WINAPI InitPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HINSTANCE hInst;
	HICON hIcon;
	HWND hwnd;

	UNREFERENCED_PARAMETER(pData);

	hInst = GetStiCiInstance();
	if (hInst) {
		hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_ICON);
		if (hwnd) {
			hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(1000), IMAGE_ICON, 32, 32, 0);
			if (hIcon) {
				SendMessage(hwnd, STM_SETICON, (WPARAM) hIcon, (LPARAM) 0);
			}
		}
	}

	hInst = GetWiaDefInstance();
	if (hInst) {
		hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_PROGRESS_ANIMATE);
		if (hwnd) {
			Animate_OpenEx(hwnd, hInst, MAKEINTRESOURCE(1001));
		}
	}
}

VOID WINAPI FreePropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HICON hIcon;
	HWND hwnd;

	UNREFERENCED_PARAMETER(pData);

	hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Close(hwnd);
	}

	hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_ICON);
	if (hwnd) {
		hIcon = (HICON) SendMessage(hwnd, STM_GETICON, (WPARAM) 0, (LPARAM) 0);
		if (hIcon) {
			DestroyIcon(hIcon);
		}
	}
}


BOOL WINAPI ShowPropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	if (pData->hThread)
		return TRUE;

	pData->hThread = CreateThread(NULL, 0, &ThreadProcShowPropertyPageAdvanced, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	ShowPropertyPageAdvancedProgress(hwndDlg);

	SetThreadPriority(pData->hThread, THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(pData->hThread);

	return TRUE;
}

DWORD WINAPI ThreadProcShowPropertyPageAdvanced(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PCOISANE_Data pData;
	LONG devices, device;
	HANDLE hThread;
	LPTSTR lpText;
	HRESULT hr;
	DWORD res;
	HWND hwnd;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	res = QueryDeviceData(pData);
	if (res != ERROR_SUCCESS) {
		HidePropertyPageAdvancedProgress(pData->hwndDlg);

		pData->bPropChanged = FALSE;
		pData->hThread = NULL;

		return 0;
	}

	if (!pData->lpHost)
		pData->lpHost = StringDup(pData->hHeap, TEXT("localhost"));

	if (!pData->usPort)
		pData->usPort = WINSANE_DEFAULT_PORT;

	hr = StringCbAPrintf(pData->hHeap, &lpText, NULL, TEXT("%s:%d"), pData->lpHost, pData->usPort);
	if (SUCCEEDED(hr)) {
		SetDlgItemText(pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT, lpText);
		HeapSafeFree(pData->hHeap, 0, lpText);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				devices = oSession->GetDevices();
				if (devices > 0) {
					if (pData->lpNames) {
						for (device = 0; pData->lpNames[device]; device++) {
							HeapSafeFree(pData->hHeap, 0, pData->lpNames[device]);
						}
						HeapSafeFree(pData->hHeap, 0, pData->lpNames);
					}
					pData->lpNames = (LPTSTR*) HeapAlloc(pData->hHeap, HEAP_ZERO_MEMORY, sizeof(LPTSTR) * (devices+1));
					if (pData->lpNames) {
						for (device = 0; device < devices; device++) {
							oDevice = oSession->GetDevice(device);
							if (oDevice) {
								pData->lpNames[device] = StringConvATo(pData->hHeap, (LPSTR) oDevice->GetName());
							}
						}
					}
				} else if (pData->hThread == hThread) {
					MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
				}
			} else if (pData->hThread == hThread) {
				MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
			}
			oSession->Exit();
		} else if (pData->hThread == hThread) {
			MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		}
		delete oSession;
	} else if (pData->hThread == hThread) {
		MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
	}

	if (pData->hThread != hThread)
		return 0;

	if (pData->lpNames) {
		hwnd = GetDlgItem(pData->hwndDlg, IDC_PROPERTIES_COMBO_SCANNER);
		if (hwnd) {
			SendMessage(hwnd, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
			for (device = 0; pData->lpNames[device]; device++) {
				SendMessage(hwnd, CB_ADDSTRING, (WPARAM) 0, (LPARAM) pData->lpNames[device]);
			}
		}
	}

	if (pData->lpName)
		SendDlgItemMessage(pData->hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) pData->lpName);

	if (pData->lpUsername)
		SetDlgItemText(pData->hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, pData->lpUsername);

	if (pData->lpPassword)
		SetDlgItemText(pData->hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, pData->lpPassword);

	HidePropertyPageAdvancedProgress(pData->hwndDlg);

	pData->bPropChanged = FALSE;
	pData->hThread = NULL;

	return 0;
}


BOOL WINAPI SavePropertyPageAdvanced(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LPTSTR lpName, lpUsername, lpPassword;
	DWORD res;

	if (pData->hThread)
		return FALSE;

	if (pData->bPropChanged == FALSE)
		return TRUE;

	lpName = NULL;
	lpUsername = NULL;
	lpPassword = NULL;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_COMBO_SCANNER, &lpName, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, &lpUsername, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		HeapSafeFree(pData->hHeap, 0, lpName);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, &lpPassword, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
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

	pData->hThread = CreateThread(NULL, 0, &ThreadProcSavePropertyPageAdvanced, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	ShowPropertyPageAdvancedProgress(pData->hwndDlg);

	SetThreadPriority(pData->hThread, THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(pData->hThread);

	return FALSE;
}

DWORD WINAPI ThreadProcSavePropertyPageAdvanced(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PCOISANE_Data pData;
	HANDLE hThread;
	LPTSTR lpText;
	HRESULT hr;
	DWORD res;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	hr = StringCbAPrintf(pData->hHeap, &lpText, NULL, TEXT("%s:%d\n%s"), pData->lpHost, pData->usPort, pData->lpName);
	if (SUCCEEDED(hr)) {
		SetDlgItemText(pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT, lpText);
		HeapSafeFree(pData->hHeap, 0, lpText);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		g_pPropertyPageData = pData;
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->Init(NULL, &PropertyPageAuthCallback) == SANE_STATUS_GOOD) {
			SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_FIND);
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(pData->lpName);
				if (oDevice) {
					SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_OPEN);
					if (UpdateDeviceInfo(pData, oDevice) == ERROR_SUCCESS &&
						UpdateDeviceData(pData, oDevice) == ERROR_SUCCESS) {

						if (oSession->Exit() == SANE_STATUS_GOOD) {
							res = UpdateInstallDeviceFlagsEx(pData->hDeviceInfoSet, pData->pDeviceInfoData, 0, DI_FLAGSEX_PROPCHANGE_PENDING);
							if (res == ERROR_SUCCESS) {
								g_pPropertyPageData = NULL;
								delete oSession;

								if (pData->hThread != hThread)
									return 0;

								HidePropertyPageAdvancedProgress(pData->hwndDlg);

								pData->bPropChanged = FALSE;
								PropSheet_UnChanged(pData->hwndPropDlg, pData->hwndDlg);
								PropSheet_CancelToClose(pData->hwndPropDlg);

								pData->hThread = NULL;
								PropSheet_PressButton(pData->hwndPropDlg, PSBTN_OK);

								return 0;
							}
						} else if (pData->hThread == hThread) {
							MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
						}
					} else if (pData->hThread == hThread) {
						MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
					}
				} else if (pData->hThread == hThread) {
					MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
				}
			} else if (pData->hThread == hThread) {
				MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_FIND_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
			}
			oSession->Exit();
		} else if (pData->hThread == hThread) {
			MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		}
		g_pPropertyPageData = NULL;
		delete oSession;
	} else if (pData->hThread == hThread) {
		MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
	}

	if (pData->hThread != hThread)
		return 0;

	HidePropertyPageAdvancedProgress(pData->hwndDlg);

	pData->hThread = NULL;

	return 0;
}


static VOID WINAPI SwitchPropertyPageAdvancedProgress(_In_ HWND hwndDlg, _In_ BOOL bVisible)
{
	int progress[] = {IDC_PROPERTIES_PROGRESS_ANIMATE, IDC_PROPERTIES_PROGRESS_TEXT_MAIN, IDC_PROPERTIES_PROGRESS_TEXT};
	int properties[] = {IDC_PROPERTIES_COMBO_SCANNER, IDC_PROPERTIES_TEXT_SCANNER, IDC_PROPERTIES_GROUP_CREDENTIALS,
						IDC_PROPERTIES_TEXT_USERNAME, IDC_PROPERTIES_EDIT_USERNAME, IDC_PROPERTIES_TEXT_PASSWORD,
						IDC_PROPERTIES_EDIT_PASSWORD, IDC_PROPERTIES_BUTTON_CHECK, IDC_PROPERTIES_BUTTON_RESET};
	int index, show;
	HWND hwnd;

	show = bVisible ? SW_SHOW : SW_HIDE;
	for (index = 0; index < sizeof(progress)/sizeof(progress[0]); index++) {
		hwnd = GetDlgItem(hwndDlg, progress[index]);
		if (hwnd) {
			ShowWindowAsync(hwnd, show);
		}
	}

	show = bVisible ? SW_HIDE : SW_SHOW;
	for (index = 0; index < sizeof(properties)/sizeof(properties[0]); index++) {
		hwnd = GetDlgItem(hwndDlg, properties[index]);
		if (hwnd) {
			ShowWindowAsync(hwnd, show);
		}
	}
}

VOID WINAPI ShowPropertyPageAdvancedProgress(_In_ HWND hwndDlg)
{
	HWND hwndParent, hwnd;
	HMENU hMenu;

	SwitchPropertyPageAdvancedProgress(hwndDlg, TRUE);

	hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Play(hwnd, 0, -1, -1);
	}

	hwndParent = GetParent(hwndDlg);
	if (hwndParent) {
		hwnd = GetDlgItem(hwndParent, IDOK);
		if (hwnd) {
			EnableWindow(hwnd, FALSE);
		}
		hwnd = GetDlgItem(hwndParent, IDCANCEL);
		if (hwnd) {
			EnableWindow(hwnd, FALSE);
		}
		hMenu = GetSystemMenu(hwndParent, FALSE);
		if (hMenu) {
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
	}
}

VOID WINAPI HidePropertyPageAdvancedProgress(_In_ HWND hwndDlg)
{
	HWND hwndParent, hwnd;
	HMENU hMenu;

	hwndParent = GetParent(hwndDlg);
	if (hwndParent) {
		hMenu = GetSystemMenu(hwndParent, FALSE);
		if (hMenu) {
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
		}
		hwnd = GetDlgItem(hwndParent, IDCANCEL);
		if (hwnd) {
			EnableWindow(hwnd, TRUE);
		}
		hwnd = GetDlgItem(hwndParent, IDOK);
		if (hwnd) {
			EnableWindow(hwnd, TRUE);
		}
	}

	hwnd = GetDlgItem(hwndDlg, IDC_PROPERTIES_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Stop(hwnd);
	}

	SwitchPropertyPageAdvancedProgress(hwndDlg, FALSE);
}


WINSANE_API_CALLBACK PropertyPageAuthCallback(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password)
{
	LPTSTR lptUsername, lptPassword;
	LPSTR lpUsername, lpPassword;
	PCOISANE_Data pData;
	BOOL res;

	if (!g_pPropertyPageData || !resource || !strlen(resource) || !username || !password)
		return;

	Trace(TEXT("------ PropertyPageAuthCallback(resource='%hs') ------"), resource);

	pData = g_pPropertyPageData;

	lptUsername = NULL;
	lptPassword = NULL;

	res = GetDlgItemAText(pData->hHeap, pData->hwndDlg, IDC_PROPERTIES_EDIT_USERNAME, &lptUsername, NULL);
	if (res == ERROR_SUCCESS) {
		res = GetDlgItemAText(pData->hHeap, pData->hwndDlg, IDC_PROPERTIES_EDIT_PASSWORD, &lptPassword, NULL);
		if (res == ERROR_SUCCESS) {
			lpUsername = StringConvToA(pData->hHeap, lptUsername);
			if (lpUsername) {
				lpPassword = StringConvToA(pData->hHeap, lptPassword);
				if (lpPassword) {
					strcpy_s(username, SANE_MAX_USERNAME_LEN, lpUsername);
					strcpy_s(password, SANE_MAX_PASSWORD_LEN, lpPassword);
					HeapSafeFree(pData->hHeap, 0, lpPassword);
				}
				HeapSafeFree(pData->hHeap, 0, lpUsername);
			}
			HeapSafeFree(pData->hHeap, 0, lptPassword);
		}
		HeapSafeFree(pData->hHeap, 0, lptUsername);
	}

	Trace(TEXT("Username: %hs (%d)"), username, strlen(username));
	Trace(TEXT("Password: ******** (%d)"), strlen(password));
}
