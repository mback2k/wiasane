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
 ***************************************************************************/

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

WINSANE_API_CALLBACK SessionAuthCallback(_In_ SANE_String_Const resource, _Inout_ SANE_Char *username, _Inout_ SANE_Char *password);

#endif
