#ifndef WIASANE_BTNS_H
#define WIASANE_BTNS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#define INITGUID
#include <initguid.h>

//
// Button GUIDS
//
DEFINE_GUID(guidScanButton, 0xa6c5a715, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);

#undef INITGUID


// #define BUTTON_SUPPORT // (uncomment this to allow BUTTON SUPPORT)
                          // button support is not functional in the test device

#define MAX_BUTTONS 1
#define MAX_BUTTON_NAME 255


extern HRESULT CreateButtonNames(HRESULT hr);
extern HRESULT FreeButtonNames(HRESULT hr);
extern VOID CheckButtonStatus(PVAL pValue);
extern VOID GetButtonPress(LONG *pButtonValue);
extern LONG GetButtonCount();

#endif
