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


	/* Internal API */
	PWINSANE_Socket GetSocket();


	/* Public API */
	BOOL Init(_In_ PSANE_Int version, _In_ SANE_Auth_Callback authorize);
	BOOL Exit();

	int GetDevices();
	WINSANE_Device* GetDevice(_In_ int index);
	WINSANE_Device* GetDevice(_In_ SANE_String_Const name);
	WINSANE_Device* GetDevice(_In_ PTSTR ptName);
	VOID ClearDevices();


private:
	int num_devices;
	PWINSANE_Device *devices;

	BOOL initialized;
	SANE_Auth_Callback auth_callback;
	PWINSANE_Socket sock;
};

#endif
