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
 * This file was inspired by the example source code provided within the
 * Windows Image Acquisition (WIA) Driver Samples package which could be
 * found at the following download location (last accessed 2013-11-21):
 * http://code.msdn.microsoft.com/windowshardware/WIA-Microdriver-1972b1c5
 *
 ***************************************************************************/

#include "wiasane.h"

#include <sti.h>
#include <wia.h>
#include <math.h>
#include <winioctl.h>
#include <usbscan.h>
#include <tchar.h>

#include "wiasane_opt.h"
#include "wiasane_def.h"
#include "wiasane_scan.h"
#include "wiasane_util.h"
#include "strutil_dbg.h"


WIAMICRO_API HRESULT MicroEntry(LONG lCommand, _Inout_ PVAL pValue)
{ // depends on command
	PWIASANE_Context pContext;
	PWINSANE_Option oOption;
	LONG lReceived;
	HANDLE hHeap;
	HRESULT hr;

	Trace(TEXT("------ MicroEntry(lCommand=%d) ------"), lCommand);

	if (!pValue || !pValue->pScanInfo)
		return E_INVALIDARG;

	if (!pValue->pScanInfo->DeviceIOHandles[1])
		pValue->pScanInfo->DeviceIOHandles[1] = GetProcessHeap();

	hHeap = pValue->pScanInfo->DeviceIOHandles[1];
	if (!hHeap)
		return E_OUTOFMEMORY;

	if (!pValue->pScanInfo->pMicroDriverContext)
		pValue->pScanInfo->pMicroDriverContext = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WIASANE_Context));

	pContext = (PWIASANE_Context) pValue->pScanInfo->pMicroDriverContext;
	if (!pContext)
		return E_OUTOFMEMORY;

	hr = E_NOTIMPL;

	switch (lCommand) {
		case CMD_SETSTIDEVICEHKEY: // offline
			Trace(TEXT("CMD_SETSTIDEVICEHKEY"));

			if (pValue->pHandle) {
				pValue->pScanInfo->DeviceIOHandles[2] = *pValue->pHandle;

				hr = ReadRegistryInformation(pValue->pScanInfo, pContext);
			} else
				hr = E_FAIL;

			break;

		case CMD_INITIALIZE: // online
			Trace(TEXT("CMD_INITIALIZE"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				hr = InitScannerDefaults(pValue->pScanInfo, pContext);

				ExitScannerSession(pValue->pScanInfo, pContext);
			}

			pValue->pScanInfo->DeviceIOHandles[2] = NULL;
			break;

		case CMD_UNINITIALIZE: // online
			Trace(TEXT("CMD_UNINITIALIZE"));

			hr = FreeScannerSession(pValue->pScanInfo, pContext);
			FreeScannerDefaults(pValue->pScanInfo, pContext);
			FreeRegistryInformation(pValue->pScanInfo, pContext);

			pValue->pScanInfo->pMicroDriverContext = NULL;
			pValue->pScanInfo->DeviceIOHandles[2] = NULL;
			pValue->pScanInfo->DeviceIOHandles[1] = NULL;
			break;

		case CMD_RESETSCANNER: // online
			Trace(TEXT("CMD_RESETSCANNER"));

		case CMD_STI_DEVICERESET: // online
			if (lCommand == CMD_STI_DEVICERESET)
				Trace(TEXT("CMD_STI_DEVICERESET"));

		case CMD_STI_DIAGNOSTIC: // online
			if (lCommand == CMD_STI_DIAGNOSTIC)
				Trace(TEXT("CMD_STI_DIAGNOSTIC"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				hr = ExitScannerSession(pValue->pScanInfo, pContext);
			}
			break;

		case CMD_STI_GETSTATUS: // online
			Trace(TEXT("CMD_STI_GETSTATUS"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				pValue->lVal = MCRO_STATUS_OK;

				ExitScannerSession(pValue->pScanInfo, pContext);
			} else {
				pValue->lVal = MCRO_ERROR_OFFLINE;
			}
			pValue->pGuid = (GUID*) &GUID_NULL;

			hr = S_OK;
			break;

		case CMD_SETXRESOLUTION: // offline, but requires device options
			Trace(TEXT("CMD_SETXRESOLUTION"));

			if (pContext->oDevice) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_RESOLUTION);
				if (!oOption) {
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					hr = E_INVALIDARG;
					break;
				}
			} else {
				hr = E_FAIL;
				break;
			}

			pValue->pScanInfo->Xresolution = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETYRESOLUTION: // offline, but requires device options
			Trace(TEXT("CMD_SETYRESOLUTION"));

			if (pValue->pScanInfo->Xresolution != pValue->lVal) {
				hr = E_INVALIDARG;
				break;
			}

			pValue->pScanInfo->Yresolution = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETCONTRAST: // offline, but requires device options
			Trace(TEXT("CMD_SETCONTRAST"));

			if (pContext->oDevice) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_CONTRAST);
				if (!oOption) {
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					hr = E_INVALIDARG;
					break;
				}
			} else {
				hr = E_FAIL;
				break;
			}

			pValue->pScanInfo->Contrast = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETINTENSITY: // offline, but requires device options
			Trace(TEXT("CMD_SETINTENSITY"));

			if (pContext->oDevice) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BRIGHTNESS);
				if (!oOption) {
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					hr = E_INVALIDARG;
					break;
				}
			} else {
				hr = E_FAIL;
				break;
			}

			pValue->pScanInfo->Intensity = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETDATATYPE: // offline
			Trace(TEXT("CMD_SETDATATYPE"));

			if (pValue->lVal == WIA_DATA_THRESHOLD && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_BW)) {
					hr = E_NOTIMPL;
					break;
			}
			if (pValue->lVal == WIA_DATA_GRAYSCALE && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_GRAYSCALE)) {
					hr = E_NOTIMPL;
					break;
			}
			if (pValue->lVal == WIA_DATA_COLOR && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_COLOR)) {
					hr = E_NOTIMPL;
					break;
			}

			pValue->pScanInfo->DataType = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETSCANMODE: // offline
			Trace(TEXT("CMD_SETSCANMODE"));

			if (pValue->lVal != SCANMODE_FINALSCAN &&
				pValue->lVal != SCANMODE_PREVIEWSCAN) {
					hr = E_INVALIDARG;
					break;
			}

			pContext->lScanMode = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETNEGATIVE: // offline
			Trace(TEXT("CMD_SETNEGATIVE"));

			pValue->pScanInfo->Negative = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_GETCAPABILITIES: // offline
			Trace(TEXT("CMD_GETCAPABILITIES"));

			pValue->lVal = 0;
			pValue->pGuid = NULL;
			pValue->ppButtonNames = NULL;

			hr = S_OK;
			break;

		case CMD_GETADFSTATUS: // online
			Trace(TEXT("CMD_GETADFSTATUS"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				pValue->lVal = MCRO_STATUS_OK;

				ExitScannerSession(pValue->pScanInfo, pContext);
			} else {
				pValue->lVal = MCRO_ERROR_OFFLINE;
			}

			hr = S_OK;
			break;

		case CMD_GETADFHASPAPER: // online
			Trace(TEXT("CMD_GETADFHASPAPER"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				if (!pContext->pTask) {
					MicroEntry(CMD_LOAD_ADF, pValue);
				}

				if (pContext->pTask && pContext->pTask->bUsingADF) {
					pValue->lVal = MCRO_STATUS_OK;
					CloseScannerDevice(pValue->pScanInfo, pContext);
				} else {
					pValue->lVal = MCRO_ERROR_PAPER_EMPTY;
					ExitScannerSession(pValue->pScanInfo, pContext);
				}
			} else {
				pValue->lVal = MCRO_ERROR_OFFLINE;
			}

			hr = S_OK;
			break;

		case CMD_LOAD_ADF: // online
			Trace(TEXT("CMD_LOAD_ADF"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
				if (oOption && pContext->pValues) {
					switch (pValue->pScanInfo->ADF) {
						case 1:
							hr = oOption->SetValueString(pContext->pValues->pszSourceADF);
							break;

						case 2:
							hr = oOption->SetValueString(pContext->pValues->pszSourceDuplex);
							break;
					}
					if (SUCCEEDED(hr)) {
						hr = Scan(pValue->pScanInfo, SCAN_FIRST, NULL, 0, &lReceived);
						if (pContext->pTask) {
							if (SUCCEEDED(hr)) {
								pContext->pTask->bUsingADF = TRUE;
							} else {
								Scan(pValue->pScanInfo, SCAN_FINISHED, NULL, 0, &lReceived);
							}
						}
						if (!pContext->pTask || !pContext->pTask->bUsingADF) {
							oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
							if (oOption) {
								oOption->SetValueString(pContext->pValues->pszSourceFlatbed);
							}
						}
					}
				} else
					hr = E_NOTIMPL;

				CloseScannerDevice(pValue->pScanInfo, pContext);
			}

			break;

		case CMD_UNLOAD_ADF: // online
			Trace(TEXT("CMD_UNLOAD_ADF"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
				if (oOption && pContext->pValues) {
					hr = oOption->SetValueString(pContext->pValues->pszSourceFlatbed);
					if (SUCCEEDED(hr)) {
						if (pContext->pTask) {
							pContext->pTask->bUsingADF = FALSE;
						}
						hr = Scan(pValue->pScanInfo, SCAN_FINISHED, NULL, 0, &lReceived);
					}
				} else
					hr = E_NOTIMPL;

				ExitScannerSession(pValue->pScanInfo, pContext);
			}
			break;

		default:
			Trace(TEXT("Unknown Command (%d)"), lCommand);
			break;
	}

	return hr;
}

WIAMICRO_API HRESULT Scan(_Inout_ PSCANINFO pScanInfo, LONG lPhase, _Out_writes_bytes_(lLength) PBYTE pBuffer, LONG lLength, _Out_ LONG *plReceived)
{ // online
	PWIASANE_Context pContext;
	SANE_Status status;
	LONG idx, aquire, aquired;
	DWORD receive;
	HANDLE hHeap;
	HRESULT hr;

	Trace(TEXT("------ Scan(lPhase=%d, lLength=%d) ------"), lPhase, lLength);

	if (plReceived)
		*plReceived = 0;

	if (!pScanInfo)
		return E_INVALIDARG;

	hHeap = pScanInfo->DeviceIOHandles[1];
	if (!hHeap)
		return E_OUTOFMEMORY;

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext)
		return E_FAIL;

	switch (lPhase) {
		case SCAN_FIRST:
			Trace(TEXT("SCAN_FIRST"));

			//
			// first phase
			//

			hr = OpenScannerDevice(pScanInfo, pContext);
			if (FAILED(hr))
				return hr;

			if (pContext->pTask && pContext->pTask->oScan)
				return WIA_ERROR_BUSY;

			hr = SetScanMode(pContext);
			if (FAILED(hr)) {
				Trace(TEXT("Failed to set scan mode: %08x"), hr);
				return hr;
			}

			hr = SetScanWindow(pContext);
			if (FAILED(hr)) {
				Trace(TEXT("Failed to set scan window: %08x"), hr);
				return hr;
			}

			hr = SetScannerSettings(pScanInfo, pContext);
			if (FAILED(hr)) {
				Trace(TEXT("Failed to set scanner settings: %08x"), hr);
				return hr;
			}

			if (!pContext->pTask)
				pContext->pTask = (PWIASANE_Task) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WIASANE_Task));

			if (!pContext->pTask)
				return E_OUTOFMEMORY;

			status = pContext->oDevice->Start(&pContext->pTask->oScan);
			if (status != SANE_STATUS_GOOD)
				return GetErrorCode(status);

			if (!pContext->pTask->oScan)
				return E_OUTOFMEMORY;

			status = pContext->pTask->oScan->Connect();
			if (status != SANE_STATUS_GOOD)
				return GetErrorCode(status);

			Trace(TEXT("Byte-order: %04x"), pContext->pTask->oScan->GetByteOrder());

			hr = FetchScannerParams(pScanInfo, pContext);
			if (FAILED(hr))
				return hr;

			if (pScanInfo->PixelBits == 1)
				pContext->pTask->lByteGapX = ((LONG) ceil(pScanInfo->Window.xExtent / 8.0)) - pScanInfo->WidthBytes;
			else if ((pScanInfo->PixelBits % 8) == 0)
				pContext->pTask->lByteGapX = (pScanInfo->Window.xExtent * (pScanInfo->PixelBits / 8)) - pScanInfo->WidthBytes;
			else
				pContext->pTask->lByteGapX = 0;

			pContext->pTask->lByteGapY = (pScanInfo->Window.yExtent - pScanInfo->Lines) *
			                             (pScanInfo->WidthBytes + pContext->pTask->lByteGapX);

			Trace(TEXT("X byte-gap: %d"), pContext->pTask->lByteGapX);
			Trace(TEXT("Y byte-gap: %d"), pContext->pTask->lByteGapY);

			pContext->pTask->uiTotal = ((pScanInfo->WidthBytes + pContext->pTask->lByteGapX) *
			                            pScanInfo->Lines) + pContext->pTask->lByteGapY;
			pContext->pTask->uiReceived = 0;

			Trace(TEXT("Data: %d/%d"), pContext->pTask->uiReceived, pContext->pTask->uiTotal);

		case SCAN_NEXT: // SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)
			if (lPhase == SCAN_NEXT)
				Trace(TEXT("SCAN_NEXT"));

			if (!pBuffer)
				break;

			//
			// next phase, get data from the scanner and set plReceived value
			//

			if (!pContext->pTask || !pContext->pTask->oScan)
				return E_FAIL;

			memset(pBuffer, 0, lLength);

			if (pContext->pTask->lByteGapX > 0) // not enough data
				receive = min(lLength, pScanInfo->WidthBytes);
			else if (pContext->pTask->lByteGapX < 0) // too much data
				receive = min(lLength, pScanInfo->WidthBytes);
			else
				receive = lLength;

			aquired = 0;

			Trace(TEXT("Receiving %d bytes from data channel"), receive);
			while ((status = pContext->pTask->oScan->AquireImage((pBuffer + *plReceived + aquired), &receive)) == SANE_STATUS_GOOD) {
				Trace(TEXT("Received %d bytes from data channel"), receive);
				if (receive > 0) {
					if (pContext->pTask->lByteGapX > 0) { // not enough data
						Trace(TEXT("Previously aquired %d bytes from data channel"), aquired);
						aquired += receive;
						Trace(TEXT("Currently aquired %d bytes from data channel"), aquired);
						if (aquired == pScanInfo->WidthBytes) { // check for boundary
							Trace(TEXT("-- Hit width boundary, returning additional data --"));
							Trace(TEXT("Previously received %d bytes from data channel"), *plReceived);
							*plReceived += aquired;
							Trace(TEXT("Currently received %d bytes from data channel"), *plReceived);
							aquire = lLength - *plReceived;
							Trace(TEXT("Remaining %d empty x-bytes"), aquire);
							if (aquire >= pContext->pTask->lByteGapX) { // skip missing data
								Trace(TEXT("Adding %d additional x-bytes"), pContext->pTask->lByteGapX);
								*plReceived += pContext->pTask->lByteGapX;
							} else { // unable to skip data, break out
								Trace(TEXT("Not enough space to add %d additional x-bytes, break out"), pContext->pTask->lByteGapX);
								break; // not enough space
							}
							Trace(TEXT("Currently received %d bytes from data channel"), *plReceived);
							aquired = 0;
						}
						aquire = pScanInfo->WidthBytes - aquired;
						if (aquire > lLength - *plReceived) {
							Trace(TEXT("Next width boundary is outside of remaining space, break out"));
							break;
						}
						receive = aquire;
					} else if (pContext->pTask->lByteGapX < 0) { // too much data
						Trace(TEXT("Previously aquired %d bytes from data channel"), aquired);
						aquired += receive;
						Trace(TEXT("Currently aquired %d bytes from data channel"), aquired);
						if (aquired == pScanInfo->WidthBytes) { // check for boundary
							Trace(TEXT("-- Hit width boundary, skipping obsolete data --"));
							Trace(TEXT("Previously received %d bytes from data channel"), *plReceived);
							*plReceived += aquired;
							Trace(TEXT("Currently received %d bytes from data channel"), *plReceived);
							aquire = 0 - pContext->pTask->lByteGapX;
							if (aquire <= *plReceived) { // rewind obsolete data
								Trace(TEXT("Rewinding %d obsolete x-bytes"), aquire);
								*plReceived -= aquire;
							} else { // unable to rewind data, break out
								Trace(TEXT("Not enough space to rewind %d obsolete x-bytes, break out"), aquire);
								break; // not enough space
							}
							aquired = 0;
						}
						aquire = pScanInfo->WidthBytes - aquired;
						if (aquire > lLength - *plReceived) {
							Trace(TEXT("Next width boundary is outside of remaining space, break out"));
							break;
						}
						receive = aquire;
					} else {
						Trace(TEXT("Previously received %d bytes from data channel"), *plReceived);
						*plReceived += receive;
						Trace(TEXT("Currently received %d bytes from data channel"), *plReceived);
						receive = lLength - *plReceived;
					}
				}
				if (receive <= 0) {
					Trace(TEXT("No more data avaiable or required, break out"), receive);
					break;
				}
				Trace(TEXT("Receiving %d bytes from data channel"), receive);
			}

			if (status == SANE_STATUS_EOF && pContext->pTask->lByteGapY > 0) {
				aquire = lLength - *plReceived;
				if (aquire > 0) {
					aquire = min(aquire, pContext->pTask->lByteGapY);
					Trace(TEXT("Adding %d additional y-bytes"), aquire);
					memset(pBuffer + *plReceived, -1, aquire);
					*plReceived += aquire;
				}
			}

			if (pScanInfo->DataType == WIA_DATA_THRESHOLD) {
				for (idx = 0; idx < lLength; idx++) {
					pBuffer[idx] = ~pBuffer[idx];
				}
			}

			pContext->pTask->uiTotal = ((pScanInfo->WidthBytes + pContext->pTask->lByteGapX) *
			                            pScanInfo->Lines) + pContext->pTask->lByteGapY;
			pContext->pTask->uiReceived += *plReceived;

			Trace(TEXT("Data: %d/%d -> %d/%d"), pContext->pTask->uiReceived, pContext->pTask->uiTotal,
			                       *plReceived, pContext->pTask->uiTotal - pContext->pTask->uiReceived);

			break;

		case SCAN_FINISHED:
		default:
			Trace(TEXT("SCAN_FINISHED"));

			//
			// stop scanner, do not set lRecieved, or write any data to pBuffer.  Those values
			// will be NULL.  This lPhase is only to allow you to stop scanning, and return the
			// scan head to the HOME position. SCAN_FINISHED will be called always for regular scans, and
			// for cancelled scans.
			//

			if (pContext->oDevice) {
				if (pContext->pTask) {
					if (pContext->pTask->oScan) {
						delete pContext->pTask->oScan;
						pContext->pTask->oScan = NULL;
					}

					ZeroMemory(pContext->pTask, sizeof(WIASANE_Task));
					HeapFree(hHeap, 0, pContext->pTask);
					pContext->pTask = NULL;
				}

				if (pContext->oDevice->IsOpen()) {
					pContext->oDevice->Cancel();
				}

				ExitScannerSession(pScanInfo, pContext);
			}

			break;
	}

	return S_OK;
}

