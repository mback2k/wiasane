/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
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
#include <algorithm>

WINSANE_Socket::WINSANE_Socket(_In_ SOCKET sock)
{
	this->sock = sock;
	this->conv = FALSE;
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
}

WINSANE_Socket::~WINSANE_Socket()
{
	this->Clear();
	this->Close();
}


SOCKET WINSANE_Socket::GetSocket()
{
	return this->sock;
}


VOID WINSANE_Socket::SetConverting(_In_ BOOL converting)
{
	this->conv = converting;
}

BOOL WINSANE_Socket::IsConverting()
{
	return this->conv;
}


DWORD WINSANE_Socket::Flush()
{
	DWORD result, offset, size;

	for (offset = 0; offset < this->buflen; ) {
		result = this->WriteSocket(this->buf + offset, this->buflen - offset);
		if (result > 0) {
			offset += result;
		} else {
			break;
		}
	}

	if (offset < this->buflen) {
		size = this->buflen - offset;
		memmove(this->buf, this->buf + offset, size);
		this->buf = this->ReallocBuffer(this->buf, this->buflen, size);
		this->buflen = size;
		this->bufoff = 0;
	} else {
		this->Clear();
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

PBYTE WINSANE_Socket::ReallocBuffer(_In_opt_ PBYTE buf, _In_opt_ DWORD oldlen, _In_ DWORD newlen)
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


int WINSANE_Socket::WriteSocket(_In_ CONST PBYTE buf, _In_ DWORD buflen)
{
	return send(this->sock, (const char*) buf, buflen, 0);
}

int WINSANE_Socket::ReadSocket(_In_ PBYTE buf, _In_ DWORD buflen)
{
	return recv(this->sock, (char*) buf, buflen, MSG_WAITALL);
}

int WINSANE_Socket::WritePlain(_In_ CONST PBYTE buf, _In_ DWORD buflen)
{
	DWORD space, size;

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

int WINSANE_Socket::ReadPlain(_In_ PBYTE buf, _In_ DWORD buflen)
{
	return this->ReadSocket(buf, buflen);
}


int WINSANE_Socket::Write(_In_ CONST PBYTE buf, _In_ DWORD buflen)
{
	PBYTE buftmp;
	int result;

	buftmp = (PBYTE) malloc(buflen);
	if (!buftmp)
		return 0;

	memset(buftmp, 0, buflen);
	memcpy(buftmp, buf, buflen);

	if (this->conv)
		std::reverse(buftmp, buftmp + buflen);

	result = this->WritePlain(buftmp, buflen);

	memset(buftmp, 0, buflen);
	free(buftmp);

	return result;
}

int WINSANE_Socket::Read(_In_ PBYTE buf, _In_ DWORD buflen)
{
	PBYTE buftmp;
	int result;

	buftmp = (PBYTE) malloc(buflen);
	if (!buftmp)
		return 0;

	memset(buftmp, 0, buflen);

	result = this->ReadPlain(buftmp, buflen);

	if (this->conv)
		std::reverse(buftmp, buftmp + buflen);

	memcpy(buf, buftmp, buflen);
	memset(buftmp, 0, buflen);
	free(buftmp);

	return result;
}


int WINSANE_Socket::WriteByte(_In_ SANE_Byte b)
{
	return this->Write((PBYTE) &b, sizeof(SANE_Byte));
}

int WINSANE_Socket::WriteWord(_In_ SANE_Word w)
{
	return this->Write((PBYTE) &w, sizeof(SANE_Word));
}

int WINSANE_Socket::WriteChar(_In_ SANE_Char c)
{
	return this->Write((PBYTE) &c, sizeof(SANE_Char));
}

int WINSANE_Socket::WriteString(_In_ SANE_String_Const s)
{
	SANE_Word length;
	int written;
	
	length = (SANE_Word) strlen(s) + 1;
	written = this->WriteWord(length);
	written += this->WritePlain((PBYTE) s, length);

	return written;
}

int WINSANE_Socket::WriteHandle(_In_ SANE_Handle h)
{
	return this->WriteWord((SANE_Word) h);
}

int WINSANE_Socket::WriteStatus(_In_ SANE_Status s)
{
	return this->WriteWord((SANE_Word) s);
}


SANE_Byte WINSANE_Socket::ReadByte()
{
	SANE_Byte b;

	if (this->Read((PBYTE) &b, sizeof(SANE_Byte)) != sizeof(SANE_Byte))
		b = 0;

	return b;
}

SANE_Word WINSANE_Socket::ReadWord()
{
	SANE_Word w;

	if (this->Read((PBYTE) &w, sizeof(SANE_Word)) != sizeof(SANE_Word))
		w = 0;

	return w;
}

SANE_Char WINSANE_Socket::ReadChar()
{
	SANE_Char c;

	if (this->Read((PBYTE) &c, sizeof(SANE_Char)) != sizeof(SANE_Char))
		c = 0;

	return c;
}

SANE_String WINSANE_Socket::ReadString()
{
	SANE_Word length;
	SANE_String s;

	length = this->ReadWord();
	if (length > 0) {
		s = new SANE_Char[length+1];
		if (this->ReadPlain((PBYTE) s, length) == length) {
			s[length] = '\0';
		} else {
			delete[] s;
			s = NULL;
		}
	} else {
		s = NULL;
	}

	return s;
}

SANE_Handle WINSANE_Socket::ReadHandle()
{
	return (SANE_Handle) this->ReadWord();
}

SANE_Status WINSANE_Socket::ReadStatus()
{
	return (SANE_Status) this->ReadWord();
}
