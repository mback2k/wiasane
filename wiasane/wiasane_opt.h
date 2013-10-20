#ifndef WIASANE_OPT_H
#define WIASANE_OPT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#include "winsane.h"

HRESULT GetOptionMaxValue(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT GetOptionValueInch(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT GetOptionMaxValueInch(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT IsValidOptionValueInch(_In_ PWINSANE_Option option, _In_ double value);
HRESULT SetOptionValueInch(_In_ PWINSANE_Option option, _In_ double value);

#endif