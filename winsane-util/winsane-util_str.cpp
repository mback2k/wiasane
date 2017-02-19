/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2014, Marc Hoersken, <info@marc-hoersken.de>
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

#include "winsane-util_str.h"

#include <stdio.h>
#include <stdarg.h>

#include "winsane-util_mem.h"

_Success_(return != NULL)
LPSTR WINAPI StringDupA(_In_ HANDLE hHeap, _In_ LPCSTR lpszString)
{
	LPSTR lpszCopy;
	size_t cbCopy;
	HRESULT hr;

	if (!lpszString)
		return NULL;

	hr = StringCbLengthA(lpszString, (STRSAFE_MAX_CCH-1) * sizeof(CHAR), &cbCopy);
	if (FAILED(hr))
		return NULL;

	if ((cbCopy + sizeof(CHAR)) <= cbCopy)
		return NULL;

	cbCopy += sizeof(CHAR);
	lpszCopy = (LPSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbCopy);
	if (!lpszCopy)
		return NULL;

	hr = StringCbCopyNA(lpszCopy, cbCopy, lpszString, cbCopy);
	if (FAILED(hr)) {
		HeapSafeFree(hHeap, 0, lpszCopy);
		return NULL;
	}

	return lpszCopy;
}

_Success_(return != NULL)
LPWSTR WINAPI StringDupW(_In_ HANDLE hHeap, _In_ LPCWSTR lpszString)
{
	LPWSTR lpszCopy;
	size_t cbCopy;
	HRESULT hr;

	if (!lpszString)
		return NULL;

	hr = StringCbLengthW(lpszString, (STRSAFE_MAX_CCH-1) * sizeof(WCHAR), &cbCopy);
	if (FAILED(hr))
		return NULL;

	if ((cbCopy + sizeof(WCHAR)) <= cbCopy)
		return NULL;

	cbCopy += sizeof(WCHAR);
	lpszCopy = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbCopy);
	if (!lpszCopy)
		return NULL;

	hr = StringCbCopyNW(lpszCopy, cbCopy, lpszString, cbCopy);
	if (FAILED(hr)) {
		HeapSafeFree(hHeap, 0, lpszCopy);
		return NULL;
	}

	return lpszCopy;
}


_Success_(return != NULL)
LPSTR WINAPI StringConvWToA(_In_ HANDLE hHeap, _In_ LPWSTR lpszString)
{
	LPSTR lpszCopy;
	int iLength;

	if (!lpszString)
		return NULL;

	iLength = WideCharToMultiByte(CP_ACP, 0, lpszString, -1, NULL, 0, NULL, NULL);
	if (!iLength)
		return NULL;

	if ((iLength+1) <= iLength)
		return NULL;

	iLength += 1;
	lpszCopy = (LPSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, iLength);
	if (!lpszCopy)
		return NULL;

	iLength = WideCharToMultiByte(CP_ACP, 0, lpszString, -1, lpszCopy, iLength, NULL, NULL);
	if (!iLength) {
		HeapSafeFree(hHeap, 0, lpszCopy);
		return NULL;
	}

	return lpszCopy;
}

_Success_(return != NULL)
LPWSTR WINAPI StringConvAToW(_In_ HANDLE hHeap, _In_ LPSTR lpszString)
{
	LPWSTR lpszCopy;
	int iLength;

	if (!lpszString)
		return NULL;

	iLength = MultiByteToWideChar(CP_ACP, 0, lpszString, -1, NULL, 0);
	if (!iLength)
		return NULL;

	if ((iLength+1) <= iLength)
		return NULL;

	iLength += 1;
	lpszCopy = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, iLength * sizeof(TCHAR));
	if (!lpszCopy)
		return NULL;

	iLength = MultiByteToWideChar(CP_ACP, 0, lpszString, -1, lpszCopy, iLength);
	if (!iLength) {
		HeapSafeFree(hHeap, 0, lpszCopy);
		return NULL;
	}

	return lpszCopy;
}


_Success_(SUCCEEDED(return))
HRESULT StringCbAPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCSTR lpszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, lpszFormat);
	hr = StringCbAVPrintfA(hHeap, plpszDest, pcbDest, lpszFormat, argList);
	va_end(argList);

	return hr;
}

