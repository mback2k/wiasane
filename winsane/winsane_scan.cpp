#include "winsane_scan.h"
#include "winsane_internal.h"

#include <stdlib.h>
#include <malloc.h>
#include <algorithm>

WINSANE_Scan::WINSANE_Scan(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Word port, SANE_Word byte_order) {
	unsigned char *p, *b;
	unsigned short ns;

	this->state = NEW;
	this->device = device;
	this->sock = sock;
	this->scan = NULL;
	this->port = port;
	this->byte_order = byte_order;
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
	this->bufpos = 0;

	ns = 0x1234;
	p = (unsigned char*) &ns;
	b = (unsigned char*) &this->byte_order;
	this->conv = *p != *b;
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
	int addrlen, result;

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

		addr_in.sin_port = htons((USHORT) this->port);
	} else if (addr.sa_family == AF_INET6) {
		addrlen = sizeof(addr_in6);
		scanaddr = (struct sockaddr*) &addr_in6;
		result = getpeername(real_sock, scanaddr, &addrlen);
		if (result)
			return CONNECTION_ERROR;

		addr_in6.sin6_port = htons((USHORT) this->port);
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
	this->scan->SetConverting(sock->IsConverting());

	this->state = CONNECTED;
	return CONTINUE;
}

WINSANE_Scan_Result WINSANE_Scan::Receive(char *buffer, long *length) {
	unsigned int record_size;
	unsigned long size;

	if (!this->buf || !this->buflen) {
		if (this->scan->Read((char*) &record_size, sizeof(record_size)) != sizeof(record_size))
			return TRANSFER_ERROR;

		if (record_size == 0) {
			this->state = CONNECTED;
			return CONTINUE;
		}
		if (record_size == -1) {
			this->state = COMPLETED;
			return SUCCESSFUL;
		}

		this->buf = (char *)malloc(record_size);
		this->buflen = record_size;
		this->bufoff = 0;
		this->bufpos = 0;

		while (this->bufoff < this->buflen) {
			size = this->scan->ReadPlain(this->buf + this->bufoff, this->buflen - this->bufoff);

			if (size == 0) {
				this->state = DISCONNECTED;
				return CONNECTION_ERROR;
			}
			if (size == SOCKET_ERROR) {
				this->state = DISCONNECTED;
				return CONNECTION_ERROR;
			}

			this->bufoff += size;
		}

		if (this->conv)
			std::reverse(this->buf, this->buf + this->bufoff);
	}

	if (this->buf && this->buflen > 0 && this->bufoff == this->buflen && this->bufpos < this->bufoff) {
		size = min(this->bufoff - this->bufpos, *length);

		if (size > 0) {
			memcpy(buffer, this->buf + this->bufpos, size);

			this->bufpos += size;

			if (this->bufpos == this->bufoff) {
				free(this->buf);
				this->buf = NULL;
				this->buflen = 0;
				this->bufoff = 0;
				this->bufpos = 0;
			}
		}

		*length = size;
	} else
		*length = 0;

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
