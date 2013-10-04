#ifndef WINSANE_DBG_H
#define WINSANE_DBG_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "winsane.h"

VOID Debug(PTSTR hostname);
VOID DebugSession(WINSANE_Session *session);
VOID DebugSessionDevice(WINSANE_Session *session, WINSANE_Device *device);
VOID DebugSessionDeviceOption(WINSANE_Session *session, WINSANE_Device *device, WINSANE_Option *option);
VOID DebugSessionDeviceParams(WINSANE_Session *session, WINSANE_Device *device, WINSANE_Params *params);
VOID DebugSessionDeviceScan(WINSANE_Session *session, WINSANE_Device *device);

#endif
