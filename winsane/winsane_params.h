#ifndef WINSANE_PARAMS_H
#define WINSANE_PARAMS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane.h"
#include "winsane_socket.h"
#include "winsane_device.h"

class WINSANE_API WINSANE_Params {
public:
	WINSANE_Params(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Parameters sane_params);
	~WINSANE_Params();


	/* Public API */
	SANE_Frame GetFormat();
	SANE_Bool IsLastFrame();
	SANE_Int GetBytesPerLine();
	SANE_Int GetPixelsPerLine();
	SANE_Int GetLines();
	SANE_Int GetDepth();


private:
	PWINSANE_Device device;
	PWINSANE_Socket sock;
	PSANE_Parameters sane_params;
};

#endif
