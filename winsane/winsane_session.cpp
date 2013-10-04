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

WINSANE_Session* WINSANE_Session::Remote(_In_ PADDRINFOT addrInfo)
{
	SOCKET sock;

	sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (sock == INVALID_SOCKET)
		return NULL;

	if (connect(sock, addrInfo->ai_addr, (int) addrInfo->ai_addrlen) != 0)
		return NULL;

	return new WINSANE_Session(sock);
}

WINSANE_Session* WINSANE_Session::Remote(_In_ PIN_ADDR addr)
{
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(_In_ PIN_ADDR addr, _In_ USHORT port)
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

WINSANE_Session* WINSANE_Session::Remote(_In_ PIN6_ADDR addr)
{
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(_In_ PIN6_ADDR addr, _In_ USHORT port)
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

WINSANE_Session* WINSANE_Session::Remote(_In_ PTSTR hostname)
{
	return WINSANE_Session::Remote(hostname, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(_In_ PTSTR hostname, _In_ USHORT port)
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


WINSANE_Socket* WINSANE_Session::GetSocket()
{
	return this->sock;
}


BOOL WINSANE_Session::Init(_In_ SANE_Int *version, _In_ SANE_Auth_Callback authorize)
{
	SANE_Word version_code;
	SANE_Status status;
	CHAR user_name[SANE_MAX_USERNAME_LEN];
	DWORD user_name_len;
	unsigned short ns;
	unsigned char *p;
	int written;

	ns = 0x1234;
	p = (unsigned char*) &ns;
	this->sock->SetConverting(*p != 0x12);

	this->auth_callback = authorize;

	version_code = SANE_VERSION_CODE(SANE_CURRENT_MAJOR, SANE_CURRENT_MINOR, 0);
	if (version)
		*version = version_code;

	user_name_len = SANE_MAX_USERNAME_LEN;
	if (!GetUserNameA(user_name, &user_name_len))
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_INIT);
	written += this->sock->WriteWord(version_code);
	written += this->sock->WriteString(user_name);
	if (this->sock->Flush() != written)
		return FALSE;

	status = this->sock->ReadStatus();
	version_code = this->sock->ReadWord();
	if (status != SANE_STATUS_GOOD)
		return FALSE;

	this->initialized = TRUE;
	return TRUE;
}

BOOL WINSANE_Session::Exit()
{
	int written;

	if (!this->initialized)
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_EXIT);
	if (this->sock->Flush() != written)
		return FALSE;

	this->initialized = FALSE;
	return TRUE;
}


int WINSANE_Session::GetDevices()
{
	SANE_Status status;
	SANE_Word array_length;
	SANE_Handle pointer;
	SANE_Device **sane_devices;
	int written, index;

	if (!this->initialized)
		return 0;

	written = this->sock->WriteWord(WINSANE_NET_GET_DEVICES);
	if (this->sock->Flush() != written)
		return 0;

	status = this->sock->ReadStatus();
	array_length = this->sock->ReadWord();

	if (status != SANE_STATUS_GOOD)
		return 0;

	if (this->num_devices > 0)
		this->ClearDevices();

	this->num_devices = 0;

	sane_devices = new SANE_Device*[array_length];

	for (index = 0; index < array_length; index++) {
		pointer = this->sock->ReadHandle();
		if (pointer != NULL)
			continue;

		SANE_Device *sane_device = new SANE_Device();
		sane_device->name = this->sock->ReadString();
		sane_device->vendor = this->sock->ReadString();
		sane_device->model = this->sock->ReadString();
		sane_device->type = this->sock->ReadString();
		sane_devices[index] = sane_device;

		this->num_devices++;
	}

	this->devices = new WINSANE_Device*[this->num_devices];

	for (index = 0; index < this->num_devices; index++) {
		WINSANE_Device *device = new WINSANE_Device(this, this->sock, sane_devices[index]);
		this->devices[index] = device;
	}

	delete[] sane_devices;

	return this->num_devices;
}

WINSANE_Device* WINSANE_Session::GetDevice(_In_ int index)
{
	if (!this->initialized)
		return NULL;

	return this->devices[index];
}

WINSANE_Device* WINSANE_Session::GetDevice(_In_ SANE_String_Const name)
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

WINSANE_Device* WINSANE_Session::GetDevice(_In_ PTSTR ptName)
{
#ifdef UNICODE
	WINSANE_Device* device;
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

	delete this->devices;

	this->num_devices = 0;
	this->devices = NULL;
}
