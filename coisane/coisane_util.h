#ifndef COISANE_UTIL_H
#define COISANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

//+---------------------------------------------------------------------------
//
// WARNING!
//
// A Coinstaller must not generate any popup to the user.
//     it should provide appropriate defaults.
//
//  OutputDebugString should be fine...
//
#if DBG
#define DbgOut(Text) OutputDebugString(TEXT("CoInstaller: ") TEXT(Text) TEXT("\n"))
#else
#define DbgOut(Text)
#endif

HINF OpenInfFile(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVINFO_DATA DeviceInfoData, _Out_opt_ PUINT ErrorLine);

#endif
