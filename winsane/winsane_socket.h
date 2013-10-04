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
	PBYTE ReallocBuffer(_In_ PBYTE buf, _In_ DWORD oldlen, _In_ DWORD newlen);
	VOID Close();

	SOCKET sock;
	PBYTE buf;
	DWORD buflen;
	DWORD bufoff;
	BOOL conv;
};

typedef WINSANE_Socket* PWINSANE_Socket;

#endif
