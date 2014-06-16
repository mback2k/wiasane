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

#include "coisane_wizard.h"

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>
#include <commctrl.h>

#include "dllmain.h"
#include "resource.h"
#include "strutil.h"
#include "strutil_dbg.h"
#include "strutil_mem.h"
#include "strutil_res.h"
#include "coisane_util.h"


static PCOISANE_Data g_pWizardPageData = NULL; // global instance of the COISANE data with device information

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

			pData->hwndDlg = hwndDlg;

			InitWizardPageServer(hwndDlg, pData);
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_NEXT | PSWIZB_CANCEL);
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					pData->hwndPropDlg = NULL;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (!NextWizardPageServer(hwndDlg, pData)) {
						MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_DISABLE, DICS_FLAG_GLOBAL);
					break;
			}
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

			pData->hwndDlg = hwndDlg;

			InitWizardPageScanner(hwndDlg, pData);
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_CANCEL);
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					pData->hwndPropDlg = NULL;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_WIZARD_PAGE_SERVER);
					return TRUE;
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (!NextWizardPageScanner(hwndDlg, pData)) {
						MessageBoxR(pData->hHeap, pData->hInstance, hwndDlg, IDS_DEVICE_OPEN_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_DISABLE, DICS_FLAG_GLOBAL);
					break;
			}
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProcWizardPageProgress(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LPPROPSHEETPAGE lpPropSheetPage;
	PCOISANE_Data pData;
	HWND hwnd;

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

			pData->hwndDlg = hwndDlg;

			InitWizardPageProgress(hwndDlg, pData);
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
					PropSheet_SetWizButtons(pData->hwndPropDlg, PSWIZB_BACK);

					hwnd = GetDlgItem(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
					if (hwnd) {
						Animate_Play(hwnd, 0, -1, -1);
					}

					if (pData->hThread) {
						SetThreadPriority(pData->hThread, THREAD_PRIORITY_BELOW_NORMAL);
						ResumeThread(pData->hThread);
					}
					break;

				case PSN_KILLACTIVE:
					Trace(TEXT("PSN_KILLACTIVE"));
					hwnd = GetDlgItem(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
					if (hwnd) {
						Animate_Stop(hwnd);
					}

					pData->hThread = NULL;
					pData->hwndPropDlg = NULL;
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;

				case PSN_QUERYCANCEL:
					Trace(TEXT("PSN_QUERYCANCEL"));
					return TRUE;
					break;
			}
			break;
	}

	return FALSE;
}


UINT CALLBACK PropSheetPageProcWizardPage(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
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


BOOL WINAPI InitWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	INFCONTEXT InfContext;
	HINF InfFile;
	LPTSTR strField;
	INT intField;
	DWORD size;
	BOOL res;

	InfFile = OpenInfFile(pData->hDeviceInfoSet, pData->pDeviceInfoData, NULL);
	if (InfFile != INVALID_HANDLE_VALUE) {
		res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Host"), &InfContext);
		if (res) {
			res = SetupGetStringField(&InfContext, 1, NULL, 0, &size);
			if (res) {
				strField = (LPTSTR) HeapAlloc(pData->hHeap, HEAP_ZERO_MEMORY, size * sizeof(TCHAR));
				if (strField) {
					res = SetupGetStringField(&InfContext, 1, strField, size, NULL);
					if (res) {
						res = SetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, strField);

						pData->lpHost = strField;
					} else {
						HeapSafeFree(pData->hHeap, 0, strField);
					}
				} else {
					res = FALSE;
				}
			}
		}

		if (res) {
			res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Port"), &InfContext);
			if (res) {
				res = SetupGetIntField(&InfContext, 1, &intField);
				if (res) {
					res = SetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, intField, FALSE);

					pData->usPort = (USHORT) intField;
				}
			}
		}

		SetupCloseInfFile(InfFile);
	} else {
		res = FALSE;
	}

	if (!pData->lpHost)
		pData->lpHost = StringAClone(pData->hHeap, TEXT("localhost"));

	if (!pData->usPort)
		pData->usPort = WINSANE_DEFAULT_PORT;

	return res;
}

BOOL WINAPI NextWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LPTSTR lpHost;
	USHORT usPort;
	BOOL bPort;
	size_t cbLength;
	DWORD res;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, &lpHost, &cbLength);
	if (res != ERROR_SUCCESS) {
		return FALSE;
	}

	usPort = (USHORT) GetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, &bPort, FALSE);
	if (!bPort) {
		HeapSafeFree(pData->hHeap, 0, lpHost);
		return FALSE;
	}

	if (pData->lpHost) {
		HeapSafeFree(pData->hHeap, 0, pData->lpHost);
	}
	pData->lpHost = lpHost;
	pData->usPort = usPort;

	pData->hThread = CreateThread(NULL, 0, &ProcWizardPageServer, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	return TRUE;
}

DWORD WINAPI ProcWizardPageServer(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PCOISANE_Data pData;
	HANDLE hThread;
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	hr = StringCbAPrintf(pData->hHeap, &lpStr, &cbLen, TEXT("%s:%d"), pData->lpHost, pData->usPort);
	if (SUCCEEDED(hr) && lpStr) {
		SetDlgItemText(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_SUB, lpStr);
		HeapSafeFree(pData->hHeap, 0, lpStr);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			if (oSession->Exit() == SANE_STATUS_GOOD) {
				if (pData->hThread == hThread) {
					PropSheet_PressButton(pData->hwndPropDlg, PSBTN_NEXT);
				}
				return 0;
			} else if (pData->hThread == hThread) {
				MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
			}
		} else if (pData->hThread == hThread) {
			MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
		}
		delete oSession;
	} else if (pData->hThread == hThread) {
		MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
	}

	if (pData->hThread == hThread) {
		PropSheet_PressButton(pData->hwndPropDlg, PSBTN_BACK);
	}
	return 0;
}


