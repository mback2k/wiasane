/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2016, Marc Hoersken, <info@marc-hoersken.de>
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

#include "coisane_wizard.h"

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>
#include <regstr.h>
#include <commctrl.h>

#include "dllmain.h"
#include "resource.h"
#include "strutil.h"
#include "strutil_dbg.h"
#include "strutil_mem.h"
#include "strutil_res.h"
#include "coisane_util.h"


DWORD WINAPI NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData)
{
	SP_NEWDEVICEWIZARD_DATA newDeviceWizardData;
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

	ZeroMemory(&newDeviceWizardData, sizeof(newDeviceWizardData));
	newDeviceWizardData.ClassInstallHeader.cbSize = sizeof(newDeviceWizardData.ClassInstallHeader);
	newDeviceWizardData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(hDeviceInfoSet, pDeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData), NULL);
	if (!res)
		return GetLastError();

	if (!(newDeviceWizardData.NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES - 3))
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
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hActCtx = hActCtx;
	propSheetPage.hInstance = hInstance;
	propSheetPage.lParam = (LPARAM) pData;

	propSheetPage.pfnDlgProc = &DialogProcWizardPageServer;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPage;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SERVER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_SUBTITLE);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	propSheetPage.pfnDlgProc = &DialogProcWizardPageProgress;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_PROGRESS);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();

	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	propSheetPage.pfnDlgProc = &DialogProcWizardPageScanner;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPage;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SCANNER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_SUBTITLE);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();

	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	propSheetPage.pfnDlgProc = &DialogProcWizardPageProgress;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_PROGRESS);

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();

	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	res = SetupDiSetClassInstallParams(hDeviceInfoSet, pDeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}


INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
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
			InitWizardPageServer(hwndDlg, pData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			return TRUE;
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_NEXT | PSWIZB_CANCEL);
					if (ShowWizardPageServer(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
					pData->hwndPropDlg = NULL;
					pData->hwndDlg = NULL;
					return TRUE;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (NextWizardPageServer(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_DISABLE, DICS_FLAG_GLOBAL);
					UpdateDeviceConfigFlags(pData->hDeviceInfoSet, pData->pDeviceInfoData, CONFIGFLAG_FINISHINSTALL_UI);
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

INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
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
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			return TRUE;
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_CANCEL);
					if (ShowWizardPageScanner(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					if (HideWizardPageScanner(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
						pData->hwndPropDlg = NULL;
						pData->hwndDlg = NULL;
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					}
					return TRUE;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_WIZARD_PAGE_SERVER);
					return TRUE;
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (NextWizardPageScanner(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_DISABLE, DICS_FLAG_GLOBAL);
					UpdateDeviceConfigFlags(pData->hDeviceInfoSet, pData->pDeviceInfoData, CONFIGFLAG_FINISHINSTALL_UI);
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

INT_PTR CALLBACK DialogProcWizardPageProgress(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
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
			InitWizardPageProgress(hwndDlg, pData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			return TRUE;
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_BACK);
					if (ShowWizardPageProgress(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
					}
					return TRUE;
					break;

				case PSN_KILLACTIVE:
				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_KILLACTIVE | PSN_QUERYCANCEL"));
					if (HideWizardPageProgress(hwndDlg, pData)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
						pData->hwndPropDlg = NULL;
						pData->hwndDlg = NULL;
					} else {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					}
					return TRUE;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					pData->hThread = NULL;
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					pData->hThread = NULL;
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
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

			FreeWizardPageProgress(hwndDlg, pData);
			pData->uiReferences--;
			break;
	}

	return FALSE;
}


UINT CALLBACK PropSheetPageProcWizardPage(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
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


VOID WINAPI InitWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	INFCONTEXT InfContext;
	HINF InfFile;
	LPTSTR lpHost;
	INT iPort;
	DWORD size;
	BOOL res;

	lpHost = NULL;
	iPort = WINSANE_DEFAULT_PORT;

	InfFile = OpenInfFile(pData->hDeviceInfoSet, pData->pDeviceInfoData, NULL);
	if (InfFile != INVALID_HANDLE_VALUE) {
		res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Host"), &InfContext);
		if (res) {
			res = SetupGetStringField(&InfContext, 1, NULL, 0, &size);
			if (res && size > 0 && (size+1) > size) {
				lpHost = (LPTSTR) HeapAlloc(pData->hHeap, HEAP_ZERO_MEMORY, (size+1) * sizeof(TCHAR));
				if (lpHost) {
					res = SetupGetStringField(&InfContext, 1, lpHost, size, NULL);
					if (!res) {
						HeapSafeFree(pData->hHeap, 0, lpHost);
						lpHost = NULL;
					}
				}
			}
		}

		res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Port"), &InfContext);
		if (res) {
			res = SetupGetIntField(&InfContext, 1, &iPort);
			if (!res) {
				iPort = WINSANE_DEFAULT_PORT;
			}
		}

		SetupCloseInfFile(InfFile);
	}

	if (pData->lpHost)
		HeapSafeFree(pData->hHeap, 0, pData->lpHost);

	if (lpHost)
		pData->lpHost = lpHost;
	else
		pData->lpHost = StringDup(pData->hHeap, TEXT("localhost"));

	pData->usPort = (USHORT) iPort;

	SetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, pData->lpHost);
	SetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, pData->usPort, FALSE);
}

BOOL WINAPI ShowWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HWND hwndParent, hwnd;
	HMENU hMenu;

	UNREFERENCED_PARAMETER(pData);

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
	}

	return TRUE;
}

BOOL WINAPI NextWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LPTSTR lpHost;
	USHORT usPort;
	BOOL bPort;
	DWORD res;

	lpHost = NULL;
	usPort = WINSANE_DEFAULT_PORT;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, &lpHost, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	usPort = (USHORT) GetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, &bPort, FALSE);
	if (!bPort) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		HeapSafeFree(pData->hHeap, 0, lpHost);
		return FALSE;
	}

	if (pData->lpHost) {
		HeapSafeFree(pData->hHeap, 0, pData->lpHost);
	}
	pData->lpHost = lpHost;
	pData->usPort = usPort;

	pData->hThread = CreateThread(NULL, 0, &ThreadProcNextWizardPageServer, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	return TRUE;
}

