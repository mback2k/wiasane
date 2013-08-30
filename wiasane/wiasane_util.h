#ifndef WIASANE_UTIL_H
#define WIASANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <wtypes.h>

HRESULT GetOLESTRResourceString(LONG lResourceID, _Outptr_ LPOLESTR *ppsz, BOOL bLocal);
VOID Trace(_In_ LPCTSTR format, ...);

#endif
