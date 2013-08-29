#ifndef WINSANE_DEVICE_H
#define WINSANE_DEVICE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "sane.h"
#include "winsane_session.h"
#include "winsane_socket.h"
#include "winsane_option.h"
#include "winsane_params.h"
#include "winsane_scan.h"

class WINSANE_API WINSANE_Device {
public:
	/* Constructer & Deconstructer */
	WINSANE_Device(WINSANE_Session *session, WINSANE_Socket *sock, SANE_Device *device);
	~WINSANE_Device();


	/* Public API */
	SANE_String_Const GetName();
	SANE_String_Const GetVendor();
	SANE_String_Const GetModel();
	SANE_String_Const GetType();

	bool Open();
	bool Close();

	int FetchOptions();
	WINSANE_Option* GetOption(int index);
	WINSANE_Option* GetOption(SANE_String_Const name);
	void ClearOptions();

	WINSANE_Params* GetParams();

	WINSANE_Scan* Start();
	bool Cancel();

private:
	int num_options;
	WINSANE_Option **options;

	bool opened;
	bool started;
	WINSANE_Session *session;
	WINSANE_Socket *sock;
	SANE_Device *sane_device;
	SANE_Handle sane_handle;
};

#endif