WIAMICRO_API HRESULT SetPixelWindow(_Inout_ PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{ // offline, but requires device options
	PWIASANE_Context pContext;
	PWINSANE_Option oOption;
	HRESULT hr;
	double tl_x, tl_y, br_x, br_y;

	Trace(TEXT("------ SetPixelWindow(x=%d, y=%d, xExtent=%d, yExtent=%d) ------"), x, y, xExtent, yExtent);

	if (!pScanInfo)
		return E_INVALIDARG;

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext || !pContext->oSession || !pContext->oDevice)
		return E_FAIL;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_TL_X);
	if (!oOption)
		return E_NOTIMPL;

	tl_x = ((double) x) / ((double) pScanInfo->Xresolution);

	hr = IsValidOptionValueInch(oOption, tl_x);
	if (FAILED(hr))
		return hr;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_TL_Y);
	if (!oOption)
		return E_NOTIMPL;

	tl_y = ((double) y) / ((double) pScanInfo->Yresolution);

	hr = IsValidOptionValueInch(oOption, tl_y);
	if (FAILED(hr))
		return hr;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_X);
	if (!oOption)
		return E_NOTIMPL;

	br_x = ((double) (x + xExtent)) / ((double) pScanInfo->Xresolution);

	hr = IsValidOptionValueInch(oOption, br_x);
	if (FAILED(hr))
		return hr;

	oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BR_Y);
	if (!oOption)
		return E_NOTIMPL;

	br_y = ((double) (y + yExtent)) / ((double) pScanInfo->Yresolution);

	hr = IsValidOptionValueInch(oOption, br_y);
	if (FAILED(hr))
		return hr;

	pContext->dblTopLeftX = tl_x;
	pContext->dblTopLeftY = tl_y;
	pContext->dblBottomRightX = br_x;
	pContext->dblBottomRightY = br_y;

	pScanInfo->Window.xPos = x;
	pScanInfo->Window.yPos = y;
	pScanInfo->Window.xExtent = xExtent;
	pScanInfo->Window.yExtent = yExtent;

