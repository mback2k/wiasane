#ifndef DEVSANE_DEVICE_H
#define DEVSANE_DEVICE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <tchar.h>

DWORD DeviceInstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ HWND hwnd, _In_ int nCmdShow);
DWORD DeviceUninstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_ HWND hwnd, _In_ int nCmdShow);

#endif
