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

#include "winsane_option.h"
#include "winsane_internal.h"

#include <math.h>

WINSANE_Option::WINSANE_Option(_In_ PWINSANE_Session session, _In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Option_Descriptor sane_option, _In_ SANE_Handle sane_handle, _In_ int index)
{
	this->session = session;
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
		delete[] this->sane_option->name;
	if (this->sane_option->title)
		delete[] this->sane_option->title;
	if (this->sane_option->desc)
		delete[] this->sane_option->desc;

	if (this->sane_option->constraint_type == SANE_CONSTRAINT_RANGE) {
		delete this->sane_option->constraint.range;
	} else if (this->sane_option->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
		delete[] this->sane_option->constraint.word_list;
	} else if (this->sane_option->constraint_type == SANE_CONSTRAINT_STRING_LIST) {
		for (index = 0; this->sane_option->constraint.string_list[index] != NULL; index++) {
			delete[] this->sane_option->constraint.string_list[index];
		}
		delete[] this->sane_option->constraint.string_list;
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


BOOL WINSANE_Option::IsValidValue(_In_ double value)
{
	BOOL is_valid;

	switch (this->sane_option->type) {
		case SANE_TYPE_BOOL:
			is_valid = this->IsValidValueBool((SANE_Bool) value);
			break;

		case SANE_TYPE_INT:
			is_valid = this->IsValidValueInt((SANE_Int) value);
			break;

		case SANE_TYPE_FIXED:
			is_valid = this->IsValidValueFixed(SANE_FIX(value));
			break;

		default:
			is_valid = FALSE;
			break;
	}

	return is_valid;
}

BOOL WINSANE_Option::IsValidValue(_In_ SANE_Word value)
{
	SANE_Word *word_list, word_list_length, word, temp;
	SANE_Range *range;
	BOOL is_valid;
	int index;

	if (this->sane_option->type == SANE_TYPE_STRING)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
			is_valid = TRUE;
			range = this->sane_option->constraint.range;
			word = value;
			if (word < range->min) {
				is_valid = FALSE;
			} else if (word > range->max) {
				is_valid = FALSE;
			} else if (range->quant) {
				temp = (unsigned int) (word - range->min + range->quant / 2) / range->quant;
				temp = temp * range->quant + range->min;
				is_valid = word == temp;
			}
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = FALSE;
			word_list = this->sane_option->constraint.word_list;
			word_list_length = *word_list;
			word = value;
			for (index = 1; index <= word_list_length; index++) {
				if (word == word_list[index]) {
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

BOOL WINSANE_Option::IsValidValueBool(_In_ SANE_Bool value_bool)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->IsValidValue(value_bool);
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_NONE:
		default:
			if (value_bool == SANE_TRUE || value_bool == SANE_FALSE) {
				is_valid = TRUE;
			} else {
				is_valid = FALSE;
			}
			break;
	}

	return is_valid;
}

BOOL WINSANE_Option::IsValidValueInt(_In_ SANE_Int value_int)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_INT)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->IsValidValue(value_int);
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

BOOL WINSANE_Option::IsValidValueFixed(_In_ SANE_Fixed value_fixed)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->IsValidValue(value_fixed);
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

BOOL WINSANE_Option::IsValidValueString(_In_ SANE_String_Const value_string)
{
	SANE_String_Const *string_list;
	BOOL is_valid;
	size_t length;
	int index;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			string_list = this->sane_option->constraint.string_list;
			length = strlen(value_string);
			for (index = 0; string_list[index] != NULL; index++) {
				if (strncmp(value_string, string_list[index], length) == 0 &&
					length == strlen(string_list[index])) {
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


BOOL WINSANE_Option::ConstrainValue(_Inout_ double *value, _Inout_ PSANE_Word info)
{
	SANE_Bool value_bool;
	SANE_Int value_int;
	SANE_Fixed value_fixed;
	BOOL is_valid;

	switch (this->sane_option->type) {
		case SANE_TYPE_BOOL:
			value_bool = *value ? SANE_TRUE : SANE_FALSE;
			is_valid = this->ConstrainValueBool(&value_bool, info);
			*value = value_bool;
			break;

		case SANE_TYPE_INT:
			value_int = (SANE_Int) *value;
			is_valid = this->ConstrainValueInt(&value_int, info);
			*value = value_int;
			break;

		case SANE_TYPE_FIXED:
			value_fixed = SANE_FIX(*value);
			is_valid = this->ConstrainValueFixed(&value_fixed, info);
			*value = SANE_UNFIX(value_fixed);
			break;

		default:
			is_valid = FALSE;
			break;
	}

	return is_valid;
}

BOOL WINSANE_Option::ConstrainValue(_Inout_ PSANE_Word value, _Inout_ PSANE_Word info)
{
	SANE_Word *word_list, word_list_length, word, val1, val2;
	SANE_Range *range;
	BOOL is_valid;
	int index, match;

	if (this->sane_option->type == SANE_TYPE_STRING)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
			is_valid = TRUE;
			range = this->sane_option->constraint.range;
			word = *value;
			if (word < range->min) {
				*value = range->min;
				if (info) {
					*info |= SANE_INFO_INEXACT;
				}
			} else if (word > range->max) {
				*value = range->max;
				if (info) {
					*info |= SANE_INFO_INEXACT;
				}
			} else if (range->quant) {
				val1 = (unsigned int) (word - range->min + range->quant / 2) / range->quant;
				val1 = val1 * range->quant + range->min;
				if (val1 > range->max) {
					val1 = range->max;
				}
				if (val1 != word) {
					*value = val1;
					if (info) {
						*info |= SANE_INFO_INEXACT;
					}
				}
			}
			break;

		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = TRUE;
			word_list = this->sane_option->constraint.word_list;
			word_list_length = *word_list;
			word = *value;
			match = 1;
			val1 = abs(word - word_list[1]);
			for (index = 1; index <= word_list_length; index++) {
				if ((val2 = abs(word - word_list[index])) < val1) {
					val1 = val2;
					match = index;
				}
			}
			if (word != word_list[match]) {
				*value = word_list[match];
				if (info) {
					*info |= SANE_INFO_INEXACT;
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

BOOL WINSANE_Option::ConstrainValueBool(_Inout_ PSANE_Bool value_bool, _Inout_ PSANE_Word info)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->ConstrainValue(value_bool, info);
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_NONE:
		default:
			if (*value_bool == SANE_TRUE || *value_bool == SANE_FALSE) {
				is_valid = TRUE;
			} else {
				is_valid = FALSE;
			}
			break;
	}

	return is_valid;
}

BOOL WINSANE_Option::ConstrainValueInt(_Inout_ PSANE_Int value_int, _Inout_ PSANE_Word info)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_INT)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->ConstrainValue(value_int, info);
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

BOOL WINSANE_Option::ConstrainValueFixed(_Inout_ PSANE_Fixed value_fixed, _Inout_ PSANE_Word info)
{
	BOOL is_valid;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = this->ConstrainValue(value_fixed, info);
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

BOOL WINSANE_Option::ConstrainValueString(_Inout_ SANE_String value_string, _Inout_ PSANE_Word info)
{
	SANE_String_Const *string_list;
	BOOL is_valid;
	size_t length, index_length, match_length;
	int index, match, num_matches;

	UNREFERENCED_PARAMETER(info);

	if (this->sane_option->type != SANE_TYPE_STRING)
		return FALSE;

	switch (this->sane_option->constraint_type) {
		case SANE_CONSTRAINT_RANGE:
		case SANE_CONSTRAINT_WORD_LIST:
			is_valid = FALSE;
			break;

		case SANE_CONSTRAINT_STRING_LIST:
			is_valid = FALSE;
			string_list = this->sane_option->constraint.string_list;
			length = strlen(value_string);
			num_matches = 0;
			match = -1;
			match_length = 0;
			for (index = 0; string_list[index] != NULL; index++) {
				if (_strnicmp(value_string, string_list[index], length) == 0 &&
					length <= (index_length = strlen(string_list[index]))) {
						match = index;
						match_length = index_length;
						if (length == match_length) {
							if (strncmp(value_string, string_list[match], match_length) != 0) {
								strncpy_s(value_string, length+1, string_list[match], match_length);
								value_string[length] = '\0';
								is_valid = TRUE;
								break;
							}
						}
						num_matches++;
				}
			}
			if (is_valid == FALSE && num_matches == 1) {
				strncpy_s(value_string, length+1, string_list[match], match_length);
				value_string[length] = '\0';
				is_valid = TRUE;
			}
			break;

		case SANE_CONSTRAINT_NONE:
		default:
			is_valid = TRUE;
			break;
	}

	return is_valid;
}


HRESULT WINSANE_Option::GetValue(_Out_ double *value)
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

HRESULT WINSANE_Option::GetValueBool(_Out_ PSANE_Bool value_bool)
{
	PBYTE value;
	HRESULT hr;

	if (!value_bool)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return E_INVALIDARG;

	hr = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Bool), &value);
	if (FAILED(hr))
		return hr;

	if (value) {
		*value_bool = ntohl(*((PSANE_Bool) value));
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueInt(_Out_ PSANE_Int value_int)
{
	PBYTE value;
	HRESULT hr;

	if (!value_int)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_INT)
		return E_INVALIDARG;

	hr = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int), &value);
	if (FAILED(hr))
		return hr;

	if (value) {
		*value_int = ntohl(*((PSANE_Int) value));
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueFixed(_Out_ PSANE_Fixed value_fixed)
{
	PBYTE value;
	HRESULT hr;

	if (!value_fixed)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return E_INVALIDARG;

	hr = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed), &value);
	if (FAILED(hr))
		return hr;

	if (value) {
		*value_fixed = ntohl(*((PSANE_Fixed) value));
		delete[] value;
		return S_OK;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::GetValueString(_Out_ PSANE_String value_string)
{
	PBYTE value;
	HRESULT hr;

	if (!value_string)
		return E_INVALIDARG;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return E_INVALIDARG;

	hr = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size, &value);
	if (FAILED(hr))
		return hr;

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
	PBYTE value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return E_INVALIDARG;

	value_bool = htonl(value_bool);

	hr = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Bool), (PBYTE) &value_bool, &value);
	if (FAILED(hr))
		return hr;

	if (value) {
		hr = value_bool == *((PSANE_Bool) value) ? S_OK : S_FALSE;
		delete[] value;
		return hr;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::SetValueInt(_In_ SANE_Int value_int)
{
	PBYTE value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_INT)
		return E_INVALIDARG;

	value_int = htonl(value_int);

	hr = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int), (PBYTE) &value_int, &value);
	if (FAILED(hr))
		return hr;

	if (value) {
		hr = value_int == *((PSANE_Int) value) ? S_OK : S_FALSE;
		delete[] value;
		return hr;
	}

	return E_FAIL;
}

HRESULT WINSANE_Option::SetValueFixed(_In_ SANE_Fixed value_fixed)
{
	PBYTE value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return E_INVALIDARG;

	value_fixed = htonl(value_fixed);

	hr = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed), (PBYTE) &value_fixed, &value);
	if (FAILED(hr))
		return hr;

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
	PBYTE value;
	HRESULT hr;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return E_INVALIDARG;

	if (value_string)
		value_size = (SANE_Word) strlen(value_string) + 1;
	else
		value_size = 0;

	hr = this->SetValue(this->sane_option->type, value_size, value_size, (PBYTE) value_string, &value);
	if (FAILED(hr))
		return hr;

	if (value_string && value) {
		hr = strcmp(value_string, (SANE_String_Const) value) == 0 ? S_OK : FALSE;
		delete[] value;
	} else {
		hr = value_string == (SANE_String_Const) value ? S_OK : S_FALSE;
	}

	return hr;
}


