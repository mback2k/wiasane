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
#include "strutil_mem.h"
#include "strutil.h"


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

			// fall through to device reset
		case CMD_STI_DEVICERESET: // online
			if (lCommand == CMD_STI_DEVICERESET)
				Trace(TEXT("CMD_STI_DEVICERESET"));

			// fall through to diagnostics, including device reset
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
					Trace(TEXT("Required option '%hs' is not supported."),
						WIASANE_OPTION_RESOLUTION);
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					Trace(TEXT("Invalid value '%d' for option '%hs'."),
						pValue->lVal, oOption->GetName());
					hr = E_INVALIDARG;
					break;
				}
			} else {
				Trace(TEXT("No device selected."));
				hr = E_FAIL;
				break;
			}

			Trace(TEXT("Set Xresolution to '%d'."), pValue->lVal);
			pValue->pScanInfo->Xresolution = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETYRESOLUTION: // offline, but requires device options
			Trace(TEXT("CMD_SETYRESOLUTION"));

			if (pValue->pScanInfo->Xresolution != pValue->lVal) {
				Trace(TEXT("Invalid value '%d' compared to '%d'."),
					pValue->lVal, pValue->pScanInfo->Xresolution);
				hr = E_INVALIDARG;
				break;
			}

			Trace(TEXT("Set Yresolution to '%d'."), pValue->lVal);
			pValue->pScanInfo->Yresolution = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETCONTRAST: // offline, but requires device options
			Trace(TEXT("CMD_SETCONTRAST"));

			if (pContext->oDevice) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_CONTRAST);
				if (!oOption) {
					oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SHARPNESS);
				}
				if (!oOption) {
					Trace(TEXT("Option '%hs' or '%hs' is not supported."),
						WIASANE_OPTION_CONTRAST, WIASANE_OPTION_SHARPNESS);
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					Trace(TEXT("Invalid value '%d' for option '%hs'."),
						pValue->lVal, oOption->GetName());
					hr = E_INVALIDARG;
					break;
				}
			} else {
				Trace(TEXT("No device selected."));
				hr = E_FAIL;
				break;
			}

			Trace(TEXT("Set Contrast to '%d'."), pValue->lVal);
			pValue->pScanInfo->Contrast = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETINTENSITY: // offline, but requires device options
			Trace(TEXT("CMD_SETINTENSITY"));

			if (pContext->oDevice) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BRIGHTNESS);
				if (!oOption) {
					Trace(TEXT("Option '%hs' or '%hs' is not supported."),
						WIASANE_OPTION_BRIGHTNESS);
					hr = E_NOTIMPL;
					break;
				}
				if (!oOption->IsValidValue(pValue->lVal)) {
					Trace(TEXT("Invalid value '%d' for option '%hs'."),
						pValue->lVal, oOption->GetName());
					hr = E_INVALIDARG;
					break;
				}
			} else {
				Trace(TEXT("No device selected."));
				hr = E_FAIL;
				break;
			}

			Trace(TEXT("Set Intensity to '%d'."), pValue->lVal);
			pValue->pScanInfo->Intensity = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETDATATYPE: // offline
			Trace(TEXT("CMD_SETDATATYPE"));

			if (pValue->lVal == WIA_DATA_THRESHOLD && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_BW)) {
					Trace(TEXT("Threshold mode is not supported."));
					hr = E_NOTIMPL;
					break;
			}
			if (pValue->lVal == WIA_DATA_GRAYSCALE && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_GRAYSCALE)) {
					Trace(TEXT("Grayscale mode is not supported."));
					hr = E_NOTIMPL;
					break;
			}
			if (pValue->lVal == WIA_DATA_COLOR && !(
			    pValue->pScanInfo->SupportedDataTypes & SUPPORT_COLOR)) {
					Trace(TEXT("Color mode is not supported."));
					hr = E_NOTIMPL;
					break;
			}

			Trace(TEXT("Set DataType to '%d'."), pValue->lVal);
			pValue->pScanInfo->DataType = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETSCANMODE: // offline
			Trace(TEXT("CMD_SETSCANMODE"));

			if (pValue->lVal != SCANMODE_FINALSCAN &&
				pValue->lVal != SCANMODE_PREVIEWSCAN) {
					Trace(TEXT("Unknown scan mode is not supported."));
					hr = E_INVALIDARG;
					break;
			}

			Trace(TEXT("Set lScanMode to '%d'."), pValue->lVal);
			pContext->lScanMode = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETNEGATIVE: // offline
			Trace(TEXT("CMD_SETNEGATIVE"));

			Trace(TEXT("Set Negative to '%d'."), pValue->lVal);
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

		case CMD_GETADFAVAILABLE: // offline
			Trace(TEXT("CMD_GETADFAVAILABLE"));

			pValue->lVal = pValue->pScanInfo->ADF > 0 ? TRUE : FALSE;
			hr = S_OK;
			break;

		case CMD_GETADFSTATUS: // online
			Trace(TEXT("CMD_GETADFSTATUS"));

			hr = OpenScannerDevice(pValue->pScanInfo, pContext);
			if (SUCCEEDED(hr)) {
				pValue->lVal = MCRO_STATUS_OK;

				if (pContext->pTask && pContext->bUsingADF) {
					CloseScannerDevice(pValue->pScanInfo, pContext);
				} else {
					ExitScannerSession(pValue->pScanInfo, pContext);
				}
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

				if (pContext->pTask && pContext->bUsingADF) {
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

			if (pContext->pTask && pContext->bUsingADF) {
				hr = S_OK;
				break;
			}

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
						pContext->bUsingADF = TRUE;
						hr = Scan(pValue->pScanInfo, SCAN_FIRST, NULL, 0, &lReceived);
						if (FAILED(hr)) {
							pContext->bUsingADF = FALSE;
							Scan(pValue->pScanInfo, SCAN_FINISHED, NULL, 0, &lReceived);
						}
						if (!pContext->pTask || !pContext->bUsingADF) {
							oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
							if (oOption) {
								oOption->SetValueString(pContext->pValues->pszSourceFlatbed);
							}
						}
					}
				} else {
					hr = E_NOTIMPL;
				}

				if (pContext->pTask && pContext->bUsingADF) {
					CloseScannerDevice(pValue->pScanInfo, pContext);
				} else {
					ExitScannerSession(pValue->pScanInfo, pContext);
				}
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
						pContext->bUsingADF = FALSE;
					}
				} else {
					hr = E_NOTIMPL;
				}

				CloseScannerDevice(pValue->pScanInfo, pContext);
			}
			break;

		case CMD_GETADFUNLOADREADY: // offline
			Trace(TEXT("CMD_GETADFUNLOADREADY"));

			pValue->lVal = !pContext->pTask && pContext->bUsingADF ? TRUE : FALSE;
			hr = S_OK;
			break;

		case CMD_GETSUPPORTEDFILEFORMATS: // unsupported
			Trace(TEXT("CMD_GETSUPPORTEDFILEFORMATS"));
			break;

		case CMD_GETSUPPORTEDMEMORYFORMATS: // unsupported
			Trace(TEXT("CMD_GETSUPPORTEDMEMORYFORMATS"));
			break;

		case CMD_SETFORMAT: // unsupported
			Trace(TEXT("CMD_SETFORMAT"));
			break;

		default:
			Trace(TEXT("Unknown Command (%d)"), lCommand);
			break;
	}

	Trace(TEXT("------ MicroEntry(lCommand=%d) => %08x ------"), lCommand, hr);

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

	if (!pScanInfo) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	hHeap = pScanInfo->DeviceIOHandles[1];
	if (!hHeap) {
		Trace(TEXT("Missing heap"));
		return E_OUTOFMEMORY;
	}

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext) {
		Trace(TEXT("Missing context"));
		return E_FAIL;
	}

	switch (lPhase) {
		case SCAN_FIRST:
			Trace(TEXT("SCAN_FIRST"));

			//
			// first phase
			//

			hr = OpenScannerDevice(pScanInfo, pContext);
			if (FAILED(hr)) {
				Trace(TEXT("Failed to open scanner device"));
				return hr;
			}

			if (pContext->pTask && pContext->pTask->oScan) {
				if (pContext->bUsingADF) {
					Trace(TEXT("SCAN_FIRST using ADF"));
				} else {
					Trace(TEXT("SCAN_FIRST during busy device"));
					return WIA_ERROR_BUSY;
				}
			} else {
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

				if (!pContext->pTask) {
					Trace(TEXT("Task does not exist, allocating it"));
					pContext->pTask = (PWIASANE_Task) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WIASANE_Task));
				}

				if (!pContext->pTask) {
					Trace(TEXT("Failed to create task, out of memory"));
					return E_OUTOFMEMORY;
				}

				status = pContext->oDevice->Start(&pContext->pTask->oScan);
				if (status != SANE_STATUS_GOOD) {
					Trace(TEXT("Failed to start scan"));
					return GetErrorCode(status);
				}

				if (!pContext->pTask->oScan) {
					Trace(TEXT("Failed to create scan, out of memory"));
					return E_OUTOFMEMORY;
				}

				status = pContext->pTask->oScan->Connect();
				if (status != SANE_STATUS_GOOD) {
					Trace(TEXT("Failed to connect to scan data port"));
					return GetErrorCode(status);
				}

				Trace(TEXT("Byte-order: %04x"), pContext->pTask->oScan->GetByteOrder());

				hr = FetchScannerParams(pScanInfo, pContext);
				if (FAILED(hr)) {
					Trace(TEXT("Failed to fetch scanner parameters"));
					return hr;
				}

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
			}

			// SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)
		case SCAN_NEXT:
			if (lPhase == SCAN_NEXT)
				Trace(TEXT("SCAN_NEXT"));

			if (!pBuffer || !plReceived)
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
				Trace(TEXT("Device exists, exiting scanner session"));
				if (pContext->pTask) {
					Trace(TEXT("Tasks exists, deleting it"));
					if (pContext->pTask->oScan) {
						Trace(TEXT("Scan exists, deleting it"));
						delete pContext->pTask->oScan;
						pContext->pTask->oScan = NULL;
					}

					HeapSafeFree(hHeap, 0, pContext->pTask);
					pContext->pTask = NULL;
				}

				if (!pContext->bUsingADF) {
					Trace(TEXT("ADF is not in use, must cancel the current scan"));
					if (pContext->oDevice->IsOpen()) {
						Trace(TEXT("Device is open, cancelling current scan"));
						pContext->oDevice->Cancel();
					}

					Trace(TEXT("Exiting scanner session"));
					ExitScannerSession(pScanInfo, pContext);
				} else {
					Trace(TEXT("ADF is in use, cannot cancel the current scan"));

					Trace(TEXT("Close scanner device"));
					CloseScannerDevice(pScanInfo, pContext);
				}
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


