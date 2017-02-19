/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2015, Marc Hoersken, <info@marc-hoersken.de>
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

#include <wincrypt.h>
#include <stdlib.h>
#include <tchar.h>

#include "winsane-util_str.h"

#define WINSANE_AUTH_MD5 "$MD5$"
#define WINSANE_AUTH_MD5_FORMAT "$MD5$%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"

WINSANE_Session::WINSANE_Session(_In_ SOCKET sock)
{
	this->auth_callback_ex_userdata = NULL;
	this->auth_callback_ex = NULL;
	this->auth_callback = NULL;
	this->sock = new WINSANE_Socket(sock);
	this->initialized = FALSE;
	this->devices = NULL;
	this->num_devices = 0;
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
	this->auth_callback = NULL;
	this->auth_callback_ex = NULL;
	this->auth_callback_ex_userdata = NULL;
}

PWINSANE_Session WINSANE_Session::Remote(_In_ PADDRINFOT addrInfo)
{
	SOCKET sock;

	sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (sock == INVALID_SOCKET) {
		return NULL;
	}

	if (connect(sock, addrInfo->ai_addr, (int) addrInfo->ai_addrlen) != 0) {
		closesocket(sock);
		return NULL;
	}

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


SANE_Status WINSANE_Session::Init(_In_opt_ PSANE_Int version, _In_opt_ SANE_Auth_Callback authorize)
{
	SANE_Word version_code;
	SANE_Status status;
	CHAR user_name[SANE_MAX_USERNAME_LEN];
	DWORD user_name_len;
	LONG written;
	HRESULT hr;

	this->auth_callback = authorize;

	if (!this->sock->IsConnected()) {
		if (!this->sock->Reconnect()) {
			return SANE_STATUS_IO_ERROR;
		}
	}

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

SANE_Status WINSANE_Session::InitEx(_In_opt_ PSANE_Int version, _In_opt_ WINSANE_Auth_Callback authorize, _In_opt_ void *userdata)
{
	this->auth_callback_ex = authorize;
	this->auth_callback_ex_userdata = userdata;

	return this->Init(version, NULL);
}

SANE_Status WINSANE_Session::Authorize(_In_ SANE_String resource)
{
	SANE_Char username[SANE_MAX_USERNAME_LEN+1];
	SANE_Char password[SANE_MAX_PASSWORD_LEN+1];
	SANE_Char *md5pos, *md5salt;
	SANE_Word dummy;
	DWORD md5len, reslen, usrlen, pwdlen;
	BYTE md5hash[16];
	LONG written;
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	HRESULT hr;

	if ((!this->auth_callback && !this->auth_callback_ex) || !resource)
		return SANE_STATUS_ACCESS_DENIED;

	reslen = (DWORD) strlen(resource);
	if (!reslen)
		return SANE_STATUS_ACCESS_DENIED;

	ZeroMemory(username, sizeof(username));
	ZeroMemory(password, sizeof(password));
	if (this->auth_callback_ex)
		this->auth_callback_ex(resource, username, password, this->auth_callback_ex_userdata);
	else if (this->auth_callback)
		this->auth_callback(resource, username, password);
	username[SANE_MAX_USERNAME_LEN] = 0;
	password[SANE_MAX_PASSWORD_LEN] = 0;

	usrlen = (DWORD) strnlen_s(username, SANE_MAX_USERNAME_LEN);
	if (!usrlen)
		return SANE_STATUS_ACCESS_DENIED;

	pwdlen = (DWORD) strnlen_s(password, SANE_MAX_PASSWORD_LEN);
	if (!pwdlen)
		return SANE_STATUS_ACCESS_DENIED;

	md5pos = strstr(resource, WINSANE_AUTH_MD5);
	if (md5pos) {
		md5len = (DWORD) strnlen_s(WINSANE_AUTH_MD5, sizeof(WINSANE_AUTH_MD5));
		md5salt = md5pos + md5len;
		if (md5salt > md5pos && md5salt < (resource + reslen)) {
			if (CryptAcquireContext(&hCryptProv, NULL, NULL,
									PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
				if (CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) {
					md5len = (DWORD) strnlen_s(md5salt, reslen - (md5salt - resource));
					CryptHashData(hHash, (const BYTE*) md5salt, md5len, 0);
					CryptHashData(hHash, (const BYTE*) password, pwdlen, 0);
					md5len = sizeof(md5hash);
					ZeroMemory(md5hash, md5len);
					if (CryptGetHashParam(hHash, HP_HASHVAL, md5hash, &md5len, 0)) {
						sprintf_s(password, SANE_MAX_PASSWORD_LEN, WINSANE_AUTH_MD5_FORMAT,
								md5hash[0], md5hash[1], md5hash[2], md5hash[3],
								md5hash[4], md5hash[5], md5hash[6], md5hash[7],
								md5hash[8], md5hash[9], md5hash[10], md5hash[11],
								md5hash[12], md5hash[13], md5hash[14], md5hash[15]);
					}
					CryptDestroyHash(hHash);
				}
				CryptReleaseContext(hCryptProv, 0);
			}
		}
	}

	written = this->sock->WriteWord(WINSANE_NET_AUTHORIZE);
	written += this->sock->WriteString(resource);
	written += this->sock->WriteString(username);
	written += this->sock->WriteString(password);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&dummy);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	return SANE_STATUS_GOOD;
}

BOOL WINSANE_Session::IsInitialized()
{
	return this->initialized && this->sock->IsConnected();
}

SANE_Status WINSANE_Session::Exit()
{
	LONG written;

	if (!this->initialized)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_EXIT);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	if (!this->sock->Disconnect())
		return SANE_STATUS_IO_ERROR;

	this->initialized = FALSE;
	this->auth_callback = NULL;
	this->auth_callback_ex = NULL;
	this->auth_callback_ex_userdata = NULL;
	return SANE_STATUS_GOOD;
}


SANE_Status WINSANE_Session::FetchDevices()
{
	SANE_Status status;
	SANE_Word array_length;
	SANE_Handle pointer;
	PSANE_Device *sane_devices;
	PSANE_Device sane_device;
	LONG written, index;
	HRESULT hr;

	if (!this->initialized)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_GET_DEVICES);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&array_length);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (status != SANE_STATUS_GOOD)
		return status;

	if (array_length < 1 || array_length > 255)
		return SANE_STATUS_INVAL;

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
		this->sock->ReadString((PSANE_String) &sane_device->name);
		this->sock->ReadString((PSANE_String) &sane_device->vendor);
		this->sock->ReadString((PSANE_String) &sane_device->model);
		this->sock->ReadString((PSANE_String) &sane_device->type);

		sane_devices[this->num_devices++] = sane_device;
	}

	if (this->num_devices > 0) {
		this->devices = new PWINSANE_Device[this->num_devices];

		for (index = 0; index < this->num_devices; index++) {
			this->devices[index] = new WINSANE_Device(this, this->sock, sane_devices[index]);
		}
	}

	delete[] sane_devices;

	if (FAILED(hr)) {
		switch (hr) {
			case E_ABORT:
				status = SANE_STATUS_CANCELLED;
				break;
			case E_INVALIDARG:
				status = SANE_STATUS_INVAL;
				break;
			case E_NOTIMPL:
				status = SANE_STATUS_UNSUPPORTED;
				break;
			case E_OUTOFMEMORY:
				status = SANE_STATUS_NO_MEM;
				break;
			default:
				status = SANE_STATUS_IO_ERROR;
				break;
		}

		this->ClearDevices();
	}

	return status;
}

