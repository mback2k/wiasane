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

#ifndef WIASANE_DEF_H
#define WIASANE_DEF_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#include "wiasane.h"

HRESULT ReadRegistryInformation(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeRegistryInformation(_Inout_ PSCANINFO pScanInfo, _In_ PWIASANE_Context pContext);

HRESULT InitScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);
HRESULT FreeScannerDefaults(_Inout_ PSCANINFO pScanInfo, _Inout_ PWIASANE_Context pContext);

#endif
