#include "coisane_str.h"

#include <stdio.h>
#include <stdarg.h>

HRESULT StringCbAPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCbAPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCchAPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCchAPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCbAVPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(CHAR);

	return hr;
}

HRESULT StringCbAVPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(WCHAR);

	return hr;
}


HRESULT StringCchAVPrintfA(_In_ HANDLE hHeap, _Inout_ LPSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList)
{
	LPSTR pszDest;
	int cbDest;

	if (!hHeap || !ppszDest || !pcbDest || !pszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	cbDest = _vscprintf(pszFormat, argList);
	if (cbDest < 0 || cbDest > STRSAFE_MAX_CCH)
		return STRSAFE_E_INVALID_PARAMETER;

	cbDest += 1;
	pszDest = (PCHAR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDest * sizeof(CHAR));
	if (pszDest) {
		cbDest = vsprintf_s(pszDest, cbDest, pszFormat, argList);
		if (cbDest >= 0) {
			*pcbDest = cbDest + 1;
			*ppszDest = pszDest;
			return S_OK;
		} else {
			HeapFree(hHeap, 0, pszDest);
		}
	}

	return STRSAFE_E_INSUFFICIENT_BUFFER;
}

HRESULT StringCchAVPrintfW(_In_ HANDLE hHeap, _Inout_ LPWSTR *ppszDest, _Inout_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList)
{
	LPWSTR pszDest;
	int cbDest;

	if (!hHeap || !ppszDest || !pcbDest || !pszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	cbDest = _vscwprintf(pszFormat, argList);
	if (cbDest < 0 || cbDest > STRSAFE_MAX_CCH)
		return STRSAFE_E_INVALID_PARAMETER;

	cbDest += 1;
	pszDest = (PWCHAR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDest * sizeof(WCHAR));
	if (pszDest) {
		cbDest = vswprintf_s(pszDest, cbDest, pszFormat, argList);
		if (cbDest >= 0) {
			*pcbDest = cbDest + 1;
			*ppszDest = pszDest;
			return S_OK;
		} else {
			HeapFree(hHeap, 0, pszDest);
		}
	}

	return STRSAFE_E_INSUFFICIENT_BUFFER;
}
