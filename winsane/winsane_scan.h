#pragma once

#include "winsane_socket.h"

class WINSANE_API WINSANE_Scan {
public:
	WINSANE_Scan(WINSANE_Device *device, SANE_Word port, SANE_Word byte_order);
	~WINSANE_Scan();

private:
	WINSANE_Device *device;
	WINSANE_Socket *sock;
	SANE_Word port;
	SANE_Word byte_order;
};
