#include "winsane_option.h"
#include "winsane_internal.h"

WINSANE_Option::WINSANE_Option(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Option_Descriptor sane_option, _In_ SANE_Handle sane_handle, _In_ int index)
{
	this->device = device;
	this->sock = sock;
	this->sane_option = sane_option;
	this->sane_handle = sane_handle;
	this->index = index;
}

WINSANE_Option::~WINSANE_Option()
{
	int index;

	if (this->sane_option->name)
		delete this->sane_option->name;
	if (this->sane_option->title)
		delete this->sane_option->title;
	if (this->sane_option->desc)
		delete this->sane_option->desc;

	if (this->sane_option->constraint_type == SANE_CONSTRAINT_RANGE) {
		delete this->sane_option->constraint.range;
	} else if (this->sane_option->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
		delete this->sane_option->constraint.word_list;
	} else if (this->sane_option->constraint_type == SANE_CONSTRAINT_STRING_LIST) {
		for (index = 0; this->sane_option->constraint.string_list[index] != NULL; index++) {
			delete this->sane_option->constraint.string_list[index];
		}
		delete this->sane_option->constraint.string_list;
	}

	delete this->sane_option;

	this->device = NULL;
	this->sock = NULL;
}


SANE_String_Const WINSANE_Option::GetName()
{
	return this->sane_option->name;
}

SANE_String_Const WINSANE_Option::GetTitle()
{
	return this->sane_option->title;
}

SANE_String_Const WINSANE_Option::GetDescription()
{
	return this->sane_option->desc;
}


SANE_Value_Type WINSANE_Option::GetType()
{
	return this->sane_option->type;
}

SANE_Unit WINSANE_Option::GetUnit()
{
	return this->sane_option->unit;
}

SANE_Int WINSANE_Option::GetSize()
{
	return this->sane_option->size;
}

SANE_Int WINSANE_Option::GetCapabilities()
{
	return this->sane_option->cap;
}


SANE_Constraint_Type WINSANE_Option::GetConstraintType()
{
	return this->sane_option->constraint_type;
}

PSANE_Range WINSANE_Option::GetConstraintRange()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_RANGE)
		return NULL;

	return this->sane_option->constraint.range;
}

PSANE_Word WINSANE_Option::GetConstraintWordList()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_WORD_LIST)
		return NULL;

	return this->sane_option->constraint.word_list;
}

PSANE_String_Const WINSANE_Option::GetConstraintStringList()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_STRING_LIST)
		return NULL;

	return this->sane_option->constraint.string_list;
}

BOOL WINSANE_Option::IsValidValue(_In_ SANE_Word value)
{
	SANE_Word *word_list, word_list_length;
	SANE_Range *range;
	BOOL is_valid;
	int index;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
			is_valid = TRUE;
			range = this->sane_option->constraint.range;
			if (value < range->min)
				is_valid = FALSE;
			else if (value > range->max)
				is_valid = FALSE;
			else if (range->quant && ((value - range->min) % range->quant))
				is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = FALSE;
			word_list = this->sane_option->constraint.word_list;
			word_list_length = *word_list;
			for (index = 1; index <= word_list_length; index++) {
				if (value == word_list[index]) {
					is_valid = TRUE;
					break;
				}
			}
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_NONE:
		default:
			is_valid = TRUE;
			break;
	}

	return is_valid;
}

BOOL WINSANE_Option::IsValidValue(_In_ SANE_String value)
{
	SANE_String_Const *string_list;
	BOOL is_valid;
	int index;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			string_list = this->sane_option->constraint.string_list;
			for (index = 0; string_list[index] != NULL; index++) {
				if (strcmp(value, string_list[index]) == 0) {
					is_valid = TRUE;
					break;
				}
			}
			break;

		case SANE_CONSTRAINT_NONE:
		default:
			is_valid = TRUE;
			break;
	}

	return is_valid;
}


HRESULT WINSANE_Option::GetValue(_Inout_ double *value)
{
	SANE_Bool value_bool;
	SANE_Int value_int;
	SANE_Fixed value_fixed;
	HRESULT hr;

	if (!value)
		return E_INVALIDARG;

	switch (this->sane_option->type) {
		case SANE_TYPE_BOOL:
			hr = this->GetValueBool(&value_bool);
			if (SUCCEEDED(hr)) {
				*value = (double) value_bool;
			}
			break;

		case SANE_TYPE_INT:
			hr = this->GetValueInt(&value_int);
			if (SUCCEEDED(hr)) {
				*value = (double) value_int;
			}
			break;

		case SANE_TYPE_FIXED:
			hr = this->GetValueFixed(&value_fixed);
			if (SUCCEEDED(hr)) {
				*value = SANE_UNFIX(value_fixed);
			}
			break;

		default:
			hr = E_NOTIMPL;
			break;
	}

	return hr;
}

HRESULT WINSANE_Option::GetValueBool(_Inout_ PSANE_Bool value_bool)
{
	PVOID value;

	if (!value_bool)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return E_INVALIDARG;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Bool));
	if (value) {
		*value_bool = *((PSANE_Bool) value);
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueInt(_Inout_ PSANE_Int value_int)
{
	PVOID value;

	if (!value_int)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_INT)
		return E_INVALIDARG;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int));
	if (value) {
		*value_int = *((PSANE_Int) value);
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueFixed(_Inout_ PSANE_Fixed value_fixed)
{
	PVOID value;

	if (!value_fixed)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return E_INVALIDARG;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed));
	if (value) {
		*value_fixed = *((PSANE_Fixed) value);
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueString(_Inout_ PSANE_String value_string)
{
	PVOID value;

	if (!value_string)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return E_INVALIDARG;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size);
	if (value) {
		*value_string = (SANE_String) value;
		return S_OK;
	}

	return E_FAIL;
}


