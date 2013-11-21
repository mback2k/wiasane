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

#ifndef WINSANE_INTERNAL_H
#define WINSANE_INTERNAL_H

#if _MSC_VER > 1000
#pragma once
#endif

enum WINSANE_Remote_Procedure {
	WINSANE_NET_INIT = 0,
	WINSANE_NET_GET_DEVICES,
	WINSANE_NET_OPEN,
	WINSANE_NET_CLOSE,
	WINSANE_NET_GET_OPTION_DESCRIPTORS,
	WINSANE_NET_CONTROL_OPTION,
	WINSANE_NET_GET_PARAMETERS,
	WINSANE_NET_START,
	WINSANE_NET_CANCEL,
	WINSANE_NET_AUTHORIZE,
	WINSANE_NET_EXIT
};

#endif
