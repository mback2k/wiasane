#ifndef WIASANE_H
#define WIASANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#define INITGUID
#include <initguid.h>

// copy any known formats defined in wiadef.h to this location for use in your driver.
DEFINE_GUID(WiaImgFmt_MEMORYBMP, 0xb96b3caa, 0x0728, 0x11d3, 0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e);
DEFINE_GUID(WiaImgFmt_BMP, 0xb96b3cab, 0x0728, 0x11d3, 0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e);
DEFINE_GUID(WiaImgFmt_JPEG, 0xb96b3cae, 0x0728, 0x11d3, 0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e);
DEFINE_GUID(WiaImgFmt_TIFF, 0xb96b3cb1, 0x0728, 0x11d3, 0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e);

#undef INITGUID



// note: MEMORYBMP, and BMP file will be added by wiafbdrv host driver.
//       do not include them in your extended list.
//

// #define _USE_EXTENDED_FORMAT_LIST (uncomment this to allow Extented file and memory formats)

#define NUM_SUPPORTED_FILEFORMATS 1
GUID g_SupportedFileFormats[NUM_SUPPORTED_FILEFORMATS];

#define NUM_SUPPORTED_MEMORYFORMATS 2
GUID g_SupportedMemoryFormats[NUM_SUPPORTED_MEMORYFORMATS];


#include "wiasane_opt.h"

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
