#ifndef WINSANE_SESSION_H
#define WINSANE_SESSION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane_socket.h"
#include "winsane_device.h"

class WINSANE_API WINSANE_Session {
public:
	/* Constructer & Deconstructer */
	WINSANE_Session(SOCKET sock);
	~WINSANE_Session();

	static WINSANE_Session* Remote(struct addrinfo *addrinfo);
	static WINSANE_Session* Remote(struct in_addr *addr);
	static WINSANE_Session* Remote(struct in_addr *addr, unsigned short port);
	static WINSANE_Session* Remote(struct in6_addr *addr);
	static WINSANE_Session* Remote(struct in6_addr *addr, unsigned short port);
	static WINSANE_Session* Remote(const char *hostname);
	static WINSANE_Session* Remote(const char *hostname, unsigned short port);


	/* Internal API */
	WINSANE_Socket* GetSocket();


	/* Public API */
	bool Init(SANE_Int *version_code, SANE_Auth_Callback authorize);
	bool Exit();

	int GetDevices();
	WINSANE_Device* GetDevice(int index);
	WINSANE_Device* GetDevice(SANE_String_Const name);
	void ClearDevices();


private:
	int num_devices;
	WINSANE_Device **devices;

	bool initialized;
	SANE_Auth_Callback auth_callback;
	WINSANE_Socket *sock;
};

#endif
