/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2014, Marc Hoersken, <info@marc-hoersken.de>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this software distribution.
 *
 * You may opt to use, copy, modify, and distribute this software for any
 * purpose with or without fee, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either expressed or implied.
 *
 ***************************************************************************/

#include "winsane_scan.h"
#include "winsane_params.h"
#include "winsane_internal.h"

#include <stdlib.h>
#include <malloc.h>
#include <algorithm>

WINSANE_Scan::WINSANE_Scan(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ SANE_Word port, _In_ SANE_Word byte_order)
{
	this->state = NEW;
	this->device = device;
	this->params = NULL;
	this->sock = sock;
	this->scan = NULL;
	this->port = port;
	this->byte_order = byte_order;
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
	this->bufpos = 0;
}

WINSANE_Scan::~WINSANE_Scan()
{
	if (this->buf) {
		free(this->buf);
	}
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
	this->bufpos = 0;
	this->Disconnect();
	this->device = NULL;
	this->params = NULL;
	this->sock = NULL;
	this->scan = NULL;
}


SANE_Status WINSANE_Scan::AquireImage(_Inout_ PBYTE buffer, _Inout_ PDWORD length)
{
	SANE_Status status;

	switch (this->state) {
		case NEW:
			status = this->Connect();
			if (length)
				*length = 0;
			break;
		case CONNECTED:
		case SCANNING:
			status = this->Receive(buffer, length);
			break;
		case COMPLETED:
			status = this->Disconnect();
			if (length)
				*length = 0;
			break;
		case DISCONNECTED:
		default:
			status = SANE_STATUS_EOF;
			if (length)
				*length = 0;
			break;
	}

	return status;
}


SANE_Status WINSANE_Scan::Connect()
{
	SOCKET sock, scan_sock;
	SOCKADDR addr, *scan_addr;
	SOCKADDR_IN addr_in;
	SOCKADDR_IN6 addr_in6;
	int addrlen, result;

	sock = this->sock->GetSocket();

	addrlen = sizeof(addr);
	result = getpeername(sock, &addr, &addrlen);
	if (result)
		return SANE_STATUS_IO_ERROR;

	if (addr.sa_family == AF_INET) {
		addrlen = sizeof(addr_in);
		scan_addr = (PSOCKADDR) &addr_in;
		result = getpeername(sock, scan_addr, &addrlen);
		if (result)
			return SANE_STATUS_IO_ERROR;

		addr_in.sin_port = htons((USHORT) this->port);
	} else if (addr.sa_family == AF_INET6) {
		addrlen = sizeof(addr_in6);
		scan_addr = (PSOCKADDR) &addr_in6;
		result = getpeername(sock, scan_addr, &addrlen);
		if (result)
			return SANE_STATUS_IO_ERROR;

		addr_in6.sin6_port = htons((USHORT) this->port);
	} else
		return SANE_STATUS_IO_ERROR;

	scan_sock = socket(scan_addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (scan_sock == INVALID_SOCKET)
		return SANE_STATUS_IO_ERROR;

	if (connect(scan_sock, scan_addr, addrlen) != 0) {
		closesocket(scan_sock);
		return SANE_STATUS_IO_ERROR;
	}

	this->scan = new WINSANE_Socket(scan_sock);
	this->state = CONNECTED;
	return this->device->GetParams(&this->params);
}

SANE_Status WINSANE_Scan::Receive(_Inout_ PBYTE buffer, _Inout_ PDWORD length)
{
	UINT32 record_size;
	DWORD size, index;

	if (this->state == COMPLETED)
		return SANE_STATUS_EOF;

	if (!this->buf || !this->buflen) {
		if (this->scan->Read((PBYTE) &record_size, sizeof(record_size)) != sizeof(record_size))
			return SANE_STATUS_IO_ERROR;

		record_size = ntohl(record_size);

		if (record_size == 0) {
			this->state = CONNECTED;
			return SANE_STATUS_GOOD;
		}
		if (record_size == -1) {
			this->state = COMPLETED;
			return SANE_STATUS_EOF;
		}

		this->buf = (PBYTE) malloc(record_size);
		if (!this->buf)
			return SANE_STATUS_NO_MEM;

		this->buflen = record_size;
		this->bufoff = 0;
		this->bufpos = 0;

		while (this->bufoff < this->buflen) {
			size = this->scan->Read(this->buf + this->bufoff, this->buflen - this->bufoff);

			if (size == 0) {
				this->state = DISCONNECTED;
				return SANE_STATUS_IO_ERROR;
			}
			if (size == SOCKET_ERROR) {
				this->state = DISCONNECTED;
				return SANE_STATUS_IO_ERROR;
			}

			this->bufoff += size;
		}

		if (this->bufoff == this->buflen && this->byte_order != 0x1234) {
			size = this->params->GetDepth();
			if (size >= 16 && (size % 8) == 0) {
				size /= 8;
				if ((this->bufoff % size) != 0) {
					return SANE_STATUS_IO_ERROR;
				}
				for (index = 0; index < this->bufoff; index += size) {
					std::reverse(this->buf + index, this->buf + index + size);
				}
			}
		}
	}

	if (this->buf && this->buflen && this->bufoff == this->buflen && this->bufpos < this->bufoff) {
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
	return SANE_STATUS_GOOD;
}

SANE_Status WINSANE_Scan::Disconnect()
{
	if (this->params != NULL) {
		delete this->params;
		this->params = NULL;
	}

	if (this->scan != NULL) {
		delete this->scan;
		this->scan = NULL;
	}

	this->state = DISCONNECTED;
	return SANE_STATUS_EOF;
}


SANE_Word WINSANE_Scan::GetByteOrder()
{
	return this->byte_order;
}
