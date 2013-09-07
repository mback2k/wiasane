#include "stdafx.h"
#include "winsane_socket.h"

#include <stdlib.h>
#include <malloc.h>
#include <algorithm>

WINSANE_Socket::WINSANE_Socket(SOCKET sock) {
	this->sock = sock;
	this->conv = FALSE;
	this->buf = NULL;
	this->buflen = 0;
	this->bufoff = 0;
}

WINSANE_Socket::~WINSANE_Socket() {
	this->Clear();
	this->Close();
}


SOCKET WINSANE_Socket::GetSocket() {
	return this->sock;
}


void WINSANE_Socket::SetConverting(bool converting) {
	this->conv = converting;
}

bool WINSANE_Socket::IsConverting() {
	return this->conv;
}


int WINSANE_Socket::Flush() {
	int size = 0, offset = 0, result = 0;

	while (offset < this->buflen) {
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

void WINSANE_Socket::Clear() {
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

char* WINSANE_Socket::ReallocBuffer(char *buf, int oldlen, int newlen) {
	char *newbuf = NULL;

	if (buf && oldlen) {
		if (newlen) {
			newbuf = (char *)realloc(buf, newlen);
		} else {
			free(buf);
		}
	} else if (newlen) {
		newbuf = (char *)malloc(newlen);
	}

	if (newbuf && newlen > oldlen) {
		memset(newbuf + oldlen, 0, newlen - oldlen);
	}

	return newbuf;
}

void WINSANE_Socket::Close() {
	if (this->sock != INVALID_SOCKET) {
		closesocket(this->sock);
		this->sock = INVALID_SOCKET;
	}
}


int WINSANE_Socket::WriteSocket(const char *buf, int buflen) {
	return send(this->sock, buf, buflen, 0);
}

int WINSANE_Socket::ReadSocket(char *buf, int buflen) {
	return recv(this->sock, buf, buflen, MSG_WAITALL);
}

int WINSANE_Socket::WritePlain(const char *buf, int buflen) {
	int size = 0, space = 0;

	if (this->buf && this->buflen)
		space = this->buflen - this->bufoff;

	if (space < buflen) {
		size = this->bufoff + buflen;
		this->buf = this->ReallocBuffer(this->buf, this->buflen, size);
		this->buflen = size;
	}

	memcpy(this->buf + this->bufoff, buf, buflen);
	this->bufoff += buflen;

	return buflen;
}

int WINSANE_Socket::ReadPlain(char *buf, int buflen) {
	return this->ReadSocket(buf, buflen);
}


int WINSANE_Socket::Write(const char *buf, int buflen) {
	char *buftmp;
	int result;

	buftmp = (char *)malloc(buflen);
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

int WINSANE_Socket::Read(char *buf, int buflen) {
	char *buftmp;
	int result;

	buftmp = (char *)malloc(buflen);
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


int WINSANE_Socket::WriteByte(SANE_Byte b) {
	return this->Write((char *)&b, sizeof(SANE_Byte));
}

int WINSANE_Socket::WriteWord(SANE_Word w) {
	return this->Write((char *)&w, sizeof(SANE_Word));
}

int WINSANE_Socket::WriteChar(SANE_Char c) {
	return this->Write(&c, sizeof(SANE_Char));
}

int WINSANE_Socket::WriteString(SANE_String_Const s) {
	int written = 0;
	SANE_Word l = (SANE_Word)strlen(s) + 1;
	written += this->WriteWord(l);
	written += this->WritePlain(s, l);
	return written;
}

int WINSANE_Socket::WriteHandle(SANE_Handle h) {
	return this->WriteWord((SANE_Word)h);
}

int WINSANE_Socket::WriteStatus(SANE_Status s) {
	return this->WriteWord((SANE_Word)s);
}


SANE_Byte WINSANE_Socket::ReadByte() {
	SANE_Byte b = 0;
	this->Read((char *)&b, sizeof(SANE_Byte));
	return b;
}

SANE_Word WINSANE_Socket::ReadWord() {
	SANE_Word w = 0;
	this->Read((char *)&w, sizeof(SANE_Word));
	return w;
}

SANE_Char WINSANE_Socket::ReadChar() {
	SANE_Char c = 0;
	this->Read(&c, sizeof(SANE_Char));
	return c;
}

SANE_String WINSANE_Socket::ReadString() {
	SANE_Word l = 0;
	SANE_String s = NULL;
	l = this->ReadWord();
	if (l) {
		s = new SANE_Char[l+1];
		this->ReadPlain(s, l);
		s[l] = '\0';
	}
	return s;
}

SANE_Handle WINSANE_Socket::ReadHandle() {
	SANE_Handle h = NULL;
	h = (SANE_Handle)this->ReadWord();
	return h;
}

SANE_Status WINSANE_Socket::ReadStatus() {
	SANE_Status s = SANE_STATUS_GOOD;
	s = (SANE_Status)this->ReadWord();
	return s;
}
