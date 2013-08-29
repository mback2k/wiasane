#include "stdafx.h"
#include "winsane_device.h"

WINSANE_Device::WINSANE_Device(WINSANE_Session *session, WINSANE_Socket *sock, SANE_Device *sane_device) {
	this->opened = FALSE;
	this->started = FALSE;
	this->session = session;
	this->sock = sock;
	this->sane_device = sane_device;
	this->sane_handle = 0;
	this->num_options = 0;
	this->options = NULL;
}

WINSANE_Device::~WINSANE_Device() {
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

	this->opened = FALSE;
	this->started = FALSE;
	this->session = NULL;
	this->sock = NULL;
	this->sane_handle = 0;
	this->num_options = 0;
	this->options = NULL;
}


SANE_String_Const WINSANE_Device::GetName() {
	return this->sane_device->name;
}

SANE_String_Const WINSANE_Device::GetVendor() {
	return this->sane_device->vendor;
}

SANE_String_Const WINSANE_Device::GetModel() {
	return this->sane_device->model;
}

SANE_String_Const WINSANE_Device::GetType() {
	return this->sane_device->type;
}


bool WINSANE_Device::Open() {
	SANE_Status status;
	SANE_Handle handle;
	SANE_String resource;
	int written;

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

	this->opened = TRUE;
	return TRUE;
}

bool WINSANE_Device::Close() {
	int written;

	if (!this->opened)
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_CLOSE);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return FALSE;

	this->sock->ReadWord();

	this->opened = FALSE;
	return TRUE;
}


int WINSANE_Device::FetchOptions() {
	SANE_Option_Descriptor **sane_options;
	SANE_Word num_options, num_values, null_pointer;
	int written;

	if (!this->opened)
		return 0;

	written = this->sock->WriteWord(WINSANE_NET_GET_OPTION_DESCRIPTORS);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return 0;

	num_options = this->sock->ReadWord();
	if (this->num_options > 0)
		this->ClearOptions();

	this->num_options = 0;

	sane_options = new SANE_Option_Descriptor*[num_options];

	for (int index = 0; index < num_options; index++) {
		null_pointer = this->sock->ReadWord();
		if (null_pointer)
			continue;

		SANE_Option_Descriptor *sane_option = new SANE_Option_Descriptor();
		sane_option->name = this->sock->ReadString();
		sane_option->title = this->sock->ReadString();
		sane_option->desc = this->sock->ReadString();

		sane_option->type = (SANE_Value_Type)this->sock->ReadWord();
		sane_option->unit = (SANE_Unit)this->sock->ReadWord();
		sane_option->size = this->sock->ReadWord();
		sane_option->cap = this->sock->ReadWord();

		sane_option->constraint_type = (SANE_Constraint_Type)this->sock->ReadWord();
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
			for (int index = 0; index < num_values; index++) {
				sane_option->constraint.word_list[index] = this->sock->ReadWord();
			}
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			num_values = this->sock->ReadWord();
			sane_option->constraint.string_list = new SANE_String_Const[num_values];
			for (int index = 0; index < num_values; index++) {
				sane_option->constraint.string_list[index] = this->sock->ReadString();
			}
			break;
		}

		sane_options[index] = sane_option;
		this->num_options++;
	}

	this->options = new WINSANE_Option*[this->num_options];

	for (int index = 0; index < this->num_options; index++) {
		WINSANE_Option *option = new WINSANE_Option(this, this->sock, sane_options[index]);
		this->options[index] = option;
	}

	delete sane_options;

	return this->num_options;
}

WINSANE_Option* WINSANE_Device::GetOption(int index) {
	if (!this->opened)
		return NULL;

	return this->options[index];
}

void WINSANE_Device::ClearOptions() {
	for (int index = 0; index < this->num_options; index++) {
		delete this->options[index];
	}

	delete this->options;

	this->options = NULL;
	this->num_options = 0;
}


WINSANE_Params* WINSANE_Device::GetParams() {
	SANE_Status status;
	SANE_Parameters *sane_params;
	int written;

	if (!this->opened)
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


WINSANE_Scan* WINSANE_Device::Start() {
	SANE_Status status;
	SANE_Word port;
	SANE_Word byte_order;
	SANE_String resource;
	int written;

	if (!this->opened || this->started)
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

	this->started = TRUE;
	return new WINSANE_Scan(this, this->sock, port, byte_order);
}

bool WINSANE_Device::Cancel() {
	int written;

	if (!this->opened || !this->started)
		return FALSE;

	written = this->sock->WriteWord(WINSANE_NET_CANCEL);
	written += this->sock->WriteHandle(this->sane_handle);
	if (this->sock->Flush() != written)
		return FALSE;

	this->sock->ReadWord();

	this->started = FALSE;
	return TRUE;
}