#ifdef _DEBUG
	Trace(TEXT("Scanner window"));
	Trace(TEXT("xpos  = %d"), pScanInfo->Window.xPos);
	Trace(TEXT("ypos  = %d"), pScanInfo->Window.yPos);
	Trace(TEXT("xext  = %d"), pScanInfo->Window.xExtent);
	Trace(TEXT("yext  = %d"), pScanInfo->Window.yExtent);
	Trace(TEXT("tl_x  = %f"), pContext->dblTopLeftX);
	Trace(TEXT("tl_y  = %f"), pContext->dblTopLeftY);
	Trace(TEXT("br_x  = %f"), pContext->dblBottomRightX);
	Trace(TEXT("br_y  = %f"), pContext->dblBottomRightY);
#endif

	return hr;
}


HRESULT CreateScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	if (pContext->oSession)
		return S_OK;

	pContext->uiDevRef = 0;

	pContext->pTask = NULL;
	pContext->oDevice = NULL;

	pContext->oSession = WINSANE_Session::Remote(pContext->pszHost, pContext->usPort);
	if (!pContext->oSession)
		return WIA_ERROR_OFFLINE;

	return S_OK;
}

HRESULT FreeScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	if (!pContext->oSession)
		return S_OK;

	ExitScannerSession(pScanInfo, pContext);

	delete pContext->oSession;
	pContext->oSession = NULL;

	pContext->oDevice = NULL;
	pContext->pTask = NULL;

	pContext->uiDevRef = 0;

	return S_OK;
}