LONG WINSANE_Session::GetDevices()
{
	if (!this->initialized || !this->devices)
		return -1;

	return this->num_devices;
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ LONG index)
{
	if (!this->initialized || !this->devices)
		return NULL;

	if (index < 0 || index >= this->num_devices)
		return NULL;

	return this->devices[index];
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ SANE_String_Const name)
{
	SANE_String_Const device_name;
	LONG index;

	if (!this->initialized || !this->devices)
		return NULL;

	for (index = 0; index < this->num_devices; index++) {
		device_name = this->devices[index]->GetName();
		if (device_name && strcmp(name, device_name) == 0) {
			return this->devices[index];
		}
	}

	return NULL;
}

PWINSANE_Device WINSANE_Session::GetDevice(_In_ PTSTR pszName)
{
	PWINSANE_Device device;
	SANE_String_Const name;
	HANDLE hHeap;

	device = NULL;

	hHeap = GetProcessHeap();
	if (hHeap) {
		name = (SANE_String_Const) StringConvToA(hHeap, pszName);
		if (name) {
			device = this->GetDevice(name);
			HeapFree(hHeap, 0, (LPVOID) name);
		}
	}

	return device;
}

VOID WINSANE_Session::ClearDevices()
{
	LONG index;

	for (index = 0; index < this->num_devices; index++) {
		delete this->devices[index];
	}

	if (this->devices) {
		delete[] this->devices;
		this->devices = NULL;
	}

	this->num_devices = 0;
}


PWINSANE_Device WINSANE_Session::CreateDevice(_In_ SANE_String_Const name)
{
	PWINSANE_Device device;
	SANE_String dev_name;
	size_t length;

	if (!this->initialized)
		return NULL;

	device = NULL;

	if (name) {
		length = strlen(name);
		if (length > 0) {
			dev_name = new SANE_Char[length+1];
			if (dev_name) {
				strncpy_s(dev_name, length+1, name, length);
				dev_name[length] = '\0';
				device = WINSANE_Device::Create(this, this->sock, dev_name);
			}
		}
	}

	return device;
}

PWINSANE_Device WINSANE_Session::CreateDevice(_In_ PTSTR pszName)
{
	PWINSANE_Device device;
	SANE_String_Const name;
	HANDLE hHeap;

	device = NULL;

	hHeap = GetProcessHeap();
	if (hHeap) {
		name = (SANE_String_Const) StringConvToA(hHeap, pszName);
		if (name) {
			device = this->CreateDevice(name);
			HeapFree(hHeap, 0, (LPVOID) name);
		}
	}

	return device;
}
