/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this software distribution.
 *
 * You may opt to use, copy, modify, and distribute this software for any
 * purpose with or without fee, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either expressed or implied.
 *
 ***************************************************************************/

#include "strutil.h"

#include <stdio.h>
#include <stdarg.h>


LPTSTR StringAClone(_In_ HANDLE hHeap, _In_ LPTSTR pszString)
{
	LPTSTR pszCopy;
	size_t cbCopy;
	HRESULT hr;

	if (!pszString)
		return NULL;

	hr = StringCbLength(pszString, STRSAFE_MAX_CCH * sizeof(TCHAR), &cbCopy);
	if (FAILED(hr))
		return NULL;

	pszCopy = (PTCHAR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbCopy + sizeof(TCHAR));
	if (!pszCopy)
		return NULL;

	hr = StringCbCopy(pszCopy, cbCopy + sizeof(TCHAR), pszString);
	if (FAILED(hr)) {
		HeapFree(hHeap, 0, pszCopy);
		return NULL;
	}

	return pszCopy;
}


HRESULT StringCbAPrintfA(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCbAPrintfW(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPWSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCbAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCchAPrintfA(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}

HRESULT StringCchAPrintfW(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPWSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCWSTR pszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, pszFormat);
	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);
	va_end(argList);

	return hr;
}


HRESULT StringCbAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfA(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(CHAR);

	return hr;
}

HRESULT StringCbAVPrintfW(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPWSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList)
{
	HRESULT hr;

	hr = StringCchAVPrintfW(hHeap, ppszDest, pcbDest, pszFormat, argList);

	if (SUCCEEDED(hr))
		*pcbDest *= sizeof(WCHAR);

	return hr;
}


HRESULT StringCchAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCSTR pszFormat, _In_ va_list argList)
{
	LPSTR pszDest;
	int cbDest;

	if (!hHeap || !ppszDest || !pcbDest || !pszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	*ppszDest = NULL;
	*pcbDest = 0;

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

HRESULT StringCchAVPrintfW(_In_ HANDLE hHeap, _Outptr_result_maybenull_ LPWSTR *ppszDest, _Out_ size_t *pcbDest, _In_ LPCWSTR pszFormat, _In_ va_list argList)
{
	LPWSTR pszDest;
	int cbDest;

	if (!hHeap || !ppszDest || !pcbDest || !pszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	*ppszDest = NULL;
	*pcbDest = 0;

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
