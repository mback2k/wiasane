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

#ifndef STRUTIL_H
#define STRUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

_Success_(return != NULL)
LPTSTR WINAPI StringDup(_In_ HANDLE hHeap, _In_ LPTSTR lpszString);

_Success_(return != NULL)
LPSTR WINAPI StringConvWToA(_In_ HANDLE hHeap, _In_ LPWSTR lpszString);
_Success_(return != NULL)
LPWSTR WINAPI StringConvAToW(_In_ HANDLE hHeap, _In_ LPSTR lpszString);

_Success_(SUCCEEDED(return))
HRESULT StringCbAPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCSTR lpszFormat, ...);
_Success_(SUCCEEDED(return))
HRESULT StringCbAPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCWSTR lpszFormat, ...);

_Success_(SUCCEEDED(return))
HRESULT StringCchAPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCSTR lpszFormat, ...);
_Success_(SUCCEEDED(return))
HRESULT StringCchAPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCWSTR lpszFormat, ...);

_Success_(SUCCEEDED(return))
HRESULT StringCbAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCSTR lpszFormat, _In_ va_list argList);
_Success_(SUCCEEDED(return))
HRESULT StringCbAVPrintfW(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcbDest, _In_ LPCWSTR lpszFormat, _In_ va_list argList);

_Success_(SUCCEEDED(return))
HRESULT StringCchAVPrintfA(_In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCSTR lpszFormat, _In_ va_list argList);
_Success_(SUCCEEDED(return))
HRESULT StringCchAVPrintfW( _In_ HANDLE hHeap, _Outptr_result_nullonfailure_ LPWSTR *plpszDest, _Out_opt_ size_t *pcchDest, _In_ LPCWSTR lpszFormat, _In_ va_list argList);

#ifdef UNICODE
#define StringConvToA StringConvWToA
#define StringConvToW StringDup
#define StringConvATo StringConvAToW
#define StringConvWTo StringDup
#define StringCbAPrintf StringCbAPrintfW
#define StringCchAPrintf StringCchAPrintfW
#define StringCbAVPrintf StringCbAVPrintfW
#define StringCchAVPrintf StringCchAVPrintfW
#else
#define StringConvToA StringDup
#define StringConvToW StringConvAToW
#define StringConvATo StringDup
#define StringConvWTo StringConvWToA
#define StringCbAPrintf StringCbAPrintfA
#define StringCchAPrintf StringCchAPrintfA
#define StringCbAVPrintf StringCbAVPrintfA
#define StringCchAVPrintf StringCchAVPrintfA
#endif

#endif
