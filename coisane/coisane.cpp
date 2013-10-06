#include "coisane.h"

#include <newdev.h>

#include "coisane_str.h"
#include "coisane_util.h"
#include "coisane_prop.h"
#include "coisane_wizard.h"


DWORD CALLBACK CoInstaller(_In_ DI_FUNCTION InstallFunction, _In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, OPTIONAL _Inout_ PCOINSTALLER_CONTEXT_DATA Context)
{
	DWORD ret;

	UNREFERENCED_PARAMETER(Context);

	ret = NO_ERROR;

	switch (InstallFunction) {
		case DIF_INSTALLDEVICE:
			Trace(TEXT("DIF_INSTALLDEVICE"));
			ret = UpdateInstallDeviceFlags(hDeviceInfoSet, pDeviceInfoData, DI_INSTALLDISABLED);
			break;

		case DIF_REMOVE:
			Trace(TEXT("DIF_REMOVE"));
			break;

		case DIF_SELECTDEVICE:
			Trace(TEXT("DIF_SELECTDEVICE"));
			break;

		case DIF_ASSIGNRESOURCES:
			Trace(TEXT("DIF_ASSIGNRESOURCES"));
			break;

		case DIF_PROPERTIES:
			Trace(TEXT("DIF_PROPERTIES"));
			break;

		case DIF_FIRSTTIMESETUP:
			Trace(TEXT("DIF_FIRSTTIMESETUP"));
			break;

		case DIF_FOUNDDEVICE:
			Trace(TEXT("DIF_FOUNDDEVICE"));
			break;

		case DIF_SELECTCLASSDRIVERS:
			Trace(TEXT("DIF_SELECTCLASSDRIVERS"));
			break;

		case DIF_VALIDATECLASSDRIVERS:
			Trace(TEXT("DIF_VALIDATECLASSDRIVERS"));
			break;

		case DIF_INSTALLCLASSDRIVERS:
			Trace(TEXT("DIF_INSTALLCLASSDRIVERS"));
			break;

		case DIF_CALCDISKSPACE:
			Trace(TEXT("DIF_CALCDISKSPACE"));
			break;

		case DIF_DESTROYPRIVATEDATA:
			Trace(TEXT("DIF_DESTROYPRIVATEDATA"));
			break;

		case DIF_VALIDATEDRIVER:
			Trace(TEXT("DIF_VALIDATEDRIVER"));
			break;

		case DIF_MOVEDEVICE:
			Trace(TEXT("DIF_MOVEDEVICE"));
			break;

		case DIF_DETECT:
			Trace(TEXT("DIF_DETECT"));
			break;

		case DIF_INSTALLWIZARD:
			Trace(TEXT("DIF_INSTALLWIZARD"));
			break;

		case DIF_DESTROYWIZARDDATA:
			Trace(TEXT("DIF_DESTROYWIZARDDATA"));
			break;

		case DIF_PROPERTYCHANGE:
			Trace(TEXT("DIF_PROPERTYCHANGE"));
			break;

		case DIF_ENABLECLASS:
			Trace(TEXT("DIF_ENABLECLASS"));
			break;

		case DIF_DETECTVERIFY:
			Trace(TEXT("DIF_DETECTVERIFY"));
			break;

		case DIF_INSTALLDEVICEFILES:
			Trace(TEXT("DIF_INSTALLDEVICEFILES"));
			break;

		case DIF_ALLOW_INSTALL:
			Trace(TEXT("DIF_ALLOW_INSTALL"));
			break;

		case DIF_SELECTBESTCOMPATDRV:
			Trace(TEXT("DIF_SELECTBESTCOMPATDRV"));
			break;

		case DIF_REGISTERDEVICE:
			Trace(TEXT("DIF_REGISTERDEVICE"));
			break;

		case DIF_NEWDEVICEWIZARD_PRESELECT:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_PRESELECT"));
			break;

		case DIF_NEWDEVICEWIZARD_SELECT:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_SELECT"));
			break;

		case DIF_NEWDEVICEWIZARD_PREANALYZE:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_PREANALYZE"));
			break;

		case DIF_NEWDEVICEWIZARD_POSTANALYZE:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_POSTANALYZE"));
			break;

		case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
			Trace(TEXT("DIF_NEWDEVICEWIZARD_FINISHINSTALL"));
			ret = NewDeviceWizardFinishInstall(InstallFunction, hDeviceInfoSet, pDeviceInfoData);
			break;

		case DIF_INSTALLINTERFACES:
			Trace(TEXT("DIF_INSTALLINTERFACES"));
			ret = UpdateInstallDeviceFlags(hDeviceInfoSet, pDeviceInfoData, DI_INSTALLDISABLED);
			break;

		case DIF_DETECTCANCEL:
			Trace(TEXT("DIF_DETECTCANCEL"));
			break;

		case DIF_REGISTER_COINSTALLERS:
			Trace(TEXT("DIF_REGISTER_COINSTALLERS"));
			break;

		case DIF_ADDPROPERTYPAGE_ADVANCED:
			Trace(TEXT("DIF_ADDPROPERTYPAGE_ADVANCED"));
			ret = AddPropertyPageAdvanced(InstallFunction, hDeviceInfoSet, pDeviceInfoData);
			break;

		case DIF_ADDPROPERTYPAGE_BASIC:
			Trace(TEXT("DIF_ADDPROPERTYPAGE_BASIC"));
			break;

		case DIF_TROUBLESHOOTER:
			Trace(TEXT("DIF_TROUBLESHOOTER"));
			break;

		case DIF_POWERMESSAGEWAKE:
			Trace(TEXT("DIF_POWERMESSAGEWAKE"));
			break;

		default:
			Trace(TEXT("Unknown install function call"));
			break;
	}

	return ret;
}


