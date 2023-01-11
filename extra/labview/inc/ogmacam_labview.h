#ifndef __ogmacam_labview_h__
#define __ogmacam_labview_h__

#include "extcode.h"

#ifdef OGMACAM_LABVIEW_EXPORTS
#define OGMACAM_LABVIEW_API(x) __declspec(dllexport)    x   __cdecl
#else
#define OGMACAM_LABVIEW_API(x) __declspec(dllimport)    x   __cdecl
#include "ogmacam.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

OGMACAM_LABVIEW_API(HRESULT) Start(HOgmacam h, LVUserEventRef* rwer);

#ifdef __cplusplus
}
#endif

#endif