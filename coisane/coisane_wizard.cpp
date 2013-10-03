#include "stdafx.h"
#include "coisane_wizard.h"

#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

#include "winsane.h"

#include "resource.h"
#include "coisane_util.h"

DWORD NewDeviceWizardFinishInstall(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData)
{
	SP_NEWDEVICEWIZARD_DATA newDeviceWizardData;
	HPROPSHEETPAGE hPropSheetPage;
	PROPSHEETPAGE propSheetPage;
	PCOISANE_Data privateData;
	HANDLE hHeap;
	BOOL res;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return ERROR_OUTOFMEMORY;

	ZeroMemory(&newDeviceWizardData, sizeof(newDeviceWizardData));
	newDeviceWizardData.ClassInstallHeader.cbSize = sizeof(newDeviceWizardData.ClassInstallHeader);
	newDeviceWizardData.ClassInstallHeader.InstallFunction = InstallFunction;
	res = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData), NULL);
	if (!res)
		return GetLastError();

	if (newDeviceWizardData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES - 1)
		return NO_ERROR;

	privateData = (PCOISANE_Data) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(COISANE_Data));
	if (!privateData)
		return ERROR_OUTOFMEMORY;

	privateData->hHeap = hHeap;
	privateData->hDeviceInfoSet = DeviceInfoSet;
	privateData->pDeviceInfoData = DeviceInfoData;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageServer;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPage;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SERVER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SERVER_HEADER_SUBTITLE);
	propSheetPage.lParam = (LPARAM) privateData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();
	
	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	ZeroMemory(&propSheetPage, sizeof(propSheetPage));
	propSheetPage.dwSize = sizeof(propSheetPage);
	propSheetPage.dwFlags = PSP_USECALLBACK | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	propSheetPage.hInstance = g_hInst;
	propSheetPage.pfnDlgProc = &DialogProcWizardPageScanner;
	propSheetPage.pfnCallback = &PropSheetPageProcWizardPage;
	propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_WIZARD_PAGE_SCANNER);
	propSheetPage.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_TITLE);
	propSheetPage.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SCANNER_HEADER_SUBTITLE);
	propSheetPage.lParam = (LPARAM) privateData;

	hPropSheetPage = CreatePropertySheetPage(&propSheetPage);
	if (!hPropSheetPage)
		return GetLastError();

	newDeviceWizardData.DynamicPages[newDeviceWizardData.NumDynamicPages++] = hPropSheetPage;

	res = SetupDiSetClassInstallParams(DeviceInfoSet, DeviceInfoData, &newDeviceWizardData.ClassInstallHeader, sizeof(newDeviceWizardData));
	if (!res)
		return GetLastError();

	return NO_ERROR;
}

