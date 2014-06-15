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

#include "strutil_res.h"

#include "strutil_mem.h"

int WINAPI LoadAString(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ UINT uID, _Out_ LPTSTR *ppszBuffer)
{
	int length;

	length = LoadString(hInstance, uID, (LPTSTR) ppszBuffer, 0);
	if (length > 0) {
		*ppszBuffer = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (length + 2) * sizeof(TCHAR));
		if (*ppszBuffer) {
			length = LoadString(hInstance, uID, *ppszBuffer, length + 1);
		}
	}

	return length;
}

int WINAPI MessageBoxR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hWnd, _In_ UINT uText, _In_ UINT uCaption, _In_ UINT uType)
{
	LPCTSTR lpText, lpCaption;
	int msgBoxID;

	msgBoxID = 0;

	if (LoadAString(hHeap, hInstance, uText, (LPTSTR*) &lpText) > 0) {
		if (LoadAString(hHeap, hInstance, uCaption, (LPTSTR*) &lpCaption) > 0) {
			msgBoxID = MessageBox(hWnd, lpText, lpCaption, uType);
			HeapSafeFree(hHeap, 0, (LPVOID) lpCaption);
		}
		HeapSafeFree(hHeap, 0, (LPVOID) lpText);
	}

	return msgBoxID;
}
