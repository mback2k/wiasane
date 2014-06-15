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

#include "strutil_mem.h"


BOOL WINAPI HeapSafeFree(_In_ HANDLE hHeap, _In_ DWORD dwFlags, _In_ LPVOID lpMem)
{
	size_t cbMem;

	if (!hHeap || !lpMem)
		return FALSE;

	cbMem = HeapSize(hHeap, dwFlags, lpMem);
	if (cbMem > 0)
		ZeroMemory(lpMem, cbMem);

	return HeapFree(hHeap, dwFlags, lpMem);
}


HLOCAL WINAPI LocalSafeFree(_In_ HLOCAL hMem)
{
	size_t cbMem;

	if (!hMem)
		return hMem;

	cbMem = LocalSize(hMem);
	if (cbMem > 0)
		ZeroMemory(hMem, cbMem);

	return LocalFree(hMem);
}

HGLOBAL WINAPI GlobalSafeFree(_In_ HGLOBAL hMem)
{
	size_t cbMem;

	if (!hMem)
		return hMem;

	cbMem = GlobalSize(hMem);
	if (cbMem > 0)
		ZeroMemory(hMem, cbMem);

	return GlobalFree(hMem);
}
