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

#include "winsane_socket.h"
#include "winsane_internal.h"

#include <stdlib.h>
#include <malloc.h>

WINSANE_Socket::WINSANE_Socket(_In_ SOCKET sock)
{
	this->sock = sock;
	this->shut = FALSE;
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
}

WINSANE_Socket::~WINSANE_Socket()
{
	this->Clear();
	this->Disconnect();
	this->Close();
}


SOCKET WINSANE_Socket::GetSocket()
{
	return this->sock;
}

BOOL WINSANE_Socket::IsConnected()
{
	return this->sock != INVALID_SOCKET && this->shut != TRUE;
}

BOOL WINSANE_Socket::Disconnect()
{
	if (this->shut != TRUE) {
		if (this->sock != INVALID_SOCKET) {
			if (shutdown(this->sock, SD_BOTH) != 0) {
				return FALSE;
			}
		}
		this->shut = TRUE;
	}
	return TRUE;
}


LONG WINSANE_Socket::Flush()
{
	ULONG offset, size;
	LONG result;

	for (offset = 0; offset < this->buflen; ) {
		result = this->WriteSocket(this->buf + offset, this->buflen - offset);
		if (result > 0) {
			offset += result;
		} else {
			break;
		}
	}

	if (offset > 0) {
		if (offset < this->buflen) {
			size = this->buflen - offset;
			memmove(this->buf, this->buf + offset, size);
			this->buf = this->ReallocBuffer(this->buf, this->buflen, size);
			this->buflen = size;
			this->bufoff = 0;
		} else if (offset == this->buflen) {
			this->Clear();
		}
	}

	return offset;
}

VOID WINSANE_Socket::Clear()
{
	if (this->buf) {
		if (this->buflen) {
			memset(this->buf, 0, this->buflen);
			this->buflen = 0;
		}

		free(this->buf);
		this->buf = NULL;
	}

	this->bufoff = 0;
}

PBYTE WINSANE_Socket::ReallocBuffer(_In_opt_ PBYTE buf, _In_opt_ LONG oldlen, _In_ LONG newlen)
{
	PBYTE newbuf;

	newbuf = NULL;

	if (buf && oldlen) {
		if (newlen) {
			newbuf = (PBYTE) realloc(buf, newlen);
		} else {
			free(buf);
		}
	} else if (newlen) {
		newbuf = (PBYTE) malloc(newlen);
	}

	if (newbuf && newlen > oldlen) {
		memset(newbuf + oldlen, 0, newlen - oldlen);
	}

	return newbuf;
}

VOID WINSANE_Socket::Close()
{
	if (this->sock != INVALID_SOCKET) {
		closesocket(this->sock);
		this->sock = INVALID_SOCKET;
	}
}


LONG WINSANE_Socket::WriteSocket(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen)
{
	LONG result;

	result = send(this->sock, (const char*) buf, buflen, 0);

	if (result == SOCKET_ERROR) {
		switch (WSAGetLastError()) {
			case WSAENETDOWN:
			case WSAENETRESET:
			case WSAENOTCONN:
			case WSAESHUTDOWN:
			case WSAEHOSTUNREACH:
			case WSAECONNABORTED:
			case WSAECONNRESET:
			case WSAETIMEDOUT:
				this->Close();
				break;
		}
	}

	return result;
}

LONG WINSANE_Socket::ReadSocket(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen)
{
	LONG result;

	result = recv(this->sock, (char*) buf, buflen, MSG_WAITALL);

	if (result == SOCKET_ERROR) {
		switch (WSAGetLastError()) {
			case WSAENETDOWN:
			case WSAENOTCONN:
			case WSAENETRESET:
			case WSAESHUTDOWN:
			case WSAECONNABORTED:
			case WSAETIMEDOUT:
			case WSAECONNRESET:
				this->Close();
				break;
		}
	} else if (buflen > 0 && result == 0) {
		this->Close();
	}

	return result;
}


