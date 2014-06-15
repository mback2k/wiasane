/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2013, Marc Hoersken, <info@marc-hoersken.de>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this software distribution.
 *
 * You may opt to use, copy, modify, and distribute this software for any
 * purpose with or without fee, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either expressed or implied.
 *
 ***************************************************************************/

#include "devsane_device.h"

#include <setupapi.h>
#include <difxapi.h>
#include <newdev.h>

#include "strutil.h"
#include "strutil_dbg.h"
#include "devsane_util.h"


DWORD CALLBACK DeviceInstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_opt_ HWND hwnd, _In_ int nCmdShow)
{
	LPTSTR lpDsInfPath, lpClassName, lpHardwareId, lpHardwareIds;
	DWORD dwFlags, cbDsInfPath, cbClassName, cbHardwareId;
	size_t cbHardwareIds;
	SP_DEVINFO_DATA deviceInfoData;
	HDEVINFO hDeviceInfoSet;
	HRESULT hr;
	DWORD res;
	GUID guid;
	int i;

	Trace(TEXT("DeviceInstall(%s, %d)"), lpInfPath, nCmdShow);

	cbDsInfPath = 0;
	res = DriverPackageGetPath(lpInfPath, NULL, &cbDsInfPath);
	if (res == ERROR_INSUFFICIENT_BUFFER) {
		lpDsInfPath = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDsInfPath * sizeof(TCHAR));
		if (lpDsInfPath) {
			res = DriverPackageGetPath(lpInfPath, lpDsInfPath, &cbDsInfPath);
			if (res == ERROR_SUCCESS) {
				Trace(TEXT("DsInfPath: %s"), lpDsInfPath);

				res = SetupDiGetINFClass(lpDsInfPath, &guid, NULL, 0, &cbClassName);
				if (!res && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					lpClassName = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbClassName * sizeof(TCHAR));
					if (lpClassName) {
						res = SetupDiGetINFClass(lpDsInfPath, &guid, lpClassName, cbClassName, &cbClassName);
						if (res) {
							Trace(TEXT("ClassName: %s"), lpClassName);

							hDeviceInfoSet = SetupDiCreateDeviceInfoList(&guid, hwnd);
							if (hDeviceInfoSet != INVALID_HANDLE_VALUE) {
								ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
								deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

								res = SetupDiCreateDeviceInfo(hDeviceInfoSet, lpClassName, &guid, NULL, hwnd, DICD_GENERATE_ID, &deviceInfoData);
								if (res) {
									res = SetupDiGetDeviceInstanceId(hDeviceInfoSet, &deviceInfoData, NULL, 0, &cbHardwareId);
									if (!res && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
										lpHardwareId = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbHardwareId * sizeof(TCHAR));
										if (lpHardwareId) {
											res = SetupDiGetDeviceInstanceId(hDeviceInfoSet, &deviceInfoData, lpHardwareId, cbHardwareId, &cbHardwareId);
											if (res) {
												Trace(TEXT("HardwareID: %s (%d)"), lpHardwareId, cbHardwareId);

												hr = StringCbAPrintf(hHeap, &lpHardwareIds, &cbHardwareIds, TEXT("Image_Network_WIASANE\n%s\n"), lpHardwareId);
												if (SUCCEEDED(hr)) {
													Trace(TEXT("HardwareIDs: %s (%d)"), lpHardwareIds, cbHardwareIds);

													for (i = 0; lpHardwareIds[i]; i++) {
														if (lpHardwareIds[i] == '\n') {
															lpHardwareIds[i] = '\0';
														}
													}

													res = SetupDiSetDeviceRegistryProperty(hDeviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, (PBYTE) lpHardwareIds, (DWORD) cbHardwareIds);
													if (res) {
														res = SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDeviceInfoSet, &deviceInfoData);
														if (res) {
															dwFlags = INSTALLFLAG_FORCE;
															if (nCmdShow == SW_HIDE)
																dwFlags |= INSTALLFLAG_NONINTERACTIVE;

															res = UpdateDriverForPlugAndPlayDevices(hwnd, lpHardwareId, lpDsInfPath, dwFlags, NULL);
															if (!res) {
																Trace(TEXT("UpdateDriverForPlugAndPlayDevices failed: %08X"), res = GetLastError());

																res = SetupDiCallClassInstaller(DIF_REMOVE, hDeviceInfoSet, &deviceInfoData);
																if (!res)
																	Trace(TEXT("SetupDiCallClassInstaller 2 failed: %08X"), res = GetLastError());
															} else
																res = ERROR_SUCCESS;
														} else
															Trace(TEXT("SetupDiCallClassInstaller 1 failed: %08X"), res = GetLastError());
													} else
														Trace(TEXT("SetupDiSetDeviceRegistryProperty failed: %08X"), res = GetLastError());

													HeapFree(hHeap, 0, lpHardwareIds);
												} else
													Trace(TEXT("StringCchAPrintf failed: %08X"), hr);
											} else
												Trace(TEXT("SetupDiGetDeviceInstanceId 2 failed: %08X"), res = GetLastError());

											HeapFree(hHeap, 0, lpHardwareId);
										} else
											Trace(TEXT("HeapAlloc failed"));
									} else
										Trace(TEXT("SetupDiGetDeviceInstanceId 1 failed: %08X"), res = GetLastError());

									SetupDiDeleteDeviceInfo(hDeviceInfoSet, &deviceInfoData);
								} else
									Trace(TEXT("SetupDiCreateDeviceInfo failed: %08X"), res = GetLastError());

								SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
							} else
								Trace(TEXT("SetupDiCreateDeviceInfoList failed: %08X"), res = GetLastError());
						} else
							Trace(TEXT("SetupDiGetINFClass 2 failed: %08X"), res = GetLastError());

						HeapFree(hHeap, 0, lpClassName);
					} else
						Trace(TEXT("HeapAlloc failed"));
				} else
					Trace(TEXT("SetupDiGetINFClass 1 failed: %08X"), res = GetLastError());
			} else
				Trace(TEXT("DriverPackageGetPath 2 failed: %08X"), res);

			HeapFree(hHeap, 0, lpDsInfPath);
		} else
			Trace(TEXT("HeapAlloc failed"));
	} else
		Trace(TEXT("DriverPackageGetPath 1 failed: %08X"), res);

	return res;
}

