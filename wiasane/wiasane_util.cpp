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



/**************************************************************************\
* GetOLDSTRResourceString (helper)
*
*   Called by the MicroDriver to Load a resource string in OLESTR format
*
* Arguments:
*
*   lResourceID  - String resource ID
*   ppsz         - Pointer to a OLESTR to be filled with the loaded string
*                  value
*   bLocal       - Possible, other source for loading a resource string.
*
*
* Return Value:
*
*    Status
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

HRESULT GetOLESTRResourceString(LONG lResourceID, _Outptr_ LPOLESTR *ppsz, BOOL bLocal)
{
    TCHAR szStringValue[255];
	HINSTANCE hInstance;
	HRESULT hr;

	hr = S_OK;

    if (bLocal) {

        //
        // We are looking for a resource in our own private resource file
        //

		hInstance = GetModuleInstance();

        INT NumTCHARs = LoadString(hInstance, lResourceID, szStringValue, sizeof(szStringValue)/sizeof(TCHAR));

        if (NumTCHARs <= 0)
        {

#ifdef UNICODE
            DWORD dwError = GetLastError();
            Trace(TEXT("NumTCHARs = %d dwError = %d Resource ID = %d (UNICODE)szString = %ws"),
                  NumTCHARs,
                  dwError,
                  lResourceID,
                  szStringValue);
#else
            DWORD dwError = GetLastError();
            Trace(TEXT("NumTCHARs = %d dwError = %d Resource ID = %d (ANSI)szString = %s"),
                  NumTCHARs,
                  dwError,
                  lResourceID,
                  szStringValue);
#endif

            return E_FAIL;
        }

        //
        // NOTE: caller must free this allocated BSTR
        //

#ifdef UNICODE

       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(szStringValue));
       if(*ppsz != NULL)
       {

           //
           // The call to LoadString previously guarantees that szStringValue is null terminated (maybe truncated)
           // so a buffer of 'sizeof(szStringValue)/sizeof(TCHAR)' should suffice
           //

           hr = StringCchCopy(*ppsz, sizeof(szStringValue)/sizeof(TCHAR), szStringValue);
       }
       else
       {
           hr =  E_OUTOFMEMORY;
       }

#else
       WCHAR wszStringValue[255];
       ZeroMemory(wszStringValue,sizeof(wszStringValue));

       //
       // convert szStringValue from char* to unsigned short* (ANSI only)
       //

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           wszStringValue,
                           (sizeof(wszStringValue)/sizeof(WCHAR)));

       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(wszStringValue));
       if(*ppsz != NULL)
       {

           //
           // The call to LoadString & MultiByteToWideChar previously guarantees that wszStringValue is null terminated
           // (maybe truncated) so a buffer of 'sizeof(wszStringValue)/sizeof(WCHAR)' should suffice
           //

           hr = StringCchCopyW(*ppsz,sizeof(wszStringValue)/sizeof(WCHAR),wszStringValue);
       }
       else
       {
           hr =  E_OUTOFMEMORY;
       }
#endif

    }
    else
    {

        //
        // looking another place for resources??
        //

        hr = E_NOTIMPL;
    }

    return hr;
}
