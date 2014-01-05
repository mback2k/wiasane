/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
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

#include "winsane-dbg.h"

int WINAPIV _tmain(int argc, TCHAR *argv[])
{
	PTSTR hostname;

	if (argc > 1) {
		hostname = argv[1];
		Debug(hostname);
		exit(0);
	} else {
		exit(1);
	}
}

VOID Debug(PTSTR hostname)
{
	WINSANE_Session *session;

	session = WINSANE_Session::Remote(hostname);
	if (session) {
		DebugSession(session);
		delete session;
	}
}

VOID DebugSession(WINSANE_Session *session)
{
	WINSANE_Device *device;
	int devices, i;

	if (session->Init(NULL, NULL) == SANE_STATUS_GOOD) {
		devices = session->GetDevices();

		for (i = 0; i < devices; i++) {
			device = session->GetDevice(i);
			if (device) {
				DebugSessionDevice(session, device);
			}
		}

		session->Exit();
	}
}

VOID DebugSessionDevice(WINSANE_Session *session, WINSANE_Device *device)
{
	WINSANE_Option *option;
	WINSANE_Params *params;
	int options, i;

	printf("------------------------------------------------------------\n");
	printf("Name:\t%s\n", device->GetName());
	printf("Vendor:\t%s\n", device->GetVendor());
	printf("Model:\t%s\n", device->GetModel());
	printf("Type:\t%s\n", device->GetType());

	if (device->Open() == SANE_STATUS_GOOD) {
		options = device->FetchOptions();

		for (i = 0; i < options; i++) {
			option = device->GetOption(i);
			if (option) {
				DebugSessionDeviceOption(session, device, option);
			}
		}

		if (device->GetParams(&params) == SANE_STATUS_GOOD) {
			DebugSessionDeviceParams(session, device, params);
			delete params;
		}

		device->Cancel();

		DebugSessionDeviceScan(session, device);

		device->Close();
	}
}