HRESULT WINSANE_Option::GetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _Out_ PBYTE *value_result)
{
	LONG written;
	HRESULT hr;
	PBYTE buf;

	if (!SANE_OPTION_IS_ACTIVE(this->sane_option->cap))
		return E_NOTIMPL;

	if (!value_result)
		return E_INVALIDARG;

	buf = new BYTE[value_size];
	memset(buf, 0, value_size);

	written = this->sock->WriteWord(WINSANE_NET_CONTROL_OPTION);
	written += this->sock->WriteHandle(this->sane_handle);
	written += this->sock->WriteWord(this->index);
	written += this->sock->WriteWord(SANE_ACTION_GET_VALUE);
	written += this->sock->WriteWord(value_type);
	written += this->sock->WriteWord(value_size);
	written += this->sock->WriteWord(element_count);
	written += this->sock->Write(buf, value_size);
	if (this->sock->Flush() != written) {
		delete[] buf;
		return E_ABORT;
	}

	hr = this->ReadValueResult(&value_type, &value_size, buf);
	if (FAILED(hr)) {
		delete[] buf;
		return hr;
	}

	*value_result = buf;
	return S_OK;
}

HRESULT WINSANE_Option::SetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _In_ PBYTE value, _Out_ PBYTE *value_result)
{
	LONG written;
	HRESULT hr;
	PBYTE buf;

	if (!SANE_OPTION_IS_SETTABLE(this->sane_option->cap))
		return E_NOTIMPL;

	if (!value_result)
		return E_INVALIDARG;

	buf = new BYTE[value_size];
	memcpy(buf, value, value_size);

	written = this->sock->WriteWord(WINSANE_NET_CONTROL_OPTION);
	written += this->sock->WriteHandle(this->sane_handle);
	written += this->sock->WriteWord(this->index);
	written += this->sock->WriteWord(SANE_ACTION_SET_VALUE);
	written += this->sock->WriteWord(value_type);
	written += this->sock->WriteWord(value_size);
	written += this->sock->WriteWord(element_count);
	written += this->sock->Write(buf, value_size);
	if (this->sock->Flush() != written) {
		delete[] buf;
		return E_ABORT;
	}

	hr = this->ReadValueResult(&value_type, &value_size, buf);
	if (FAILED(hr)) {
		delete[] buf;
		return hr;
	}

	*value_result = buf;
	return S_OK;
}


