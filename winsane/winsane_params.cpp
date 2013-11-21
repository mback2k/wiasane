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

#include "winsane_params.h"
#include "winsane_internal.h"

WINSANE_Params::WINSANE_Params(_In_ PWINSANE_Device device, _In_ PWINSANE_Socket sock, _In_ PSANE_Parameters sane_params)
{
	this->device = device;
	this->sock = sock;
	this->sane_params = sane_params;
}

WINSANE_Params::~WINSANE_Params()
{
	delete this->sane_params;

	this->device = NULL;
	this->sock = NULL;
}


SANE_Frame WINSANE_Params::GetFormat()
{
	return this->sane_params->format;
}

SANE_Bool WINSANE_Params::IsLastFrame()
{
	return this->sane_params->last_frame;
}

SANE_Int WINSANE_Params::GetBytesPerLine()
{
	return this->sane_params->bytes_per_line;
}

SANE_Int WINSANE_Params::GetPixelsPerLine()
{
	return this->sane_params->pixels_per_line;
}

SANE_Int WINSANE_Params::GetLines()
{
	return this->sane_params->lines;
}

SANE_Int WINSANE_Params::GetDepth()
{
	return this->sane_params->depth;
}
