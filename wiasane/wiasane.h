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

#define WIASANE_OPTION_BR_X			"br-x"
#define WIASANE_OPTION_BR_Y			"br-y"
#define WIASANE_OPTION_BRIGHTNESS	"brightness"
#define WIASANE_OPTION_COMPRESSION	"compression"
#define WIASANE_OPTION_CONTRAST		"contrast"
#define WIASANE_OPTION_MODE			"mode"
#define WIASANE_OPTION_PREVIEW		"preview"
#define WIASANE_OPTION_RESOLUTION	"resolution"
#define WIASANE_OPTION_SOURCE		"source"
#define WIASANE_OPTION_TL_X			"tl-x"
#define WIASANE_OPTION_TL_Y			"tl-y"

#define WIASANE_SOURCE_ADF			"adf"
#define WIASANE_SOURCE_ADF_EX		"automatic document feeder"
#define WIASANE_SOURCE_DUPLEX		"duplex"
#define WIASANE_SOURCE_FLATBED		"flatbed"

#define WIASANE_MODE_LINEART		"lineart"
#define WIASANE_MODE_THRESHOLD		"threshold"
#define WIASANE_MODE_GRAY			"gray"
#define WIASANE_MODE_GRAYSCALE		"grayscale"
#define WIASANE_MODE_COLOR			"color"

typedef struct _WIASANE_Values {
	PSTR pszModeThreshold;
	PSTR pszModeGrayscale;
	PSTR pszModeColor;
	PSTR pszSourceFlatbed;
	PSTR pszSourceADF;
	PSTR pszSourceDuplex;
} WIASANE_Values, *PWIASANE_Values;

typedef struct _WIASANE_Task {
	PWINSANE_Scan oScan;
	UINT uiTotal;
	UINT uiReceived;
	LONG lByteGapX;
	LONG lByteGapY;
	BOOL bUsingADF;
} WIASANE_Task, *PWIASANE_Task;

typedef struct _WIASANE_Context {
	PWINSANE_Session oSession;
	PWINSANE_Device oDevice;
	PWIASANE_Values pValues;
	PWIASANE_Task pTask;
	PTSTR pszHost;
	PTSTR pszName;
	USHORT usPort;
	UINT uiDevRef;
	LONG lScanMode;
	double dblTopLeftX;
	double dblTopLeftY;
	double dblBottomRightX;
	double dblBottomRightY;
} WIASANE_Context, *PWIASANE_Context;

HRESULT ReadRegistryInformation(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeRegistryInformation(_Inout_ PSCANINFO pScanInfo, _In_ PWIASANE_Context pContext);
HRESULT InitScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT OpenScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT CloseScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT InitScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FetchScannerParams(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT SetScannerSettings(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT SetScanWindow(_Inout_ PWIASANE_Context pContext);
HRESULT SetScanMode(_Inout_ PWIASANE_Context pContext);

#endif
