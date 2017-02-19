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

#include "wiasane_scan.h"

#include <sti.h>
#include <wia.h>

#include "wiasane_opt.h"
#include "winsane-util_dbg.h"

HRESULT FetchScannerParams(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Params oParams;
	SANE_Frame frame;
	SANE_Int depth;
	HRESULT hr;

	if (!pScanInfo || !pContext ||
		!pContext->oSession || !pContext->oSession->IsInitialized() ||
		!pContext->oDevice || !pContext->oDevice->IsOpen())
			return E_INVALIDARG;

	if (pContext->oDevice->GetParams(&oParams) != SANE_STATUS_GOOD)
		return E_FAIL;

	if (!oParams)
		return E_OUTOFMEMORY;

	frame = oParams->GetFormat();
	depth = oParams->GetDepth();

	switch (frame) {
		case SANE_FRAME_GRAY:
			pScanInfo->RawDataFormat = WIA_PACKED_PIXEL;
			pScanInfo->RawPixelOrder = 0;
			pScanInfo->DataType = depth == 1 ? WIA_DATA_THRESHOLD : WIA_DATA_GRAYSCALE;
			pScanInfo->PixelBits = depth;
			hr = S_OK;
			break;

		case SANE_FRAME_RGB:
			pScanInfo->RawDataFormat = WIA_PACKED_PIXEL;
			pScanInfo->RawPixelOrder = WIA_ORDER_BGR;
			pScanInfo->DataType = WIA_DATA_COLOR;
			pScanInfo->PixelBits = depth * 3;
			hr = S_OK;
			break;

		case SANE_FRAME_RED:
		case SANE_FRAME_GREEN:
		case SANE_FRAME_BLUE:
			pScanInfo->RawDataFormat = WIA_PLANAR;
			pScanInfo->RawPixelOrder = WIA_ORDER_BGR;
			pScanInfo->DataType = WIA_DATA_COLOR;
			pScanInfo->PixelBits = depth * 3;
			hr = S_OK;
			break;

		default:
			hr = E_NOTIMPL;
			break;
	}

	if (SUCCEEDED(hr)) {
		pScanInfo->WidthBytes = oParams->GetBytesPerLine();
		pScanInfo->WidthPixels = oParams->GetPixelsPerLine();
		pScanInfo->Lines = oParams->GetLines();
	}

	delete oParams;

#ifdef _DEBUG
	Trace(TEXT("Scanner parameters"));
	Trace(TEXT("x res = %d"), pScanInfo->Xresolution);
	Trace(TEXT("y res = %d"), pScanInfo->Yresolution);
	Trace(TEXT("bpp   = %d"), pScanInfo->PixelBits);
	Trace(TEXT("xpos  = %d"), pScanInfo->Window.xPos);
	Trace(TEXT("ypos  = %d"), pScanInfo->Window.yPos);
	Trace(TEXT("xext  = %d"), pScanInfo->Window.xExtent);
	Trace(TEXT("yext  = %d"), pScanInfo->Window.yExtent);
	Trace(TEXT("wbyte = %d"), pScanInfo->WidthBytes);
	Trace(TEXT("wpixl = %d"), pScanInfo->WidthPixels);
	Trace(TEXT("lines = %d"), pScanInfo->Lines);
#endif

	return hr;
}

HRESULT SetScannerSettings(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Option oOption;
	HRESULT hr;

	if (!pScanInfo || !pContext || !pContext->pValues ||
		!pContext->oSession || !pContext->oSession->IsInitialized() ||
		!pContext->oDevice || !pContext->oDevice->IsOpen())
			return E_INVALIDARG;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_MODE);
	if (oOption && oOption->GetType() == SANE_TYPE_STRING) {
		switch (pScanInfo->DataType) {
			case WIA_DATA_THRESHOLD:
				hr = oOption->SetValueString(pContext->pValues->pszModeThreshold);
				break;
			case WIA_DATA_GRAYSCALE:
				hr = oOption->SetValueString(pContext->pValues->pszModeGrayscale);
				break;
			case WIA_DATA_COLOR:
				hr = oOption->SetValueString(pContext->pValues->pszModeColor);
				break;
			default:
				hr = E_INVALIDARG;
				break;
		}
		if (FAILED(hr)) {
			Trace(TEXT("Failed to set option '%hs' according to '%d': %08x"),
				oOption->GetName(), pScanInfo->DataType, hr);
			return hr;
		}
	} else {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_MODE);
		return E_NOTIMPL;
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_RESOLUTION);
	if (oOption) {
		hr = oOption->SetValue(pScanInfo->Xresolution);
		if (FAILED(hr)) {
			Trace(TEXT("Failed to set option '%hs' to '%d': %08x"),
				oOption->GetName(), pScanInfo->Xresolution, hr);
			return hr;
		}
	} else {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_RESOLUTION);
		return E_NOTIMPL;
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_CONTRAST);
	if (!oOption) {
		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SHARPNESS);
	}
	if (oOption) {
		hr = oOption->SetValue(pScanInfo->Contrast);
		if (FAILED(hr) && hr != E_NOTIMPL) {
			Trace(TEXT("Failed to set option '%hs' to '%d': %08x"),
				oOption->GetName(), pScanInfo->Contrast, hr);
			return hr;
		}
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BRIGHTNESS);
	if (oOption) {
		hr = oOption->SetValue(pScanInfo->Intensity);
		if (FAILED(hr) && hr != E_NOTIMPL) {
			Trace(TEXT("Failed to set option '%hs' to '%d': %08x"),
				oOption->GetName(), pScanInfo->Intensity, hr);
			return hr;
		}
	}

	return S_OK;
}


