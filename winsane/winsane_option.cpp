#include "winsane_option.h"
#include "winsane_internal.h"

WINSANE_Option::WINSANE_Option(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Option_Descriptor *sane_option, SANE_Handle sane_handle, int index)
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

SANE_Range* WINSANE_Option::GetConstraintRange()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_RANGE)
		return NULL;

	return this->sane_option->constraint.range;
}

SANE_Word* WINSANE_Option::GetConstraintWordList()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_WORD_LIST)
		return NULL;

	return this->sane_option->constraint.word_list;
}

SANE_String_Const* WINSANE_Option::GetConstraintStringList()
{
	if (this->sane_option->constraint_type != SANE_CONSTRAINT_STRING_LIST)
		return NULL;

	return this->sane_option->constraint.string_list;
}

bool WINSANE_Option::IsValidValue(SANE_Word value)
{
	SANE_Word *word_list, word_list_length;
	SANE_Range *range;
	bool is_valid;
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

bool WINSANE_Option::IsValidValue(SANE_String value)
{
	SANE_String_Const *string_list;
	bool is_valid;
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


SANE_Bool WINSANE_Option::GetValueBool()
{
	SANE_Bool value_bool;
	void *value;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return FALSE;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Bool));
	if (value) {
		value_bool = *((SANE_Bool*) value);
		delete value;
	} else
		return FALSE;

	return value_bool;
}

SANE_Int WINSANE_Option::GetValueInt()
{
	SANE_Int value_int;
	void *value;

	if (this->sane_option->type != SANE_TYPE_INT)
		return 0;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int));
	if (value) {
		value_int = *((SANE_Int*) value);
		delete value;
	} else
		return 0;

	return value_int;
}

SANE_Fixed WINSANE_Option::GetValueFixed()
{
	SANE_Fixed value_fixed;
	void *value;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return 0;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed));
	if (value) {
		value_fixed = *((SANE_Fixed*) value);
		delete value;
	} else
		return 0;

	return value_fixed;
}

SANE_String WINSANE_Option::GetValueString()
{
	SANE_String value_string;
	void *value;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return NULL;

	value = this->GetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size);
	value_string = (SANE_String) value;

	return value_string;
}


SANE_Bool WINSANE_Option::SetValueBool(SANE_Bool value_bool)
{
	void *value;

	if (this->sane_option->type != SANE_TYPE_BOOL)
		return 0;

	value = (void*) &value_bool;
	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Word), &value);
	if (value) {
		value_bool = *((SANE_Bool*) value);
		delete value;
	} else
		return FALSE;

	return value_bool;
}

SANE_Int WINSANE_Option::SetValueInt(SANE_Int value_int)
{
	void *value;

	if (this->sane_option->type != SANE_TYPE_INT)
		return 0;

	value = (void*) &value_int;
	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Int), value);
	if (value) {
		value_int = *((SANE_Int*) value);
		delete value;
	} else
		return 0;

	return value_int;
}

SANE_Fixed WINSANE_Option::SetValueFixed(SANE_Fixed value_fixed)
{
	void *value;

	if (this->sane_option->type != SANE_TYPE_FIXED)
		return 0;

	value = (void*) &value_fixed;
	value = this->SetValue(this->sane_option->type, this->sane_option->size, this->sane_option->size / sizeof(SANE_Fixed), value);
	if (value) {
		value_fixed = *((SANE_Fixed*) value);
		delete value;
	} else
		return 0;

	return value_fixed;
}

SANE_String WINSANE_Option::SetValueString(SANE_String_Const value_string)
{
	SANE_Word value_size;
	void *value;

	if (this->sane_option->type != SANE_TYPE_STRING)
		return 0;

	if (value_string)
		value_size = (SANE_Word) strlen(value_string) + 1;
	else
		value_size = 0;

	value = (void*) value_string;
	value = this->SetValue(this->sane_option->type, value_size, value_size, value);

	return (SANE_String) value;
}


void* WINSANE_Option::GetValue(SANE_Word value_type, SANE_Word value_size, SANE_Word element_count)
{
	SANE_Status status;
	SANE_Word info, pointer;
	SANE_String resource;
	int written;
	char *buf;

	buf = new char[value_size];
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

void* WINSANE_Option::SetValue(SANE_Word value_type, SANE_Word value_size, SANE_Word element_count, void* value)
{
	SANE_Status status;
	SANE_Word info, pointer;
	SANE_String resource;
	int written;
	char *buf;

	buf = new char[value_size];
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
