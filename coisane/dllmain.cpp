/***************************************************************************
 *                  _       ___       _____
 *  Project        | |     / (_)___ _/ ___/____ _____  ___
 *                 | | /| / / / __ `/\__ \/ __ `/ __ \/ _ \
 *                 | |/ |/ / / /_/ /___/ / /_/ / / / /  __/
 *                 |__/|__/_/\__,_//____/\__,_/_/ /_/\___/
 *
 * Copyright (C) 2012 - 2014, Marc Hoersken, <info@marc-hoersken.de>
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

#include "dllmain.h"

#include <commctrl.h>

static HINSTANCE g_hModuleInstance = NULL; // instance of this CoInstaller (used for loading from a resource)
static HINSTANCE g_hStiCiInstance = NULL; // instance of the sti_ci.dll (used for loading from a resource)
static HANDLE g_hActivationContext = NULL; // global ActivationContext handle
static BOOL g_bCommonControls = FALSE;	// global CommonControls initialization status

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX initCommonControlsEx;
	ACTCTX actCtx;

	UNREFERENCED_PARAMETER(lpReserved);

	g_hModuleInstance = (HINSTANCE) hModule;

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			if (!g_hStiCiInstance) {
				g_hStiCiInstance = LoadLibrary(TEXT("sti_ci.dll"));
			}

		case DLL_THREAD_ATTACH:
			if (!g_bCommonControls) {
				ZeroMemory(&initCommonControlsEx, sizeof(INITCOMMONCONTROLSEX));
				initCommonControlsEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
				initCommonControlsEx.dwICC = ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
				g_bCommonControls = InitCommonControlsEx(&initCommonControlsEx);
			}
			if (!g_hActivationContext) {
				ZeroMemory(&actCtx, sizeof(ACTCTX));
				actCtx.cbSize = sizeof(ACTCTX);
				actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
				actCtx.hModule = hModule;
				actCtx.lpResourceName = TEXT("MANIFEST");
				g_hActivationContext = CreateActCtx(&actCtx);
			}
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if (g_hStiCiInstance) {
				FreeLibrary(g_hStiCiInstance);
				g_hStiCiInstance = NULL;
			}
			break;
	}

	return TRUE;
}

HINSTANCE GetModuleInstance()
{
	return g_hModuleInstance;
}

HINSTANCE GetStiCiInstance()
{
	return g_hStiCiInstance;
}

HANDLE GetActivationContext()
{
	return g_hActivationContext;
}
