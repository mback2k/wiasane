#ifndef COISANE_H
#define COISANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#include <setupapi.h>

typedef struct _COISANE_Data {
	UINT uiReferences;
	HANDLE hHeap;
	HDEVINFO hDeviceInfoSet;
	PSP_DEVINFO_DATA pDeviceInfoData;
	USHORT usPort;
	LPTSTR lpHost;			// use malloc/free
	LPTSTR lpName;			// use malloc/free
	LPTSTR lpUsername;		// use malloc/free
	LPTSTR lpPassword;		// use malloc/free
} COISANE_Data, *PCOISANE_Data;

#endif
