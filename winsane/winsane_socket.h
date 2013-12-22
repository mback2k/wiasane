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

#ifndef WINSANE_SOCKET_H
#define WINSANE_SOCKET_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <winsock2.h>

#include "sane.h"

class WINSANE_Socket {
public:
	/* Constructer & Deconstructer */
	WINSANE_Socket(_In_ SOCKET sock);
	~WINSANE_Socket();


	/* Internal API */
	SOCKET GetSocket();

	VOID SetConverting(_In_ BOOL converting);
	BOOL IsConverting();

	DWORD Flush();
	VOID Clear();

	int WritePlain(_In_ CONST PBYTE buf, _In_ DWORD buflen);
	int ReadPlain(_In_ PBYTE buf, _In_ DWORD buflen);

	int Write(_In_ CONST PBYTE buf, _In_ DWORD buflen);
	int Read(_In_ PBYTE buf, _In_ DWORD buflen);

	int WriteByte(_In_ SANE_Byte b);
	int WriteWord(_In_ SANE_Word w);
	int WriteChar(_In_ SANE_Char c);
	int WriteString(_In_ SANE_String_Const s);
	int WriteHandle(_In_ SANE_Handle h);
	int WriteStatus(_In_ SANE_Status s);

	SANE_Byte ReadByte();
	SANE_Word ReadWord();
	SANE_Char ReadChar();
	SANE_String ReadString();
	SANE_Handle ReadHandle();
	SANE_Status ReadStatus();


protected:
	int WriteSocket(_In_ CONST PBYTE buf, _In_ DWORD buflen);
	int ReadSocket(_In_ PBYTE buf, _In_ DWORD buflen);


private:
	PBYTE ReallocBuffer(_In_opt_ PBYTE buf, _In_opt_ DWORD oldlen, _In_ DWORD newlen);
	VOID Close();

	SOCKET sock;
	PBYTE buf;
	DWORD buflen;
	DWORD bufoff;
	BOOL conv;
};

typedef WINSANE_Socket* PWINSANE_Socket;

#endif
