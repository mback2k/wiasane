/* sane - Scanner Access Now Easy.
   Copyright (C) 1997-1999 David Mosberger-Tang and Andreas Beck
   This file is part of the SANE package.

   This file is in the public domain.  You may use and modify it as
   you see fit, as long as this copyright message is included and
   that there is an indication as to what modifications have been
   made (if any).

   SANE is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   This file declares SANE application interface.  See the SANE
   standard for a detailed explanation of the interface.  */

#ifndef SANE_H
#define SANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>

#define SANE_CURRENT_MAJOR	1
#define SANE_CURRENT_MINOR	0

#define SANE_VERSION_CODE(major, minor, build)	\
  (  (((SANE_Word) (major) &   0xff) << 24)	\
   | (((SANE_Word) (minor) &   0xff) << 16)	\
   | (((SANE_Word) (build) & 0xffff) <<  0))

#define SANE_VERSION_MAJOR(code)	((((SANE_Word)(code)) >> 24) &   0xff)
#define SANE_VERSION_MINOR(code)	((((SANE_Word)(code)) >> 16) &   0xff)
#define SANE_VERSION_BUILD(code)	((((SANE_Word)(code)) >>  0) & 0xffff)

#define SANE_FALSE	0
#define SANE_TRUE	1

typedef BYTE SANE_Byte, *PSANE_Byte;
typedef INT32 SANE_Word, *PSANE_Word;
typedef SANE_Word SANE_Bool, *PSANE_Bool;
typedef SANE_Word SANE_Int, *PSANE_Int;
typedef CHAR SANE_Char, *PSANE_Char;
typedef PSTR SANE_String, *PSANE_String;
typedef PCSTR SANE_String_Const, *PSANE_String_Const;
typedef PVOID SANE_Handle, *PSANE_Handle;
typedef SANE_Word SANE_Fixed, *PSANE_Fixed;

#define SANE_FIXED_SCALE_SHIFT	16
#define SANE_FIX(v)		((SANE_Word) ((v) * (1 << SANE_FIXED_SCALE_SHIFT)))
#define SANE_UNFIX(v)	((double)(v) / (1 << SANE_FIXED_SCALE_SHIFT))

typedef enum {
	SANE_STATUS_GOOD = 0,		/* everything A-OK */
	SANE_STATUS_UNSUPPORTED,	/* operation is not supported */
	SANE_STATUS_CANCELLED,		/* operation was cancelled */
	SANE_STATUS_DEVICE_BUSY,	/* device is busy; try again later */
	SANE_STATUS_INVAL,			/* data is invalid (includes no dev at open) */
	SANE_STATUS_EOF,			/* no more data available (end-of-file) */
	SANE_STATUS_JAMMED,			/* document feeder jammed */
	SANE_STATUS_NO_DOCS,		/* document feeder out of documents */
	SANE_STATUS_COVER_OPEN,		/* scanner cover is open */
	SANE_STATUS_IO_ERROR,		/* error during device I/O */
	SANE_STATUS_NO_MEM,			/* out of memory */
	SANE_STATUS_ACCESS_DENIED	/* access to resource has been denied */
} SANE_Status, *PSANE_Status;

typedef enum {
	SANE_TYPE_BOOL = 0,
	SANE_TYPE_INT,
	SANE_TYPE_FIXED,
	SANE_TYPE_STRING,
	SANE_TYPE_BUTTON,
	SANE_TYPE_GROUP
} SANE_Value_Type, *PSANE_Value_Type;

typedef enum {
	SANE_UNIT_NONE = 0,			/* the value is unit-less (e.g., # of scans) */
	SANE_UNIT_PIXEL,			/* value is number of pixels */
	SANE_UNIT_BIT,				/* value is number of bits */
	SANE_UNIT_MM,				/* value is millimeters */
	SANE_UNIT_DPI,				/* value is resolution in dots/inch */
	SANE_UNIT_PERCENT,			/* value is a percentage */
	SANE_UNIT_MICROSECOND		/* value is micro seconds */
} SANE_Unit, *PSANE_Unit;

typedef struct {
	SANE_String_Const name;		/* unique device name */
	SANE_String_Const vendor;	/* device vendor string */
	SANE_String_Const model;	/* device model name */
	SANE_String_Const type;		/* device type (e.g., "flatbed scanner") */
} SANE_Device, *PSANE_Device;

#define SANE_CAP_SOFT_SELECT		(1 << 0)
#define SANE_CAP_HARD_SELECT		(1 << 1)
#define SANE_CAP_SOFT_DETECT		(1 << 2)
#define SANE_CAP_EMULATED			(1 << 3)
#define SANE_CAP_AUTOMATIC			(1 << 4)
#define SANE_CAP_INACTIVE			(1 << 5)
#define SANE_CAP_ADVANCED			(1 << 6)

#define SANE_OPTION_IS_ACTIVE(cap)		(((cap) & SANE_CAP_INACTIVE) == 0)
#define SANE_OPTION_IS_SETTABLE(cap)	(((cap) & SANE_CAP_SOFT_SELECT) != 0)

#define SANE_INFO_INEXACT			(1 << 0)
#define SANE_INFO_RELOAD_OPTIONS	(1 << 1)
#define SANE_INFO_RELOAD_PARAMS		(1 << 2)

typedef enum {
	SANE_CONSTRAINT_NONE = 0,
	SANE_CONSTRAINT_RANGE,
	SANE_CONSTRAINT_WORD_LIST,
	SANE_CONSTRAINT_STRING_LIST
} SANE_Constraint_Type, *PSANE_Constraint_Type;

typedef struct {
	SANE_Word min;				/* minimum (element) value */
	SANE_Word max;				/* maximum (element) value */
	SANE_Word quant;			/* quantization value (0 if none) */
} SANE_Range, *PSANE_Range;

typedef struct {
	SANE_String_Const name;		/* name of this option (command-line name) */
	SANE_String_Const title;	/* title of this option (single-line) */
	SANE_String_Const desc;		/* description of this option (multi-line) */
	SANE_Value_Type type;		/* how are values interpreted? */
	SANE_Unit unit;				/* what is the (physical) unit? */
	SANE_Int size;
	SANE_Int cap;				/* capabilities */

	SANE_Constraint_Type constraint_type;
	union {
		SANE_String_Const *string_list;		/* NULL-terminated list */
		SANE_Word *word_list;				/* first element is list-length */
		SANE_Range *range;
	} constraint;
} SANE_Option_Descriptor, *PSANE_Option_Descriptor;

typedef enum {
	SANE_ACTION_GET_VALUE = 0,
	SANE_ACTION_SET_VALUE,
	SANE_ACTION_SET_AUTO
} SANE_Action, *PSANE_Action;

typedef enum {
	SANE_FRAME_GRAY,	/* band covering human visual range */
	SANE_FRAME_RGB,		/* pixel-interleaved red/green/blue bands */
	SANE_FRAME_RED,		/* red band only */
	SANE_FRAME_GREEN,	/* green band only */
	SANE_FRAME_BLUE 	/* blue band only */
} SANE_Frame, *PSANE_Frame;

typedef struct {
	SANE_Frame format;
	SANE_Bool last_frame;
	SANE_Int bytes_per_line;
	SANE_Int pixels_per_line;
	SANE_Int lines;
	SANE_Int depth;
} SANE_Parameters, *PSANE_Parameters;

struct SANE_Auth_Data;

#define SANE_MAX_USERNAME_LEN	128
#define SANE_MAX_PASSWORD_LEN	128

typedef void (*SANE_Auth_Callback) (SANE_String_Const resource, SANE_Char *username, SANE_Char *password);

#endif
