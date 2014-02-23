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
#include <shlwapi.h>

#include "wiasane_opt.h"
#include "wiasane_util.h"


WIAMICRO_API HRESULT MicroEntry(LONG lCommand, _Inout_ PVAL pValue)
{
	PWIASANE_Context pContext;
	PWINSANE_Option oOption;
	PWINSANE_Params oParams;
	LONG lReceived;
	HANDLE hHeap;
	HRESULT hr;

#ifdef _DEBUG
	Trace(TEXT("Command Value (%d)"), lCommand);
#endif

	if (!pValue || !pValue->pScanInfo)
		return E_INVALIDARG;

	if (!pValue->pScanInfo->DeviceIOHandles[1])
		pValue->pScanInfo->DeviceIOHandles[1] = GetProcessHeap();

	hHeap = pValue->pScanInfo->DeviceIOHandles[1];
	if (!hHeap)
		return E_OUTOFMEMORY;

	if (!pValue->pScanInfo->pMicroDriverContext)
		pValue->pScanInfo->pMicroDriverContext = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WIASANE_Context));

	hr = E_NOTIMPL;
	pContext = (PWIASANE_Context) pValue->pScanInfo->pMicroDriverContext;

    switch (lCommand) {
		case CMD_SETSTIDEVICEHKEY:
			Trace(TEXT("CMD_SETSTIDEVICEHKEY"));

			if (pValue->pHandle)
				pValue->pScanInfo->DeviceIOHandles[2] = *pValue->pHandle;

			if (pContext)
				hr = ReadRegistryInformation(pValue->pScanInfo, pContext);
			else
				hr = E_FAIL;

			break;

		case CMD_INITIALIZE:
			Trace(TEXT("CMD_INITIALIZE"));

			if (pContext) {
				if (pContext->oSession)
					hr = UninitializeScanner(pValue->pScanInfo, pContext);
				else
					hr = S_OK;

				if (SUCCEEDED(hr))
					hr = InitializeScanner(pValue->pScanInfo, pContext);
			} else
				hr = E_FAIL;

			if (SUCCEEDED(hr))
				hr = InitScannerDefaults(pValue->pScanInfo, pContext);

			pValue->pScanInfo->DeviceIOHandles[2] = NULL;

			break;

		case CMD_UNINITIALIZE:
			Trace(TEXT("CMD_UNINITIALIZE"));

			if (pContext) {
				if (pContext->oSession)
					hr = UninitializeScanner(pValue->pScanInfo, pContext);
				else
					hr = S_OK;

				if (SUCCEEDED(hr))
					hr = FreeScanner(pValue->pScanInfo, pContext);

				if (SUCCEEDED(hr))
					pContext = NULL;
			} else
				hr = S_OK;

			pValue->pScanInfo->pMicroDriverContext = pContext;

			break;

		case CMD_RESETSCANNER:
			Trace(TEXT("CMD_RESETSCANNER"));
		case CMD_STI_DEVICERESET:
			Trace(TEXT("CMD_STI_DEVICERESET"));
		case CMD_STI_DIAGNOSTIC:
			Trace(TEXT("CMD_STI_DIAGNOSTIC"));

			if (pContext && pContext->oSession && pContext->oDevice && pContext->oSession->IsConnected()) {
				hr = GetErrorCode(pContext->oDevice->Cancel());
			} else {
				hr = WIA_ERROR_OFFLINE;
			}

			break;

		case CMD_STI_GETSTATUS:
			Trace(TEXT("CMD_STI_GETSTATUS"));

			pValue->lVal = MCRO_ERROR_OFFLINE;
			pValue->pGuid = (GUID*) &GUID_NULL;

			if (pContext && pContext->oSession && pContext->oDevice && pContext->oSession->IsConnected()) {
				if (pContext->oDevice->GetParams(&oParams) == SANE_STATUS_GOOD) {
					delete oParams;
					pValue->lVal = MCRO_STATUS_OK;
				}
			}

			hr = S_OK;
			break;

		case CMD_SETXRESOLUTION:
			Trace(TEXT("CMD_SETXRESOLUTION"));
		case CMD_SETYRESOLUTION:
			Trace(TEXT("CMD_SETYRESOLUTION"));

			if (pContext && pContext->oSession && pContext->oDevice) {
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

			switch (lCommand) {
				case CMD_SETXRESOLUTION:
					pValue->pScanInfo->Xresolution = pValue->lVal;
					break;

				case CMD_SETYRESOLUTION:
					pValue->pScanInfo->Yresolution = pValue->lVal;
					break;
			}

			hr = S_OK;
			break;

		case CMD_SETCONTRAST:
			Trace(TEXT("CMD_SETCONTRAST"));

			if (pContext && pContext->oSession && pContext->oDevice) {
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

		case CMD_SETINTENSITY:
			Trace(TEXT("CMD_SETINTENSITY"));

			if (pContext && pContext->oSession && pContext->oDevice) {
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

		case CMD_SETDATATYPE:
			Trace(TEXT("CMD_SETDATATYPE"));

			pValue->pScanInfo->DataType = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETNEGATIVE:
			Trace(TEXT("CMD_SETNEGATIVE"));

			pValue->pScanInfo->Negative = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_GETADFSTATUS:
			Trace(TEXT("CMD_GETADFSTATUS"));

			if (pContext && pContext->oSession && pContext->oDevice) {
				pValue->lVal = MCRO_STATUS_OK;
			} else {
				pValue->lVal = MCRO_ERROR_OFFLINE;
			}
			hr = S_OK;
			break;

		case CMD_GETADFHASPAPER:
			Trace(TEXT("CMD_GETADFHASPAPER"));

			if (pContext && pContext->oSession && pContext->oDevice) {
				if (!pContext->pTask) {
					MicroEntry(CMD_LOAD_ADF, pValue);
				}
				if (pContext->pTask && pContext->pTask->bUsingADF) {
					pValue->lVal = MCRO_STATUS_OK;
				} else {
					pValue->lVal = MCRO_ERROR_PAPER_EMPTY;
				}
			} else {
				pValue->lVal = MCRO_ERROR_OFFLINE;
			}
			hr = S_OK;
			break;

		case CMD_LOAD_ADF:
			Trace(TEXT("CMD_LOAD_ADF"));

			if (pContext && pContext->oSession && pContext->oDevice && pContext->pValues) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
				if (oOption) {
					switch (pValue->pScanInfo->ADF) {
						case 1:
							hr = oOption->SetValueString(pContext->pValues->pszcSourceADF);
							break;

						case 2:
							hr = oOption->SetValueString(pContext->pValues->pszcSourceDuplex);
							break;
					}
					if (hr == S_OK) {
						hr = Scan(pValue->pScanInfo, SCAN_FIRST, NULL, 0, &lReceived);
						if (pContext->pTask) {
							if (hr == S_OK) {
								pContext->pTask->bUsingADF = TRUE;
							} else {
								Scan(pValue->pScanInfo, SCAN_FINISHED, NULL, 0, &lReceived);
							}
						}
						if (!pContext->pTask || !pContext->pTask->bUsingADF) {
							oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
							if (oOption) {
								oOption->SetValueString(pContext->pValues->pszcSourceFlatbed);
							}
						}
					}
				}
			}
			break;

		case CMD_UNLOAD_ADF:
			Trace(TEXT("CMD_UNLOAD_ADF"));

			if (pContext && pContext->oSession && pContext->oDevice && pContext->pValues) {
				oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
				if (oOption) {
					hr = oOption->SetValueString(pContext->pValues->pszcSourceFlatbed);
					if (hr == S_OK) {
						if (pContext->pTask) {
							pContext->pTask->bUsingADF = FALSE;
						}
						hr = Scan(pValue->pScanInfo, SCAN_FINISHED, NULL, 0, &lReceived);
					}
				}
			}
			break;

		case CMD_GETCAPABILITIES:
			Trace(TEXT("CMD_GETCAPABILITIES"));

			pValue->lVal = 0;
			pValue->pGuid = NULL;
			pValue->ppButtonNames = NULL;

			hr = S_OK;
			break;

		case CMD_SETSCANMODE:
			Trace(TEXT("CMD_SETSCANMODE"));
			hr = SetScanMode(pValue->pScanInfo, pValue->lVal);
			break;

		default:
			Trace(TEXT("Unknown Command (%d)"), lCommand);
			break;
	}

	return hr;
}

WIAMICRO_API HRESULT Scan(_Inout_ PSCANINFO pScanInfo, LONG lPhase, _Out_writes_bytes_(lLength) PBYTE pBuffer, LONG lLength, _Out_ LONG *plReceived)
{
	PWIASANE_Context pContext;
	LONG idx, aquired;
	DWORD aquire;
	HANDLE hHeap;
	HRESULT hr;

	if (plReceived)
		*plReceived = 0;

    Trace(TEXT("------ Scan Requesting %d ------"), lLength);

	if (pScanInfo == NULL)
		return E_INVALIDARG;

	hHeap = pScanInfo->DeviceIOHandles[1];
	if (!hHeap)
		return E_OUTOFMEMORY;

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext)
		return E_OUTOFMEMORY;

    switch (lPhase) {
		case SCAN_FIRST:
			Trace(TEXT("SCAN_FIRST"));

			//
			// first phase
			//

			if (pContext->oSession && pContext->oDevice && (!pContext->pTask || !pContext->pTask->oScan)) {
				if (!pContext->oSession->IsConnected())
					return WIA_ERROR_OFFLINE;

				hr = SetScannerSettings(pScanInfo, pContext);
				if (FAILED(hr))
					return hr;

				if (!pContext->pTask)
					pContext->pTask = (PWIASANE_Task) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(WIASANE_Task));

				if (!pContext->pTask)
					return E_OUTOFMEMORY;

				if (pContext->oDevice->Start(&pContext->pTask->oScan) != SANE_STATUS_GOOD)
					return E_FAIL;

				if (!pContext->pTask->oScan)
					return E_OUTOFMEMORY;

				if (pContext->pTask->oScan->Connect() != CONTINUE)
					return E_FAIL;

				hr = FetchScannerParams(pScanInfo, pContext);
				if (FAILED(hr))
					return hr;

				if (pScanInfo->PixelBits > 1)
					pContext->pTask->lByteGapX = (pScanInfo->Window.xExtent * pScanInfo->PixelBits / 8) - pScanInfo->WidthBytes;
				else
					pContext->pTask->lByteGapX = ((LONG) floor(pScanInfo->Window.xExtent + 7.0) / 8) - pScanInfo->WidthBytes;
				pContext->pTask->lByteGapY = (pScanInfo->Window.yExtent - pScanInfo->Lines) * pScanInfo->WidthBytes;

				pContext->pTask->uiTotal = pScanInfo->WidthBytes * pScanInfo->Lines;
				pContext->pTask->uiReceived = 0;
				Trace(TEXT("Data: %d/%d"), pContext->pTask->uiReceived, pContext->pTask->uiTotal);
			}

		case SCAN_NEXT: // SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)
			if (lPhase == SCAN_NEXT)
				Trace(TEXT("SCAN_NEXT"));

			if (!pBuffer)
				break;

			//
			// next phase, get data from the scanner and set plReceived value
			//

			if (pContext->oSession && pContext->oDevice && pContext->pTask && pContext->pTask->oScan) {
				memset(pBuffer, 0, lLength);

				aquire = pContext->pTask->lByteGapX ? min(lLength, pScanInfo->WidthBytes) : lLength;
				aquired = 0;

				while (pContext->pTask->oScan->AquireImage((pBuffer + *plReceived + aquired), &aquire) == CONTINUE) {
					if (aquire > 0) {
						if (pContext->pTask->lByteGapX) {
							aquired += aquire;
							if (aquired == pScanInfo->WidthBytes) {
								*plReceived += aquired;
								if (lLength - *plReceived >= pContext->pTask->lByteGapX)
									*plReceived += pContext->pTask->lByteGapX;
								aquired = 0;
							}
							if (lLength - *plReceived < pScanInfo->WidthBytes - aquired)
								break;
							aquire = pScanInfo->WidthBytes - aquired;
						} else {
							*plReceived += aquire;
							aquire = lLength - *plReceived;
						}
					}
					if (aquire <= 0)
						break;
				}

				if (pContext->pTask->lByteGapY > 0 && *plReceived < lLength) {
					aquired = min(lLength - *plReceived, pContext->pTask->lByteGapY);
					if (aquired > 0) {
						memset(pBuffer + *plReceived, -1, aquired);
						*plReceived += aquired;
					}
				}

				if (pScanInfo->DataType == WIA_DATA_THRESHOLD) {
					for (idx = 0; idx < lLength; idx++) {
						pBuffer[idx] = ~pBuffer[idx];
					}
				}

				pContext->pTask->uiTotal = pScanInfo->WidthBytes * pScanInfo->Lines;
				pContext->pTask->uiReceived += *plReceived;
				Trace(TEXT("Data: %d/%d -> %d/%d"), pContext->pTask->uiReceived, pContext->pTask->uiTotal, pContext->pTask->uiTotal - pContext->pTask->uiReceived, lLength);
			}

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

			if (pContext->oSession && pContext->oDevice) {
				if (pContext->pTask) {
					if (pContext->pTask->oScan) {
						delete pContext->pTask->oScan;
						pContext->pTask->oScan = NULL;
					}

					ZeroMemory(pContext->pTask, sizeof(WIASANE_Task));
					HeapFree(hHeap, 0, pContext->pTask);
					pContext->pTask = NULL;
				}

				pContext->oDevice->Cancel();
			}

			break;
    }

    return S_OK;
}

