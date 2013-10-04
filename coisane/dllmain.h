#ifndef DLLMAIN_H
#define DLLMAIN_H

#if _MSC_VER > 1000
#pragma once
#endif

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <DriverSpecs.h>
__user_code

#include <windows.h>

HINSTANCE GetModuleInstance();
HANDLE GetActivationContext();

#endif
