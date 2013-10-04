#ifndef WINSANE_H
#define WINSANE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "sane.h"

#define WINSANE_DEFAULT_PORT			6566

#ifdef WINSANE_EXPORTS
#define WINSANE_API __declspec(dllexport)
#else
#define WINSANE_API __declspec(dllimport)
#endif

class WINSANE_API WINSANE_Session;
class WINSANE_API WINSANE_Device;
class WINSANE_API WINSANE_Option;
class WINSANE_API WINSANE_Params;
class WINSANE_API WINSANE_Scan;

#include "winsane_session.h"
#include "winsane_device.h"
#include "winsane_option.h"
#include "winsane_params.h"
#include "winsane_scan.h"

#endif