BOOL WINAPI InitWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	LONG index;
	HWND hwnd;
	BOOL res;

	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		if (oSession->Init(NULL, NULL) == SANE_STATUS_GOOD) {
			hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER);
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

	return res;
}

BOOL WINAPI NextWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	LPTSTR lpName, lpUsername, lpPassword;
	size_t cbLength;
	DWORD res;

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER, &lpName, &cbLength);
	if (res != ERROR_SUCCESS) {
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_USERNAME, &lpUsername, &cbLength);
	if (res != ERROR_SUCCESS) {
		HeapSafeFree(pData->hHeap, 0, lpName);
		return FALSE;
	}

	res = GetDlgItemAText(pData->hHeap, hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_PASSWORD, &lpPassword, &cbLength);
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

	pData->hThread = CreateThread(NULL, 0, &ProcWizardPageScanner, pData, CREATE_SUSPENDED, NULL);
	if (!pData->hThread)
		return FALSE;

	return TRUE;
}

DWORD WINAPI ProcWizardPageScanner(_In_ LPVOID lpParameter)
{
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PCOISANE_Data pData;
	HANDLE hThread;
	size_t cbLen;
	PTSTR lpStr;
	HRESULT hr;

	pData = (PCOISANE_Data) lpParameter;
	if (!pData)
		return 0;

	hThread = pData->hThread;

	hr = StringCbAPrintf(pData->hHeap, &lpStr, &cbLen, TEXT("%s:%d\n%s"), pData->lpHost, pData->usPort, pData->lpName);
	if (SUCCEEDED(hr) && lpStr) {
		SetDlgItemText(pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_SUB, lpStr);
		HeapSafeFree(pData->hHeap, 0, lpStr);
	}

	SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_CONNECT);
	oSession = WINSANE_Session::Remote(pData->lpHost, pData->usPort);
	if (oSession) {
		g_pWizardPageData = pData;
		SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_SESSION_STEP_INIT);
		if (oSession->Init(NULL, &WizardPageAuthCallback) == SANE_STATUS_GOOD) {
			SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_FIND);
			if (oSession->FetchDevices() == SANE_STATUS_GOOD) {
				SetDlgItemTextR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDC_WIZARD_PAGE_PROGRESS_TEXT_MAIN, IDS_DEVICE_STEP_OPEN);
				oDevice = oSession->GetDevice(pData->lpName);
				if (oDevice) {
					UpdateDeviceInfo(pData, oDevice);
					UpdateDeviceData(pData, oDevice);

					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_ENABLE, DICS_FLAG_GLOBAL);
					ChangeDeviceState(pData->hDeviceInfoSet, pData->pDeviceInfoData, DICS_PROPCHANGE, DICS_FLAG_GLOBAL);

					if (oSession->Exit() == SANE_STATUS_GOOD) {
						g_pWizardPageData = NULL;
						delete oSession;

						if (pData->hThread == hThread) {
							PropSheet_PressButton(pData->hwndPropDlg, PSBTN_NEXT);
						}
						return 0;
					} else if (pData->hThread == hThread) {
						MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_INIT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
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
		g_pWizardPageData = NULL;
		delete oSession;
	} else if (pData->hThread == hThread) {
		MessageBoxR(pData->hHeap, pData->hInstance, pData->hwndDlg, IDS_SESSION_CONNECT_FAILED, IDS_PROPERTIES_SCANNER_DEVICE, MB_ICONERROR | MB_OK);
	}

	if (pData->hThread == hThread) {
		PropSheet_PressButton(pData->hwndPropDlg, PSBTN_BACK);
	}
	return 0;
}


BOOL WINAPI InitWizardPageProgress(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data pData)
{
	HINSTANCE hInst;
	HANDLE hIcon;
	HWND hwnd;
	BOOL res;

	UNREFERENCED_PARAMETER(pData);

	hInst = GetWiaDefInstance();
	if (hInst) {
		hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ICON);
		if (hwnd) {
			hIcon = LoadImage(hInst, MAKEINTRESOURCE(105), IMAGE_ICON, 32, 32, LR_SHARED);
			if (hIcon) {
				res = PostMessage(hwnd, STM_SETICON, (WPARAM) hIcon, (LPARAM) 0);
			} else {
				res = FALSE;
			}
		} else {
			res = FALSE;
		}

		hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_PROGRESS_ANIMATE);
		if (hwnd) {
			res = Animate_OpenEx(hwnd, hInst, MAKEINTRESOURCE(1001));
		} else {
			res = FALSE;
		}
	} else {
		res = FALSE;
	}

	return res;
}


WINSANE_API_CALLBACK WizardPageAuthCallback(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password)
{
	LPSTR lpUsername, lpPassword;
	HANDLE hHeap;

	if (!g_pWizardPageData || !resource || !strlen(resource) || !username || !password)
		return;

	Trace(TEXT("------ WizardPageAuthCallback(resource='%hs') ------"), resource);

	hHeap = GetProcessHeap();
	if (!hHeap)
		return;

	lpUsername = StringToA(hHeap, g_pWizardPageData->lpUsername);
	if (lpUsername) {
		lpPassword = StringToA(hHeap, g_pWizardPageData->lpPassword);
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