WIAMICRO_API HRESULT SetPixelWindow(_Inout_ PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{
	PWIASANE_Context pContext;
	PWINSANE_Option oOptionTLx, oOptionTLy, oOptionBRx, oOptionBRy;
	HRESULT hr;
	double tl_x, tl_y, br_x, br_y;

    if (pScanInfo == NULL)
        return E_INVALIDARG;

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext)
		return E_OUTOFMEMORY;

	if (!pContext->oSession || !pContext->oDevice)
		return E_FAIL;

	if (!pContext->oSession->IsConnected())
		return WIA_ERROR_OFFLINE;

	oOptionTLx = pContext->oDevice->GetOption(WIASANE_OPTION_TL_X);
	oOptionTLy = pContext->oDevice->GetOption(WIASANE_OPTION_TL_Y);
	oOptionBRx = pContext->oDevice->GetOption(WIASANE_OPTION_BR_X);
	oOptionBRy = pContext->oDevice->GetOption(WIASANE_OPTION_BR_Y);

	if (!oOptionTLx || !oOptionTLy || !oOptionBRx || !oOptionBRy)
		return E_INVALIDARG;

	tl_x = ((double) x) / ((double) pScanInfo->Xresolution);
	tl_y = ((double) y) / ((double) pScanInfo->Yresolution);
	br_x = ((double) (x + xExtent)) / ((double) pScanInfo->Xresolution);
	br_y = ((double) (y + yExtent)) / ((double) pScanInfo->Yresolution);

	hr = IsValidOptionValueInch(oOptionTLx, tl_x);
	if (FAILED(hr))
		return hr;

	hr = IsValidOptionValueInch(oOptionTLy, tl_y);
	if (FAILED(hr))
		return hr;

	hr = IsValidOptionValueInch(oOptionBRx, br_x);
	if (FAILED(hr))
		return hr;

	hr = IsValidOptionValueInch(oOptionBRy, br_y);
	if (FAILED(hr))
		return hr;

	hr = SetOptionValueInch(oOptionTLx, tl_x);
	if (FAILED(hr))
		return hr;

	hr = SetOptionValueInch(oOptionTLy, tl_y);
	if (FAILED(hr))
		return hr;

	hr = SetOptionValueInch(oOptionBRx, br_x);
	if (FAILED(hr))
		return hr;

	hr = SetOptionValueInch(oOptionBRy, br_y);
	if (FAILED(hr))
		return hr;

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
#endif

	if (!pContext->oSession->IsConnected())
		return WIA_ERROR_OFFLINE;

	return S_OK;
}


