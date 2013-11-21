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

#ifndef WIASANE_OPT_H
#define WIASANE_OPT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#include "winsane.h"

HRESULT GetOptionMaxValue(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT GetOptionValueInch(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT GetOptionMaxValueInch(_In_ PWINSANE_Option option, _Out_ double *value);
HRESULT IsValidOptionValueInch(_In_ PWINSANE_Option option, _In_ double value);
HRESULT SetOptionValueInch(_In_ PWINSANE_Option option, _In_ double value);

#endif