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

	if (this->sane_handle != INVALID_SANE_HANDLE)
		this->Close();

	if (this->sane_device->name)
		delete[] this->sane_device->name;
	if (this->sane_device->vendor)
		delete[] this->sane_device->vendor;
	if (this->sane_device->model)
		delete[] this->sane_device->model;
	if (this->sane_device->type)
		delete[] this->sane_device->type;

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


SANE_Status WINSANE_Device::Open()
{
	SANE_Status status;
	SANE_Handle handle;
	SANE_String resource;
	LONG written;
	HRESULT hr;

	written = this->sock->WriteWord(WINSANE_NET_OPEN);
	written += this->sock->WriteString(this->sane_device->name);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadHandle(&handle);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadString(&resource);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (resource) {
		if (strlen(resource) > 0)
			status = this->session->Authorize(resource);
		delete[] resource;
		if (status != SANE_STATUS_GOOD)
			return status;
	}

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	if (status != SANE_STATUS_GOOD)
		return status;

	this->sane_handle = handle;

	return SANE_STATUS_GOOD;
}

SANE_Status WINSANE_Device::Close()
{
	SANE_Word dummy;
	LONG written;
	HRESULT hr;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_CLOSE);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&dummy);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	this->sane_handle = INVALID_SANE_HANDLE;

	return SANE_STATUS_GOOD;
}

BOOL WINSANE_Device::IsOpen()
{
	return this->sane_handle != INVALID_SANE_HANDLE;
}


SANE_Status WINSANE_Device::FetchOptions()
{
	PSANE_Option_Descriptor *sane_options;
	PSANE_Option_Descriptor sane_option;
	SANE_Word num_options, num_values, null_pointer;
	SANE_Status status;
	LONG written;
	HRESULT hr;
	int index, value;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_GET_OPTION_DESCRIPTORS);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&num_options);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	status = SANE_STATUS_GOOD;

	if (this->num_options > 0)
		this->ClearOptions();

	this->num_options = 0;

	sane_options = new PSANE_Option_Descriptor[num_options];

	for (index = 0; index < num_options; index++) {
		hr = this->sock->ReadWord(&null_pointer);
		if (FAILED(hr))
			break;

		if (null_pointer)
			continue;

		sane_option = new SANE_Option_Descriptor();
		this->sock->ReadString((PSANE_String) &sane_option->name);
		this->sock->ReadString((PSANE_String) &sane_option->title);
		this->sock->ReadString((PSANE_String) &sane_option->desc);

		this->sock->ReadWord((PSANE_Word) &sane_option->type);
		this->sock->ReadWord((PSANE_Word) &sane_option->unit);
		this->sock->ReadWord((PSANE_Word) &sane_option->size);
		this->sock->ReadWord((PSANE_Word) &sane_option->cap);

		this->sock->ReadWord((PSANE_Word) &sane_option->constraint_type);

		switch (sane_option->constraint_type) {
			case SANE_CONSTRAINT_NONE:
				break;

			case SANE_CONSTRAINT_RANGE:
				hr = this->sock->ReadWord(&null_pointer);
				if (FAILED(hr) || null_pointer)
					break;

				sane_option->constraint.range = new SANE_Range();
				this->sock->ReadWord(&sane_option->constraint.range->min);
				this->sock->ReadWord(&sane_option->constraint.range->max);
				this->sock->ReadWord(&sane_option->constraint.range->quant);
				break;

			case SANE_CONSTRAINT_WORD_LIST:
				hr = this->sock->ReadWord(&num_values);
				if (FAILED(hr))
					break;

				sane_option->constraint.word_list = new SANE_Word[num_values];
				for (value = 0; value < num_values; value++) {
					this->sock->ReadWord(&sane_option->constraint.word_list[value]);
				}
				break;

			case SANE_CONSTRAINT_STRING_LIST:
				hr = this->sock->ReadWord(&num_values);
				if (FAILED(hr))
					break;

				sane_option->constraint.string_list = new SANE_String_Const[num_values];
				for (value = 0; value < num_values; value++) {
					this->sock->ReadString((PSANE_String) &sane_option->constraint.string_list[value]);
				}
				break;
		}

		sane_options[this->num_options++] = sane_option;
	}

	if (this->num_options > 0) {
		this->options = new PWINSANE_Option[this->num_options];

		for (index = 0; index < this->num_options; index++) {
			this->options[index] = new WINSANE_Option(this->session, this, this->sock, sane_options[index], this->sane_handle, index);
		}
	}

	delete[] sane_options;

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

		this->ClearOptions();
	}

	return status;
}

