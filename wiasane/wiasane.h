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

#ifndef WIASANE_H
#define WIASANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_driver_)

#include <windows.h>
#include <wiamicro.h>

#include "winsane.h"

typedef struct _WIASANE_Values {
	SANE_String_Const pszcModeThreshold;
	SANE_String_Const pszcModeGrayscale;
	SANE_String_Const pszcModeColor;
	SANE_String_Const pszcSourceFlatbed;
	SANE_String_Const pszcSourceADF;
	SANE_String_Const pszcSourceDuplex;
} WIASANE_Values, *PWIASANE_Values;

typedef struct _WIASANE_Task {
	PWINSANE_Scan oScan;
	UINT uiTotal;
	UINT uiReceived;
	LONG lByteGapX;
	LONG lByteGapY;
} WIASANE_Task, *PWIASANE_Task;

typedef struct _WIASANE_Context {
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PWIASANE_Values pValues;
	PWIASANE_Task pTask;
	USHORT usPort;
	PTSTR pszHost;
	PTSTR pszName;
} WIASANE_Context, *PWIASANE_Context;

HRESULT ReadRegistryInformation(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT InitializeScanner(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT UninitializeScanner(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeScanner(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT InitScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT SetScannerSettings(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FetchScannerParams(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT SetScanMode(_Inout_ PSCANINFO pScanInfo, _In_ LONG lScanMode);

#endif
