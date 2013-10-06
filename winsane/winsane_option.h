#ifndef WINSANE_OPTION_H
#define WINSANE_OPTION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane.h"
#include "winsane_socket.h"
#include "winsane_device.h"

class WINSANE_API WINSANE_Option {
public:
	WINSANE_Option(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Option_Descriptor sane_option, _In_ SANE_Handle sane_handle, _In_ int index);
	~WINSANE_Option();


	/* Public API */
	SANE_String_Const GetName();
	SANE_String_Const GetTitle();
	SANE_String_Const GetDescription();

	SANE_Value_Type GetType();
	SANE_Unit GetUnit();
	SANE_Int GetSize();
	SANE_Int GetCapabilities();

	SANE_Constraint_Type GetConstraintType();
	PSANE_Range GetConstraintRange();
	PSANE_Word GetConstraintWordList();
	PSANE_String_Const GetConstraintStringList();

	BOOL IsValidValue(_In_ double value);
	BOOL IsValidValue(_In_ SANE_String_Const value);

	HRESULT GetValue(_Inout_ double *value);
	HRESULT GetValueBool(_Inout_ PSANE_Bool value_bool);
	HRESULT GetValueInt(_Inout_ PSANE_Int value_int);
	HRESULT GetValueFixed(_Inout_ PSANE_Fixed value_fixed);
	HRESULT GetValueString(_Inout_ PSANE_String value_string);

	HRESULT SetValue(_In_ double value);
	HRESULT SetValueBool(_In_ SANE_Bool value_bool);
	HRESULT SetValueInt(_In_ SANE_Int value_int);
	HRESULT SetValueFixed(_In_ SANE_Fixed value_fixed);
	HRESULT SetValueString(_In_ SANE_String_Const value_string);


protected:
	PVOID GetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count);
	PVOID SetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _In_ PVOID value);


private:
	PWINSANE_Device device;
	PWINSANE_Socket sock;
	PSANE_Option_Descriptor sane_option;
	SANE_Handle sane_handle;
	int index;
};

#endif