DWORD WINAPI ThreadProcNextWizardPageServer(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PCOISANE_Data pData;
	LONG devices, device;
	HANDLE hThread;
	LPTSTR lpText;
	HRESULT hr;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	hr = StringCbAPrintf(pData->hHeap, &lpText, NULL, TEXT("%s:%d"), pData->lpHost, pData->usPort);
	if (SUCCEEDED(hr)) {
		SetDlgItemText(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_SUB, lpText);
		HeapSafeFree(pData->hHeap, 0, lpText);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				if (pData->lpNames) {
					for (device = 0; pData->lpNames[device]; device++) {
						HeapSafeFree(pData->hHeap, 0, pData->lpNames[device]);
					}
					HeapSafeFree(pData->hHeap, 0, pData->lpNames);
				}
				devices = oSession->GetDevices();
				if (devices > 0) {
					pData->lpNames = (LPTSTR*) HeapAlloc(pData->hHeap, HEAP_ZERO_MEMORY, sizeof(LPTSTR) * (devices+1));
					if (pData->lpNames) {
						pData->lpNames[devices] = NULL;
						for (device = 0; device < devices; device++) {
							oDevice = oSession->GetDevice(device);
							if (oDevice) {
								pData->lpNames[device] = StringConvATo(pData->hHeap, (LPSTR) oDevice->GetName());
							}
						}
					}
				}
				if (oSession->Exit() == SANE_STATUS_GOOD) {
					delete oSession;

					if (pData->hThread == hThread) {
						pData->hThread = NULL;
						PropSheet_PressButton(pData->hwndPropDlg, PSBTN_NEXT);
					}
					return 0;
				} else if (pData->hThread == hThread) {
					MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
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

	if (pData->hThread == hThread) {
		pData->hThread = NULL;
		PropSheet_PressButton(pData->hwndPropDlg, PSBTN_BACK);
	}
	return 0;
}


BOOL WINAPI ShowWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LONG device;
	HWND hwnd;

	if (!pData->lpNames)
		return FALSE;

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER);
	if (hwnd) {
		SendMessage(hwnd, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
		for (device = 0; pData->lpNames[device]; device++) {
			SendMessage(hwnd, CB_ADDSTRING, (WPARAM) 0, (LPARAM) pData->lpNames[device]);
		}
		if (pData->lpName) {
			SendMessage(hwnd, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) pData->lpName);
		}
	}

	return TRUE;
}

BOOL WINAPI HideWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HWND hwnd;

	UNREFERENCED_PARAMETER(pData);

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER);
	if (hwnd) {
		SendMessage(hwnd, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
	}

	return TRUE;
}

BOOL WINAPI NextWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LPTSTR lpName, lpUsername, lpPassword;
	DWORD res;

	lpName = NULL;
	lpUsername = NULL;
	lpPassword = NULL;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER, &lpName, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_USERNAME, &lpUsername, NULL);
	if (res != ERROR_SUCCESS) {
		MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		HeapSafeFree(pData->hHeap, 0, lpName);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_PASSWORD, &lpPassword, NULL);
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

	pData->hThread = CreateThread(NULL, 0, &ThreadProcNextWizardPageScanner, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	return TRUE;
}

