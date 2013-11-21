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
