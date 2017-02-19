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

#include "winsane-util_dbg.h"

#ifdef _DEBUG

#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>

#include "winsane-util_str.h"
#include "winsane-util_mem.h"

VOID Trace_(_In_ LPCTSTR lpszPrefix, _In_ LPCTSTR lpszFormat, ...)
{
	LPTSTR lpszMsg, lpszOut;
	va_list argList;
	HANDLE hHeap;
	HRESULT hr;

	hHeap = GetProcessHeap();
	if (hHeap) {
		va_start(argList, lpszFormat);
		hr = StringCchAVPrintf(hHeap, &lpszMsg, NULL, lpszFormat, argList);
		va_end(argList);
		if (SUCCEEDED(hr)) {
			hr = StringCchAPrintf(hHeap, &lpszOut, NULL, TEXT("%s: %s\r\n"), lpszPrefix, lpszMsg);
			if (SUCCEEDED(hr)) {
				OutputDebugString(lpszOut);
				HeapSafeFree(hHeap, 0, lpszOut);
			}
			HeapSafeFree(hHeap, 0, lpszMsg);
		}
	}
}

#endif