HRESULT ReadRegistryInformation(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	HKEY hKey, hOpenKey;
	DWORD dwWritten, dwType, dwPort;
	PTSTR pszHost, pszName;
	LSTATUS st;
	HRESULT hr;

	hr = S_OK;
	hKey = (HKEY) pScanInfo->DeviceIOHandles[2];
	hOpenKey = NULL;

	pContext->usPort = WINSANE_DEFAULT_PORT;
	if (pContext->pszHost) {
		HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pszHost);
		pContext->pszHost = NULL;
	}
	if (pContext->pszName) {
		HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pszName);
		pContext->pszName = NULL;
	}

	//
	// Open DeviceData section to read driver specific information
	//

	st = RegOpenKeyEx(hKey, TEXT("DeviceData"), 0, KEY_QUERY_VALUE|KEY_READ, &hOpenKey);
	if (st == ERROR_SUCCESS) {
		dwWritten = sizeof(DWORD);
		dwType = REG_DWORD;
		dwPort = WINSANE_DEFAULT_PORT;

		st = RegQueryValueEx(hOpenKey, TEXT("Port"), NULL, &dwType, (LPBYTE)&dwPort, &dwWritten);
		if (st == ERROR_SUCCESS) {
			pContext->usPort = (USHORT) dwPort;
		} else
			hr = E_FAIL;

		dwWritten = 0;
		dwType = REG_SZ;
		pszHost = NULL;

		st = RegQueryValueEx(hOpenKey, TEXT("Host"), NULL, &dwType, (LPBYTE)pszHost, &dwWritten);
		if (st == ERROR_SUCCESS && dwWritten > 0) {
			pszHost = (PTSTR) HeapAlloc(pScanInfo->DeviceIOHandles[1], HEAP_ZERO_MEMORY, dwWritten);
			if (pszHost) {
				st = RegQueryValueEx(hOpenKey, TEXT("Host"), NULL, &dwType, (LPBYTE)pszHost, &dwWritten);
				if (st == ERROR_SUCCESS) {
					pContext->pszHost = pszHost;
				} else {
					HeapFree(pScanInfo->DeviceIOHandles[1], 0, pszHost);
					hr = E_FAIL;
				}
			} else
				hr = E_OUTOFMEMORY;
		} else
			hr = E_FAIL;

		dwWritten = 0;
		dwType = REG_SZ;
		pszName = NULL;

		st = RegQueryValueEx(hOpenKey, TEXT("Name"), NULL, &dwType, (LPBYTE)pszName, &dwWritten);
		if (st == ERROR_SUCCESS && dwWritten > 0) {
			pszName = (PTSTR) HeapAlloc(pScanInfo->DeviceIOHandles[1], HEAP_ZERO_MEMORY, dwWritten);
			if (pszName) {
				st = RegQueryValueEx(hOpenKey, TEXT("Name"), NULL, &dwType, (LPBYTE)pszName, &dwWritten);
				if (st == ERROR_SUCCESS) {
					pContext->pszName = pszName;
				} else {
					HeapFree(pScanInfo->DeviceIOHandles[1], 0, pszName);
					hr = E_FAIL;
				}
			} else
				hr = E_OUTOFMEMORY;
		} else
			hr = E_FAIL;
	} else
		hr = E_ACCESSDENIED;

	return hr;
}

