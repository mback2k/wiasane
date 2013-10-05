#ifndef COISANE_H
#define COISANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>
#include <difxapi.h>

typedef struct _COISANE_Data {
	UINT uiReferences;
	HANDLE hHeap;
	HDEVINFO hDeviceInfoSet;
	PSP_DEVINFO_DATA pDeviceInfoData;
	USHORT usPort;
	LPTSTR lpHost;			// use malloc/free
	LPTSTR lpName;			// use malloc/free
	LPTSTR lpUsername;		// use malloc/free
	LPTSTR lpPassword;		// use malloc/free
} COISANE_Data, *PCOISANE_Data;

DWORD CALLBACK CoInstaller(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, OPTIONAL _Inout_ PCOINSTALLER_CONTEXT_DATA Context);

VOID CALLBACK DriverInstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow);
VOID CALLBACK DriverUninstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow);
VOID CALLBACK DeviceInstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow);

DWORD InstallDevice(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData);

HRESULT CreateInstallInfo(_In_ HANDLE hHeap, _In_ LPSTR lpszCmdLine, _Inout_ LPTSTR *lpInfPath, _Out_opt_ PINSTALLERINFO pInstallerInfo);

#endif