DWORD CALLBACK DeviceUninstall(_In_ HANDLE hHeap, _In_ LPTSTR lpInfPath, _In_opt_ HWND hwnd, _In_ int nCmdShow)
{
	LPTSTR lpDsInfPath, lpClassName, lpHardwareIds;
	DWORD dwType, cbDsInfPath, cbClassName, cbHardwareIds;
	SP_DEVINFO_DATA deviceInfoData;
	HDEVINFO hDeviceInfoSet;
	DWORD res;
	GUID guid;
	int i;

	UNREFERENCED_PARAMETER(nCmdShow);

	Trace(TEXT("DeviceUninstall(%s, %d)"), lpInfPath, nCmdShow);

	cbDsInfPath = 0;
	res = DriverPackageGetPath(lpInfPath, NULL, &cbDsInfPath);
	if (res == ERROR_INSUFFICIENT_BUFFER) {
		lpDsInfPath = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDsInfPath * sizeof(TCHAR));
		if (lpDsInfPath) {
			res = DriverPackageGetPath(lpInfPath, lpDsInfPath, &cbDsInfPath);
			if (res == ERROR_SUCCESS) {
				Trace(TEXT("DsInfPath: %s"), lpDsInfPath);

				res = SetupDiGetINFClass(lpDsInfPath, &guid, NULL, 0, &cbClassName);
				if (!res && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					lpClassName = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbClassName * sizeof(TCHAR));
					if (lpClassName) {
						res = SetupDiGetINFClass(lpDsInfPath, &guid, lpClassName, cbClassName, &cbClassName);
						if (res) {
							Trace(TEXT("ClassName: %s"), lpClassName);

							hDeviceInfoSet = SetupDiGetClassDevs(&guid, NULL, hwnd, 0);
							if (hDeviceInfoSet != INVALID_HANDLE_VALUE) {
								ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
								deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

								for (i = 0; deviceInfoData.cbSize; i++) {
									res = SetupDiEnumDeviceInfo(hDeviceInfoSet, i, &deviceInfoData);
									if (res) {
										res = SetupDiGetDeviceRegistryProperty(hDeviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &dwType, NULL, 0, &cbHardwareIds);
										if (!res && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
											lpHardwareIds = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbHardwareIds);
											if (lpHardwareIds) {
												res = SetupDiGetDeviceRegistryProperty(hDeviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &dwType, (PBYTE) lpHardwareIds, cbHardwareIds, &cbHardwareIds);
												if (res) {
													if ((dwType == REG_SZ || dwType == REG_MULTI_SZ) && lstrcmp(lpHardwareIds, TEXT("Image_Network_WIASANE\0")) == 0) {
														res = SetupDiCallClassInstaller(DIF_REMOVE, hDeviceInfoSet, &deviceInfoData);
														if (!res)
															Trace(TEXT("SetupDiCallClassInstaller failed: %08X"), res = GetLastError());
													}
												} else
													Trace(TEXT("SetupDiGetDeviceRegistryProperty 2 failed: %08X"), res = GetLastError());

												HeapFree(hHeap, 0, lpHardwareIds);
											} else
												Trace(TEXT("HeapAlloc failed"));
										} else
											Trace(TEXT("SetupDiGetDeviceRegistryProperty 1 failed: %08X"), res = GetLastError());
									} else if (GetLastError() == ERROR_NO_MORE_ITEMS) {
										break;
									} else
										Trace(TEXT("SetupDiEnumDeviceInfo failed: %08X"), res = GetLastError());
								}

								SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
							} else
								Trace(TEXT("SetupDiGetClassDevs failed: %08X"), res = GetLastError());
						} else
							Trace(TEXT("SetupDiGetINFClass 2 failed: %08X"), res = GetLastError());

						HeapFree(hHeap, 0, lpClassName);
					} else
						Trace(TEXT("HeapAlloc failed"));
				} else
					Trace(TEXT("SetupDiGetINFClass 1 failed: %08X"), res = GetLastError());
			} else
				Trace(TEXT("DriverPackageGetPath 2 failed: %08X"), res);

			HeapFree(hHeap, 0, lpDsInfPath);
		} else
			Trace(TEXT("HeapAlloc failed"));
	} else
		Trace(TEXT("DriverPackageGetPath 1 failed: %08X"), res);

	return res;
}
