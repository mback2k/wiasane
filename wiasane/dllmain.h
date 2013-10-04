#ifndef DLLMAIN_H
#define DLLMAIN_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_driver_)

#include <windows.h>

HINSTANCE GetModuleInstance();

#endif