VOID CALLBACK DriverInstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow)
{
	INSTALLERINFO installerInfo;
	LPTSTR lpInfPath;
	DWORD dwFlags;
	BOOL needReboot;
	HANDLE hHeap;
	HRESULT hr;
	DWORD res;

	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(hInst);

	if (!lpszCmdLine)
		return Trace(TEXT("Missing parameters"));

	Trace(TEXT("DriverInstall(%08X, %08X, %hs, %d)"), hwnd, hInst, lpszCmdLine, nCmdShow);

	hHeap = GetProcessHeap();
	if (!hHeap)
		return Trace(TEXT("Missing heap"));

	hr = CreateInstallInfo(hHeap, lpszCmdLine, &lpInfPath, &installerInfo);
	if (FAILED(hr))
		return Trace(TEXT("Missing install info"));

	if (nCmdShow == SW_HIDE)
		dwFlags = DRIVER_PACKAGE_SILENT;
	else
		dwFlags = 0;

	res = DriverPackageInstall(lpInfPath, dwFlags, &installerInfo, &needReboot);
	if (res != ERROR_SUCCESS && res != ERROR_NO_SUCH_DEVINST)
		Trace(TEXT("DriverPackageInstall failed: %08X"), res);

	HeapFree(hHeap, 0, lpInfPath);
}

VOID CALLBACK DriverUninstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow)
{
	INSTALLERINFO installerInfo;
	LPTSTR lpInfPath, lpDsInfPath;
	DWORD dwFlags, cbDsInfPath;
	BOOL needReboot;
	HANDLE hHeap;
	HRESULT hr;
	DWORD res;

	UNREFERENCED_PARAMETER(hwnd);
	UNREFERENCED_PARAMETER(hInst);

	if (!lpszCmdLine)
		return Trace(TEXT("Missing parameters"));

	Trace(TEXT("DriverUninstall(%08X, %08X, %hs, %d)"), hwnd, hInst, lpszCmdLine, nCmdShow);

	hHeap = GetProcessHeap();
	if (!hHeap)
		return Trace(TEXT("Missing heap"));

	hr = CreateInstallInfo(hHeap, lpszCmdLine, &lpInfPath, &installerInfo);
	if (FAILED(hr))
		return Trace(TEXT("Missing install info"));

	res = DriverPackageGetPath(lpInfPath, NULL, &cbDsInfPath);
	if (res == ERROR_INSUFFICIENT_BUFFER) {
		lpDsInfPath = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbDsInfPath * sizeof(TCHAR));
		if (lpDsInfPath) {
			res = DriverPackageGetPath(lpInfPath, lpDsInfPath, &cbDsInfPath);
			if (res == ERROR_SUCCESS) {
				Trace(TEXT("DsInfPath: %s"), lpDsInfPath);

				if (nCmdShow == SW_HIDE)
					dwFlags = DRIVER_PACKAGE_SILENT;
				else
					dwFlags = 0;

				res = DriverPackageUninstall(lpDsInfPath, dwFlags, &installerInfo, &needReboot);
				if (res != ERROR_SUCCESS)
					Trace(TEXT("DriverPackageUninstall failed: %08X"), res);
			} else
				Trace(TEXT("DriverPackageGetPath 2 failed: %08X"), res);

			HeapFree(hHeap, 0, lpDsInfPath);
		} else
			Trace(TEXT("HeapAlloc failed"));
	} else
		Trace(TEXT("DriverPackageGetPath 1 failed: %08X"), res);

	HeapFree(hHeap, 0, lpInfPath);
}

