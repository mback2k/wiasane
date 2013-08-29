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
	WINSANE_Socket(SOCKET sock);
	~WINSANE_Socket();


	/* Internal API */
	SOCKET GetSocket();

	void SetConverting(bool converting);
	bool IsConverting();

	int Flush();
	void Clear();

	int WritePlain(const char *buf, int buflen);
	int ReadPlain(char *buf, int buflen);

	int Write(const char *buf, int buflen);
	int Read(char *buf, int buflen);

	int WriteByte(SANE_Byte b);
	int WriteWord(SANE_Word w);
	int WriteChar(SANE_Char c);
	int WriteString(SANE_String_Const s);
	int WriteHandle(SANE_Handle h);
	int WriteStatus(SANE_Status s);

	SANE_Byte ReadByte();
	SANE_Word ReadWord();
	SANE_Char ReadChar();
	SANE_String ReadString();
	SANE_Handle ReadHandle();
	SANE_Status ReadStatus();


protected:
	int WriteSocket(const char *buf, int buflen);
	int ReadSocket(char *buf, int buflen);


private:
	char* ReallocBuffer(char *buf, int oldlen, int newlen);
	void Close();

	SOCKET sock;
	char *buf;
	int buflen;
	int bufoff;
	bool conv;
};

#endif
