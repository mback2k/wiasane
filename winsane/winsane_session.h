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
 ***************************************************************************/

#ifndef WINSANE_SESSION_H
#define WINSANE_SESSION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "winsane.h"
#include "winsane_socket.h"
#include "winsane_device.h"

class WINSANE_API WINSANE_Session {
public:
	/* Constructer & Deconstructer */
	WINSANE_Session(_In_ SOCKET sock);
	~WINSANE_Session();

	static PWINSANE_Session Remote(_In_ PADDRINFOT addrInfo);
	static PWINSANE_Session Remote(_In_ PIN_ADDR addr);
	static PWINSANE_Session Remote(_In_ PIN_ADDR addr, _In_ USHORT port);
	static PWINSANE_Session Remote(_In_ PIN6_ADDR addr);
	static PWINSANE_Session Remote(_In_ PIN6_ADDR addr, _In_ USHORT port);
	static PWINSANE_Session Remote(_In_ PTSTR hostname);
	static PWINSANE_Session Remote(_In_ PTSTR hostname, _In_ USHORT port);


	/* Public API */
	SANE_Status Init(_In_opt_ PSANE_Int version, _In_opt_ SANE_Auth_Callback authorize);
	BOOL IsInitialized();
	SANE_Status Exit();

	SANE_Status FetchDevices();
	LONG GetDevices();
	WINSANE_Device* GetDevice(_In_ LONG index);
	WINSANE_Device* GetDevice(_In_ SANE_String_Const name);
	WINSANE_Device* GetDevice(_In_ PTSTR ptName);
	VOID ClearDevices();


private:
	LONG num_devices;
	PWINSANE_Device *devices;

	BOOL initialized;
	SANE_Auth_Callback auth_callback;
	PWINSANE_Socket sock;
};

#endif