HRESULT WINSANE_Option::ReadValueResult(_Inout_ PSANE_Word value_type, _Inout_ PSANE_Word value_size, _Inout_ PBYTE value_result)
{
	SANE_Status status;
	SANE_Word info, pointer;
	SANE_String resource;
	BOOL required_auth;
	LONG readlen;
	HRESULT hr;

	if (!value_type || !value_size || !value_result)
		return E_INVALIDARG;

	do {
		hr = this->sock->ReadStatus(&status);
		if (FAILED(hr))
			return hr;

		hr = this->sock->ReadWord(&info);
		if (FAILED(hr))
			return hr;

		hr = this->sock->ReadWord(value_type);
		if (FAILED(hr))
			return hr;

		hr = this->sock->ReadWord(value_size);
		if (FAILED(hr))
			return hr;

		hr = this->sock->ReadWord(&pointer);
		if (FAILED(hr))
			return hr;

		if (pointer) {
			readlen = this->sock->Read(value_result, *value_size);
		} else
			readlen = 0;

		hr = this->sock->ReadWord(&pointer);
		if (FAILED(hr))
			return hr;

		if (pointer) {
			hr = this->sock->ReadString(&resource);
			if (FAILED(hr))
				return hr;

			if (resource) {
				if (strlen(resource) > 0)
					status = this->session->Authorize(resource);
				delete[] resource;
				if (status != SANE_STATUS_GOOD)
					return status;
				required_auth = TRUE;
			} else {
				required_auth = FALSE;
			}
		} else {
			required_auth = FALSE;
		}
	} while (required_auth);

	if (!this->sock->IsConnected())
		return E_ABORT;

	if (readlen != *value_size)
		return E_FAIL;

	return S_OK;
}
