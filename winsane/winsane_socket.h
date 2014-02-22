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

	LONG WritePlain(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen);
	LONG ReadPlain(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen);

	LONG Write(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen);
	LONG Read(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen);

	LONG WriteByte(_In_ SANE_Byte b);
	LONG WriteWord(_In_ SANE_Word w);
	LONG WriteChar(_In_ SANE_Char c);
	LONG WriteString(_In_ SANE_String_Const s);
	LONG WriteHandle(_In_ SANE_Handle h);
	LONG WriteStatus(_In_ SANE_Status s);

	SANE_Byte ReadByte();
	SANE_Word ReadWord();
	SANE_Char ReadChar();
	SANE_String ReadString();
	SANE_Handle ReadHandle();
	SANE_Status ReadStatus();


protected:
	LONG WriteSocket(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen);
	LONG ReadSocket(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen);


private:
	PBYTE ReallocBuffer(_In_opt_ PBYTE buf, _In_opt_ LONG oldlen, _In_ LONG newlen);
	VOID Close();

	SOCKET sock;
	PBYTE buf;
	DWORD buflen;
	DWORD bufoff;
	BOOL conv;
};

typedef WINSANE_Socket* PWINSANE_Socket;

#endif
