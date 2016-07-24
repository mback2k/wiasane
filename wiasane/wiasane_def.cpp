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

#include "wiasane_def.h"

#include <sti.h>
#include <wia.h>

#include "wiasane_opt.h"
#include "wiasane_scan.h"
#include "strutil_dbg.h"
#include "strutil_mem.h"
#include "strutil_reg.h"
#include "strutil.h"

HRESULT ReadRegistryInformation(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	HKEY hKey, hOpenKey;
	LPTSTR lpszValue;
	DWORD dwValue;
	HANDLE hHeap;
	LSTATUS st;

	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	hHeap = pScanInfo->DeviceIOHandles[1];
	hKey = (HKEY) pScanInfo->DeviceIOHandles[2];
	hOpenKey = NULL;

	//
	// Open DeviceData section to read driver specific information
	//

	st = RegOpenKeyEx(hKey, TEXT("DeviceData"), 0, KEY_QUERY_VALUE|KEY_READ, &hOpenKey);
	if (st != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(st);

	pContext->usPort = WINSANE_DEFAULT_PORT;

	st = ReadRegistryLong(hHeap, hOpenKey, TEXT("Port"), &dwValue);
	if (st != ERROR_SUCCESS) {
		RegCloseKey(hOpenKey);
		return HRESULT_FROM_WIN32(st);
	}

	pContext->usPort = (USHORT) dwValue;

	st = ReadRegistryString(hHeap, hOpenKey, TEXT("Host"), &lpszValue, NULL);
	if (st != ERROR_SUCCESS) {
		RegCloseKey(hOpenKey);
		return HRESULT_FROM_WIN32(st);
	}

	if (pContext->pszHost)
		HeapSafeFree(hHeap, 0, pContext->pszHost);
	pContext->pszHost = lpszValue;

	st = ReadRegistryString(hHeap, hOpenKey, TEXT("Name"), &lpszValue, NULL);
	if (st != ERROR_SUCCESS) {
		RegCloseKey(hOpenKey);
		return HRESULT_FROM_WIN32(st);
	}

	if (pContext->pszName)
		HeapSafeFree(hHeap, 0, pContext->pszName);
	pContext->pszName = lpszValue;

	st = ReadRegistryString(hHeap, hOpenKey, TEXT("Username"), &lpszValue, NULL);
	if (st != ERROR_SUCCESS && st != ERROR_FILE_NOT_FOUND) {
		RegCloseKey(hOpenKey);
		return HRESULT_FROM_WIN32(st);
	}

	if (pContext->pszUsername)
		HeapSafeFree(hHeap, 0, pContext->pszUsername);
	pContext->pszUsername = lpszValue;

	st = ReadRegistryString(hHeap, hOpenKey, TEXT("Password"), &lpszValue, NULL);
	if (st != ERROR_SUCCESS && st != ERROR_FILE_NOT_FOUND) {
		RegCloseKey(hOpenKey);
		return HRESULT_FROM_WIN32(st);
	}

	if (pContext->pszPassword)
		HeapSafeFree(hHeap, 0, pContext->pszPassword);
	pContext->pszPassword = lpszValue;

	RegCloseKey(hOpenKey);
	return S_OK;
}

HRESULT FreeRegistryInformation(_Inout_ PSCANINFO pScanInfo, _In_ PWIASANE_Context pContext)
{
	HANDLE hHeap;

	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	hHeap = pScanInfo->DeviceIOHandles[1];

	if (pContext->pszHost)
		HeapSafeFree(hHeap, 0, pContext->pszHost);

	if (pContext->pszName)
		HeapSafeFree(hHeap, 0, pContext->pszName);

	if (pContext->pszUsername)
		HeapSafeFree(hHeap, 0, pContext->pszUsername);

	if (pContext->pszPassword)
		HeapSafeFree(hHeap, 0, pContext->pszPassword);

	HeapSafeFree(hHeap, 0, pContext);

	return S_OK;
}


HRESULT InitScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Option oOption;
	PSANE_String_Const string_list;
	PSANE_Range range;
	HRESULT hr;
	double dbl;
	int index;

	if (!pScanInfo || !pContext ||
		!pContext->oSession || !pContext->oSession->IsInitialized() ||
		!pContext->oDevice || !pContext->oDevice->IsOpen())
			return E_INVALIDARG;

	if (pContext->pValues)
		FreeScannerDefaults(pScanInfo, pContext);

	pContext->pValues = (PWIASANE_Values) HeapAlloc(pScanInfo->DeviceIOHandles[1], HEAP_ZERO_MEMORY, sizeof(WIASANE_Values));
	if (!pContext->pValues)
		return E_OUTOFMEMORY;

	pScanInfo->ADF                = 0;
	pScanInfo->bNeedDataAlignment = TRUE;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
	if (oOption && oOption->GetType() == SANE_TYPE_STRING && oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
		string_list = oOption->GetConstraintStringList();
		for (index = 0; string_list[index] != NULL; index++) {
			if (!_stricmp(string_list[index], WIASANE_SOURCE_ADF) ||
				!_stricmp(string_list[index], WIASANE_SOURCE_ADF_EX)) {
				pScanInfo->ADF = max(pScanInfo->ADF, 1);
				pContext->pValues->pszSourceADF = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			} else if (!_stricmp(string_list[index], WIASANE_SOURCE_DUPLEX)) {
				pScanInfo->ADF = max(pScanInfo->ADF, 2);
				pContext->pValues->pszSourceDuplex = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			} else if (!_stricmp(string_list[index], WIASANE_SOURCE_FLATBED) ||
				       !_stricmp(string_list[index], WIASANE_SOURCE_GLASS) ||
				       !pContext->pValues->pszSourceFlatbed) {
				pContext->pValues->pszSourceFlatbed = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			}
		}
	}

	pScanInfo->SupportedCompressionType = 0;
	pScanInfo->SupportedDataTypes       = 0;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_MODE);
	if (oOption && oOption->GetType() == SANE_TYPE_STRING && oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
		string_list = oOption->GetConstraintStringList();
		for (index = 0; string_list[index] != NULL; index++) {
			if (!_stricmp(string_list[index], WIASANE_MODE_LINEART) ||
				!_stricmp(string_list[index], WIASANE_MODE_THRESHOLD)) {
				pScanInfo->SupportedDataTypes |= SUPPORT_BW;
				pContext->pValues->pszModeThreshold = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			} else if (!_stricmp(string_list[index], WIASANE_MODE_GRAY) ||
				       !_stricmp(string_list[index], WIASANE_MODE_GRAYSCALE)) {
				pScanInfo->SupportedDataTypes |= SUPPORT_GRAYSCALE;
				pContext->pValues->pszModeGrayscale = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			} else if (!_stricmp(string_list[index], WIASANE_MODE_COLOR)) {
				pScanInfo->SupportedDataTypes |= SUPPORT_COLOR;
				pContext->pValues->pszModeColor = StringDupA(pScanInfo->DeviceIOHandles[1], string_list[index]);
			}
		}
	}

	pScanInfo->BedWidth  = 8500;  // 1000's of an inch (WIA compatible unit)
	pScanInfo->BedHeight = 11000; // 1000's of an inch (WIA compatible unit)

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_X);
	if (oOption) {
		hr = GetOptionMaxValueInch(oOption, &dbl);
		if (SUCCEEDED(hr)) {
			pScanInfo->BedWidth = (LONG) (dbl * 1000.0);
		}
	}
	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_Y);
	if (oOption) {
		hr = GetOptionMaxValueInch(oOption, &dbl);
		if (SUCCEEDED(hr)) {
			pScanInfo->BedHeight = (LONG) (dbl * 1000.0);
		}
	}

	pScanInfo->OpticalXResolution = 300;
	pScanInfo->Xresolution = pScanInfo->OpticalXResolution;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_RESOLUTION);
	if (oOption) {
		hr = GetOptionMaxValue(oOption, &dbl);
		if (SUCCEEDED(hr)) {
			pScanInfo->OpticalXResolution = (LONG) dbl;
		}
		hr = oOption->GetValue(&dbl);
		if (SUCCEEDED(hr) == S_OK) {
			pScanInfo->Xresolution = (LONG) dbl;
		}
	}

	pScanInfo->OpticalYResolution = pScanInfo->OpticalXResolution;
	pScanInfo->Yresolution = pScanInfo->Xresolution;

	pScanInfo->Contrast            = 0;
	pScanInfo->ContrastRange.lMin  = 0;
	pScanInfo->ContrastRange.lMax  = 100;
	pScanInfo->ContrastRange.lStep = 1;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_CONTRAST);
	if (!oOption) {
		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SHARPNESS);
	}
	if (oOption) {
		if (oOption->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
			range = oOption->GetConstraintRange();
			pScanInfo->Contrast            = range->min + ((range->max - range->min) / 2);
			pScanInfo->ContrastRange.lMin  = range->min;
			pScanInfo->ContrastRange.lMax  = range->max;
			pScanInfo->ContrastRange.lStep = range->quant ? range->quant : 1;
		}
		hr = oOption->GetValue(&dbl);
		if (SUCCEEDED(hr)) {
			pScanInfo->Contrast = (LONG) dbl;
		}
	}
		
	pScanInfo->Intensity            = 0;
	pScanInfo->IntensityRange.lMin  = 0;
	pScanInfo->IntensityRange.lMax  = 100;
	pScanInfo->IntensityRange.lStep = 1;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BRIGHTNESS);
	if (oOption) {
		if (oOption->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
			range = oOption->GetConstraintRange();
			pScanInfo->Intensity            = range->min + ((range->max - range->min) / 2);
			pScanInfo->IntensityRange.lMin  = range->min;
			pScanInfo->IntensityRange.lMax  = range->max;
			pScanInfo->IntensityRange.lStep = range->quant ? range->quant : 1;
		}
		hr = oOption->GetValue(&dbl);
		if (SUCCEEDED(hr)) {
			pScanInfo->Intensity = (LONG) dbl;
		}
	}

	pScanInfo->WidthPixels            = (LONG) (((double) (pScanInfo->BedWidth  * pScanInfo->Xresolution)) / 1000.0);
	pScanInfo->Lines                  = (LONG) (((double) (pScanInfo->BedHeight * pScanInfo->Yresolution)) / 1000.0);

	pScanInfo->Window.xPos            = 0;
	pScanInfo->Window.yPos            = 0;
	pScanInfo->Window.xExtent         = pScanInfo->WidthPixels;
	pScanInfo->Window.yExtent         = pScanInfo->Lines;

	// Scanner options
	pScanInfo->Negative               = 0;
	pScanInfo->Mirror                 = 0;
	pScanInfo->AutoBack               = 0;
	pScanInfo->DitherPattern          = 0;
	pScanInfo->ColorDitherPattern     = 0;
	pScanInfo->ToneMap                = 0;
	pScanInfo->Compression            = 0;

	hr = FetchScannerParams(pScanInfo, pContext);
	if (FAILED(hr))
		return hr;

#ifdef _DEBUG
	Trace(TEXT("bw    = %d"), pScanInfo->BedWidth);
	Trace(TEXT("bh    = %d"), pScanInfo->BedHeight);
#endif

	return S_OK;
}

HRESULT FreeScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	if (!pContext->pValues)
		return S_OK;

	if (pContext->pValues->pszModeThreshold) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszModeThreshold);
		pContext->pValues->pszModeThreshold = NULL;
	}
	if (pContext->pValues->pszModeGrayscale) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszModeGrayscale);
		pContext->pValues->pszModeGrayscale = NULL;
	}
	if (pContext->pValues->pszModeColor) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszModeColor);
		pContext->pValues->pszModeColor = NULL;
	}
	if (pContext->pValues->pszSourceFlatbed) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszSourceFlatbed);
		pContext->pValues->pszSourceFlatbed = NULL;
	}
	if (pContext->pValues->pszSourceADF) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszSourceADF);
		pContext->pValues->pszSourceADF = NULL;
	}
	if (pContext->pValues->pszSourceDuplex) {
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues->pszSourceDuplex);
		pContext->pValues->pszSourceDuplex = NULL;
	}

	HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues);
	pContext->pValues = NULL;

	return S_OK;
}
