#include "stdafx.h"
#include "winsane_scan.h"
#include "winsane_device.h"
#include "winsane_session.h"

WINSANE_Scan::WINSANE_Scan(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Word port, SANE_Word byte_order) {
	this->state = NEW;
	this->device = device;
	this->sock = sock;
	this->scan = NULL;
	this->port = port;
	this->byte_order = byte_order;
	this->buffered = 0;
}

WINSANE_Scan::~WINSANE_Scan() {
	this->Disconnect();
	this->device = NULL;
}


WINSANE_Scan_Result WINSANE_Scan::AquireImage(char *buffer, long *length) {
	WINSANE_Scan_Result result = SUCCESSFUL;

	switch (this->state) {
		case NEW:
			result = this->Connect();
			*length = 0;
			break;
		case CONNECTED:
		case SCANNING:
			result = this->Receive(buffer, length);
			break;
		case COMPLETED:
			result = this->Disconnect();
		case DISCONNECTED:
			*length = 0;
			break;
	}

	return result;
}


WINSANE_Scan_Result WINSANE_Scan::Connect() {
	SOCKET real_sock, scan_sock;
	struct sockaddr addr, *scanaddr;
	struct sockaddr_in addr_in;
	struct sockaddr_in6 addr_in6;
	unsigned char *p, *b;
	unsigned short ns;
	int addrlen;
	int result;

	real_sock = this->sock->GetSocket();

	addrlen = sizeof(addr);
	result = getpeername(real_sock, &addr, &addrlen);
	if (result)
		return CONNECTION_ERROR;

	if (addr.sa_family == AF_INET) {
		addrlen = sizeof(addr_in);
		scanaddr = (struct sockaddr*) &addr_in;
		result = getpeername(real_sock, scanaddr, &addrlen);
		if (result)
			return CONNECTION_ERROR;

		addr_in.sin_port = htons(this->port);
	} else if (addr.sa_family == AF_INET6) {
		addrlen = sizeof(addr_in6);
		scanaddr = (struct sockaddr*) &addr_in6;
		result = getpeername(real_sock, scanaddr, &addrlen);
		if (result)
			return CONNECTION_ERROR;

		addr_in6.sin6_port = htons(this->port);
	} else
		return CONNECTION_ERROR;

	scan_sock = socket(scanaddr->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (scan_sock == INVALID_SOCKET)
		return CONNECTION_ERROR;

	if (connect(scan_sock, scanaddr, addrlen) != 0) {
		closesocket(scan_sock);
		return CONNECTION_ERROR;
	}

	this->scan = new WINSANE_Socket(scan_sock);
	if (sock->IsConverting()) {
		this->scan->SetConverting(true);
	} else {
		ns = 0x1234;
		p = (unsigned char*) &ns;
		b = (unsigned char*) &this->byte_order;
		this->scan->SetConverting(*p != *b);
	}

	this->state = CONNECTED;
	return CONTINUE;
}

WINSANE_Scan_Result WINSANE_Scan::Receive(char *buffer, long *length) {
	unsigned int received, receive;

	if (this->buffered == 0) {
		if (this->scan->Read((char*) &received, sizeof(received)) != sizeof(received))
			return TRANSFER_ERROR;

		if (received == 0) {
			this->state = CONNECTED;
			return CONTINUE;
		}
		if (received == -1) {
			this->state = COMPLETED;
			return SUCCESSFUL;
		}
	} else
		received = this->buffered;

	if ((long) received > *length)
		receive = *length;
	else
		receive = received;

	*length = this->scan->Read(buffer, receive);
	this->buffered = received - *length;

	this->state = SCANNING;
	return CONTINUE;
}

WINSANE_Scan_Result WINSANE_Scan::Disconnect() {
	if (this->scan != NULL) {
		delete this->scan;
		this->scan = NULL;
	}

	this->state = DISCONNECTED;
	return SUCCESSFUL;
}
