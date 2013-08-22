#include "stdafx.h"
#include "wiasane_btns.h"

#include <objbase.h>

#include "resource.h"
#include "wiasane_util.h"

//
// Button GUID array used in Capability negotiation.
// Set your BUTTON guids here.  These must match the GUIDS specified in
// your INF.  The Scan Button GUID is public to all scanners with a
// scan button.
//

GUID g_Buttons[MAX_BUTTONS] = {{0xa6c5a715, 0x8c6e, 0x11d2, {0x97, 0x7a,  0x0,  0x0, 0xf8, 0x7a, 0x92, 0x6f}}};
BOOL g_bButtonNamesCreated = FALSE;
WCHAR* g_ButtonNames[MAX_BUTTONS] = {0};

HRESULT CreateButtonNames(HRESULT hr)
{
	INT index = 0;

	if (!g_bButtonNamesCreated)
    {
        for(index = 0; index < MAX_BUTTONS; index++)
        {
            g_ButtonNames[index] = (WCHAR*)CoTaskMemAlloc(MAX_BUTTON_NAME);
            if (!g_ButtonNames[index])
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = GetOLESTRResourceString(IDS_SCAN_BUTTON_NAME,&g_ButtonNames[0],TRUE);
        }

        if(SUCCEEDED(hr))
        {
            g_bButtonNamesCreated = TRUE;
        }
        else
        {
            for(index = 0; index < MAX_BUTTONS; index++)
            {
                if (g_ButtonNames[index])
                {
                    CoTaskMemFree(g_ButtonNames[index]);
                    g_ButtonNames[index] = NULL;
                }
            }
        }
    }

	return hr;
}

HRESULT FreeButtonNames(HRESULT hr)
{
	INT index = 0;

	if(g_bButtonNamesCreated)
    {
        g_bButtonNamesCreated = FALSE;

        for(index = 0; index < MAX_BUTTONS; index++)
        {
            if (g_ButtonNames[index])
            {
                CoTaskMemFree(g_ButtonNames[index]);
                g_ButtonNames[index] = NULL;
            }
        }
    }

	return hr;
}

/**************************************************************************\
* CheckButtonStatus (helper)
*
*   Called by the MicroDriver to Set the current Button pressed value.
*
* Arguments:
*
*   pValue       - VAL structure used for settings
*
*
* Return Value:
*
*    VOID
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

VOID CheckButtonStatus(PVAL pValue)
{
    //
    // Button Polling is done here...
    //

    //
    // Check your device for button presses
    //

    LONG lButtonValue = 0;

    GetButtonPress(&lButtonValue);
    switch (lButtonValue) {
    case 1:
        pValue->pGuid = (GUID*) &guidScanButton;
        Trace(TEXT("Scan Button Pressed!"));
        break;
    default:
        pValue->pGuid = (GUID*) &GUID_NULL;
        break;
    }
}

/**************************************************************************\
* GetButtonPress (helper)
*
*   Called by the MicroDriver to set the actual button value pressed
*
* Arguments:
*
*   pButtonValue       - actual button pressed
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

VOID GetButtonPress(LONG *pButtonValue)
{

    //
    // This where you can set your button value
    //

    pButtonValue = 0;
}

/**************************************************************************\
* GetButtonCount (helper)
*
*   Called by the MicroDriver to get the number of buttons a device supports
*
* Arguments:
*
*    none
*
* Return Value:
*
*    LONG - number of supported buttons
*
* History:
*
*    1/20/2000 Original Version
*
\**************************************************************************/

LONG GetButtonCount()
{
    LONG ButtonCount  = 0;

    //
    // Since the test device does not have a button,
    // set this value to 0.  For a real device with a button,
    // set (LONG ButtonCount  = 1;)
    //

    //
    // determine the button count of your device
    //

    return ButtonCount;
}
