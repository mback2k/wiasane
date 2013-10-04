#ifndef WINSANE_SCAN_H
#define WINSANE_SCAN_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane.h"
#include "winsane_socket.h"
#include "winsane_device.h"

enum WINSANE_Scan_State {
	NEW,
	CONNECTED,
	SCANNING,
	COMPLETED,
	DISCONNECTED
};

enum WINSANE_Scan_Result {
	CONTINUE,
	SUCCESSFUL,
	CONNECTION_ERROR,
	TRANSFER_ERROR,
};

class WINSANE_API WINSANE_Scan {
public:
	/* Constructer & Deconstructer */
	WINSANE_Scan(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ SANE_Word port, _In_ SANE_Word byte_order);
	~WINSANE_Scan();


	/* Public API */
	WINSANE_Scan_Result Connect();
	WINSANE_Scan_Result AquireImage(_Inout_ PBYTE buffer, _Inout_ PDWORD length);


protected:
	WINSANE_Scan_Result Receive(_Inout_ PBYTE buffer, _Inout_ PDWORD length);
	WINSANE_Scan_Result Disconnect();


private:
	WINSANE_Scan_State state;
	PWINSANE_Device device;
	PWINSANE_Socket sock, scan;
	SANE_Word port;
	SANE_Word byte_order;
	PBYTE buf;
	DWORD buflen;
	DWORD bufoff;
	DWORD bufpos;
	BOOL conv;
};

#endif
