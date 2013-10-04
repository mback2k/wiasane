#ifndef WINSANE_DEVICE_H
#define WINSANE_DEVICE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane.h"
#include "winsane_session.h"
#include "winsane_socket.h"
#include "winsane_option.h"
#include "winsane_params.h"
#include "winsane_scan.h"

class WINSANE_API WINSANE_Device {
public:
	/* Constructer & Deconstructer */
	WINSANE_Device(_In_ PWINSANE_Session session, _In_ PWINSANE_Socket sock, _In_ PSANE_Device device);
	~WINSANE_Device();


	/* Public API */
	SANE_String_Const GetName();
	SANE_String_Const GetVendor();
	SANE_String_Const GetModel();
	SANE_String_Const GetType();

	BOOL Open();
	BOOL Close();

	int FetchOptions();
	PWINSANE_Option GetOption(_In_ int index);
	PWINSANE_Option GetOption(_In_ SANE_String_Const name);
	VOID ClearOptions();

	PWINSANE_Params GetParams();

	PWINSANE_Scan Start();
	BOOL Cancel();

private:
	int num_options;
	PWINSANE_Option *options;

	BOOL opened;
	BOOL started;
	PWINSANE_Session session;
	PWINSANE_Socket sock;
	PSANE_Device sane_device;
	SANE_Handle sane_handle;
};

#endif
