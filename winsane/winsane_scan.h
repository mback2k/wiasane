#pragma once

#include "winsane_socket.h"

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
	WINSANE_Scan(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Word port, SANE_Word byte_order);
	~WINSANE_Scan();


	/* Public API */
	WINSANE_Scan_Result AquireImage(char *buffer, long *length);


protected:
	WINSANE_Scan_Result Connect();
	WINSANE_Scan_Result Receive(char *buffer, long *length);
	WINSANE_Scan_Result Disconnect();


private:
	WINSANE_Scan_State state;
	WINSANE_Device *device;
	WINSANE_Socket *sock, *scan;
	SANE_Word port;
	SANE_Word byte_order;
	unsigned int buffered;
};