_Success_(SUCCEEDED(return))
HRESULT StringCbAPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCWSTR lpszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, lpszFormat);
	hr = StringCbAVPrintfW(hHeap, plpszDest, pcbDest, lpszFormat, argList);
	va_end(argList);

	return hr;
}


_Success_(SUCCEEDED(return))
HRESULT StringCchAPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCSTR lpszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, lpszFormat);
	hr = StringCchAVPrintfA(hHeap, plpszDest, pcchDest, lpszFormat, argList);
	va_end(argList);

	return hr;
}

_Success_(SUCCEEDED(return))
HRESULT StringCchAPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCWSTR lpszFormat, ...)
{
	va_list argList;
	HRESULT hr;

	va_start(argList, lpszFormat);
	hr = StringCchAVPrintfW(hHeap, plpszDest, pcchDest, lpszFormat, argList);
	va_end(argList);

	return hr;
}


_Success_(SUCCEEDED(return))
HRESULT StringCbAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCSTR lpszFormat, _In_ va_list argList)
{
	size_t cchDest;
	HRESULT hr;

	cchDest = 0;
	hr = StringCchAVPrintfA(hHeap, plpszDest, &cchDest, lpszFormat, argList);

	if (SUCCEEDED(hr) && pcbDest)
		*pcbDest = cchDest * sizeof(CHAR);

	return hr;
}

_Success_(SUCCEEDED(return))
HRESULT StringCbAVPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCWSTR lpszFormat, _In_ va_list argList)
{
	size_t cchDest;
	HRESULT hr;

	cchDest = 0;
	hr = StringCchAVPrintfW(hHeap, plpszDest, &cchDest, lpszFormat, argList);

	if (SUCCEEDED(hr) && pcbDest)
		*pcbDest = cchDest * sizeof(WCHAR);

	return hr;
}


_Success_(SUCCEEDED(return))
HRESULT StringCchAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCSTR lpszFormat, _In_ va_list argList)
{
	LPSTR lpszDest;
	int cchDest;

	if (!hHeap || !plpszDest || !lpszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	*plpszDest = NULL;
	if (pcchDest)
		*pcchDest = 0;

	cchDest = _vscprintf(lpszFormat, argList);
	if (cchDest < 0 || cchDest > STRSAFE_MAX_CCH)
		return STRSAFE_E_INVALID_PARAMETER;

	if ((cchDest+1) <= cchDest)
		return STRSAFE_E_INVALID_PARAMETER;

	cchDest += 1;
	lpszDest = (LPSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cchDest * sizeof(CHAR));
	if (lpszDest) {
		cchDest = vsprintf_s(lpszDest, cchDest, lpszFormat, argList);
		if (cchDest >= 0) {
			*plpszDest = lpszDest;
			if (pcchDest)
				*pcchDest = cchDest;
			return S_OK;
		} else {
			HeapSafeFree(hHeap, 0, lpszDest);
		}
	}

	return STRSAFE_E_INSUFFICIENT_BUFFER;
}

_Success_(SUCCEEDED(return))
HRESULT StringCchAVPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCWSTR lpszFormat, _In_ va_list argList)
{
	LPWSTR lpszDest;
	int cchDest;

	if (!hHeap || !plpszDest || !lpszFormat)
		return STRSAFE_E_INVALID_PARAMETER;

	*plpszDest = NULL;
	if (pcchDest)
		*pcchDest = 0;

	cchDest = _vscwprintf(lpszFormat, argList);
	if (cchDest < 0 || cchDest > STRSAFE_MAX_CCH)
		return STRSAFE_E_INVALID_PARAMETER;

	if ((cchDest+1) <= cchDest)
		return STRSAFE_E_INVALID_PARAMETER;

	cchDest += 1;
	lpszDest = (LPWSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cchDest * sizeof(WCHAR));
	if (lpszDest) {
		cchDest = vswprintf_s(lpszDest, cchDest, lpszFormat, argList);
		if (cchDest >= 0) {
			*plpszDest = lpszDest;
			if (pcchDest)
				*pcchDest = cchDest;
			return S_OK;
		} else {
			HeapSafeFree(hHeap, 0, lpszDest);
		}
	}

	return STRSAFE_E_INSUFFICIENT_BUFFER;
}
