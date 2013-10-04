#ifndef WIASANE_OPT_H
#define WIASANE_OPT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#include "winsane.h"

HRESULT GetOptionMaxValue(WINSANE_Option *option, double *value);
HRESULT GetOptionValueInch(WINSANE_Option *option, double *value);
HRESULT GetOptionMaxValueInch(WINSANE_Option *option, double *value);
HRESULT IsValidOptionValueInch(WINSANE_Option *option, double value);
HRESULT SetOptionValueInch(WINSANE_Option *option, double value);

#endif