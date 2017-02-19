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

#include "winsane-util_res.h"

#include "winsane-util_mem.h"

_Success_(return)
BOOL WINAPI LoadAString(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ UINT uID, _Outptr_result_nullonfailure_ LPTSTR *plpszString, _Out_opt_ int *piLength)
{
	LPTSTR lpszString;
	int iLength;

	if (!plpszString)
		return FALSE;

	*plpszString = NULL;
	if (piLength)
		*piLength = 0;

	iLength = LoadString(hInstance, uID, (LPTSTR) &lpszString, 0);
	if (iLength <= 0)
		return FALSE;

	if ((iLength+1) <= iLength)
		return FALSE;

	iLength += 1;
	lpszString = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, iLength * sizeof(TCHAR));
	if (!lpszString)
		return FALSE;

	iLength = LoadString(hInstance, uID, lpszString, iLength);
	if (iLength <= 0) {
		HeapSafeFree(hHeap, 0, lpszString);
		return FALSE;
	}

	*plpszString = lpszString;
	if (piLength)
		*piLength = iLength;

	return TRUE;
}

int WINAPI MessageBoxR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hWnd, _In_ UINT uText, _In_ UINT uCaption, _In_ UINT uType)
{
	LPTSTR lpszText, lpszCaption;
	int msgBoxID;

	msgBoxID = 0;

	if (LoadAString(hHeap, hInstance, uText, &lpszText, NULL)) {
		if (LoadAString(hHeap, hInstance, uCaption, &lpszCaption, NULL)) {
			msgBoxID = MessageBox(hWnd, lpszText, lpszCaption, uType);
			HeapSafeFree(hHeap, 0, (LPVOID) lpszCaption);
		}
		HeapSafeFree(hHeap, 0, (LPVOID) lpszText);
	}

	return msgBoxID;
}


BOOL WINAPI SetWindowTextR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hWnd, _In_ UINT uText)
{
	LPTSTR lpszText;
	BOOL res;

	if (LoadAString(hHeap, hInstance, uText, &lpszText, NULL)) {
		res = SetWindowText(hWnd, lpszText);
		HeapSafeFree(hHeap, 0, (LPVOID) lpszText);
	} else {
		res = SetWindowText(hWnd, TEXT(""));
	}

	return res;
}

BOOL WINAPI SetDlgItemTextR(_In_ HANDLE hHeap, _In_opt_ HINSTANCE hInstance, _In_ HWND hDlg, _In_ int nIDDlgItem, _In_ UINT uText)
{
	LPTSTR lpszText;
	BOOL res;

	if (LoadAString(hHeap, hInstance, uText, &lpszText, NULL)) {
		res = SetDlgItemText(hDlg, nIDDlgItem, lpszText);
		HeapSafeFree(hHeap, 0, (LPVOID) lpszText);
	} else {
		res = SetDlgItemText(hDlg, nIDDlgItem, TEXT(""));
	}

	return res;
}
