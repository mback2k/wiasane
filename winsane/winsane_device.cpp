/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
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

#include "winsane_device.h"
#include "winsane_internal.h"

#define INVALID_SANE_HANDLE ((SANE_Handle)-1)

WINSANE_Device::WINSANE_Device(_In_ PWINSANE_Session session, _In_ PWINSANE_Socket sock, _In_ PSANE_Device sane_device)
{
	this->session = session;
	this->sock = sock;
	this->sane_device = sane_device;
	this->sane_handle = INVALID_SANE_HANDLE;
	this->num_options = 0;
	this->options = NULL;
}

WINSANE_Device::~WINSANE_Device()
{
	if (this->num_options > 0)
		this->ClearOptions();

	if (this->sane_device->name)
		delete this->sane_device->name;
	if (this->sane_device->vendor)
		delete this->sane_device->vendor;
	if (this->sane_device->model)
		delete this->sane_device->model;
	if (this->sane_device->type)
		delete this->sane_device->type;

	delete this->sane_device;

	this->session = NULL;
	this->sock = NULL;
	this->sane_handle = INVALID_SANE_HANDLE;
	this->num_options = 0;
	this->options = NULL;
}


SANE_String_Const WINSANE_Device::GetName()
{
	return this->sane_device->name;
}

SANE_String_Const WINSANE_Device::GetVendor()
{
	return this->sane_device->vendor;
}

SANE_String_Const WINSANE_Device::GetModel()
{
	return this->sane_device->model;
}

SANE_String_Const WINSANE_Device::GetType()
{
	return this->sane_device->type;
}


BOOL WINSANE_Device::Open()
{
	SANE_Status status;
	SANE_Handle handle;
	SANE_String resource;
	DWORD written;

	written = this->sock->WriteWord(WINSANE_NET_OPEN);
	written += this->sock->WriteString(this->sane_device->name);
	if (this->sock->Flush() != written)
		return FALSE;

	status = this->sock->ReadStatus();
	handle = this->sock->ReadHandle();
	resource = this->sock->ReadString();

	delete resource;

	if (status != SANE_STATUS_GOOD)
		return FALSE;

	this->sane_handle = handle;

	return TRUE;
}

BOOL WINSANE_Device::Close()
{
	DWORD written;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_CLOSE);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return FALSE;

	this->sock->ReadWord();

	this->sane_handle = INVALID_SANE_HANDLE;

	return TRUE;
}


int WINSANE_Device::FetchOptions()
{
	PSANE_Option_Descriptor *sane_options;
	PSANE_Option_Descriptor sane_option;
	SANE_Word num_options, num_values, null_pointer;
	DWORD written;
	int index, value;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return 0;

	written = this->sock->WriteWord(WINSANE_NET_GET_OPTION_DESCRIPTORS);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return 0;

	num_options = this->sock->ReadWord();
	if (this->num_options > 0)
		this->ClearOptions();

	this->num_options = 0;

	sane_options = new PSANE_Option_Descriptor[num_options];

	for (index = 0; index < num_options; index++) {
		null_pointer = this->sock->ReadWord();
		if (null_pointer)
			continue;

		sane_option = new SANE_Option_Descriptor();
		sane_option->name = this->sock->ReadString();
		sane_option->title = this->sock->ReadString();
		sane_option->desc = this->sock->ReadString();

		sane_option->type = (SANE_Value_Type) this->sock->ReadWord();
		sane_option->unit = (SANE_Unit) this->sock->ReadWord();
		sane_option->size = this->sock->ReadWord();
		sane_option->cap = this->sock->ReadWord();

		sane_option->constraint_type = (SANE_Constraint_Type) this->sock->ReadWord();
		switch (sane_option->constraint_type) {
		case SANE_CONSTRAINT_NONE:
			break;

		case SANE_CONSTRAINT_RANGE:
			null_pointer = this->sock->ReadWord();
			if (null_pointer)
				break;
			sane_option->constraint.range = new SANE_Range();
			sane_option->constraint.range->min = this->sock->ReadWord();
			sane_option->constraint.range->max = this->sock->ReadWord();
			sane_option->constraint.range->quant = this->sock->ReadWord();
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			num_values = this->sock->ReadWord();
			sane_option->constraint.word_list = new SANE_Word[num_values];
			for (value = 0; value < num_values; value++) {
				sane_option->constraint.word_list[value] = this->sock->ReadWord();
			}
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			num_values = this->sock->ReadWord();
			sane_option->constraint.string_list = new SANE_String_Const[num_values];
			for (value = 0; value < num_values; value++) {
				sane_option->constraint.string_list[value] = this->sock->ReadString();
			}
			break;
		}

		sane_options[index] = sane_option;
		this->num_options++;
	}

	this->options = new PWINSANE_Option[this->num_options];

	for (index = 0; index < this->num_options; index++) {
		this->options[index] = new WINSANE_Option(this, this->sock, sane_options[index], this->sane_handle, index);
	}

	delete[] sane_options;

	return this->num_options;
}

PWINSANE_Option WINSANE_Device::GetOption(_In_ int index)
{
	if (this->sane_handle == INVALID_SANE_HANDLE || !this->options)
		return NULL;

	return this->options[index];
}

PWINSANE_Option WINSANE_Device::GetOption(_In_ SANE_String_Const name)
{
	SANE_String_Const option_name;
	int index;

	if (this->sane_handle == INVALID_SANE_HANDLE || !this->options)
		return NULL;

	for (index = 0; index < this->num_options; index++) {
		option_name = this->options[index]->GetName();
		if (option_name && strcmp(name, option_name) == 0) {
			return this->options[index];
		}
	}

	return NULL;
}

VOID WINSANE_Device::ClearOptions()
{
	int index;

	for (index = 0; index < this->num_options; index++) {
		delete this->options[index];
	}

	delete this->options;

	this->options = NULL;
	this->num_options = 0;
}


PWINSANE_Params WINSANE_Device::GetParams()
{
	SANE_Status status;
	SANE_Parameters *sane_params;
	DWORD written;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return NULL;

	written = this->sock->WriteWord(WINSANE_NET_GET_PARAMETERS);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return NULL;

	status = this->sock->ReadStatus();

	sane_params = new SANE_Parameters();
	sane_params->format = (SANE_Frame)this->sock->ReadWord();
	sane_params->last_frame = this->sock->ReadWord() == 1;
	sane_params->bytes_per_line = this->sock->ReadWord();
	sane_params->pixels_per_line = this->sock->ReadWord();
	sane_params->lines = this->sock->ReadWord();
	sane_params->depth = this->sock->ReadWord();

	if (status != SANE_STATUS_GOOD) {
		delete sane_params;
		return NULL;
	}

	return new WINSANE_Params(this, this->sock, sane_params);
}


PWINSANE_Scan WINSANE_Device::Start()
{
	SANE_Status status;
	SANE_Word port;
	SANE_Word byte_order;
	SANE_String resource;
	DWORD written;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return NULL;

	written = this->sock->WriteWord(WINSANE_NET_START);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return NULL;

	status = this->sock->ReadStatus();
	port = this->sock->ReadWord();
	byte_order = this->sock->ReadWord();
	resource = this->sock->ReadString();

	delete resource;

	if (status != SANE_STATUS_GOOD)
		return NULL;

	return new WINSANE_Scan(this, this->sock, port, byte_order);
}

BOOL WINSANE_Device::Cancel()
{
	DWORD written;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_CANCEL);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return FALSE;

	this->sock->ReadWord();

	return TRUE;
}
