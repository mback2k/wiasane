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

// define your custom-defined supported formats here
// {3B5DE639-B2C6-4952-98A9-1DC06F3703BD}
DEFINE_GUID(WiaImgFmt_MYNEWFORMAT, 0x3b5de639, 0xb2c6, 0x4952, 0x98, 0xa9, 0x1d, 0xc0, 0x6f, 0x37, 0x3, 0xbd);

#undef INITGUID



// note: MEMORYBMP, and BMP file will be added by wiafbdrv host driver.
//       do not include them in your extended list.
//

// #define _USE_EXTENDED_FORMAT_LIST (uncomment this to allow Extented file and memory formats)

#define NUM_SUPPORTED_FILEFORMATS 1
GUID g_SupportedFileFormats[NUM_SUPPORTED_FILEFORMATS];

#define NUM_SUPPORTED_MEMORYFORMATS 2
GUID g_SupportedMemoryFormats[NUM_SUPPORTED_MEMORYFORMATS];



INT g_PalIndex = 0;     // simple palette index counter (test driver specific)
BOOL g_bDown = FALSE;   // simple band direction bool   (test drvier specific)

BOOL    InitializeScanner(PSCANINFO pScanInfo);
VOID    InitScannerDefaults(PSCANINFO pScanInfo);
BOOL    SetScannerSettings(PSCANINFO pScanInfo);
HRESULT GetInterruptEvent(PVAL pValue);
VOID    ReadRegistryInformation(PVAL pValue);

#endif
