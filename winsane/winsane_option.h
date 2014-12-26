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

#ifndef WINSANE_OPTION_H
#define WINSANE_OPTION_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "winsane.h"
#include "winsane_socket.h"
#include "winsane_device.h"
#include "winsane_session.h"

class WINSANE_API WINSANE_Option {
public:
	WINSANE_Option(_In_ PWINSANE_Session session, _In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Option_Descriptor sane_option, _In_ SANE_Handle sane_handle, _In_ int index);
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

	HRESULT GetValue(_Out_ double *value);
	HRESULT GetValueBool(_Out_ PSANE_Bool value_bool);
	HRESULT GetValueInt(_Out_ PSANE_Int value_int);
	HRESULT GetValueFixed(_Out_ PSANE_Fixed value_fixed);
	HRESULT GetValueString(_Out_ PSANE_String value_string);

	HRESULT SetValue(_In_ double value);
	HRESULT SetValueBool(_In_ SANE_Bool value_bool);
	HRESULT SetValueInt(_In_ SANE_Int value_int);
	HRESULT SetValueFixed(_In_ SANE_Fixed value_fixed);
	HRESULT SetValueString(_In_ SANE_String_Const value_string);


protected:
	HRESULT GetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _Out_ PBYTE *value_result);
	HRESULT SetValue(_In_ SANE_Word value_type, _In_ SANE_Word value_size, _In_ SANE_Word element_count, _In_ PBYTE value, _Out_ PBYTE *value_result);


private:
	HRESULT ReadValueResult(_Inout_ PSANE_Word value_type, _Inout_ PSANE_Word value_size, _Inout_ PBYTE value_result);

	PWINSANE_Session session;
	PWINSANE_Device device;
	PWINSANE_Socket sock;
	PSANE_Option_Descriptor sane_option;
	SANE_Handle sane_handle;
	int index;
};

#endif
