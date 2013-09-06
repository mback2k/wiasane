#include "stdafx.h"
#include "wiasane.h"

#include <STI.H>
#include <math.h>
#include <winioctl.h>
#include <usbscan.h>

#include "wiasane_util.h"


WIAMICRO_API HRESULT MicroEntry(LONG lCommand, _Inout_ PVAL pValue)
{
	WIASANE_Context *context;
	WINSANE_Option *option;
	WINSANE_Params *params;
	HRESULT hr;

#ifdef DEBUG
	if (lCommand != CMD_STI_GETSTATUS)
		Trace(TEXT("Command Value (%d)"),lCommand);
#endif

	if (!pValue || !pValue->pScanInfo)
		return E_INVALIDARG;

	hr = E_NOTIMPL;
	context = (WIASANE_Context*) pValue->pScanInfo->pMicroDriverContext;

    switch(lCommand) {
		case CMD_INITIALIZE:
			if (context) {
				UninitializeScanner(pValue->pScanInfo, context);
				context = NULL;
			}

			context = new WIASANE_Context;
			if (context) {
				pValue->pScanInfo->pMicroDriverContext = context;
				hr = InitializeScanner(pValue->pScanInfo, context);
			} else {
				pValue->pScanInfo->pMicroDriverContext = NULL;
				hr = E_OUTOFMEMORY;
			}

			if (hr == S_OK)
				hr = InitScannerDefaults(pValue->pScanInfo, context);

			break;

		case CMD_UNINITIALIZE:
			if (context) {
				UninitializeScanner(pValue->pScanInfo, context);
				context = NULL;
			}

			hr = S_OK;
			break;

		case CMD_RESETSCANNER:
		case CMD_STI_DEVICERESET:
			if (context && context->session && context->device) {
				context->device->Cancel();
			}

			hr = S_OK;
			break;

		case CMD_STI_DIAGNOSTIC:
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
				if (option->IsValidValue(pValue->lVal)) {
					hr = S_OK;
				}
			} else {
				hr = E_FAIL;
				break;
			}

			switch (lCommand) {
				case CMD_SETXRESOLUTION:
					pValue->pScanInfo->Xresolution = pValue->lVal;
					if (hr == S_OK)
						pValue->pScanInfo->OpticalXResolution = pValue->lVal;
					break;

				case CMD_SETYRESOLUTION:
					pValue->pScanInfo->Yresolution = pValue->lVal;
					if (hr == S_OK)
						pValue->pScanInfo->OpticalYResolution = pValue->lVal;
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
				option = context->device->GetOption("intensity");
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

		case CMD_SETSTIDEVICEHKEY:
			hr = ReadRegistryInformation(pValue);
			break;

#ifdef _USE_EXTENDED_FORMAT_LIST

		// note: MEMORYBMP, and BMP file will be added by wiafbdrv host driver.
		//       do not include them in your extended list.
		//

		case CMD_GETSUPPORTEDFILEFORMATS:
			g_SupportedFileFormats[0] = WiaImgFmt_JPEG;
			pValue->lVal = NUM_SUPPORTED_FILEFORMATS;
			pValue->pGuid = g_SupportedFileFormats;
			hr = S_OK;
			break;

		case CMD_GETSUPPORTEDMEMORYFORMATS:
			g_SupportedMemoryFormats[0] = WiaImgFmt_TIFF;
			g_SupportedMemoryFormats[1] = WiaImgFmt_MYNEWFORMAT;
			pValue->lVal = NUM_SUPPORTED_MEMORYFORMATS;
			pValue->pGuid = g_SupportedMemoryFormats;
			hr = S_OK;
	        break;
#endif

		default:
			Trace(TEXT("Unknown Command (%d)"),lCommand);
			break;
    }

    return hr;
}

