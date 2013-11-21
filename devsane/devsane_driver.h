#ifndef DEVSANE_DRIVER_H
#define DEVSANE_DRIVER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>

DWORD DriverInstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ int nCmdShow);
DWORD DriverUninstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ int nCmdShow);

#endif