INT_PTR CALLBACK DialogProcWizardPageServer(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LPPROPSHEETPAGE lpPropSheetPage;
	PCOISANE_Data privateData;

	Trace(TEXT("DialogProcWizardPageServer(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			lpPropSheetPage = (LPPROPSHEETPAGE) lParam;
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			InitWizardPageServer(hwndDlg, privateData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			switch (((LPNMHDR) lParam)->code) {
				case PSN_SETACTIVE:
					Trace(TEXT("PSN_SETACTIVE"));
					PropSheet_SetWizButtons(((LPNMHDR) lParam)->hwndFrom, PSWIZB_NEXT);
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (!NextWizardPageServer(hwndDlg, privateData)) {
						MessageBox(hwndDlg, TEXT("Unable to connect to the specified server!"), TEXT("Error"), MB_OK | MB_ICONERROR);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;
			}
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProcWizardPageScanner(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LPPROPSHEETPAGE lpPropSheetPage;
	PCOISANE_Data privateData;

	Trace(TEXT("DialogProcWizardPageScanner(%d, %d, %d, %d)"), hwndDlg, uMsg, wParam, lParam);

	switch (uMsg) {
		case WM_INITDIALOG:
			Trace(TEXT("WM_INITDIALOG"));
			lpPropSheetPage = (LPPROPSHEETPAGE) lParam;
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			InitWizardPageScanner(hwndDlg, privateData);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			break;

		case WM_NOTIFY:
			Trace(TEXT("WM_NOTIFY"));
			lpPropSheetPage = (LPPROPSHEETPAGE) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			privateData = (PCOISANE_Data) lpPropSheetPage->lParam;

			switch (((LPNMHDR) lParam)->code) {
				case PSN_SETACTIVE:
					Trace(TEXT("PSN_SETACTIVE"));
					PropSheet_SetWizButtons(((LPNMHDR) lParam)->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
					break;

				case PSN_WIZBACK:
					Trace(TEXT("PSN_WIZBACK"));
					break;

				case PSN_WIZNEXT:
					Trace(TEXT("PSN_WIZNEXT"));
					if (!NextWizardPageScanner(hwndDlg, privateData)) {
						MessageBox(hwndDlg, TEXT("Unable to select the specified scanner!"), TEXT("Error"), MB_OK | MB_ICONERROR);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
					Trace(TEXT("PSN_WIZFINISH"));
					break;
			}
			break;
	}

	return FALSE;
}

UINT CALLBACK PropSheetPageProcWizardPage(_In_ HWND hwnd, _In_ UINT uMsg, _Inout_ LPPROPSHEETPAGE ppsp)
{
	PCOISANE_Data privateData;
	UINT ret;

	Trace(TEXT("PropSheetPageProcWizardPageServer(%d, %d, %d)"), hwnd, uMsg, ppsp->lParam);
	
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

BOOL InitWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	INFCONTEXT InfContext;
	HINF InfFile;
	LPTSTR strField;
	INT intField;
	DWORD size;
	BOOL res;

	InfFile = OpenInfFile(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, NULL);
	if (InfFile != INVALID_HANDLE_VALUE) {
		res = SetupFindFirstLine(InfFile, TEXT("WIASANE.DeviceData"), TEXT("Host"), &InfContext);
		if (res) {
			res = SetupGetStringField(&InfContext, 1, NULL, 0, &size);
			if (res) {
				strField = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, size * sizeof(TCHAR));
				if (strField) {
					res = SetupGetStringField(&InfContext, 1, strField, size, NULL);
					if (res) {
						res = SetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, strField);

						privateData->lpHost = _tcsdup(strField);
					}
					HeapFree(privateData->hHeap, 0, strField);
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

					privateData->usPort = (USHORT) intField;
				}
			}
		}

		SetupCloseInfFile(InfFile);
	} else {
		res = FALSE;
	}

	if (!privateData->lpHost)
		privateData->lpHost = _tcsdup(TEXT("localhost"));

	if (!privateData->usPort)
		privateData->usPort = WINSANE_DEFAULT_PORT;

	return res;
}

BOOL NextWizardPageServer(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	WINSANE_Session *session;
	LPTSTR lpHost;
	USHORT usPort;
	BOOL res;

	lpHost = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
	if (lpHost) {
		res = GetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_HOST, lpHost, MAX_PATH);
		if (res) {
			if (privateData->lpHost) {
				free(privateData->lpHost);
			}
			privateData->lpHost = _tcsdup(lpHost);
		}
		HeapFree(privateData->hHeap, 0, lpHost);
	} else {
		res = FALSE;
	}

	if (res) {
		usPort = (USHORT) GetDlgItemInt(hwndDlg, IDC_WIZARD_PAGE_SERVER_EDIT_PORT, &res, FALSE);
		if (res) {
			privateData->usPort = usPort;

			session = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
			if (session) {
				if (session->Init(NULL, NULL)) {
					res = session->Exit();
				} else {
					res = FALSE;
				}
				delete session;
			} else {
				res = FALSE;
			}
		}
	}

	return res;
}

BOOL InitWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	WINSANE_Session *session;
	WINSANE_Device *device;
	int devices, i;
	HWND hwnd;
	BOOL res;

	session = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
	if (session) {
		if (session->Init(NULL, NULL)) {
			hwnd = GetDlgItem(hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER);
			devices = session->GetDevices();
			for (i = 0; i < devices; i++) {
				device = session->GetDevice(i);
				if (device) {
					SendMessageA(hwnd, CB_ADDSTRING, 0, (LPARAM) device->GetName());
				}
			}
			res = session->Exit();
		} else {
			res = FALSE;
		}
		delete session;
	} else {
		res = FALSE;
	}

	return res;
}

BOOL NextWizardPageScanner(_In_ HWND hwndDlg, _Inout_ PCOISANE_Data privateData)
{
	CHAR StringBuf[MAX_PATH];
	WINSANE_Session *session;
	WINSANE_Device *device;
	int devices;
	LPTSTR lpName;
	LPTSTR lpUsername;
	LPTSTR lpPassword;
	HRESULT hr;
	size_t len;
	BOOL res;

	lpName = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
	if (lpName) {
		res = GetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SCANNER_COMBO_SCANNER, lpName, MAX_PATH);
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
			if (GetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_USERNAME, lpUsername, MAX_PATH)) {
				if (privateData->lpUsername) {
					free(privateData->lpUsername);
				}
				privateData->lpUsername = _tcsdup(lpUsername);
			}
			HeapFree(privateData->hHeap, 0, lpUsername);
		}

		lpPassword = (LPTSTR) HeapAlloc(privateData->hHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PATH);
		if (lpPassword) {
			if (GetDlgItemText(hwndDlg, IDC_WIZARD_PAGE_SCANNER_EDIT_PASSWORD, lpPassword, MAX_PATH)) {
				if (privateData->lpPassword) {
					free(privateData->lpPassword);
				}
				privateData->lpPassword = _tcsdup(lpPassword);
			}
			HeapFree(privateData->hHeap, 0, lpPassword);
		}

		session = WINSANE_Session::Remote(privateData->lpHost, privateData->usPort);
		if (session) {
			if (session->Init(NULL, NULL)) {
				devices = session->GetDevices();
				if (devices > 0) {
					device = session->GetDevice(privateData->lpName);
					if (device) {
						hr = StringCbPrintfA(StringBuf, sizeof(StringBuf), "%s %s", device->GetVendor(), device->GetModel());
						if (SUCCEEDED(hr)) {							hr = StringCbLengthA(StringBuf, sizeof(StringBuf), &len);							if (SUCCEEDED(hr)) {
								SetupDiSetDeviceRegistryPropertyA(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_FRIENDLYNAME, (PBYTE) StringBuf, (DWORD) len);
							}
						}

						hr = StringCbPrintfA(StringBuf, sizeof(StringBuf), "%s %s %s (%s)", device->GetVendor(), device->GetModel(), device->GetType(), device->GetName());
						if (SUCCEEDED(hr)) {							hr = StringCbLengthA(StringBuf, sizeof(StringBuf), &len);							if (SUCCEEDED(hr)) {
								SetupDiSetDeviceRegistryPropertyA(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_DEVICEDESC, (PBYTE) StringBuf, (DWORD) len);
							}
						}

						hr = StringCbPrintfA(StringBuf, sizeof(StringBuf), "%s", device->GetVendor());
						if (SUCCEEDED(hr)) {							hr = StringCbLengthA(StringBuf, sizeof(StringBuf), &len);							if (SUCCEEDED(hr)) {
								SetupDiSetDeviceRegistryPropertyA(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, SPDRP_MFG, (PBYTE) StringBuf, (DWORD) len);
							}
						}

						ChangeDeviceState(privateData->hDeviceInfoSet, privateData->pDeviceInfoData, DICS_PROPCHANGE, DICS_FLAG_GLOBAL);

						res = TRUE;
					} else {
						res = FALSE;
					}
				} else {
					res = FALSE;
				}
				if (!session->Exit()) {
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
