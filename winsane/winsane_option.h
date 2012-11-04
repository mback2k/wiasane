#pragma once

#include "winsane_socket.h"

class WINSANE_API WINSANE_Option {
public:
	WINSANE_Option(WINSANE_Device *device, SANE_Option_Descriptor *sane_option);
	~WINSANE_Option();

	/* Public API */
	SANE_String_Const GetName();
	SANE_String_Const GetTitle();
	SANE_String_Const GetDescription();

	SANE_Value_Type GetType();
	SANE_Unit GetUnit();
	SANE_Int GetSize();
	SANE_Int GetCapabilities();

private:
	WINSANE_Device *device;
	WINSANE_Socket *sock;
	SANE_Option_Descriptor *sane_option;
};
