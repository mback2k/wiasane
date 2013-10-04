#include "stdafx.h"

#include <tchar.h>
#include <strsafe.h>

HRESULT StringCbAPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, ...);
HRESULT StringCbAPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, ...);

HRESULT StringCchAPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, ...);
HRESULT StringCchAPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, ...);

HRESULT StringCbAVPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, va_list argList);
HRESULT StringCbAVPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, va_list argList);

HRESULT StringCchAVPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, va_list argList);
HRESULT StringCchAVPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, va_list argList);

#ifdef UNICODE
#define StringCbAPrintf StringCbAPrintfW
#define StringCchAPrintf StringCchAPrintfW
#define StringCbAVPrintf StringCbAPrintfW
#define StringCchAVPrintf StringCchAPrintfW
#else
#define StringCbAPrintf StringCbAPrintfA
#define StringCchAPrintf StringCchAPrintfA
#define StringCbAVPrintf StringCbAPrintfA
#define StringCchAVPrintf StringCchAPrintfA
#endif
