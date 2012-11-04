#pragma once

#include "winsane_socket.h"

class WINSANE_API WINSANE_Params {
public:
	WINSANE_Params(WINSANE_Device *device, SANE_Parameters *sane_params);
	~WINSANE_Params();

	/* Public API */
	SANE_Frame GetFormat();
	SANE_Bool IsLastFrame();
	SANE_Int GetBytesPerLine();
	SANE_Int GetPixelsPerLine();
	SANE_Int GetLines();
	SANE_Int GetDepth();

private:
	WINSANE_Device *device;
	WINSANE_Socket *sock;
	SANE_Parameters *sane_params;
};
