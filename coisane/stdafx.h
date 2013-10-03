#ifndef STDAFX_H
#define STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>

extern HINSTANCE g_hInst; // instance of this CoInstaller (used for loading from a resource)

#endif
