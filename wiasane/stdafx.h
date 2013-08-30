#ifndef STDAFX_H
#define STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_driver_)

#include <windows.h>
#include <wiamicro.h>

#include "winsane.h"

#pragma comment(lib, "winsane.lib")

extern HINSTANCE g_hInst; // instance of this MicroDriver (used for loading from a resource)

#endif
