#include "stdafx.h"
#include "wiasane.h"

#include <STI.H>
#include <math.h>
#include <winioctl.h>
#include <usbscan.h>

#include "wiasane_opt.h"
#include "wiasane_util.h"


WIAMICRO_API HRESULT MicroEntry(LONG lCommand, _Inout_ PVAL pValue)
{
	WIASANE_Context *context;
	WINSANE_Option *option;
	WINSANE_Params *params;
	HRESULT hr;

#ifdef _DEBUG
	if (lCommand != CMD_STI_GETSTATUS)
		Trace(TEXT("Command Value (%d)"),lCommand);
#endif

	if (!pValue || !pValue->pScanInfo)
		return E_INVALIDARG;

	hr = E_NOTIMPL;
	context = (WIASANE_Context*) pValue->pScanInfo->pMicroDriverContext;

    switch (lCommand) {
		case CMD_SETSTIDEVICEHKEY:
			if (!context) {
				context = new WIASANE_Context;
				if (context)
					memset(context, 0, sizeof(WIASANE_Context));
			}
			if (!context) {
				pValue->pScanInfo->pMicroDriverContext = NULL;
				hr = E_OUTOFMEMORY;
				break;
			}

			pValue->pScanInfo->pMicroDriverContext = context;

			hr = ReadRegistryInformation(context, pValue->pHandle);
			break;

		case CMD_INITIALIZE:
			if (context) {
				if (context->session)
					hr = UninitializeScanner(context);
				else
					hr = S_OK;

				if (hr == S_OK)
					hr = InitializeScanner(context);
			} else
				hr = E_FAIL;

			if (hr == S_OK)
				hr = InitScannerDefaults(pValue->pScanInfo, context);

			break;

		case CMD_UNINITIALIZE:
			if (context) {
				if (context->session)
					hr = UninitializeScanner(context);
				else
					hr = S_OK;

				if (hr == S_OK)
					hr = FreeScanner(context);

				if (hr == S_OK)
					context = NULL;
			} else
				hr = S_OK;

			pValue->pScanInfo->pMicroDriverContext = context;

			break;

		case CMD_RESETSCANNER:
		case CMD_STI_DEVICERESET:
			if (context && context->session && context->device) {
				if (!context->device->Cancel()) {
					hr = E_FAIL;
					break;
				}
			}

			hr = S_OK;
			break;

		case CMD_STI_DIAGNOSTIC:
			if (context && context->session && context->device) {
				if (!context->device->FetchOptions()) {
					hr = E_FAIL;
					break;
				}
			}

			hr = S_OK;
			break;

		case CMD_STI_GETSTATUS:
			pValue->lVal = MCRO_ERROR_OFFLINE;
			pValue->pGuid = (GUID*) &GUID_NULL;

			if (context && context->session && context->device) {
				params = context->device->GetParams();
				if (params) {
					pValue->lVal = MCRO_STATUS_OK;
				}
			}

			hr = S_OK;
			break;

		case CMD_SETXRESOLUTION:
		case CMD_SETYRESOLUTION:
			if (context && context->session && context->device) {
				option = context->device->GetOption("resolution");
				if (!option) {
					hr = E_NOTIMPL;
					break;
				}
				if (!option->IsValidValue(pValue->lVal)) {
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
			if (context && context->session && context->device) {
				option = context->device->GetOption("contrast");
				if (!option) {
					hr = E_NOTIMPL;
					break;
				}
				if (!option->IsValidValue(pValue->lVal)) {
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
			if (context && context->session && context->device) {
				option = context->device->GetOption("brightness");
				if (!option) {
					hr = E_NOTIMPL;
					break;
				}
				if (!option->IsValidValue(pValue->lVal)) {
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
			pValue->pScanInfo->DataType = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_SETNEGATIVE:
			pValue->pScanInfo->Negative = pValue->lVal;
			hr = S_OK;
			break;

		case CMD_GETADFSTATUS:
		case CMD_GETADFHASPAPER:
			// pValue->lVal = MCRO_ERROR_PAPER_EMPTY;
			// hr = S_OK;
			break;

		case CMD_GET_INTERRUPT_EVENT:
			break;

		case CMD_GETCAPABILITIES:
			pValue->lVal = 0;
			pValue->pGuid = NULL;
			pValue->ppButtonNames = NULL;

			hr = S_OK;
			break;

		case CMD_SETSCANMODE:
			hr = SetScanMode(pValue->pScanInfo, pValue->lVal);
			break;

		default:
			Trace(TEXT("Unknown Command (%d)"),lCommand);
			break;
    }

    return hr;
}

WIAMICRO_API HRESULT Scan(_Inout_ PSCANINFO pScanInfo, LONG lPhase, _Out_writes_bytes_(lLength) PBYTE pBuffer, LONG lLength, _Out_ LONG *plReceived)
{
	WIASANE_Context *context;
	LONG aquire, aquired;
	HRESULT hr;
	LONG idx;

	if (plReceived)
		*plReceived = 0;

    Trace(TEXT("------ Scan Requesting %d ------"), lLength);

	if (pScanInfo == NULL)
		return E_INVALIDARG;

	context = (WIASANE_Context*) pScanInfo->pMicroDriverContext;

    switch (lPhase) {
		case SCAN_FIRST:
			Trace(TEXT("SCAN_FIRST"));

			//
			// first phase
			//

			if (context && context->session && context->device) {
				hr = SetScannerSettings(pScanInfo, context);
				if (hr != S_OK)
					return hr;

				context->task = new WIASANE_Task;
				if (context->task)
					memset(context->task, 0, sizeof(WIASANE_Task));
				else
					return E_OUTOFMEMORY;

				context->task->scan = context->device->Start();
				if (context->task->scan->Connect() != CONTINUE)
					return E_FAIL;

				hr = FetchScannerParams(pScanInfo, context);
				if (hr != S_OK)
					return hr;

				if (pScanInfo->PixelBits > 1)
					context->task->xbytegap = (pScanInfo->Window.xExtent * pScanInfo->PixelBits / 8) - pScanInfo->WidthBytes;
				else
					context->task->xbytegap = ((LONG) floor(pScanInfo->Window.xExtent + 7.0) / 8) - pScanInfo->WidthBytes;
				context->task->ybytegap = (pScanInfo->Window.yExtent - pScanInfo->Lines) * pScanInfo->WidthBytes;

				context->task->total = pScanInfo->WidthBytes * pScanInfo->Lines;
				context->task->received = 0;
				Trace(TEXT("Data: %d/%d"), context->task->received, context->task->total);
			}

		case SCAN_NEXT: // SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)
			if (lPhase == SCAN_NEXT)
				Trace(TEXT("SCAN_NEXT"));

			//
			// next phase, get data from the scanner and set plReceived value
			//

			if (context && context->session && context->device && context->task && context->task->scan) {
				memset(pBuffer, 0, lLength);

				aquire = context->task->xbytegap ? min(lLength, pScanInfo->WidthBytes) : lLength;
				aquired = 0;

				while (context->task->scan->AquireImage((char*) (pBuffer + *plReceived + aquired), &aquire) == CONTINUE) {
					if (aquire > 0) {
						if (context->task->xbytegap) {
							aquired += aquire;
							if (aquired == pScanInfo->WidthBytes) {
								*plReceived += aquired;
								if (lLength - *plReceived >= context->task->xbytegap)
									*plReceived += context->task->xbytegap;
								aquired = 0;
							}
							aquire = pScanInfo->WidthBytes - aquired;
							if (lLength - *plReceived < aquire)
								break;
						} else {
							*plReceived += aquire;
							aquire = lLength - *plReceived;
						}
					}
					if (aquire <= 0)
						break;
				}

				if (context->task->ybytegap > 0 && *plReceived < lLength) {
					aquired = min(lLength - *plReceived, context->task->ybytegap);
					memset(pBuffer + *plReceived, -1, aquired);
					*plReceived += aquired;
				}

				if (pScanInfo->DataType == WIA_DATA_THRESHOLD) {
					for (idx = 0; idx < *plReceived; idx++) {
						pBuffer[idx] = ~pBuffer[idx];
					}
				}

				context->task->total = pScanInfo->WidthBytes * pScanInfo->Lines;
				context->task->received += *plReceived;
				Trace(TEXT("Data: %d/%d -> %d/%d"), context->task->received, context->task->total, context->task->total - context->task->received, lLength);
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

			if (context && context->session && context->device) {
				if (context->task->scan) {
					delete context->task->scan;
					context->task->scan = NULL;
				}
				if (context->task) {
					delete context->task;
					context->task = NULL;
				}

				context->device->Cancel();
			}

			break;
    }

    return S_OK;
}

WIAMICRO_API HRESULT SetPixelWindow(_Inout_ PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{
	WIASANE_Context *context;
	WINSANE_Option *opt_tl_x, *opt_tl_y, *opt_br_x, *opt_br_y;
	HRESULT hr;
	double tl_x, tl_y, br_x, br_y;

    if (pScanInfo == NULL)
        return E_INVALIDARG;

	context = (WIASANE_Context*) pScanInfo->pMicroDriverContext;
	if (!context || !context->session || !context->device)
		return E_FAIL;

	opt_tl_x = context->device->GetOption("tl-x");
	opt_tl_y = context->device->GetOption("tl-y");
	opt_br_x = context->device->GetOption("br-x");
	opt_br_y = context->device->GetOption("br-y");

	if (!opt_tl_x || !opt_tl_y || !opt_br_x || !opt_br_y)
		return E_INVALIDARG;

	tl_x = ((double) x) / ((double) pScanInfo->Xresolution);
	tl_y = ((double) y) / ((double) pScanInfo->Yresolution);
	br_x = ((double) (x + xExtent)) / ((double) pScanInfo->Xresolution);
	br_y = ((double) (y + yExtent)) / ((double) pScanInfo->Yresolution);

	hr = IsValidOptionValueInch(opt_tl_x, tl_x);
	if (hr != S_OK)
		return hr;

	hr = IsValidOptionValueInch(opt_tl_y, tl_y);
	if (hr != S_OK)
		return hr;

	hr = IsValidOptionValueInch(opt_br_x, br_x);
	if (hr != S_OK)
		return hr;

	hr = IsValidOptionValueInch(opt_br_y, br_y);
	if (hr != S_OK)
		return hr;

	hr = SetOptionValueInch(opt_tl_x, tl_x);
	if (hr != S_OK)
		return hr;

	hr = SetOptionValueInch(opt_tl_y, tl_y);
	if (hr != S_OK)
		return hr;

	hr = SetOptionValueInch(opt_br_x, br_x);
	if (hr != S_OK)
		return hr;

	hr = SetOptionValueInch(opt_br_y, br_y);
	if (hr != S_OK)
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

    return S_OK;
}


HRESULT ReadRegistryInformation(WIASANE_Context *context, HANDLE *pHandle)
{
	HKEY hKey, hOpenKey;
	DWORD dwWritten, dwType, dwPort;
	PCHAR pcHost, pcName;
	LSTATUS st;
	HRESULT hr;

	if (!pHandle)
		return E_INVALIDARG;

	hr = S_OK;
	hKey = (HKEY) *pHandle;
	hOpenKey = NULL;

	if (context->host)
		free(context->host);

	if (context->name)
		free(context->name);

	context->port = WINSANE_DEFAULT_PORT;
	context->host = _strdup("localhost");
	context->name = NULL;

	//
	// Open DeviceData section to read driver specific information
	//

	st = RegOpenKeyExA(hKey, "DeviceData", 0, KEY_QUERY_VALUE|KEY_READ, &hOpenKey);
	if (st == ERROR_SUCCESS) {
		dwWritten = sizeof(DWORD);
		dwType = REG_DWORD;
		dwPort = WINSANE_DEFAULT_PORT;

		st = RegQueryValueExA(hOpenKey, "Port", NULL, &dwType, (LPBYTE)&dwPort, &dwWritten);
		if (st == ERROR_SUCCESS) {
			context->port = dwPort;
		} else
			hr = E_FAIL;

		dwWritten = 0;
		dwType = REG_SZ;
		pcHost = NULL;

		st = RegQueryValueExA(hOpenKey, "Host", NULL, &dwType, (LPBYTE)pcHost, &dwWritten);
		if (st == ERROR_SUCCESS && dwWritten > 0) {
			pcHost = (PCHAR) malloc(dwWritten);
			if (pcHost) {
				pcHost[dwWritten - 1] = '\0';
				st = RegQueryValueExA(hOpenKey, "Host", NULL, &dwType, (LPBYTE)pcHost, &dwWritten);
				if (st == ERROR_SUCCESS) {
					if (context->host)
						free(context->host);

					context->host = _strdup(pcHost);
				} else
					hr = E_FAIL;

				free(pcHost);
			} else
				hr = E_OUTOFMEMORY;
		} else
			hr = E_FAIL;

		dwWritten = 0;
		dwType = REG_SZ;
		pcName = NULL;

		st = RegQueryValueExA(hOpenKey, "Name", NULL, &dwType, (LPBYTE)pcName, &dwWritten);
		if (st == ERROR_SUCCESS && dwWritten > 0) {
			pcName = (PCHAR) malloc(dwWritten);
			if (pcName) {
				pcName[dwWritten - 1] = '\0';
				st = RegQueryValueExA(hOpenKey, "Name", NULL, &dwType, (LPBYTE)pcName, &dwWritten);
				if (st == ERROR_SUCCESS) {
					if (context->name)
						free(context->name);

					context->name = _strdup(pcName);
				} else
					hr = E_FAIL;

				free(pcName);
			} else
				hr = E_OUTOFMEMORY;
		} else
			hr = E_FAIL;
	} else
		hr = E_ACCESSDENIED;

	return hr;
}

HRESULT InitializeScanner(WIASANE_Context *context)
{
	context->task = NULL;
	context->session = WINSANE_Session::Remote(context->host, (unsigned short) context->port);
	if (context->session && context->session->Init(NULL, NULL) && context->session->GetDevices() > 0) {
		context->device = context->session->GetDevice(context->name);
		if (context->device && context->device->Open()) {
			context->device->FetchOptions();
		} else {
			context->device = NULL;
			return E_FAIL;
		}
	} else {
		context->device = NULL;
		return E_FAIL;
	}

    return S_OK;
}

HRESULT UninitializeScanner(WIASANE_Context *context)
{
	if (context->session) {
		if (context->device) {
			if (context->task) {
				if (context->task->scan) {
					context->device->Cancel();
					delete context->task->scan;
					context->task->scan = NULL;
				}
				delete context->task;
				context->task = NULL;
			}
			context->device->Close();
			context->device = NULL;
		}
		context->session->Exit();
		delete context->session;
		context->session = NULL;
	}

	return S_OK;
}

HRESULT FreeScanner(WIASANE_Context *context)
{
	if (context->host)
		free(context->host);

	if (context->name)
		free(context->name);

	memset(context, 0, sizeof(WIASANE_Context));
	delete context;

	return S_OK;
}

HRESULT InitScannerDefaults(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	WINSANE_Option *option;
	SANE_String_Const *string_list;
	SANE_Range *range;
	HRESULT hr;
	double dbl;
	int index;

	if (context && context->session && context->device) {
		pScanInfo->ADF                = 0; // set to no ADF in Test device
		pScanInfo->bNeedDataAlignment = TRUE;

		pScanInfo->SupportedCompressionType = 0;
		pScanInfo->SupportedDataTypes       = 0;

		option = context->device->GetOption("mode");
		if (option && option->GetType() == SANE_TYPE_STRING && option->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
			string_list = option->GetConstraintStringList();
			for (index = 0; string_list[index] != NULL; index++) {
				if (strcmp(string_list[index], "Lineart") == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_BW;
				} else if (strcmp(string_list[index], "Gray") == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_GRAYSCALE;
				} else if (strcmp(string_list[index], "Color") == 0) {
					pScanInfo->SupportedDataTypes |= SUPPORT_COLOR;
				}
			}
		}

		pScanInfo->BedWidth  = 8500;  // 1000's of an inch (WIA compatible unit)
		pScanInfo->BedHeight = 11000; // 1000's of an inch (WIA compatible unit)

		option = context->device->GetOption("br-x");
		if (option) {
			hr = GetOptionMaxValueInch(option, &dbl);
			if (hr == S_OK) {
				pScanInfo->BedWidth = (LONG) (dbl * 1000.0);
			}
		}
		option = context->device->GetOption("br-y");
		if (option) {
			hr = GetOptionMaxValueInch(option, &dbl);
			if (hr == S_OK) {
				pScanInfo->BedHeight = (LONG) (dbl * 1000.0);
			}
		}

		pScanInfo->OpticalXResolution = 300;
		pScanInfo->Xresolution = pScanInfo->OpticalXResolution;

		option = context->device->GetOption("resolution");
		if (option) {
			hr = GetOptionMaxValue(option, &dbl);
			if (hr == S_OK) {
				pScanInfo->OpticalXResolution = (LONG) dbl;
			}
			hr = GetOptionValue(option, &dbl);
			if (hr == S_OK) {
				pScanInfo->Xresolution = (LONG) dbl;
			}
		}

		pScanInfo->OpticalYResolution = pScanInfo->OpticalXResolution;
		pScanInfo->Yresolution = pScanInfo->Xresolution;

		pScanInfo->Contrast            = 0;
		pScanInfo->ContrastRange.lMin  = 0;
		pScanInfo->ContrastRange.lMax  = 100;
		pScanInfo->ContrastRange.lStep = 1;

		option = context->device->GetOption("contrast");
		if (option) {
			if (option->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
				range = option->GetConstraintRange();
				pScanInfo->Contrast            = (range->min + range->max) / 2;
				pScanInfo->ContrastRange.lMin  = range->min;
				pScanInfo->ContrastRange.lMax  = range->max;
				pScanInfo->ContrastRange.lStep = range->quant ? range->quant : 1;
			}
			hr = GetOptionValue(option, &dbl);
			if (hr == S_OK) {
				pScanInfo->Contrast = (LONG) dbl;
			}
		}
		
		pScanInfo->Intensity            = 0;
		pScanInfo->IntensityRange.lMin  = 0;
		pScanInfo->IntensityRange.lMax  = 100;
		pScanInfo->IntensityRange.lStep = 1;

		option = context->device->GetOption("brightness");
		if (option) {
			if (option->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
				range = option->GetConstraintRange();
				pScanInfo->IntensityRange.lMin  = (range->min + range->max) / 2;
				pScanInfo->IntensityRange.lMax  = range->max;
				pScanInfo->IntensityRange.lStep = range->quant ? range->quant : 1;
			}
			hr = GetOptionValue(option, &dbl);
			if (hr == S_OK) {
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

		hr = FetchScannerParams(pScanInfo, context);
		if (hr != S_OK)
			return hr;
	}

#ifdef _DEBUG
    Trace(TEXT("bw    = %d"), pScanInfo->BedWidth);
    Trace(TEXT("bh    = %d"), pScanInfo->BedHeight);
#endif

	return S_OK;
}

HRESULT SetScannerSettings(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	WINSANE_Option *option;
	HRESULT hr;

	if (context && context->session && context->device) {
		option = context->device->GetOption("mode");
		if (option && option->GetType() == SANE_TYPE_STRING) {
			switch (pScanInfo->DataType) {
				case WIA_DATA_THRESHOLD:
					hr = SetOptionValue(option, "Lineart");
					break;
				case WIA_DATA_GRAYSCALE:
					hr = SetOptionValue(option, "Gray");
					break;
				case WIA_DATA_COLOR:
					hr = SetOptionValue(option, "Color");
					break;
				default:
					hr = E_INVALIDARG;
					break;
			}
			if (hr != S_OK)
				return hr;
		} else
			return E_NOTIMPL;

		option = context->device->GetOption("resolution");
		if (option) {
			hr = SetOptionValue(option, pScanInfo->Xresolution);
			if (hr != S_OK)
				return hr;
		} else
			return E_NOTIMPL;

		option = context->device->GetOption("contrast");
		if (option) {
			hr = SetOptionValue(option, pScanInfo->Contrast);
			if (hr != S_OK && hr != E_NOTIMPL)
				return hr;
		}

		option = context->device->GetOption("brightness");
		if (option) {
			hr = SetOptionValue(option, pScanInfo->Intensity);
			if (hr != S_OK && hr != E_NOTIMPL)
				return hr;
		}
	}

    return S_OK;
}

HRESULT FetchScannerParams(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	WINSANE_Params *params;
	SANE_Frame frame;
	SANE_Int depth;
	HRESULT hr;

	params = context->device->GetParams();
	if (!params)
		return E_FAIL;

	frame = params->GetFormat();
	depth = params->GetDepth();

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

	if (hr == S_OK) {
		pScanInfo->WidthBytes = params->GetBytesPerLine();
		pScanInfo->WidthPixels = params->GetPixelsPerLine();
		pScanInfo->Lines = params->GetLines();
	}

	delete params;

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

HRESULT SetScanMode(PSCANINFO pScanInfo, LONG lScanMode)
{
	WIASANE_Context *context;
	WINSANE_Option *option;
	SANE_String_Const *string_list;
	HRESULT hr;
	int idx;

	hr = E_NOTIMPL;

	context = (WIASANE_Context*) pScanInfo->pMicroDriverContext;

	switch (lScanMode) {
		case SCANMODE_FINALSCAN:
			Trace(TEXT("Final Scan"));

			if (context && context->session && context->device) {
				option = context->device->GetOption("compression");
				if (option && option->GetType() == SANE_TYPE_STRING) {
					hr = SetOptionValue(option, "None");
					if (hr != S_OK && hr != E_NOTIMPL)
						break;
				}
			}

			hr = S_OK;
			break;

		case SCANMODE_PREVIEWSCAN:
			Trace(TEXT("Preview Scan"));

			if (context && context->session && context->device) {
				option = context->device->GetOption("compression");
				if (option && option->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
					string_list = option->GetConstraintStringList();
					for (idx = 0; string_list[idx] != NULL; idx++) {
						if (_stricmp(string_list[idx], "None")) {
							option->SetValueString(string_list[idx]);
							break;
						}
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