LONG WINSANE_Device::GetOptions()
{
	if (!this->options)
		return -1;

	return this->num_options;
}

PWINSANE_Option WINSANE_Device::GetOption(_In_ LONG index)
{
	if (!this->options)
		return NULL;

	if (index < 0 || index >= this->num_options)
		return NULL;

	return this->options[index];
}

PWINSANE_Option WINSANE_Device::GetOption(_In_ SANE_String_Const name)
{
	SANE_String_Const option_name;
	LONG index;

	if (!this->options)
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
	LONG index;

	for (index = 0; index < this->num_options; index++) {
		delete this->options[index];
	}

	if (this->options) {
		delete[] this->options;
		this->options = NULL;
	}

	this->num_options = 0;
}


SANE_Status WINSANE_Device::GetParams(_Outptr_result_maybenull_ PWINSANE_Params *params)
{
	SANE_Status status;
	SANE_Parameters *sane_params;
	LONG written;
	HRESULT hr;

	if (!params)
		return SANE_STATUS_INVAL;

	*params = NULL;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_GET_PARAMETERS);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	sane_params = new SANE_Parameters();

	this->sock->ReadWord((PSANE_Word) &sane_params->format);
	if (FAILED(hr))
		goto fail;

	this->sock->ReadWord(&sane_params->last_frame);
	if (FAILED(hr))
		goto fail;

	this->sock->ReadWord(&sane_params->bytes_per_line);
	if (FAILED(hr))
		goto fail;

	this->sock->ReadWord(&sane_params->pixels_per_line);
	if (FAILED(hr))
		goto fail;

	this->sock->ReadWord(&sane_params->lines);
	if (FAILED(hr))
		goto fail;

	this->sock->ReadWord(&sane_params->depth);
	if (FAILED(hr))
		goto fail;

	if (!this->sock->IsConnected())
		goto fail;

	if (status != SANE_STATUS_GOOD) {
		delete sane_params;
		return status;
	}

	*params = new WINSANE_Params(this, this->sock, sane_params);
	if (!*params)
		return SANE_STATUS_NO_MEM;

	return SANE_STATUS_GOOD;

fail:
	delete sane_params;
	return SANE_STATUS_IO_ERROR;
}


SANE_Status WINSANE_Device::Start(_Outptr_result_maybenull_ PWINSANE_Scan *scan)
{
	SANE_Status status;
	SANE_Word port;
	SANE_Word byte_order;
	SANE_String resource;
	LONG written;
	HRESULT hr;

	if (!scan)
		return SANE_STATUS_INVAL;

	*scan = NULL;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_START);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadStatus(&status);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&port);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&byte_order);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadString(&resource);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (resource) {
		if (strlen(resource) > 0)
			status = this->session->Authorize(resource);
		delete[] resource;
		if (status != SANE_STATUS_GOOD)
			return status;
	}

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	if (status != SANE_STATUS_GOOD)
		return status;

	*scan = new WINSANE_Scan(this, this->sock, port, byte_order);
	if (!*scan)
		return SANE_STATUS_NO_MEM;

	return SANE_STATUS_GOOD;
}

SANE_Status WINSANE_Device::Cancel()
{
	SANE_Word dummy;
	LONG written;
	HRESULT hr;

	if (this->sane_handle == INVALID_SANE_HANDLE)
		return SANE_STATUS_INVAL;

	written = this->sock->WriteWord(WINSANE_NET_CANCEL);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return SANE_STATUS_IO_ERROR;

	hr = this->sock->ReadWord(&dummy);
	if (FAILED(hr))
		return SANE_STATUS_IO_ERROR;

	if (!this->sock->IsConnected())
		return SANE_STATUS_IO_ERROR;

	return SANE_STATUS_GOOD;
}
