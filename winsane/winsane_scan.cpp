#include "stdafx.h"
#include "winsane_scan.h"
#include "winsane_device.h"
#include "winsane_session.h"

WINSANE_Scan::WINSANE_Scan(WINSANE_Device *device, SANE_Word port, SANE_Word byte_order) {
	this->device = device;
	this->sock = this->device->GetSession()->GetSocket();
	this->port = port;
	this->byte_order = byte_order;
}

WINSANE_Scan::~WINSANE_Scan() {
	this->port = 0;
	this->byte_order = 0;

	this->device = NULL;
	this->sock = NULL;
}
