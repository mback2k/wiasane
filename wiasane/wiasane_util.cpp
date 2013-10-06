#include "wiasane_util.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

#include <tchar.h>
#include <strsafe.h>
#include <objbase.h>

#include "dllmain.h"


VOID Trace(_In_ LPCTSTR format, ...)
{
#ifdef _DEBUG
	TCHAR input[1024], output[2048];
	va_list arglist;
	HRESULT hr;

	va_start(arglist, format);
	hr = StringCchVPrintf(input, sizeof(input) / sizeof(TCHAR), format, arglist);
	va_end(arglist);

	if (SUCCEEDED(hr)) {
		hr = StringCchPrintf(output, sizeof(output) / sizeof(TCHAR), TEXT("wiasane: %s\r\n"), input);

		if (SUCCEEDED(hr)) {
			OutputDebugString(output);
		}
	}
#else
	UNREFERENCED_PARAMETER(format);
#endif
}
