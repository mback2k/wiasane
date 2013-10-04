#include "winsane_params.h"
#include "winsane_internal.h"

WINSANE_Params::WINSANE_Params(WINSANE_Device *device, WINSANE_Socket *sock, SANE_Parameters *sane_params)
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
