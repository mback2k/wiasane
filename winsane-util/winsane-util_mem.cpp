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

#include "winsane-util_mem.h"

BOOL WINAPI HeapSafeFree(_Inout_ HANDLE hHeap, _In_ DWORD dwFlags, _In_opt_ _Post_ptr_invalid_ LPVOID lpMem)
{
	SIZE_T cbMem;
	BOOL res;

	if (!hHeap || !lpMem)
		return FALSE;

	if (!HeapLock(hHeap))
		return FALSE;

	cbMem = HeapSize(hHeap, dwFlags, lpMem);
	if (cbMem > 0 && cbMem != (SIZE_T)-1)
		SecureZeroMemory(lpMem, cbMem);

	res = HeapFree(hHeap, dwFlags, lpMem);

	HeapUnlock(hHeap);

	return res;
}


HLOCAL WINAPI LocalSafeFree(_Pre_opt_valid_ HLOCAL hMem)
{
	SIZE_T cbMem;

	if (!hMem)
		return hMem;

	if (!LocalLock(hMem))
		return hMem;

	cbMem = LocalSize(hMem);
	if (cbMem > 0)
		SecureZeroMemory(hMem, cbMem);

	return LocalFree(hMem);
}

HGLOBAL WINAPI GlobalSafeFree(_Pre_opt_valid_ HGLOBAL hMem)
{
	SIZE_T cbMem;

	if (!hMem)
		return hMem;

	if (!GlobalLock(hMem))
		return hMem;

	cbMem = GlobalSize(hMem);
	if (cbMem > 0)
		SecureZeroMemory(hMem, cbMem);

	return GlobalFree(hMem);
}
