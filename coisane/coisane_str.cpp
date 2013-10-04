#include "coisane_str.h"

#include <stdio.h>
#include <stdarg.h>

HRESULT StringCbAPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCbAPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCchAPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCchAPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCbAVPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(CHAR);

	return hr;
}

HRESULT StringCbAVPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(WCHAR);

	return hr;
}


HRESULT StringCchAVPrintfA(HANDLE hHeap, LPSTR *ppszDest, size_t *pcbDest, LPCSTR pszFormat, va_list argList)
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

HRESULT StringCchAVPrintfW(HANDLE hHeap, LPWSTR *ppszDest, size_t *pcbDest, LPCWSTR pszFormat, va_list argList)
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
