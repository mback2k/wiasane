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

#ifndef STRUTIL_REG_H
#define STRUTIL_REG_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

LONG WINAPI ReadRegistryLong(_In_ HANDLE hHeap, _In_ HKEY hKey, _In_opt_ LPCTSTR lpcszValueName, _Out_ DWORD *pdwValue);
LONG WINAPI ReadRegistryString(_In_ HANDLE hHeap, _In_ HKEY hKey, _In_opt_ LPCTSTR lpcszValueName, _Outptr_result_maybenull_ LPTSTR *plpszValue, _Out_ DWORD *pdwLength);

#endif
