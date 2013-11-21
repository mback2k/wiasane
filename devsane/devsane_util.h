#ifndef DEVSANE_UTIL_H
#define DEVSANE_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <DriverSpecs.h>
__user_code

#include <windows.h>
#include <setupapi.h>
#include <difxapi.h>

#include "devsane.h"

VOID Trace(_In_ LPCTSTR pszFormat, ...);

DWORD CreateInstallInfo(_In_ HANDLE hHeap, _Inout_ PINSTALLERINFO pInstallerInfo, _Inout_ LPVOID *plpData);

#endif