DWORD WINAPI ThreadProcNextWizardPageScanner(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PCOISANE_Data pData;
	BOOL delDevice;
	HANDLE hThread;
	LPTSTR lpText;
	HRESULT hr;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	hr = StringCbAPrintf(pData->hHeap, &lpText, NULL, TEXT("%s:%d\n%s"), pData->lpHost, pData->usPort, pData->lpName);
	if (SUCCEEDED(hr)) {
		SetDlgItemText(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_SUB, lpText);
		HeapSafeFree(pData->hHeap, 0, lpText);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->InitEx(NULL, &WizardPageAuthCallbackEx, pData) == SANE_STATUS_GOOD) {
			SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_FIND);
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				oDevice = oSession->GetDevice(pData->lpName);
				delDevice = FALSE;
				if (!oDevice) {
					oDevice = oSession->CreateDevice(pData->lpName);
					delDevice = TRUE;
				}
				if (oDevice) {
					SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_OPEN);
					if (UpdateDeviceInfo(pData, oDevice) == ERROR_SUCCESS &&
						UpdateDeviceData(pData, oDevice) == ERROR_SUCCESS) {

						ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_ENABLE, DICS_FLAG_GLOBAL);
						ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_PROPCHANGE, DICS_FLAG_GLOBAL);

						if (oSession->Exit() == SANE_STATUS_GOOD) {
							delete oSession;
							if (delDevice) {
								delete oDevice;
							}

							if (pData->hThread == hThread) {
								pData->hThread = NULL;
								PropSheet_PressButton(pData->hwndPropDlg, PSBTN_NEXT);
							}
							return 0;
						} else if (pData->hThread == hThread) {
							MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
						}
					} else if (pData->hThread == hThread) {
						MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
					}
					if (delDevice) {
						delete oDevice;
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

	if (pData->hThread == hThread) {
		pData->hThread = NULL;
		PropSheet_PressButton(pData->hwndPropDlg, PSBTN_BACK);
	}
	return 0;
}


VOID WINAPI InitWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HINSTANCE hInst;
	HICON hIcon;
	HWND hwnd;

	UNREFERENCED_PARAMETER(pData);

	hInst = GetWiaDefInstance();
	if (!hInst)
		return;

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ICON);
	if (hwnd) {
		hIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(105), IMAGE_ICON, 32, 32, 0);
		if (hIcon) {
			SendMessage(hwnd, STM_SETICON, (WPARAM) hIcon, (LPARAM) 0);
		}
	}

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_OpenEx(hwnd, hInst, MAKEINTRESOURCE(1001));
	}
}

VOID WINAPI FreeWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HICON hIcon;
	HWND hwnd;

	UNREFERENCED_PARAMETER(pData);

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Close(hwnd);
	}

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ICON);
	if (hwnd) {
		hIcon = (HICON) SendMessage(hwnd, STM_GETICON, (WPARAM) 0, (LPARAM) 0);
		if (hIcon) {
			DestroyIcon(hIcon);
		}
	}
}

BOOL WINAPI ShowWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HWND hwndParent, hwnd;
	HMENU hMenu;
	DWORD res;

	if (!pData->hThread)
		return FALSE;

	SetThreadPriority(pData->hThread, THREAD_PRIORITY_BELOW_NORMAL);
	res = ResumeThread(pData->hThread);
	if (res == (DWORD)-1)
		return FALSE;

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Play(hwnd, 0, -1, -1);
	}

	hwndParent = GetParent(hwndDlg);
	if (hwndParent) {
		hwnd = GetDlgItem(hwndParent, IDCANCEL);
		if (hwnd) {
			EnableWindow(hwnd, FALSE);
		}
		hMenu = GetSystemMenu(hwndParent, FALSE);
		if (hMenu) {
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
	}

	return TRUE;
}

BOOL WINAPI HideWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HWND hwndParent, hwnd;
	HMENU hMenu;

	if (pData->hThread)
		return FALSE;

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
	}

	hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
	if (hwnd) {
		Animate_Stop(hwnd);
	}

	return TRUE;
}


WINSANE_API_CALLBACK WizardPageAuthCallbackEx(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password, _In_ void *userdata)
{
	LPSTR lpUsername, lpPassword;
	PCOISANE_Data pData;

	if (!resource || !strlen(resource) || !username || !password || !userdata)
		return;

	Trace(TEXT("------ WizardPageAuthCallback(resource='%hs') ------"), resource);

	pData = (PCOISANE_Data) userdata;

	lpUsername = StringConvToA(pData->hHeap, pData->lpUsername);
	if (lpUsername) {
		lpPassword = StringConvToA(pData->hHeap, pData->lpPassword);
		if (lpPassword) {
			strcpy_s(username, SANE_MAX_USERNAME_LEN, lpUsername);
			strcpy_s(password, SANE_MAX_PASSWORD_LEN, lpPassword);
			HeapSafeFree(pData->hHeap, 0, lpPassword);
		}
		HeapSafeFree(pData->hHeap, 0, lpUsername);
	}

	Trace(TEXT("Username: %hs (%d)"), username, strlen(username));
	Trace(TEXT("Password: ******** (%d)"), strlen(password));
}