LONG WINSANE_Socket::Write(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen)
{
	LONG space, size;

	if (this->buf && this->buflen)
		space = this->buflen - this->bufoff;
	else
		space = 0;

	if (space < buflen) {
		size = this->bufoff + buflen;
		this->buf = this->ReallocBuffer(this->buf, this->buflen, size);
		this->buflen = size;
	}

	memcpy(this->buf + this->bufoff, buf, buflen);
	this->bufoff += buflen;

	return buflen;
}

LONG WINSANE_Socket::Read(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen)
{
	return this->ReadSocket(buf, buflen);
}


LONG WINSANE_Socket::WriteByte(_In_ SANE_Byte b)
{
	return this->Write((PBYTE) &b, sizeof(SANE_Byte));
}

LONG WINSANE_Socket::WriteWord(_In_ SANE_Word w)
{
	w = htonl(w);

	return this->Write((PBYTE) &w, sizeof(SANE_Word));
}

LONG WINSANE_Socket::WriteChar(_In_ SANE_Char c)
{
	return this->Write((PBYTE) &c, sizeof(SANE_Char));
}

LONG WINSANE_Socket::WriteString(_In_ SANE_String_Const s)
{
	SANE_Word length;
	LONG written;
	
	length = (SANE_Word) strlen(s) + 1;
	written = this->WriteWord(length);
	written += this->Write((PBYTE) s, length);

	return written;
}

LONG WINSANE_Socket::WriteHandle(_In_ SANE_Handle h)
{
	return this->WriteWord((SANE_Word) h);
}

LONG WINSANE_Socket::WriteStatus(_In_ SANE_Status s)
{
	return this->WriteWord((SANE_Word) s);
}


HRESULT WINSANE_Socket::ReadByte(_Out_ PSANE_Byte b)
{
	LONG readlen;

	if (!b)
		return E_INVALIDARG;

	*b = 0;

	readlen = this->Read((PBYTE) b, sizeof(SANE_Byte));
	if (readlen != sizeof(SANE_Byte))
		return E_FAIL;

	return S_OK;
}

HRESULT WINSANE_Socket::ReadWord(_Out_ PSANE_Word w)
{
	LONG readlen;

	if (!w)
		return E_INVALIDARG;

	*w = 0;

	readlen = this->Read((PBYTE) w, sizeof(SANE_Word));
	if (readlen != sizeof(SANE_Word))
		return E_FAIL;

	*w = ntohl(*w);

	return S_OK;
}

HRESULT WINSANE_Socket::ReadChar(_Out_ PSANE_Char c)
{
	LONG readlen;

	if (!c)
		return E_INVALIDARG;

	*c = 0;

	readlen = this->Read((PBYTE) c, sizeof(SANE_Char));
	if (readlen != sizeof(SANE_Char))
		return E_FAIL;

	return S_OK;
}

HRESULT WINSANE_Socket::ReadString(_Out_ PSANE_String s)
{
	SANE_Word length;
	HRESULT hr;

	if (!s)
		return E_INVALIDARG;

	hr = this->ReadWord(&length);
	if (FAILED(hr))
		return hr;

	if (length > 0) {
		*s = new SANE_Char[length+1];
		if (this->Read((PBYTE) *s, length) == length) {
			(*s)[length] = '\0';
		} else {
			delete[] *s;
			*s = NULL;
			return E_FAIL;
		}
	} else {
		*s = NULL;
	}

	return S_OK;
}

HRESULT WINSANE_Socket::ReadHandle(_Out_ PSANE_Handle h)
{
	SANE_Word w;
	HRESULT hr;

	if (!h)
		return E_INVALIDARG;

	hr = this->ReadWord(&w);
	if (FAILED(hr))
		return hr;

	*h = (SANE_Handle) w;
	return S_OK;
}

HRESULT WINSANE_Socket::ReadStatus(_Out_ PSANE_Status s)
{
	SANE_Word w;
	HRESULT hr;

	if (!s)
		return E_INVALIDARG;

	hr = this->ReadWord(&w);
	if (FAILED(hr))
		return hr;

	*s = (SANE_Status) w;
	return S_OK;
}
