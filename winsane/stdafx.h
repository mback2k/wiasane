#pragma once

#ifdef WINSANE_EXPORTS
#define WINSANE_API __declspec(dllexport)
#else
#define WINSANE_API __declspec(dllimport)
#endif

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