HRESULT SetScanWindow(_Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Option oOption;
	HRESULT hr;

	if (!pContext ||
		!pContext->oSession || !pContext->oSession->IsInitialized() ||
		!pContext->oDevice || !pContext->oDevice->IsOpen())
			return E_INVALIDARG;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_TL_X);
	if (!oOption) {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_TL_X);
		return E_NOTIMPL;
	}

	hr = SetOptionValueInch(oOption, pContext->dblTopLeftX);
	if (FAILED(hr)) {
		Trace(TEXT("Failed to set option '%hs' to '%f': %08x"),
			oOption->GetName(), pContext->dblTopLeftX, hr);
		return hr;
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_TL_Y);
	if (!oOption) {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_TL_Y);
		return E_NOTIMPL;
	}

	hr = SetOptionValueInch(oOption, pContext->dblTopLeftY);
	if (FAILED(hr)) {
		Trace(TEXT("Failed to set option '%hs' to '%f': %08x"),
			oOption->GetName(), pContext->dblTopLeftY, hr);
		return hr;
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_X);
	if (!oOption) {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_BR_X);
		return E_NOTIMPL;
	}

	hr = SetOptionValueInch(oOption, pContext->dblBottomRightX);
	if (FAILED(hr)) {
		Trace(TEXT("Failed to set option '%hs' to '%f': %08x"),
			oOption->GetName(), pContext->dblBottomRightX, hr);
		return hr;
	}

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_Y);
	if (!oOption) {
		Trace(TEXT("Required option '%hs' is not supported."),
			WIASANE_OPTION_BR_Y);
		return E_NOTIMPL;
	}

	hr = SetOptionValueInch(oOption, pContext->dblBottomRightY);
	if (FAILED(hr)) {
		Trace(TEXT("Failed to set option '%hs' to '%f': %08x"),
			oOption->GetName(), pContext->dblBottomRightY, hr);
		return hr;
	}

	return hr;
}

HRESULT SetScanMode(_Inout_ PWIASANE_Context pContext)
{
	PSANE_String_Const string_list;
	PWINSANE_Option oOption;
	HRESULT hr;

	if (!pContext ||
		!pContext->oSession || !pContext->oSession->IsInitialized() ||
		!pContext->oDevice || !pContext->oDevice->IsOpen())
			return E_INVALIDARG;

	switch (pContext->lScanMode) {
		case SCANMODE_FINALSCAN:
			Trace(TEXT("Final Scan"));

			oOption = pContext->oDevice->GetOption(WIASANE_OPTION_PREVIEW);
			if (oOption && oOption->GetType() == SANE_TYPE_BOOL) {
				oOption->SetValueBool(SANE_FALSE);
			}

			oOption = pContext->oDevice->GetOption(WIASANE_OPTION_COMPRESSION);
			if (oOption && oOption->GetType() == SANE_TYPE_STRING
				&& oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
				string_list = oOption->GetConstraintStringList();
				if (string_list[0] != NULL) {
					oOption->SetValueString(string_list[0]);
				}
			}

			hr = S_OK;
			break;

		case SCANMODE_PREVIEWSCAN:
			Trace(TEXT("Preview Scan"));

			oOption = pContext->oDevice->GetOption(WIASANE_OPTION_PREVIEW);
			if (oOption && oOption->GetType() == SANE_TYPE_BOOL) {
				oOption->SetValueBool(SANE_TRUE);
			}

			oOption = pContext->oDevice->GetOption(WIASANE_OPTION_COMPRESSION);
			if (oOption && oOption->GetType() == SANE_TYPE_STRING
				&& oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
				string_list = oOption->GetConstraintStringList();
				if (string_list[0] != NULL && string_list[1] != NULL) {
					oOption->SetValueString(string_list[1]);
				}
			}

			hr = S_OK;
			break;

		default:
			Trace(TEXT("Unknown Scan Mode (%d)"), pContext->lScanMode);

			hr = E_INVALIDARG;
			break;
	}

	return hr;
}
