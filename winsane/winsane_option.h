#ifndef WINSANE_OPTION_H
#define WINSANE_OPTION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "sane.h"
#include "winsane_socket.h"
#include "winsane_device.h"

class WINSANE_API WINSANE_Option {
public:
	WINSANE_Option(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Option_Descriptor *sane_option, SANE_Handle sane_handle, int index);
	~WINSANE_Option();


	/* Public API */
	SANE_String_Const GetName();
	SANE_String_Const GetTitle();
	SANE_String_Const GetDescription();

	SANE_Value_Type GetType();
	SANE_Unit GetUnit();
	SANE_Int GetSize();
	SANE_Int GetCapabilities();


	SANE_Bool GetValueBool();
	SANE_Int GetValueInt();
	SANE_Fixed GetValueFixed();
	SANE_String GetValueString();

	SANE_Bool SetValueBool(SANE_Bool value_bool);
	SANE_Int SetValueInt(SANE_Int value_int);
	SANE_Fixed SetValueFixed(SANE_Fixed value_fixed);
	SANE_String SetValueString(SANE_String value_string);


protected:
	void* GetValue(SANE_Word value_type, SANE_Word value_size, SANE_Word element_count);
	void* SetValue(SANE_Word value_type, SANE_Word value_size, SANE_Word element_count, void* value);


private:
	WINSANE_Device *device;
	WINSANE_Socket *sock;
	SANE_Option_Descriptor *sane_option;
	SANE_Handle sane_handle;
	int index;
};

#endif
