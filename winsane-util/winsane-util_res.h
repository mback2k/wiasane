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

#ifndef WINSANE_UTIL_RES_H
#define WINSANE_UTIL_RES_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

_Success_(return)
BOOL WINAPI LoadAString(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ UINT uID, _Outptr_result_nullonfailure_ LPTSTR *plpszString, _Out_opt_ int *piLength);

int WINAPI MessageBoxR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hwnd, _In_ UINT uText, _In_ UINT uCaption, _In_ UINT uType);

BOOL WINAPI SetWindowTextR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hWnd, _In_ UINT uText);
BOOL WINAPI SetDlgItemTextR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hDlg, _In_ int nIDDlgItem, _In_ UINT uText);

#endif
