#pragma once

#include <algorithm>

#include "winsane_internal.h"

enum WINSANE_Socket_Type {
	NO_CONVERSION,
	LITTLE_TO_BIG,
	BIG_TO_LITTLE
};

class WINSANE_Socket {
public:
	WINSANE_Socket(SOCKET sock);
	~WINSANE_Socket();

	void SetType(WINSANE_Socket_Type type);
	WINSANE_Socket_Type GetType();

	int Flush();
	void Clear();

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

	int WritePlain(const char *buf, int buflen);
	int ReadPlain(char *buf, int buflen);

private:
	char* WINSANE_Socket::ReallocBuffer(char *buf, int oldlen, int newlen);

	WINSANE_Socket_Type type;
	SOCKET sock;
	char *buf;
	int buflen;
	int bufoff;
};
