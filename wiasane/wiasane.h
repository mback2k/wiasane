#ifndef WIASANE_H
#define WIASANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

typedef struct WIASANE_Context {
	WINSANE_Session *session;
	WINSANE_Device *device;
	WINSANE_Scan *scan;
	DWORD port;
	PCHAR host;
	PCHAR name;
	LONG total;
	LONG received;
} WIASANE_Context;

HRESULT ReadRegistryInformation(WIASANE_Context *context, HANDLE *pHandle);
HRESULT InitializeScanner(WIASANE_Context *context);
HRESULT UninitializeScanner(WIASANE_Context *context);
HRESULT FreeScanner(WIASANE_Context *context);
HRESULT InitScannerDefaults(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT SetScannerSettings(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT FetchScannerParams(PSCANINFO pScanInfo, WIASANE_Context *context);
HRESULT SetScanMode(PSCANINFO pScanInfo, LONG lScanMode);

#endif