WIAMICRO_API HRESULT Scan(_Inout_ PSCANINFO pScanInfo, LONG lPhase, _Out_writes_bytes_(lLength) PBYTE pBuffer, LONG lLength, _Out_ LONG *plReceived)
{
	WIASANE_Context *context;
	LONG lAquired;
	HRESULT hr;
	LONG idx;

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

				context->scan = context->device->Start();

				context->total = pScanInfo->WidthBytes * pScanInfo->Lines;
				context->received = 0;
				Trace(TEXT("Data: %d/%d"), context->received, context->total);
			}

		case SCAN_NEXT: // SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)
			if (lPhase == SCAN_NEXT)
				Trace(TEXT("SCAN_NEXT"));

			//
			// next phase, get data from the scanner and set plReceived value
			//

			*plReceived = 0;

			if (context && context->session && context->device && context->scan) {
				memset(pBuffer, 0, lLength);

				lAquired = lLength;
				while (context->scan->AquireImage((char*) (pBuffer + *plReceived), &lAquired) == CONTINUE) {
					*plReceived += lAquired;
					lAquired = lLength - *plReceived;
					if (!lAquired)
						break;
				}

				if (pScanInfo->DataType == WIA_DATA_THRESHOLD) {
					for (idx = 0; idx < *plReceived; idx++) {
						pBuffer[idx] = ~pBuffer[idx];
					}
				}

				hr = FetchScannerParams(pScanInfo, context);
				if (hr != S_OK)
					return hr;

				context->total = pScanInfo->WidthBytes * pScanInfo->Lines;
				context->received += *plReceived;
				Trace(TEXT("Data: %d/%d -> %d/%d"), context->received, context->total, context->total - context->received, lLength);
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
				if (context->scan) {
					delete context->scan;
					context->scan = NULL;
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
	HRESULT hr;

    if (pScanInfo == NULL)
        return E_INVALIDARG;

	context = (WIASANE_Context*) pScanInfo->pMicroDriverContext;
	if (context && context->session && context->device) {
		hr = SetOptionValue(context, "resolution", pScanInfo->OpticalXResolution);
		if (hr != S_OK)
			return hr;

		context->device->FetchOptions();

		hr = IsValidOptionValueInch(context, "tl-x", (double) x / pScanInfo->Xresolution);
		if (hr != S_OK)
			return hr;

		hr = IsValidOptionValueInch(context, "tl-y", (double) y / pScanInfo->Yresolution);
		if (hr != S_OK)
			return hr;

		hr = IsValidOptionValueInch(context, "br-x", (double) xExtent / pScanInfo->Xresolution);
		if (hr != S_OK)
			return hr;

		hr = IsValidOptionValueInch(context, "br-y", (double) yExtent / pScanInfo->Yresolution);
		if (hr != S_OK)
			return hr;
	} else
		return E_FAIL;

    pScanInfo->Window.xPos = x;
    pScanInfo->Window.yPos = y;
    pScanInfo->Window.xExtent = xExtent;
    pScanInfo->Window.yExtent = yExtent;

    return S_OK;
}


HRESULT ReadRegistryInformation(PVAL pValue)
{
    HKEY hKey, hOpenKey;
	DWORD dwWritten, dwType;
	LONG lSampleEntry;

    if (pValue->pHandle != NULL) {
        hKey = (HKEY) *(pValue->pHandle);
		hOpenKey = NULL;

        //
        // Open DeviceData section to read driver specific information
        //

        if (RegOpenKeyEx(hKey,                     // handle to open key
                         TEXT("DeviceData"),       // address of name of subkey to open
                         0,                        // options (must be NULL)
                         KEY_QUERY_VALUE|KEY_READ, // just want to QUERY a value
                         &hOpenKey                 // address of handle to open key
                        ) == ERROR_SUCCESS) {

            dwWritten = sizeof(DWORD);
            dwType = REG_DWORD;

            lSampleEntry = 0;
            RegQueryValueEx(hOpenKey,
                            TEXT("Sample Entry"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lSampleEntry,
                            &dwWritten);

            Trace(TEXT("lSampleEntry Value = %d"), lSampleEntry);
        } else {
            Trace(TEXT("Could not open DeviceData section"));
        }
    }

	return S_OK;
}

HRESULT InitializeScanner(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	UNREFERENCED_PARAMETER(pScanInfo);
	
	context->scan = NULL;
	context->session = WINSANE_Session::Remote("10.0.0.1");
	if (context->session && context->session->Init(NULL, NULL) && context->session->GetDevices() > 0) {
		context->device = context->session->GetDevice(0);
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

HRESULT UninitializeScanner(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	if (context->session) {
		if (context->device) {
			if (context->scan) {
				context->device->Cancel();
				delete context->scan;
				context->scan = NULL;
			}
			context->device->Close();
			context->device = NULL;
		}
		context->session->Exit();
		delete context->session;
		context->session = NULL;
	}
	pScanInfo->pMicroDriverContext = NULL;
	delete context;

	return S_OK;
}

HRESULT InitScannerDefaults(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	WINSANE_Option *mode, *resolution, *contrast, *intensity;
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

		hr = SetOptionValue(context, "compression", "None");
		if (hr != S_OK && hr != E_NOTIMPL)
			return hr;

		mode = context->device->GetOption("mode");
		if (mode && mode->GetType() == SANE_TYPE_STRING && mode->GetConstraintType() == SANE_CONSTRAINT_STRING_LIST) {
			string_list = mode->GetConstraintStringList();
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

		hr = GetOptionValueInch(context, "br-x", &dbl);
		if (hr == S_OK)
			pScanInfo->BedWidth = (LONG) ceil(dbl * 1000);
		else
			pScanInfo->BedWidth = 8500;   // 1000's of an inch (WIA compatible unit)

		hr = GetOptionValueInch(context, "br-y", &dbl);
		if (hr == S_OK)
			pScanInfo->BedHeight = (LONG) ceil(dbl * 1000);
		else
			pScanInfo->BedHeight = 11000; // 1000's of an inch (WIA compatible unit)

		resolution = context->device->GetOption("resolution");
		if (resolution && resolution->GetType() == SANE_TYPE_INT) {
			pScanInfo->OpticalXResolution = resolution->GetValueInt();
		} else {
			pScanInfo->OpticalXResolution = 300;
		}

		pScanInfo->OpticalYResolution = pScanInfo->OpticalXResolution;
		pScanInfo->Xresolution = pScanInfo->OpticalXResolution;
		pScanInfo->Yresolution = pScanInfo->OpticalYResolution;

		contrast = context->device->GetOption("contrast");
		if (contrast && contrast->GetType() == SANE_TYPE_INT) {
			pScanInfo->Contrast = contrast->GetValueInt();
			if (contrast->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
				range = contrast->GetConstraintRange();
				pScanInfo->ContrastRange.lMin  = range->min;
				pScanInfo->ContrastRange.lMax  = range->max;
				pScanInfo->ContrastRange.lStep = range->quant ? range->quant : 1;
			} else {
				pScanInfo->ContrastRange.lMin  = 0;
				pScanInfo->ContrastRange.lMax  = 100;
				pScanInfo->ContrastRange.lStep = 1;
			}
		} else {
			pScanInfo->Contrast            = 0;
			pScanInfo->ContrastRange.lMin  = 0;
			pScanInfo->ContrastRange.lMax  = 0;
			pScanInfo->ContrastRange.lStep = 0;
		}
		
		intensity = context->device->GetOption("intensity");
		if (intensity && intensity->GetType() == SANE_TYPE_INT) {
			pScanInfo->Intensity = intensity->GetValueInt();
			if (intensity->GetConstraintType() == SANE_CONSTRAINT_RANGE) {
				range = intensity->GetConstraintRange();
				pScanInfo->IntensityRange.lMin  = range->min;
				pScanInfo->IntensityRange.lMax  = range->max;
				pScanInfo->IntensityRange.lStep = range->quant ? range->quant : 1;
			} else {
				pScanInfo->IntensityRange.lMin  = 0;
				pScanInfo->IntensityRange.lMax  = 100;
				pScanInfo->IntensityRange.lStep = 1;
			}
		} else {
			pScanInfo->Intensity            = 0;
			pScanInfo->IntensityRange.lMin  = 0;
			pScanInfo->IntensityRange.lMax  = 0;
			pScanInfo->IntensityRange.lStep = 0;
		}

		pScanInfo->Window.xPos            = 0;
		pScanInfo->Window.yPos            = 0;
		pScanInfo->Window.xExtent         = (pScanInfo->BedWidth  * pScanInfo->Xresolution) / 1000;
		pScanInfo->Window.yExtent         = (pScanInfo->BedHeight * pScanInfo->Yresolution) / 1000;

		// Scanner options
		pScanInfo->DitherPattern          = 0;
		pScanInfo->Negative               = 0;
		pScanInfo->Mirror                 = 0;
		pScanInfo->AutoBack               = 0;
		pScanInfo->ColorDitherPattern     = 0;
		pScanInfo->ToneMap                = 0;
		pScanInfo->Compression            = 0;

		hr = FetchScannerParams(pScanInfo, context);
		if (hr != S_OK)
			return hr;
	}

#ifdef DEBUG
    Trace(TEXT("bw    = %d"), pScanInfo->BedWidth);
    Trace(TEXT("bh    = %d"), pScanInfo->BedHeight);
#endif

	return S_OK;
}

HRESULT SetScannerSettings(PSCANINFO pScanInfo, WIASANE_Context *context)
{
	HRESULT hr;

	if (context && context->session && context->device) {
		switch (pScanInfo->DataType) {
			case WIA_DATA_THRESHOLD:
				hr = SetOptionValue(context, "mode", "Lineart");
				break;
			case WIA_DATA_GRAYSCALE:
				hr = SetOptionValue(context, "mode", "Gray");
				break;
			case WIA_DATA_COLOR:
				hr = SetOptionValue(context, "mode", "Color");
				break;
			default:
				hr = E_INVALIDARG;
				break;
		}
		if (hr != S_OK)
			return hr;

		hr = SetOptionValue(context, "resolution", pScanInfo->OpticalXResolution);
		if (hr != S_OK)
			return hr;

		hr = SetOptionValue(context, "contrast", pScanInfo->Contrast);
		if (hr != S_OK && hr != E_NOTIMPL)
			return hr;

		hr = SetOptionValue(context, "intensity", pScanInfo->Intensity);
		if (hr != S_OK && hr != E_NOTIMPL)
			return hr;

		hr = SetOptionValueInch(context, "tl-x", (double) pScanInfo->Window.xPos / pScanInfo->Xresolution);
		if (hr != S_OK)
			return hr;

		hr = SetOptionValueInch(context, "tl-y", (double) pScanInfo->Window.yPos / pScanInfo->Yresolution);
		if (hr != S_OK)
			return hr;

		hr = SetOptionValueInch(context, "br-x", (double) pScanInfo->Window.xExtent / pScanInfo->Xresolution);
		if (hr != S_OK)
			return hr;

		hr = SetOptionValueInch(context, "br-y", (double) pScanInfo->Window.yExtent / pScanInfo->Yresolution);
		if (hr != S_OK)
			return hr;

		hr = FetchScannerParams(pScanInfo, context);
		if (hr != S_OK)
			return hr;

		if (!context->device->FetchOptions())
			return E_FAIL;
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

#ifdef DEBUG
    Trace(TEXT("New scanner settings"));
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
	WINSANE_Option *resolution;
	SANE_Word *word_list;
	HRESULT hr;

	hr = E_NOTIMPL;

	context = (WIASANE_Context*) pScanInfo->pMicroDriverContext;

	switch(lScanMode) {
		case SCANMODE_FINALSCAN:
			Trace(TEXT("Final Scan"));

			hr = S_OK;
			break;

		case SCANMODE_PREVIEWSCAN:
			Trace(TEXT("Preview Scan"));

			if (context && context->session && context->device) {
				resolution = context->device->GetOption("resolution");
				if (resolution && resolution->GetConstraintType() == SANE_CONSTRAINT_WORD_LIST) {
					word_list = resolution->GetConstraintWordList();
					if (word_list && word_list[0] > 0) {
						pScanInfo->OpticalXResolution = word_list[1];
						pScanInfo->OpticalYResolution = word_list[1];
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