HRESULT WINSANE_Option::SetValue(_In_ double value)
{
	HRESULT hr;

	switch (this->sane_option->type) {
		case SANE_TYPE_BOOL:
			hr = this->SetValueBool((SANE_Bool) value);
			break;

		case SANE_TYPE_INT:
			hr = this->SetValueInt((SANE_Int) value);
			break;

		case SANE_TYPE_FIXED:
			hr = this->SetValueFixed(SANE_FIX(value));
			break;

		default:
			hr = E_NOTIMPL;
			break;
	}

	return hr;
}

HRESULT WINSANE_Option::SetValueBool(_In_ SANE_Bool value_bool)
{
	PVOID value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return E_INVALIDARG;

	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Bool), (PVOID) &value_bool);
	if (value) {
		hr = value_bool == *((PSANE_Bool) value) ? S_OK : S_FALSE;
		delete[] value;
		return hr;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::SetValueInt(_In_ SANE_Int value_int)
{
	PVOID value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_INT)
		return E_INVALIDARG;

	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int), (PVOID) &value_int);
	if (value) {
		hr = value_int == *((PSANE_Int) value) ? S_OK : S_FALSE;
		delete[] value;
		return hr;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::SetValueFixed(_In_ SANE_Fixed value_fixed)
{
	PVOID value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return E_INVALIDARG;

	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed), (PVOID) &value_fixed);
	if (value) {
		hr = value_fixed == *((PSANE_Fixed) value) ? S_OK : S_FALSE;
		delete[] value;
		return hr;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::SetValueString(_In_ SANE_String_Const value_string)
{
	SANE_Word value_size;
	PVOID value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return E_INVALIDARG;

	if (value_string)
		value_size = (SANE_Word) strlen(value_string) + 1;
	else
		value_size = 0;

	value = this->SetValue(this->sane_option->type, value_size, value_size, (PVOID) value_string);
	if (value_string && value)
		hr = strcmp(value_string, (SANE_String_Const) value) == 0 ? S_OK : FALSE;
	else
		hr = value_string == (SANE_String_Const) value ? S_OK : S_FALSE;

	return hr;
}


PVOID WINSANE_Option::GetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count)
{
	SANE_Status status;
	SANE_Word info, pointer;
	SANE_String resource;
	DWORD written;
	PBYTE buf;

	buf = new BYTE[value_size];
	memset(buf, 0, value_size);

	written = this->sock->WriteWord(WINSANE_NET_CONTROL_OPTION);
	written += this->sock->WriteHandle(this->sane_handle);
	written += this->sock->WriteWord(this->index);
	written += this->sock->WriteWord(SANE_ACTION_GET_VALUE);
	written += this->sock->WriteWord(value_type);
	written += this->sock->WriteWord(value_size);
	written += this->sock->WriteWord(element_count);
	if (value_type == SANE_TYPE_STRING)
		written += this->sock->WritePlain(buf, value_size);
	else
		written += this->sock->Write(buf, value_size);
	if (this->sock->Flush() != written) {
		delete[] buf;
		return NULL;
	}

	status = this->sock->ReadStatus();
	info = this->sock->ReadWord();
	value_type = this->sock->ReadWord();
	value_size = this->sock->ReadWord();
	pointer = this->sock->ReadWord();
	if (pointer) {
		if (value_type == SANE_TYPE_STRING)
			this->sock->ReadPlain(buf, value_size);
		else
			this->sock->Read(buf, value_size);
	}
	pointer = this->sock->ReadWord();
	if (pointer)
		resource = this->sock->ReadString();

	return buf;
}

PVOID WINSANE_Option::SetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _In_ PVOID value)
{
	SANE_Status status;
	SANE_Word info, pointer;
	SANE_String resource;
	DWORD written;
	PBYTE buf;

	buf = new BYTE[value_size];
	memset(buf, 0, value_size);
	memcpy(buf, value, value_size);

	written = this->sock->WriteWord(WINSANE_NET_CONTROL_OPTION);
	written += this->sock->WriteHandle(this->sane_handle);
	written += this->sock->WriteWord(this->index);
	written += this->sock->WriteWord(SANE_ACTION_SET_VALUE);
	written += this->sock->WriteWord(value_type);
	written += this->sock->WriteWord(value_size);
	written += this->sock->WriteWord(element_count);
	if (value_type == SANE_TYPE_STRING)
		written += this->sock->WritePlain(buf, value_size);
	else
		written += this->sock->Write(buf, value_size);
	if (this->sock->Flush() != written) {
		delete[] buf;
		return NULL;
	}

	status = this->sock->ReadStatus();
	info = this->sock->ReadWord();
	value_type = this->sock->ReadWord();
	value_size = this->sock->ReadWord();
	pointer = this->sock->ReadWord();
	if (pointer) {
		if (value_type == SANE_TYPE_STRING)
			this->sock->ReadPlain(buf, value_size);
		else
			this->sock->Read(buf, value_size);
	}
	pointer = this->sock->ReadWord();
	if (pointer)
		resource = this->sock->ReadString();

	return buf;
}
