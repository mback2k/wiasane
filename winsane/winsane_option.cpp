#include "stdafx.h"
#include "winsane_option.h"
#include "winsane_device.h"
#include "winsane_session.h"

WINSANE_Option::WINSANE_Option(WINSANE_Device *device, SANE_Option_Descriptor *sane_option) {
	this->device = device;
	this->sock = this->device->GetSession()->GetSocket();
	this->sane_option = sane_option;
}

WINSANE_Option::~WINSANE_Option() {
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
		for (int index = 0; this->sane_option->constraint.string_list[index] != NULL; index++) {
			delete this->sane_option->constraint.string_list[index];
		}
		delete this->sane_option->constraint.string_list;
	}

	delete this->sane_option;

	this->device = NULL;
	this->sock = NULL;
}


SANE_String_Const WINSANE_Option::GetName() {
	return this->sane_option->name;
}

SANE_String_Const WINSANE_Option::GetTitle() {
	return this->sane_option->title;
}

SANE_String_Const WINSANE_Option::GetDescription() {
	return this->sane_option->desc;
}


SANE_Value_Type WINSANE_Option::GetType() {
	return this->sane_option->type;
}

SANE_Unit WINSANE_Option::GetUnit() {
	return this->sane_option->unit;
}

SANE_Int WINSANE_Option::GetSize() {
	return this->sane_option->size;
}

SANE_Int WINSANE_Option::GetCapabilities() {
	return this->sane_option->cap;
}
