#ifndef DLLMAIN_H
#define DLLMAIN_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>

BOOL IsWinsockStarted();

#endif