VOID CALLBACK DeviceInstall(_In_ HWND hwnd, _In_ HINSTANCE hInst, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow)
{
	LPTSTR lpInfPath, lpDsInfPath, lpClassName, lpHardwareId, lpHardwareIds;
	DWORD dwFlags, cbDsInfPath, cbClassName, cbHardwareId;
	SP_DEVINFO_DATA deviceInfoData;
	HDEVINFO hDeviceInfoSet;
	size_t cbHardwareIds;
	HANDLE hHeap;
	HRESULT hr;
	DWORD res;
	GUID guid;
	int i;

	UNREFERENCED_PARAMETER(hInst);

	if (!lpszCmdLine)
		return Trace(TEXT("Missing parameters"));

	Trace(TEXT("DeviceInstall(%08X, %08X, %hs, %d)"), hwnd, hInst, lpszCmdLine, nCmdShow);

	hHeap = GetProcessHeap();
	if (!hHeap)
		return Trace(TEXT("Missing heap"));

	hr = CreateInstallInfo(hHeap, lpszCmdLine, &lpInfPath, NULL);
	if (FAILED(hr))
		return Trace(TEXT("Missing install info"));

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
																Trace(TEXT("UpdateDriverForPlugAndPlayDevices failed: %08X"), GetLastError());

																res = SetupDiCallClassInstaller(DIF_REMOVE, hDeviceInfoSet, &deviceInfoData);
																if (!res)
																	Trace(TEXT("SetupDiCallClassInstaller 2 failed: %08X"), GetLastError());
															}
														} else
															Trace(TEXT("SetupDiCallClassInstaller 1 failed: %08X"), GetLastError());
													} else
														Trace(TEXT("SetupDiSetDeviceRegistryProperty failed: %08X"), GetLastError());

													HeapFree(hHeap, 0, lpHardwareIds);
												} else
													Trace(TEXT("StringCchAPrintf failed: %08X"), hr);
											} else
												Trace(TEXT("SetupDiGetDeviceInstanceId 2 failed: %08X"), hr);

											HeapFree(hHeap, 0, lpHardwareId);
										} else
											Trace(TEXT("HeapAlloc failed"));
									} else
										Trace(TEXT("SetupDiGetDeviceInstanceId 1 failed: %08X"), hr);

									SetupDiDeleteDeviceInfo(hDeviceInfoSet, &deviceInfoData);
								} else
									Trace(TEXT("SetupDiCreateDeviceInfo failed: %08X"), GetLastError());

								SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
							} else
								Trace(TEXT("SetupDiCreateDeviceInfoList failed: %08X"), GetLastError());
						} else
							Trace(TEXT("SetupDiGetINFClass 2 failed: %08X"), GetLastError());

						HeapFree(hHeap, 0, lpClassName);
					} else
						Trace(TEXT("HeapAlloc failed"));
				} else
					Trace(TEXT("SetupDiGetINFClass 1 failed: %08X"), GetLastError());
			} else
				Trace(TEXT("DriverPackageGetPath 2 failed: %08X"), res);

			HeapFree(hHeap, 0, lpDsInfPath);
		} else
			Trace(TEXT("HeapAlloc failed"));
	} else
		Trace(TEXT("DriverPackageGetPath 1 failed: %08X"), res);

	HeapFree(hHeap, 0, lpInfPath);
}


DWORD UpdateInstallDeviceFlags(_In_ HDEVINFO hDeviceInfoSet, _In_ PSP_DEVINFO_DATA pDeviceInfoData, _In_ DWORD dwFlags)
{
	SP_DEVINSTALL_PARAMS devInstallParams;
	BOOL res;

	ZeroMemory(&devInstallParams, sizeof(devInstallParams));
	devInstallParams.cbSize = sizeof(devInstallParams);
	res = SetupDiGetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	devInstallParams.Flags |= dwFlags;
	res = SetupDiSetDeviceInstallParams(hDeviceInfoSet, pDeviceInfoData, &devInstallParams);
	if (!res)
		return GetLastError();

	return NO_ERROR;
}


HRESULT CreateInstallInfo(_In_ HANDLE hHeap, _In_ LPSTR lpszCmdLine, _Inout_ LPTSTR *lpInfPath, _Out_opt_ PINSTALLERINFO pInstallerInfo)
{
	size_t cbInfPath;

	if (pInstallerInfo) {
		ZeroMemory(pInstallerInfo, sizeof(INSTALLERINFO));
		pInstallerInfo->pApplicationId = TEXT("{B7D5E900-AF40-11DD-AD8B-0800200C9A65}");
		pInstallerInfo->pDisplayName = TEXT("Scanner Access Now Easy - WIA Driver");
		pInstallerInfo->pProductName = pInstallerInfo->pDisplayName;
		pInstallerInfo->pMfgName = TEXT("Marc Hörsken");
	}

	return StringCchAPrintf(hHeap, lpInfPath, &cbInfPath, TEXT("%hs"), lpszCmdLine);
}
