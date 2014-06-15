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

#include "strutil_reg.h"


LONG WINAPI ReadRegistryLong(_In_ HANDLE hHeap, _In_ HKEY hKey, _In_opt_ LPCTSTR lpcszValueName, _Out_ DWORD *pdwValue)
{
	DWORD dwType, dwLength, dwValue;
	LONG lStatus;

	UNREFERENCED_PARAMETER(hHeap);

	if (!pdwValue)
		return ERROR_INVALID_PARAMETER;

	*pdwValue = 0;

	dwType = REG_DWORD;
	dwLength = sizeof(DWORD);

	lStatus = RegQueryValueEx(hKey, lpcszValueName, NULL, &dwType, (LPBYTE) &dwValue, &dwLength);
	if (lStatus != ERROR_SUCCESS)
		return lStatus;

	if (dwType != REG_DWORD)
		return ERROR_INVALID_DATATYPE;

	if (dwLength != sizeof(DWORD))
		return ERROR_INVALID_DATA;

	*pdwValue = dwValue;
	return ERROR_SUCCESS;
}

LONG WINAPI ReadRegistryString(_In_ HANDLE hHeap, _In_ HKEY hKey, _In_opt_ LPCTSTR lpcszValueName, _Outptr_result_maybenull_ LPTSTR *plpszValue, _Out_ DWORD *pdwLength)
{
	DWORD dwType, dwLength;
	LPTSTR lpszValue;
	LONG lStatus;

	if (!plpszValue || !pdwLength)
		return ERROR_INVALID_PARAMETER;

	*plpszValue = NULL;
	*pdwLength = 0;

	dwType = REG_SZ;
	dwLength = 0;

	lStatus = RegQueryValueEx(hKey, lpcszValueName, NULL, &dwType, NULL, &dwLength);
	if (lStatus != ERROR_SUCCESS)
		return lStatus;

	if (dwType != REG_SZ)
		return ERROR_INVALID_DATATYPE;

	if (!dwLength)
		return ERROR_EMPTY;

	lpszValue = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLength);
	if (!lpszValue)
		return GetLastError();

	lStatus = RegQueryValueEx(hKey, lpcszValueName, NULL, &dwType, (LPBYTE) lpszValue, &dwLength);
	if (lStatus != ERROR_SUCCESS) {
		HeapFree(hHeap, 0, lpszValue);
		return lStatus;
	}

	if (dwType != REG_SZ) {
		HeapFree(hHeap, 0, lpszValue);
		return ERROR_INVALID_DATATYPE;
	}

	if (!dwLength) {
		HeapFree(hHeap, 0, lpszValue);
		return ERROR_EMPTY;
	}

	*plpszValue = lpszValue;
	*pdwLength = dwLength;
	return ERROR_SUCCESS;
}
