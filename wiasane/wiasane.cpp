#include "stdafx.h"
#include "wiasane.h"

#include <STI.H>
#include <math.h>
#include <winioctl.h>
#include <usbscan.h>

#include "wiasane_btns.h"
#include "wiasane_util.h"


/**************************************************************************\
* MicroEntry (MicroDriver Entry point)
*
*   Called by the WIA driver to communicate with the MicroDriver.
*
* Arguments:
*
*   lCommand     - MicroDriver Command, sent from the WIA driver
*   pValue       - VAL structure used for settings
*
*
* Return Value:
*
*    Status
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

WIAMICRO_API HRESULT MicroEntry(LONG lCommand, _Inout_ PVAL pValue)
{
    HRESULT hr = E_NOTIMPL;

//#define _DEBUG_COMMANDS

#ifdef _DEBUG_COMMANDS
    if(lCommand != CMD_STI_GETSTATUS)
        Trace(TEXT("Command Value (%d)"),lCommand);
#endif

    if( !pValue || !(pValue->pScanInfo))
    {
        return E_INVALIDARG;
    }

    switch(lCommand)
    {
    case CMD_INITIALIZE:
        hr = S_OK;

        //
        // create any DeviceIO handles needed, use index (1 - MAX_IO_HANDLES) to store these handles.
        // Index '0' is reserved by the WIA flatbed driver. The CreateFile Name is stored in the szVal
        // member of the VAL structure.
        //

        // pValue->pScanInfo->DeviceIOHandles[1] = CreateFileA( pValue->szVal,
        //                                   GENERIC_READ | GENERIC_WRITE, // Access mask
        //                                   0,                            // Share mode
        //                                   NULL,                         // SA
        //                                   OPEN_EXISTING,                // Create disposition
        //                                   FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,        // Attributes
        //                                   NULL );

        //
        // if your device supports buttons, create the BUTTON name information here..
        //

		hr = CreateButtonNames(hr);

        //
        // Initialize the scanner's default settings
        //

        InitScannerDefaults(pValue->pScanInfo);

        break;
    case CMD_UNINITIALIZE:

        //
        // close any open handles created by the Micro driver
        //

        if(pValue->pScanInfo->DeviceIOHandles[1] != NULL)
        {
            CloseHandle(pValue->pScanInfo->DeviceIOHandles[1]);
        }

        //
        // if your device supports buttons, free/destroy the BUTTON name information here..
        //

		hr = FreeButtonNames(hr);

        //
        // close/unload libraries
        //

        hr = S_OK;
        break;
    case CMD_RESETSCANNER:

        //
        // reset scanner
        //

        hr = S_OK;
        break;
    case CMD_STI_DIAGNOSTIC:
    case CMD_STI_DEVICERESET:

        //
        // reset device
        //

        hr = S_OK;
        break;
    case CMD_STI_GETSTATUS:

        //
        // set status flag to ON-LINE
        //

        pValue->lVal = MCRO_STATUS_OK;
        pValue->pGuid = (GUID*) &GUID_NULL;

        //
        // button polling support
        //

#ifdef BUTTON_SUPPORT
        CheckButtonStatus(pValue);
#endif

        hr = S_OK;
        break;
    case CMD_SETXRESOLUTION:
        pValue->pScanInfo->Xresolution = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETYRESOLUTION:
        pValue->pScanInfo->Yresolution = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETCONTRAST:
        pValue->pScanInfo->Contrast    = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETINTENSITY:
        pValue->pScanInfo->Intensity   = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETDATATYPE:
        pValue->pScanInfo->DataType    = pValue->lVal;
        hr = S_OK;
        break;
    case CMD_SETNEGATIVE:
        pValue->pScanInfo->Negative    = pValue->lVal;
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
        hr = S_OK;
        switch(pValue->lVal)
        {
        case SCANMODE_FINALSCAN:
            Trace(TEXT("Final Scan"));
            break;
        case SCANMODE_PREVIEWSCAN:
            Trace(TEXT("Preview Scan"));
            break;
        default:
            Trace(TEXT("Unknown Scan Mode (%d)"),pValue->lVal);
            hr = E_FAIL;
            break;
        }
        break;
    case CMD_SETSTIDEVICEHKEY:
        ReadRegistryInformation(pValue);
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

/**************************************************************************\
* Scan (MicroDriver Entry point)
*
*   Called by the WIA driver to acquire data from the MicroDriver.
*
* Arguments:
*
*   pScanInfo    - SCANINFO structure used for settings
*   lPhase       - Current Scan phase, SCAN_FIRST, SCAN_NEXT, SCAN_FINISH...
*   pBuffer      - data buffer to be filled with scanned data
*   lLength      - Maximum length of pBuffer
*   plReceived   - Number of actual bytes written to pBuffer.
*
*
* Return Value:
*
*    Status
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

WIAMICRO_API HRESULT Scan(_Inout_ PSCANINFO pScanInfo, LONG lPhase, _Out_writes_bytes_(lLength) PBYTE pBuffer, LONG lLength, _Out_ LONG *plReceived)
{
    if(pScanInfo == NULL) {
        return E_INVALIDARG;
    }

    INT i = 0;
    *plReceived = 0; // Initialize *plReceived as 0. It has a return value in case of SCAN_FIRST and SCAN_NEXT.

    Trace(TEXT("------ Scan Requesting %d ------"),lLength);
    switch (lPhase) {
    case SCAN_FIRST:
        if (!SetScannerSettings(pScanInfo)) {
            return E_FAIL;
        }

        Trace(TEXT("SCAN_FIRST"));

        g_PalIndex = 0;
        g_bDown = FALSE;

        //
        // first phase
        //

        Trace(TEXT("Start Scan.."));

    case SCAN_NEXT: // SCAN_FIRST will fall through to SCAN_NEXT (because it is expecting data)

        //
        // next phase
        //

        if(lPhase == SCAN_NEXT)
            Trace(TEXT("SCAN_NEXT"));

        //
        // get data from the scanner and set plReceived value
        //

        //
        // read data
        //

        switch(pScanInfo->DataType) {
        case WIA_DATA_THRESHOLD:

            //
            // make buffer alternate black/White, for sample 1-bit data
            //

            memset(pBuffer,0,lLength);
            memset(pBuffer,255,lLength/2);
            break;
        case WIA_DATA_GRAYSCALE:

            //
            // make buffer grayscale data, for sample 8-bit data
            //

            if(!g_bDown){
                g_PalIndex+=10;
                if(g_PalIndex > 255){
                    g_PalIndex = 255;
                    g_bDown = TRUE;
                }
            }
            else {
                g_PalIndex-=10;
                if(g_PalIndex < 0){
                    g_PalIndex = 0;
                    g_bDown = FALSE;
                }
            }
            memset(pBuffer,g_PalIndex,lLength);
            break;
        case WIA_DATA_COLOR:

            //
            // make buffer red, for sample color data
            //

            for (i = 0;i+2<lLength;i+=3) {
                memset(pBuffer+i,255,1);
                memset(pBuffer+(i+1),0,1);
                memset(pBuffer+(i+2),0,1);
            }
            break;
        default:
            break;
        }

        //
        // test device always returns the exact amount of scanned data
        //

        *plReceived = lLength;
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

        break;
    }

    return S_OK;
}

/**************************************************************************\
* SetPixelWindow (MicroDriver Entry point)
*
*   Called by the WIA driver to set the scan selection area to the MicroDriver.
*
* Arguments:
*
*   pScanInfo    - SCANINFO structure used for settings
*   pValue       - VAL structure used for settings
*   x            - X Position of scan rect (upper left x coordinate)
*   y            - Y Position of scan rect (upper left y coordinate)
*   xExtent      - Width of scan rect  (in pixels)
*   yExtent      - Height of scan rect (in pixels)
*
*
* Return Value:
*
*    Status
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

WIAMICRO_API HRESULT SetPixelWindow(_Inout_ PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{
    if(pScanInfo == NULL) {
        return E_INVALIDARG;
    }

    pScanInfo->Window.xPos = x;
    pScanInfo->Window.yPos = y;
    pScanInfo->Window.xExtent = xExtent;
    pScanInfo->Window.yExtent = yExtent;
    return S_OK;
}


/**************************************************************************\
* ReadRegistryInformation (helper)
*
*   Called by the MicroDriver to Read registry information from the device's
*   installed device section. The HKEY passed in will be closed by the host
*   driver after CMD_INITIALIZE is completed.
*
* Arguments:
*
*    none
*
* Return Value:
*
*    void
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/
VOID ReadRegistryInformation(PVAL pValue)
{
    HKEY hKey = NULL;
    if(NULL != pValue->pHandle){
        hKey = (HKEY)*pValue->pHandle;

        //
        // Open DeviceData section to read driver specific information
        //

        HKEY hOpenKey = NULL;
        if (RegOpenKeyEx(hKey,                     // handle to open key
                         TEXT("DeviceData"),       // address of name of subkey to open
                         0,                        // options (must be NULL)
                         KEY_QUERY_VALUE|KEY_READ, // just want to QUERY a value
                         &hOpenKey                 // address of handle to open key
                        ) == ERROR_SUCCESS) {

            DWORD dwWritten = sizeof(DWORD);
            DWORD dwType = REG_DWORD;

            LONG lSampleEntry = 0;
            RegQueryValueEx(hOpenKey,
                            TEXT("Sample Entry"),
                            NULL,
                            &dwType,
                            (LPBYTE)&lSampleEntry,
                            &dwWritten);
            Trace(TEXT("lSampleEntry Value = %d"),lSampleEntry);
        } else {
            Trace(TEXT("Could not open DeviceData section"));
        }
    }
}

/**************************************************************************\
* InitScannerDefaults (helper)
*
*   Called by the MicroDriver to Initialize the SCANINFO structure
*
* Arguments:
*
*    none
*
* Return Value:
*
*    void
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

VOID InitScannerDefaults(PSCANINFO pScanInfo)
{

    pScanInfo->ADF                    = 0; // set to no ADF in Test device
    pScanInfo->RawDataFormat          = WIA_PACKED_PIXEL;
    pScanInfo->RawPixelOrder          = WIA_ORDER_BGR;
    pScanInfo->bNeedDataAlignment     = TRUE;

    pScanInfo->SupportedCompressionType = 0;
    pScanInfo->SupportedDataTypes     = SUPPORT_BW|SUPPORT_GRAYSCALE|SUPPORT_COLOR;

    pScanInfo->BedWidth               = 8500;  // 1000's of an inch (WIA compatible unit)
    pScanInfo->BedHeight              = 11000; // 1000's of an inch (WIA compatible unit)

    pScanInfo->OpticalXResolution     = 300;
    pScanInfo->OpticalYResolution     = 300;

    pScanInfo->IntensityRange.lMin    = -127;
    pScanInfo->IntensityRange.lMax    =  127;
    pScanInfo->IntensityRange.lStep   = 1;

    pScanInfo->ContrastRange.lMin     = -127;
    pScanInfo->ContrastRange.lMax     = 127;
    pScanInfo->ContrastRange.lStep    = 1;

    // Scanner settings
    pScanInfo->Intensity              = 0;
    pScanInfo->Contrast               = 0;

    pScanInfo->Xresolution            = 150;
    pScanInfo->Yresolution            = 150;

    pScanInfo->Window.xPos            = 0;
    pScanInfo->Window.yPos            = 0;
    pScanInfo->Window.xExtent         = (pScanInfo->Xresolution * pScanInfo->BedWidth)/1000;
    pScanInfo->Window.yExtent         = (pScanInfo->Yresolution * pScanInfo->BedHeight)/1000;

    // Scanner options
    pScanInfo->DitherPattern          = 0;
    pScanInfo->Negative               = 0;
    pScanInfo->Mirror                 = 0;
    pScanInfo->AutoBack               = 0;
    pScanInfo->ColorDitherPattern     = 0;
    pScanInfo->ToneMap                = 0;
    pScanInfo->Compression            = 0;

        // Image Info
    pScanInfo->DataType               = WIA_DATA_GRAYSCALE;
    pScanInfo->WidthPixels            = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos);

    switch(pScanInfo->DataType) {
    case WIA_DATA_THRESHOLD:
        pScanInfo->PixelBits = 1;
        break;
    case WIA_DATA_COLOR:
        pScanInfo->PixelBits = 24;
        break;
    case WIA_DATA_GRAYSCALE:
    default:
        pScanInfo->PixelBits = 8;
        break;
    }

    pScanInfo->WidthBytes = pScanInfo->Window.xExtent * (pScanInfo->PixelBits/8);
    pScanInfo->Lines      = pScanInfo->Window.yExtent;
}

/**************************************************************************\
* SetScannerSettings (helper)
*
*   Called by the MicroDriver to set the values stored in the SCANINFO structure
*   to the actual device.
*
* Arguments:
*
*     none
*
*
* Return Value:
*
*    TRUE - Success, FALSE - Failure
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

BOOL SetScannerSettings(PSCANINFO pScanInfo)
{
    if(pScanInfo->DataType == WIA_DATA_THRESHOLD) {
        pScanInfo->PixelBits = 1;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/7);

        //
        // Set data type to device
        //

        // if the set fails..
        // return FALSE;
    }
    else if(pScanInfo->DataType == WIA_DATA_GRAYSCALE) {
        pScanInfo->PixelBits = 8;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/8);

        //
        // Set data type to device
        //

        // if the set fails..
        // return FALSE;

    }
    else {
        pScanInfo->PixelBits = 24;
        pScanInfo->WidthBytes         = (pScanInfo->Window.xExtent)-(pScanInfo->Window.xPos) * (pScanInfo->PixelBits/8);

        //
        // Set data type to device
        //

        // if the set fails..
        // return FALSE;

    }

#ifdef DEBUG
    Trace(TEXT("ScanInfo"));
    Trace(TEXT("x res = %d"),pScanInfo->Xresolution);
    Trace(TEXT("y res = %d"),pScanInfo->Yresolution);
    Trace(TEXT("bpp   = %d"),pScanInfo->PixelBits);
    Trace(TEXT("xpos  = %d"),pScanInfo->Window.xPos);
    Trace(TEXT("ypos  = %d"),pScanInfo->Window.yPos);
    Trace(TEXT("xext  = %d"),pScanInfo->Window.xExtent);
    Trace(TEXT("yext  = %d"),pScanInfo->Window.yExtent);
#endif

    //
    // send other values to device, use the values set in pScanInfo to set them to your
    // device.
    //

    return TRUE;
}

/**************************************************************************\
* InitializeScanner (helper)
*
*   Called by the MicroDriver to Iniitialize any device specific operations
*
* Arguments:
*
*    none
*
* Return Value:
*
*    TRUE - Success, FALSE - Failure
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

BOOL InitializeScanner(PSCANINFO pScanInfo)
{
    UNREFERENCED_PARAMETER(pScanInfo);

    HRESULT hr = S_OK;

    //
    // Do any device initialization here...
    // The test device does not need any.
    //

    if (SUCCEEDED(hr)) {
        return TRUE;
    }
    return FALSE;
}
