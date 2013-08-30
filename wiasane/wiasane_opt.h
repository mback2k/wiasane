#ifndef WIASANE_OPT_H
#define WIASANE_OPT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

typedef struct WIASANE_Context {
	WINSANE_Session *session;
	WINSANE_Device *device;
	WINSANE_Scan *scan;
} WIASANE_Context;

HRESULT GetOptionValue(WIASANE_Context *context, SANE_String name, double *value);
HRESULT GetOptionValue(WIASANE_Context *context, SANE_String name, char **value);

HRESULT SetOptionValue(WIASANE_Context *context, SANE_String name, double value);
HRESULT SetOptionValue(WIASANE_Context *context, SANE_String name, char *value);

HRESULT GetOptionValueInch(WIASANE_Context *context, SANE_String name, double *value);
HRESULT IsValidOptionValueInch(WIASANE_Context *context, SANE_String name, double value);
HRESULT SetOptionValueInch(WIASANE_Context *context, SANE_String name, double value);

#endif