HRESULT InitScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	if (pContext->oSession && pContext->oSession->IsInitialized() && pContext->oDevice)
		return S_OK;

	if (!pContext->oSession) {
		hr = CreateScannerSession(pScanInfo, pContext);
		if (FAILED(hr))
			return hr;
		if (!pContext->oSession)
			return E_FAIL;
	}

	if (!pContext->oSession->IsInitialized()) {
		status = pContext->oSession->Init(NULL, NULL);
		if (status != SANE_STATUS_GOOD) {
			return GetErrorCode(status);
		}
	}

	status = pContext->oSession->FetchDevices();
	if (status != SANE_STATUS_GOOD) {
		pContext->oSession->Exit();
		return GetErrorCode(status);
	}

	pContext->oDevice = pContext->oSession->GetDevice(pContext->pszName);
	if (!pContext->oDevice) {
		pContext->oSession->Exit();
		return WIA_ERROR_USER_INTERVENTION;
	}

	return S_OK;
}

HRESULT ExitScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;

	if (!pScanInfo || !pContext || !pContext->oSession)
		return E_INVALIDARG;

	if (pContext->oDevice) {
		CloseScannerDevice(pScanInfo, pContext);
	}

	if (pContext->oSession->IsInitialized()) {
		status = pContext->oSession->Exit();
		if (status != SANE_STATUS_GOOD)
			return GetErrorCode(status);
	}

	return S_OK;
}