HRESULT InitializeScanner(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	UNREFERENCED_PARAMETER(pScanInfo);

	pContext->pTask = NULL;
	pContext->oSession = WINSANE_Session::Remote(pContext->pszHost, pContext->usPort);
	if (pContext->oSession) {
		status = pContext->oSession->Init(NULL, NULL);
		if (status == SANE_STATUS_GOOD) {
			status = pContext->oSession->FetchDevices();
			if (status == SANE_STATUS_GOOD) {
				pContext->oDevice = pContext->oSession->GetDevice(pContext->pszName);
				if (pContext->oDevice) {
					status = pContext->oDevice->Open();
					if (status == SANE_STATUS_GOOD) {
						status = pContext->oDevice->FetchOptions();
						return GetErrorCode(status);
					} else {
						hr = GetErrorCode(status);
					}
				} else {
					hr = WIA_ERROR_USER_INTERVENTION;
				}
			} else {
				hr = GetErrorCode(status);
			}

			pContext->oSession->Exit();
		} else {
			hr = GetErrorCode(status);
		}

		delete pContext->oSession;
		pContext->oSession = NULL;
	} else {
		hr = WIA_ERROR_OFFLINE;
	}

	pContext->oDevice = NULL;
	return hr;
}

HRESULT UninitializeScanner(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	SANE_Status status;
	HRESULT hr;

	hr = S_OK;

	if (pContext->oSession) {
		if (pContext->oDevice) {
			if (pContext->pTask) {
				if (pContext->pTask->oScan) {
					status = pContext->oDevice->Cancel();
					if (status != SANE_STATUS_GOOD)
						hr = GetErrorCode(status);

					delete pContext->pTask->oScan;
					pContext->pTask->oScan = NULL;
				}

				ZeroMemory(pContext->pTask, sizeof(WIASANE_Task));
				HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pTask);
				pContext->pTask = NULL;
			}

			if (pContext->pValues) {
				ZeroMemory(pContext->pValues, sizeof(WIASANE_Values));
				HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pValues);
				pContext->pValues = NULL;
			}

			status = pContext->oDevice->Close();
			if (status != SANE_STATUS_GOOD)
				hr = GetErrorCode(status);

			pContext->oDevice = NULL;
		}

		status = pContext->oSession->Exit();
		if (status != SANE_STATUS_GOOD)
			hr = GetErrorCode(status);

		delete pContext->oSession;
		pContext->oSession = NULL;
	}

	return hr;
}