WINSANE_API_CALLBACK SessionAuthCallbackEx(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password, _In_ void *userdata)
{
	LPSTR lpUsername, lpPassword;
	PWIASANE_Context pContext;
	HANDLE hHeap;

	if (!resource || !strlen(resource) || !username || !password || !userdata)
		return;

	Trace(TEXT("------ SessionAuthCallback(resource='%hs') ------"), resource);

	pContext = (PWIASANE_Context) userdata;

	hHeap = GetProcessHeap();
	if (!hHeap)
		return;

	lpUsername = StringConvToA(hHeap, pContext->pszUsername);
	if (lpUsername) {
		lpPassword = StringConvToA(hHeap, pContext->pszPassword);
		if (lpPassword) {
			strcpy_s(username, SANE_MAX_USERNAME_LEN, lpUsername);
			strcpy_s(password, SANE_MAX_PASSWORD_LEN, lpPassword);
			HeapSafeFree(hHeap, 0, lpPassword);
		}
		HeapSafeFree(hHeap, 0, lpUsername);
	}

	Trace(TEXT("Username: %hs (%d)"), username, strlen(username));
	Trace(TEXT("Password: ******** (%d)"), strlen(password));
}


HRESULT CreateScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	Trace(TEXT("------ CreateScannerSession() ------"));

	if (!pScanInfo || !pContext) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	if (pContext->oSession) {
		Trace(TEXT("Session already exists"));
		return S_OK;
	}

	Trace(TEXT("Initializing device reference count"));
	pContext->uiDevRef = 0;
	pContext->pTask = NULL;

	pContext->oDevice = NULL;
	pContext->oSession = WINSANE_Session::Remote(pContext->pszHost, pContext->usPort);
	if (!pContext->oSession) {
		Trace(TEXT("Session could not be created, device probably offline"));
		return WIA_ERROR_OFFLINE;
	}

	return S_OK;
}