HRESULT OpenScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	if (!pScanInfo || !pContext)
		return E_INVALIDARG;

	if (!pContext->oSession || !pContext->oSession->IsInitialized() || !pContext->oDevice) {
		hr = InitScannerSession(pScanInfo, pContext);
		if (FAILED(hr))
			return hr;
		if (!pContext->oSession || !pContext->oDevice)
			return E_FAIL;
	}

	if (!pContext->oDevice->IsOpen()) {
		status = pContext->oDevice->Open();
		if (status != SANE_STATUS_GOOD)
			return GetErrorCode(status);

		status = pContext->oDevice->FetchOptions();
		if (status != SANE_STATUS_GOOD)
			return GetErrorCode(status);
	}

	pContext->uiDevRef++;
	return S_OK;
}

HRESULT CloseScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;

	if (!pScanInfo || !pContext || !pContext->oSession || !pContext->oDevice)
		return E_INVALIDARG;

	if (pContext->uiDevRef > 0)
		pContext->uiDevRef--;

	if (pContext->uiDevRef > 0)
		return S_OK;

	if (pContext->pTask) {
		if (pContext->pTask->oScan) {
			if (pContext->oDevice->IsOpen()) {
				pContext->oDevice->Cancel();
			}

			delete pContext->pTask->oScan;
			pContext->pTask->oScan = NULL;
		}

		ZeroMemory(pContext->pTask, sizeof(WIASANE_Task));
		HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pTask);
		pContext->pTask = NULL;
	}

	if (pContext->oDevice->IsOpen()) {
		status = pContext->oDevice->Close();
		if (status != SANE_STATUS_GOOD)
			return GetErrorCode(status);
	}

	return S_OK;
}
