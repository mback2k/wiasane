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

#ifndef WINSANE_H
#define WINSANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "sane.h"

#define WINSANE_DEFAULT_PORT			6566

#define WINSANE_API_CALLBACK extern "C" void

typedef void (*WINSANE_Auth_Callback) (SANE_String_Const resource, SANE_Char *username, SANE_Char *password, void *userdata);

#ifdef WINSANE_EXPORTS
#define WINSANE_API __declspec(dllexport)
#else
#define WINSANE_API __declspec(dllimport)
#endif

class WINSANE_API WINSANE_Session;
class WINSANE_API WINSANE_Device;
class WINSANE_API WINSANE_Option;
class WINSANE_API WINSANE_Params;
class WINSANE_API WINSANE_Scan;

typedef WINSANE_Session* PWINSANE_Session;
typedef WINSANE_Device* PWINSANE_Device;
typedef WINSANE_Option* PWINSANE_Option;
typedef WINSANE_Params* PWINSANE_Params;
typedef WINSANE_Scan* PWINSANE_Scan;

#include "winsane_session.h"
#include "winsane_device.h"
#include "winsane_option.h"
#include "winsane_params.h"
#include "winsane_scan.h"

#endif
