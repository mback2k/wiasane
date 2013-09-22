#ifndef STDAFX_H
#define STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "winsane_internal.h"

extern BOOL g_bStarted; // global Winsock initialization status

#endif
