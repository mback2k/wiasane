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
	BOOL IsConnected();

	VOID SetConverting(_In_ BOOL converting);
	BOOL IsConverting();

	LONG Flush();
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

	HRESULT ReadByte(_Out_ PSANE_Byte b);
	HRESULT ReadWord(_Out_ PSANE_Word w);
	HRESULT ReadChar(_Out_ PSANE_Char c);
	HRESULT ReadString(_Out_ PSANE_String s);
	HRESULT ReadHandle(_Out_ PSANE_Handle h);
	HRESULT ReadStatus(_Out_ PSANE_Status s);


protected:
	LONG WriteSocket(_In_reads_bytes_(buflen) CONST PBYTE buf, _In_ LONG buflen);
	LONG ReadSocket(_Out_writes_bytes_(buflen) PBYTE buf, _In_ LONG buflen);


private:
	PBYTE ReallocBuffer(_In_opt_ PBYTE buf, _In_opt_ LONG oldlen, _In_ LONG newlen);
	VOID Close();

	SOCKET sock;
	PBYTE buf;
	ULONG buflen;
	ULONG bufoff;
	BOOL conv;
};

typedef WINSANE_Socket* PWINSANE_Socket;

#endif