VOID DebugSessionDeviceOption(WINSANE_Session *session, WINSANE_Device *device, WINSANE_Option *option)
{
	SANE_String_Const *string_list;
	SANE_Word *word_list;
	SANE_Range *range;
	SANE_Bool value_bool;
	SANE_Int value_int;
	SANE_Fixed value_fixed;
	SANE_String value_string;
	HRESULT hr;
	int index;

	UNREFERENCED_PARAMETER(session);
	UNREFERENCED_PARAMETER(device);

	printf("------------------------------------------------------------\n");
	printf("Name:\t\t%s\n", option->GetName());
	printf("Title:\t\t%s\n", option->GetTitle());
	printf("Description:\t%s\n", option->GetDescription());

	switch (option->GetType()) {
		case SANE_TYPE_BOOL:
			hr = option->GetValueBool(&value_bool);
			if (SUCCEEDED(hr))
				printf("Value:\t\t%d\n", value_bool);
			break;
		case SANE_TYPE_INT:
			hr = option->GetValueInt(&value_int);
			if (SUCCEEDED(hr))
				printf("Value:\t\t%d\n", value_int);
			break;
		case SANE_TYPE_FIXED:
			hr = option->GetValueFixed(&value_fixed);
			if (SUCCEEDED(hr))
				printf("Value:\t\t%f\t(%d)\n", SANE_UNFIX(value_fixed), value_fixed);
			break;
		case SANE_TYPE_STRING:
			hr = option->GetValueString(&value_string);
			if (SUCCEEDED(hr)) {
				if (value_string) {
					printf("Value:\t\t%s\n", value_string);
					delete[] value_string;
				} else {
					printf("Value:\t\t<NULL>\n");
				}
			}
			break;
		default:
			hr = E_NOTIMPL;
			break;
	}

	if (FAILED(hr))
		printf("Value:\t\t<FAIL>\n");

	switch (option->GetUnit()) {
		case SANE_UNIT_NONE:
			printf("Unit:\t\tNone\n");
			break;
		case SANE_UNIT_PIXEL:
			printf("Unit:\t\tPixel\n");
			break;
		case SANE_UNIT_BIT:
			printf("Unit:\t\tBit\n");
			break;
		case SANE_UNIT_MM:
			printf("Unit:\t\tMillimeter\n");
			break;
		case SANE_UNIT_DPI:
			printf("Unit:\t\tDPI\n");
			break;
		case SANE_UNIT_PERCENT:
			printf("Unit:\t\tPercent\n");
			break;
		case SANE_UNIT_MICROSECOND:
			printf("Unit:\t\tMicrosecond\n");
			break;
	}

	switch (option->GetConstraintType()) {
		case SANE_CONSTRAINT_RANGE: {
			range = option->GetConstraintRange();
			if (option->GetType() == SANE_TYPE_FIXED) {
				printf("Min:\t\t%f\t(%d)\n", SANE_UNFIX(range->min), range->min);
				printf("Max:\t\t%f\t(%d)\n", SANE_UNFIX(range->max), range->max);
				printf("Quant:\t\t%f\t(%d)\n", SANE_UNFIX(range->quant), range->quant);
			} else {
				printf("Min:\t\t%d\n", range->min);
				printf("Max:\t\t%d\n", range->max);
				printf("Quant:\t\t%d\n", range->quant);
			}
			break;
		}
		case SANE_CONSTRAINT_WORD_LIST: {
			word_list = option->GetConstraintWordList();
			for (index = 1; index <= word_list[0]; index++) {
				if (option->GetType() == SANE_TYPE_FIXED) {
					printf("Word:\t\t%f\t(%d)\n", SANE_UNFIX(word_list[index]), word_list[index]);
				} else {
					printf("Word:\t\t%d\n", word_list[index]);
				}
			}
			break;
		}
		case SANE_CONSTRAINT_STRING_LIST: {
			string_list = option->GetConstraintStringList();
			for (index = 0; string_list[index] != NULL; index++) {
				printf("String:\t\t%s\n", string_list[index]);
			}
			break;
		}
	}

	if (option->GetName()) {
		if (strcmp(option->GetName(), "mode") == 0) {
			hr = option->SetValueString("Color");
			hr = option->GetValueString(&value_string);
			if (SUCCEEDED(hr) && value_string) {
				printf("Mode:\t\t%s\n", value_string);
				delete[] value_string;
			}
		} else if (strcmp(option->GetName(), "compression") == 0) {
			hr = option->SetValueString("None");
			hr = option->GetValueString(&value_string);
			if (SUCCEEDED(hr) && value_string) {
				printf("Compression:\t%s\n", value_string);
				delete[] value_string;
			}
		} else if (strcmp(option->GetName(), "resolution") == 0) {
			hr = option->SetValueInt(300);
			hr = option->GetValueInt(&value_int);
			if (SUCCEEDED(hr)) {
				printf("Resolution:\t%d\n", value_int);
			}
		}
	}
}

VOID DebugSessionDeviceParams(WINSANE_Session *session, WINSANE_Device *device, WINSANE_Params *params)
{
	UNREFERENCED_PARAMETER(session);
	UNREFERENCED_PARAMETER(device);

	printf("------------------------------------------------------------\n");
	printf("Format:\t\t%d\n", params->GetFormat());
	printf("Last Frame:\t%d\n", params->IsLastFrame());
	printf("Bytes per Line:\t%d\n", params->GetBytesPerLine());
	printf("Pixel per Line:\t%d\n", params->GetPixelsPerLine());
	printf("Lines:\t\t%d\n", params->GetLines());
	printf("Depth:\t\t%d\n", params->GetDepth());
}

VOID DebugSessionDeviceScan(WINSANE_Session *session, WINSANE_Device *device)
{
	WINSANE_Scan *scan;
	HANDLE output;
	DWORD written;
	DWORD length;
	PBYTE buffer;
	BOOL doscan;

	UNREFERENCED_PARAMETER(session);

	printf("Scan? (y/n)\n");
	doscan = fgetc(stdin) == 'y';
	while (fgetc(stdin) != '\n');

	if (doscan) {
		if (device->Start(&scan) == SANE_STATUS_GOOD) {
			if (scan) {
				printf("Begin scanning image ...\n");
				output = CreateFile(L"winsane-dbg.scan", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				buffer = new BYTE[1024];
				if (buffer) {
					length = 1024;
					ZeroMemory(buffer, length);
					while (scan->AquireImage(buffer, &length) == CONTINUE) {
						printf("Received %d bytes of scanned image ...\n", length);
						WriteFile(output, buffer, length, &written, NULL);
						length = 1024;
					}
					delete[] buffer;
				}

				CloseHandle(output);
				printf("Finished scanning image!\n");

				delete scan;
			}

			device->Cancel();
		}
	}
}