HRESULT FreeScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	Trace(TEXT("------ FreeScannerSession() ------"));

	if (!pScanInfo || !pContext) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	Trace(TEXT("Before freeing the session, exit it"));
	ExitScannerSession(pScanInfo, pContext);

	if (pContext->oDevice) {
		Trace(TEXT("Device exists, delete it"));
		delete pContext->oDevice;
		pContext->oDevice = NULL;
	}

	if (pContext->oSession) {
		Trace(TEXT("Session exists, delete it"));
		delete pContext->oSession;
		pContext->oSession = NULL;
	}

	Trace(TEXT("Reset task and device reference count without checking it"));
	pContext->pTask = NULL;
	pContext->uiDevRef = 0;

	return S_OK;
}


HRESULT InitScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	Trace(TEXT("------ InitScannerSession() ------"));

	if (!pScanInfo || !pContext) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	if (pContext->oSession && pContext->oSession->IsInitialized() && pContext->oDevice) {
		Trace(TEXT("Session is already initialized and device exists"));
		return S_OK;
	}

	if (!pContext->oSession) {
		Trace(TEXT("Session does not exist, creating it"));
		hr = CreateScannerSession(pScanInfo, pContext);
		if (FAILED(hr)) {
			Trace(TEXT("Creating session failed"));
			return hr;
		}
		if (!pContext->oSession) {
			Trace(TEXT("Creating session failed, session missing"));
			return E_FAIL;
		}
	}

	if (!pContext->oSession->IsInitialized()) {
		Trace(TEXT("Session is not initialized, initializing it"));
		status = pContext->oSession->InitEx(NULL, &SessionAuthCallbackEx, pContext);
		if (status != SANE_STATUS_GOOD) {
			Trace(TEXT("Initializing session failed"));
			return GetErrorCode(status);
		}
	}

	if (!pContext->oDevice) {
		Trace(TEXT("Device does not exist, creating it by name"));
		pContext->oDevice = pContext->oSession->CreateDevice(pContext->pszName);
		if (!pContext->oDevice) {
			Trace(TEXT("Creating device by name failed, user intervention required, exiting session"));
			pContext->oSession->Exit();
			return WIA_ERROR_USER_INTERVENTION;
		}
	}

	return S_OK;
}

