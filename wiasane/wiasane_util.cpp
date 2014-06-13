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

#include "wiasane_util.h"

#include <sti.h>
#include <wia.h>

HRESULT GetErrorCode(_In_ SANE_Status status)
{
	switch (status) {
		case SANE_STATUS_GOOD:
			return S_OK;
		case SANE_STATUS_UNSUPPORTED:
			return WIA_ERROR_INVALID_COMMAND;
		case SANE_STATUS_CANCELLED:
			return E_ABORT;
		case SANE_STATUS_DEVICE_BUSY:
			return WIA_ERROR_BUSY;
		case SANE_STATUS_INVAL:
			return WIA_ERROR_INCORRECT_HARDWARE_SETTING;
		case SANE_STATUS_EOF:
			return S_FALSE;
		case SANE_STATUS_JAMMED:
			return WIA_ERROR_PAPER_JAM;
		case SANE_STATUS_NO_DOCS:
			return WIA_ERROR_PAPER_EMPTY;
		case SANE_STATUS_COVER_OPEN:
			return WIA_ERROR_COVER_OPEN;
		case SANE_STATUS_IO_ERROR:
			return WIA_ERROR_DEVICE_COMMUNICATION;
		case SANE_STATUS_NO_MEM:
			return E_OUTOFMEMORY;
		case SANE_STATUS_ACCESS_DENIED:
			return E_ACCESSDENIED;
		default:
			return WIA_ERROR_GENERAL_ERROR;
	}
}
