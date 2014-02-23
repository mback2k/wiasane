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

#include "winsane_session.h"
#include "winsane_internal.h"

#include <stdlib.h>
#include <tchar.h>

WINSANE_Session::WINSANE_Session(_In_ SOCKET sock)
{
	this->sock = new WINSANE_Socket(sock);
	this->initialized = FALSE;
	this->num_devices = 0;
	this->devices = NULL;
}

WINSANE_Session::~WINSANE_Session()
{
	if (this->num_devices > 0)
		this->ClearDevices();

	if (this->initialized)
		this->Exit();

	if (this->sock) {
		delete this->sock;
		this->sock = NULL;
	}

	this->initialized = FALSE;
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PADDRINFOT addrInfo)
{
	SOCKET sock;

	sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (sock == INVALID_SOCKET)
		return NULL;

	if (connect(sock, addrInfo->ai_addr, (int) addrInfo->ai_addrlen) != 0)
		return NULL;

	return new WINSANE_Session(sock);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PIN_ADDR addr)
{
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PIN_ADDR addr, _In_ USHORT port)
{
	SOCKADDR_IN sockAddr;
	ADDRINFOT addrInfo;

	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_addr = *addr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = port;

	ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.ai_addr = (PSOCKADDR) &sockAddr;
	addrInfo.ai_addrlen = sizeof(SOCKADDR);
	addrInfo.ai_family = AF_INET;
	addrInfo.ai_socktype = SOCK_STREAM;
	addrInfo.ai_protocol = IPPROTO_TCP;

	return WINSANE_Session::Remote(&addrInfo);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PIN6_ADDR addr)
{
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PIN6_ADDR addr, _In_ USHORT port)
{
	SOCKADDR_IN6 sockAddr;
	ADDRINFOT addrInfo;

	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin6_addr = *addr;
	sockAddr.sin6_family = AF_INET6;
	sockAddr.sin6_port = port;

	ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.ai_addr = (PSOCKADDR) &sockAddr;
	addrInfo.ai_addrlen = sizeof(SOCKADDR);
	addrInfo.ai_family = AF_INET6;
	addrInfo.ai_socktype = SOCK_STREAM;
	addrInfo.ai_protocol = IPPROTO_TCP;

	return WINSANE_Session::Remote(&addrInfo);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PTSTR hostname)
{
	return WINSANE_Session::Remote(hostname, WINSANE_DEFAULT_PORT);
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PTSTR hostname, _In_ USHORT port)
{
	PADDRINFOT pAddrInfo;
	ADDRINFOT addrInfoHint;
	WINSANE_Session* session;
	TCHAR port_str[10];

	ZeroMemory(&addrInfoHint, sizeof(addrInfoHint));
	addrInfoHint.ai_family = AF_UNSPEC;
	addrInfoHint.ai_socktype = SOCK_STREAM;
	addrInfoHint.ai_protocol = IPPROTO_TCP;

	if (_itot_s(port, port_str, 10, 10) != 0)
		return NULL;

	pAddrInfo = NULL;
	if (GetAddrInfo(hostname, port_str, &addrInfoHint, &pAddrInfo) != 0)
		return NULL;

	session = WINSANE_Session::Remote(pAddrInfo);

	FreeAddrInfo(pAddrInfo);

	return session;
}


BOOL WINSANE_Session::IsConnected()
{
	return this->sock->IsConnected();
}


SANE_Status WINSANE_Session::Init(_In_opt_ PSANE_Int version, _In_opt_ SANE_Auth_Callback authorize)
{
	SANE_Word version_code;
	SANE_Status status;
	CHAR user_name[SANE_MAX_USERNAME_LEN];
	DWORD user_name_len;
	unsigned short ns;
	unsigned char *p;
	LONG written;
	HRESULT hr;

	ns = 0x1234;
	p = (unsigned char*) &ns;
	this->sock->SetConverting(*p != 0x12);

	this->auth_callback = authorize;

	version_code = SANE_VERSION_CODE(SANE_CURRENT_MAJOR, SANE_CURRENT_MINOR, 0);
	if (version)
		*version = version_code;

	user_name_len = SANE_MAX_USERNAME_LEN;
	if (!GetUserNameA(user_name, &user_name_len))
		return SANE_STATUS_NO_MEM;

	written = this->sock->WriteWord(WINSANE_NET_INIT);
	written += this->sock->WriteWord(version_code);
	written += this->sock->WriteString(user_name);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&version_code);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	if (status != SANE_STATUS_GOOD)
		return status;

	this->initialized = TRUE;
	return SANE_STATUS_GOOD;
}