HRESULT FreeScanner(_Inout_ PSCANINFO pScanInfo, _In_ PWIASANE_Context pContext)
{
	if (pContext->pszHost)
		HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pszHost);

	if (pContext->pszName)
		HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext->pszName);

	ZeroMemory(pContext, sizeof(WIASANE_Context));
	HeapFree(pScanInfo->DeviceIOHandles[1], 0, pContext);

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

	if (pContext && pContext->oSession && pContext->oDevice) {
		pContext->pValues = (PWIASANE_Values) HeapAlloc(pScanInfo->DeviceIOHandles[1], HEAP_ZERO_MEMORY, sizeof(WIASANE_Values));
		if (!pContext->pValues)
			return E_OUTOFMEMORY;

		pScanInfo->ADF                = 0;
		pScanInfo->bNeedDataAlignment = TRUE;

		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_SOURCE);
		if (oOption && oOption->GetType() == SANE_TYPE_STRING && oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
			string_list = oOption->GetConstraintStringList();
			for (index = 0; string_list[index] != NULL; index++) {
				if (StrStrIA(string_list[index], WIASANE_SOURCE_ADF) ||
					StrStrIA(string_list[index], WIASANE_SOURCE_ADF_EX)) {
					pScanInfo->ADF = max(pScanInfo->ADF, 1);
					pContext->pValues->pszcSourceADF = string_list[index];
				} else if (StrStrIA(string_list[index], WIASANE_SOURCE_DUPLEX)) {
					pScanInfo->ADF = max(pScanInfo->ADF, 2);
					pContext->pValues->pszcSourceDuplex = string_list[index];
				} else if (StrStrIA(string_list[index], WIASANE_SOURCE_FLATBED) ||
					       !pContext->pValues->pszcSourceFlatbed) {
					pContext->pValues->pszcSourceFlatbed = string_list[index];
				}
			}
		}

		pScanInfo->SupportedCompressionType = 0;
		pScanInfo->SupportedDataTypes       = 0;

		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_MODE);
		if (oOption && oOption->GetType() == SANE_TYPE_STRING && oOption->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
			string_list = oOption->GetConstraintStringList();
			for (index = 0; string_list[index] != NULL; index++) {
				if (StrCmpIA(string_list[index], WIASANE_MODE_LINEART) == 0 ||
					StrCmpIA(string_list[index], WIASANE_MODE_THRESHOLD) == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_BW;
					pContext->pValues->pszcModeThreshold = string_list[index];
				} else if (StrCmpIA(string_list[index], WIASANE_MODE_GRAY) == 0 ||
					       StrCmpIA(string_list[index], WIASANE_MODE_GRAYSCALE) == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_GRAYSCALE;
					pContext->pValues->pszcModeGrayscale = string_list[index];
				} else if (StrCmpIA(string_list[index], WIASANE_MODE_COLOR) == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_COLOR;
					pContext->pValues->pszcModeColor = string_list[index];
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
		if (oOption) {
			if (oOption->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
				range = oOption->GetConstraintRange();
				pScanInfo->Contrast            = (range->min + range->max) / 2;
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
				pScanInfo->IntensityRange.lMin  = (range->min + range->max) / 2;
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
	}

#ifdef _DEBUG
    Trace(TEXT("bw    = %d"), pScanInfo->BedWidth);
    Trace(TEXT("bh    = %d"), pScanInfo->BedHeight);
#endif

	return S_OK;
}

HRESULT SetScannerSettings(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Option oOption;
	HRESULT hr;

	if (pContext && pContext->oSession && pContext->oDevice && pContext->pValues) {
		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_MODE);
		if (oOption && oOption->GetType() == SANE_TYPE_STRING) {
			switch (pScanInfo->DataType) {
				case WIA_DATA_THRESHOLD:
					hr = oOption->SetValueString(pContext->pValues->pszcModeThreshold);
					break;
				case WIA_DATA_GRAYSCALE:
					hr = oOption->SetValueString(pContext->pValues->pszcModeGrayscale);
					break;
				case WIA_DATA_COLOR:
					hr = oOption->SetValueString(pContext->pValues->pszcModeColor);
					break;
				default:
					hr = E_INVALIDARG;
					break;
			}
			if (FAILED(hr))
				return hr;
		} else
			return E_NOTIMPL;

		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_RESOLUTION);
		if (oOption) {
			hr = oOption->SetValue(pScanInfo->Xresolution);
			if (FAILED(hr))
				return hr;
		} else
			return E_NOTIMPL;

		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_CONTRAST);
		if (oOption) {
			hr = oOption->SetValue(pScanInfo->Contrast);
			if (FAILED(hr) && hr != E_NOTIMPL)
				return hr;
		}

		oOption = pContext->oDevice->GetOption(WIASANE_OPTION_BRIGHTNESS);
		if (oOption) {
			hr = oOption->SetValue(pScanInfo->Intensity);
			if (FAILED(hr) && hr != E_NOTIMPL)
				return hr;
		}
	}

    return S_OK;
}

