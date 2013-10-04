#ifndef COISANE_STR_H
#define COISANE_STR_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

HRESULT StringCbAPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...);
HRESULT StringCbAPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...);

HRESULT StringCchAPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...);
HRESULT StringCchAPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...);

HRESULT StringCbAVPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList);
HRESULT StringCbAVPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList);

HRESULT StringCchAVPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList);
HRESULT StringCchAVPrintfW( _In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList);

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

#endif