HRESULT ExitScannerSession(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;

	Trace(TEXT("------ ExitScannerSession() ------"));

	if (!pScanInfo || !pContext || !pContext->oSession) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	if (pContext->oDevice) {
		Trace(TEXT("Device still exists, close it"));
		CloseScannerDevice(pScanInfo, pContext);
	}

	if (pContext->uiDevRef > 0) {
		Trace(TEXT("Cannot exit session, because device reference count is still greater than 0"));
		return S_FALSE;
	}

	if (pContext->oSession->IsInitialized()) {
		Trace(TEXT("Session is initialized, exiting it"));
		status = pContext->oSession->Exit();
		if (status != SANE_STATUS_GOOD) {
			Trace(TEXT("Exiting session failed"));
			return GetErrorCode(status);
		}
	} else {
		Trace(TEXT("Session was not initialized"));
	}

	return S_OK;
}


HRESULT OpenScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	Trace(TEXT("------ OpenScannerDevice() ------"));

	if (!pScanInfo || !pContext) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	if (!pContext->oSession || !pContext->oSession->IsInitialized() || !pContext->oDevice) {
		Trace(TEXT("Device or session does not exist or is not initialized, initializing session"));
		hr = InitScannerSession(pScanInfo, pContext);
		if (FAILED(hr)) {
			Trace(TEXT("Session initialization failed"));
			return hr;
		}
		if (!pContext->oSession || !pContext->oDevice) {
			Trace(TEXT("Session initialization failed, session or device missing"));
			return E_FAIL;
		}
	}

	if (!pContext->oDevice->IsOpen()) {
		Trace(TEXT("Device is not open, opening it"));
		status = pContext->oDevice->Open();
		if (status != SANE_STATUS_GOOD) {
			Trace(TEXT("Opening device failed"));
			return GetErrorCode(status);
		}

		Trace(TEXT("Fetching device options"));
		status = pContext->oDevice->FetchOptions();
		if (status != SANE_STATUS_GOOD) {
			Trace(TEXT("Fetching device options failed"));
			return GetErrorCode(status);
		}
	} else {
		Trace(TEXT("Device is already open"));
	}

	Trace(TEXT("Device reference count = %d"), pContext->uiDevRef);

	Trace(TEXT("Increment device reference count"));
	pContext->uiDevRef++;

	Trace(TEXT("Device reference count = %d"), pContext->uiDevRef);

	return S_OK;
}

