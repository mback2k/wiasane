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
	WINSANE_Scan *scan;
	UINT total;
	UINT received;
	LONG xbytegap;
	LONG ybytegap;
} WIASANE_Task, *PWIASANE_Task;

typedef struct _WIASANE_Context {
	WINSANE_Session *session;
	WINSANE_Device *device;
	WIASANE_Task *task;
	USHORT port;
	PTSTR host;
	PTSTR name;
} WIASANE_Context, *PWIASANE_Context;

HRESULT ReadRegistryInformation(WIASANE_Context *context, HANDLE *pHandle);
HRESULT InitializeScanner(WIASANE_Context *context);
HRESULT UninitializeScanner(WIASANE_Context *context);
HRESULT FreeScanner(WIASANE_Context *context);
HRESULT InitScannerDefaults(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT SetScannerSettings(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT FetchScannerParams(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT SetScanMode(PSCANINFO pScanInfo, LONG lScanMode);

#endif
