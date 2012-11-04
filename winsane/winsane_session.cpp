#include "stdafx.h"
#include "winsane_session.h"

WINSANE_Session::WINSANE_Session(SOCKET sock) {
	this->initialized = FALSE;
	this->real_sock = sock;
	if (this->real_sock != INVALID_SOCKET)
		this->sock = new WINSANE_Socket(this->real_sock);
	else
		throw;
	this->num_devices = 0;
	this->devices = NULL;
}

WINSANE_Session::~WINSANE_Session() {
	if (this->num_devices > 0)
		this->ClearDevices();

	if (this->initialized)
		this->Exit();

	if (this->real_sock != INVALID_SOCKET) {
		delete this->sock;
		closesocket(this->real_sock);
	}

	this->initialized = FALSE;
	this->real_sock = INVALID_SOCKET;
}

WINSANE_Session* WINSANE_Session::Remote(struct addrinfo *addrinfo) {
	SOCKET sock = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
	if (sock == INVALID_SOCKET)
		return NULL;

	if (connect(sock, addrinfo->ai_addr, (int)addrinfo->ai_addrlen) != 0)
		return NULL;

	return new WINSANE_Session(sock);
}

WINSANE_Session* WINSANE_Session::Remote(struct in_addr *addr) {
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(struct in_addr *addr, unsigned short port) {
	struct sockaddr_in sockaddr;
	struct addrinfo addrinfo;

	ZeroMemory(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_addr = *addr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = port;

	ZeroMemory(&addrinfo, sizeof(addrinfo));
	addrinfo.ai_addr = (struct sockaddr*) &sockaddr;
	addrinfo.ai_addrlen = sizeof(sockaddr);
	addrinfo.ai_family = AF_INET;
	addrinfo.ai_socktype = SOCK_STREAM;
	addrinfo.ai_protocol = IPPROTO_TCP;

	return WINSANE_Session::Remote(&addrinfo);
}

WINSANE_Session* WINSANE_Session::Remote(struct in6_addr *addr) {
	return WINSANE_Session::Remote(addr, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(struct in6_addr *addr, unsigned short port) {
	struct sockaddr_in6 sockaddr;
	struct addrinfo addrinfo;

	ZeroMemory(&sockaddr, sizeof(sockaddr));
	sockaddr.sin6_addr = *addr;
	sockaddr.sin6_family = AF_INET6;
	sockaddr.sin6_port = port;

	ZeroMemory(&addrinfo, sizeof(addrinfo));
	addrinfo.ai_addr = (struct sockaddr*) &sockaddr;
	addrinfo.ai_addrlen = sizeof(sockaddr);
	addrinfo.ai_family = AF_INET6;
	addrinfo.ai_socktype = SOCK_STREAM;
	addrinfo.ai_protocol = IPPROTO_TCP;

	return WINSANE_Session::Remote(&addrinfo);
}

WINSANE_Session* WINSANE_Session::Remote(const char *hostname) {
	return WINSANE_Session::Remote(hostname, WINSANE_DEFAULT_PORT);
}

WINSANE_Session* WINSANE_Session::Remote(const char *hostname, unsigned short port) {
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	char port_str[10];

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (_itoa_s(port, port_str, 10, 10) != 0)
		return NULL;

	if (getaddrinfo(hostname, port_str, &hints, &result) != 0)
		return NULL;

	return WINSANE_Session::Remote(result);

	freeaddrinfo(result);
}


WINSANE_Socket* WINSANE_Session::GetSocket() {
	return this->sock;
}


bool WINSANE_Session::Init(SANE_Int *version, SANE_Auth_Callback authorize) {
	SANE_Word version_code = SANE_VERSION_CODE(SANE_CURRENT_MAJOR, SANE_CURRENT_MINOR, 0);
	SANE_Status status = SANE_STATUS_GOOD;
	CHAR user_name[SANE_MAX_USERNAME_LEN];
	DWORD user_name_len = SANE_MAX_USERNAME_LEN;

	GetUserNameA(user_name, &user_name_len);

	{
		short ns = 0x1234;
		unsigned char *p = (unsigned char *)(&ns);

		if (version)
			*version = version_code;

		if (*p == 0x12)
			this->sock->SetType(NO_CONVERSION);
		else
			this->sock->SetType(LITTLE_TO_BIG);

		this->auth_callback = authorize;
	}

	{
		int written = 0;
		written += this->sock->WriteWord(WINSANE_NET_INIT);
		written += this->sock->WriteWord(version_code);
		written += this->sock->WriteString(user_name);

		if (this->sock->Flush() != written)
			return FALSE;
	}

	{
		status = this->sock->ReadStatus();
		version_code = this->sock->ReadWord();

		if (status != SANE_STATUS_GOOD)
			return FALSE;
	}

	this->initialized = TRUE;
	return TRUE;
}

bool WINSANE_Session::Exit() {
	if (!this->initialized)
		return FALSE;

	{
		int written = 0;
		written += this->sock->WriteWord(WINSANE_NET_EXIT);
	
		if (this->sock->Flush() != written)
			return FALSE;
	}

	this->initialized = FALSE;
	return TRUE;
}


int WINSANE_Session::GetDevices() {
	SANE_Status status = SANE_STATUS_GOOD;
	SANE_Word array_length = 0;
	SANE_Handle pointer = NULL;
	SANE_Device **sane_devices = NULL;

	if (!this->initialized)
		return 0;

	{
		int written = 0;
		written += this->sock->WriteWord(WINSANE_NET_GET_DEVICES);
	
		if (this->sock->Flush() != written)
			return 0;
	}

	{
		status = this->sock->ReadStatus();
		array_length = this->sock->ReadWord();

		if (status != SANE_STATUS_GOOD)
			return 0;

		if (this->num_devices > 0)
			this->ClearDevices();

		this->num_devices = 0;

		sane_devices = new SANE_Device*[array_length];

		for (int index = 0; index < array_length; index++) {
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

		for (int index = 0; index < this->num_devices; index++) {
			WINSANE_Device *device = new WINSANE_Device(this, sane_devices[index]);
			this->devices[index] = device;
		}

		delete sane_devices;
	}

	return this->num_devices;
}

WINSANE_Device* WINSANE_Session::GetDevice(int index) {
	if (!this->initialized)
		return NULL;

	return this->devices[index];
}

void WINSANE_Session::ClearDevices() {
	for (int index = 0; index < this->num_devices; index++) {
		delete this->devices[index];
	}

	delete this->devices;

	this->num_devices = 0;
	this->devices = NULL;
}