HRESULT FetchScannerParams(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext)
{
	PWINSANE_Params oParams;
	SANE_Frame frame;
	SANE_Int depth;
	HRESULT hr;

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

HRESULT SetScanMode(_Inout_ PSCANINFO pScanInfo, _In_ LONG lScanMode)
{
	PWIASANE_Context pContext;
	PWINSANE_Option oOption;
	PSANE_String_Const string_list;
	HRESULT hr;

	pContext = (PWIASANE_Context) pScanInfo->pMicroDriverContext;
	if (!pContext)
		return E_OUTOFMEMORY;

	hr = E_NOTIMPL;

	switch (lScanMode) {
		case SCANMODE_FINALSCAN:
			Trace(TEXT("Final Scan"));

			if (pContext->oSession && pContext->oDevice) {
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
			}

			hr = S_OK;
			break;

		case SCANMODE_PREVIEWSCAN:
			Trace(TEXT("Preview Scan"));

			if (pContext->oSession && pContext->oDevice) {
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
			}

			hr = S_OK;
			break;

		default:
			Trace(TEXT("Unknown Scan Mode (%d)"), lScanMode);

			hr = E_FAIL;
			break;
	}

	return hr;
}

HRESULT GetErrorCode(_In_ SANE_Status status)
{
	switch (status) {
		case SANE_STATUS_GOOD:
			return S_OK;
		case SANE_STATUS_UNSUPPORTED:
			return WIA_ERROR_INVALID_COMMAND;
		case SANE_STATUS_CANCELLED:
			return E_ABORT;
		case SANE_STATUS_DEVICE_BUSY:
			return WIA_ERROR_BUSY;
		case SANE_STATUS_INVAL:
			return WIA_ERROR_INCORRECT_HARDWARE_SETTING;
		case SANE_STATUS_EOF:
			return S_FALSE;
		case SANE_STATUS_JAMMED:
			return WIA_ERROR_PAPER_JAM;
		case SANE_STATUS_NO_DOCS:
			return WIA_ERROR_PAPER_EMPTY;
		case SANE_STATUS_COVER_OPEN:
			return WIA_ERROR_COVER_OPEN;
		case SANE_STATUS_IO_ERROR:
			return WIA_ERROR_DEVICE_COMMUNICATION;
		case SANE_STATUS_NO_MEM:
			return E_OUTOFMEMORY;
		case SANE_STATUS_ACCESS_DENIED:
			return E_ACCESSDENIED;
		default:
			return WIA_ERROR_GENERAL_ERROR;
	}
}