SANE_Status WINSANE_Session::Exit()
{
	LONG written;

	if (!this->initialized)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_EXIT);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	this->initialized = FALSE;
	return SANE_STATUS_GOOD;
}


int WINSANE_Session::GetDevices()
{
	SANE_Status status;
	SANE_Word array_length;
	SANE_Handle pointer;
	PSANE_Device *sane_devices;
	PSANE_Device sane_device;
	LONG written;
	HRESULT hr;
	int index;

	if (!this->initialized)
		return 0;

	written = this->sock->WriteWord(WINSANE_NET_GET_DEVICES);
	if (this->sock->Flush() != written)
		return 0;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return 0;

	hr = this->sock->ReadWord(&array_length);
	if (FAILED(hr))
		return 0;

	if (status != SANE_STATUS_GOOD)
		return 0;

	if (this->num_devices > 0)
		this->ClearDevices();

	this->num_devices = 0;

	sane_devices = new PSANE_Device[array_length];

	for (index = 0; index < array_length; index++) {
		hr = this->sock->ReadHandle(&pointer);
		if (FAILED(hr))
			break;

		if (pointer != NULL)
			continue;

		sane_device = new SANE_Device();

		hr = this->sock->ReadString((PSANE_String) &sane_device->name);
		if (FAILED(hr)) {
			delete sane_device;
			break;
		}

		hr = this->sock->ReadString((PSANE_String) &sane_device->vendor);
		if (FAILED(hr)) {
			delete sane_device;
			break;
		}

		hr = this->sock->ReadString((PSANE_String) &sane_device->model);
		if (FAILED(hr)) {
			delete sane_device;
			break;
		}

		hr = this->sock->ReadString((PSANE_String) &sane_device->type);
		if (FAILED(hr)) {
			delete sane_device;
			break;
		}

		sane_devices[this->num_devices++] = sane_device;
	}

	this->devices = new PWINSANE_Device[this->num_devices];

	for (index = 0; index < this->num_devices; index++) {
		this->devices[index] = new WINSANE_Device(this, this->sock, sane_devices[index]);
	}

	delete[] sane_devices;

	return this->num_devices;
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ int index)
{
	if (!this->initialized)
		return NULL;

	return this->devices[index];
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ SANE_String_Const name)
{
	SANE_String_Const device_name;
	int index;

	if (!this->initialized)
		return NULL;

	for (index = 0; index < this->num_devices; index++) {
		device_name = this->devices[index]->GetName();
		if (device_name && strcmp(name, device_name) == 0) {
			return this->devices[index];
		}
	}

	return NULL;
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ PTSTR ptName)
{
#ifdef UNICODE
	PWINSANE_Device device;
	SANE_String_Const name;
	HANDLE hHeap;
	int length;

	device = NULL;

	hHeap = GetProcessHeap();
	if (hHeap) {
		length = WideCharToMultiByte(CP_ACP, 0, ptName, -1, NULL, 0, NULL, NULL);
		if (length) {
			name = (SANE_String_Const) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, length);
			if (name) {
				length = WideCharToMultiByte(CP_ACP, 0, ptName, -1, (LPSTR) name, length, NULL, NULL);
				if (length) {
					device = WINSANE_Session::GetDevice(name);
				}
				HeapFree(hHeap, 0, (LPVOID) name);
			}
		}
	}

	return device;
#else
	return WINSANE_Session::GetDevice((SANE_String_Const) name);
#endif
}

VOID WINSANE_Session::ClearDevices()
{
	int index;

	for (index = 0; index < this->num_devices; index++) {
		delete this->devices[index];
	}

	if (this->devices) {
		delete[] this->devices;
		this->devices = NULL;
	}

	this->num_devices = 0;
}