HRESULT CloseScannerDevice(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;

	Trace(TEXT("------ CloseScannerDevice() ------"));

	if (!pScanInfo || !pContext || !pContext->oSession || !pContext->oDevice) {
		Trace(TEXT("Invalid or missing arguments"));
		return E_INVALIDARG;
	}

	Trace(TEXT("Device reference count = %d"), pContext->uiDevRef);

	if (pContext->uiDevRef > 0) {
		Trace(TEXT("Decrement device reference count"));
		pContext->uiDevRef--;
	}

	Trace(TEXT("Device reference count = %d"), pContext->uiDevRef);

	if (pContext->uiDevRef > 0) {
		Trace(TEXT("Device reference count is still greater than 0"));
		return S_FALSE;
	}

	if (pContext->pTask) {
		Trace(TEXT("Current task exists"));
		if (pContext->pTask->oScan) {
			Trace(TEXT("Current scan exists"));
			if (pContext->oDevice->IsOpen()) {
				Trace(TEXT("Device is open, cancelling current scan"));
				pContext->oDevice->Cancel();
			} else {
				Trace(TEXT("Device is already closed"));
			}

			Trace(TEXT("Deleting current scan"));
			delete pContext->pTask->oScan;
			pContext->pTask->oScan = NULL;
		}

		Trace(TEXT("Deleting current task"));
		ZeroMemory(pContext->pTask, sizeof(WIASANE_Task));
		HeapSafeFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pTask);
		pContext->pTask = NULL;
	} else {
		Trace(TEXT("No current task"));
	}

	if (pContext->oDevice->IsOpen()) {
		Trace(TEXT("Device is open, closing it"));
		status = pContext->oDevice->Close();
		if (status != SANE_STATUS_GOOD) {
			Trace(TEXT("Closing failed"));
			return GetErrorCode(status);
		}
	} else {
		Trace(TEXT("Device is already closed"));
	}

	return S_OK;
}
