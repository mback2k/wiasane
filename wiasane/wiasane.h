#ifndef WIASANE_H
#define WIASANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_driver_)

#include <windows.h>
#include <wiamicro.h>

#include "winsane.h"

typedef struct _WIASANE_Task {
	PWINSANE_Scan scan;
	UINT total;
	UINT received;
	LONG xbytegap;
	LONG ybytegap;
} WIASANE_Task, *PWIASANE_Task;

typedef struct _WIASANE_Context {
	PWINSANE_Session session;
	PWINSANE_Device device;
	PWIASANE_Task task;
	USHORT port;
	PTSTR host;
	PTSTR name;
} WIASANE_Context, *PWIASANE_Context;

HRESULT ReadRegistryInformation(PWIASANE_Context context, HANDLE *pHandle);
HRESULT InitializeScanner(PWIASANE_Context context);
HRESULT UninitializeScanner(PWIASANE_Context context);
HRESULT FreeScanner(HANDLE hHeap, PWIASANE_Context context);
HRESULT InitScannerDefaults(PSCANINFO pScanInfo, PWIASANE_Context context);
HRESULT SetScannerSettings(PSCANINFO pScanInfo, PWIASANE_Context context);
HRESULT FetchScannerParams(PSCANINFO pScanInfo, PWIASANE_Context context);
HRESULT SetScanMode(PSCANINFO pScanInfo, LONG lScanMode);

#endif
