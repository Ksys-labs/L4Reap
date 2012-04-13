/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifndef MMSFBBASE_H_
#define MMSFBBASE_H_

#include <cstring>
#include <string.h>
#include "mmstools/base.h"
#include "mmstools/mmstypes.h"
#include "mmstools/mmsmutex.h"
#include "mmstools/mmstafffile.h"

#ifdef __HAVE_XLIB__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#endif

#ifdef __HAVE_XV__
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/xf86vmode.h>
#endif


#ifdef  __HAVE_DIRECTFB__
extern "C" {
#include <direct/debug.h>
#include <direct/trace.h>

#include <directfb.h>
#include <directfb_keynames.h>
#include <directfb_strings.h>
#include <directfb_util.h>
}
#endif

#ifdef MMSFB_TRACE_ENABLE
#define MMSFB_TRACE()    direct_trace_print_stack(NULL)
#else
#define MMSFB_TRACE()    do {} while (0)
#endif

#ifdef MMSFB_BREAK_ENABLE
#define MMSFB_BREAK()    D_BREAK("bad function")
#else
#define MMSFB_BREAK()    do {} while (0)
#endif



// switch between window managers
//#define USE_DFB_WINMAN
#define USE_MMSFB_WINMAN

// access to the last error text
extern string MMSFB_LastErrorString;

// screen should be rotated by 180°?
extern bool MMSFBBase_rotate180;



//! surface blitting flags
typedef unsigned int MMSFBBlittingFlags;

//! max combination of blitting flags
#define MMSFB_BLIT_CNT					0x40

//! surface blitting flag: no effects
#define MMSFB_BLIT_NOFX					0x00000000
//! surface blitting flag: use alphachannel from source
#define MMSFB_BLIT_BLEND_ALPHACHANNEL	0x00000001
//! surface blitting flag: use alphachannel from color
#define MMSFB_BLIT_BLEND_COLORALPHA		0x00000002
//! surface blitting flag: modulates source pixel with color
#define MMSFB_BLIT_COLORIZE				0x00000004
//! surface blitting flag: modulates the source color with the source alpha
#define MMSFB_BLIT_SRC_PREMULTIPLY		0x00000008
//! surface blitting flag: modulates the source color with the color alpha
#define MMSFB_BLIT_SRC_PREMULTCOLOR		0x00000010
//! surface blitting flag: antialiasing (stretchBlit(), not all pixelformats will be supported)
#define MMSFB_BLIT_ANTIALIASING			0x00000020

//! surface drawing flags
typedef unsigned int MMSFBDrawingFlags;

//! max combination of drawing flags
#define MMSFB_DRAW_CNT					0x04

//! surface drawing flag: no effects
#define MMSFB_DRAW_NOFX					0x00000000
//! surface drawing flag: alpha from surface color
#define MMSFB_DRAW_BLEND				0x00000001
//! surface drawing flag: multiplies the color with the alpha channel
#define MMSFB_DRAW_SRC_PREMULTIPLY		0x00000002


//! surface flip flags
typedef unsigned int MMSFBFlipFlags;

//! surface flip flag: none
#define MMSFB_FLIP_NONE					0x00000000
//! surface flip flag: returns upon vertical sync
#define MMSFB_FLIP_WAIT					0x00000001
//! surface flip flag: flip upon the next vertical sync
#define MMSFB_FLIP_ONSYNC				0x00000002
//! surface flip flag: WAIT and ONSYNC
#define MMSFB_FLIP_WAITFORSYNC			0x00000004
//! surface flip flag: send pan event to fbdev every time the display buffer has changed
#define MMSFB_FLIP_FLUSH				0x00000008


//! surface lock flags
typedef unsigned int MMSFBLockFlags;

//! surface lock flag: none
#define MMSFB_LOCK_NONE				0x00000000
//! surface lock flag: read access
#define MMSFB_LOCK_READ				0x00000001
//! surface lock flag: write access
#define MMSFB_LOCK_WRITE				0x00000002


//! surface merging mode
typedef enum {
	//! surface merging mode: anaglyph red-cyan (red channel from first, green and blue channels from second source surface)
	MMSFB_MM_ANAGLYPH_RED_CYAN = 0,
	//! surface merging mode: line interleaved (even lines from first, odd lines from second source surface)
	MMSFB_MM_LINE_INTERLEAVED
} MMSFBMergingMode;




//! buffer mode: none
#define MMSFB_BM_NONE       ""
//! buffer mode: no backbuffer
#define MMSFB_BM_FRONTONLY  "FRONTONLY"
//! buffer mode: backbuffer in video memory
#define MMSFB_BM_BACKVIDEO  "BACKVIDEO"
//! buffer mode: backbuffer in system memory
#define MMSFB_BM_BACKSYSTEM "BACKSYSTEM"
//! buffer mode: triple buffering
#define MMSFB_BM_TRIPLE     "TRIPLE"
//! buffer mode: no layer buffers at all, using buffer of each window
#define MMSFB_BM_WINDOWS    "WINDOWS"


//! layer option: none
#define MMSFB_LO_NONE               ""
//! layer option: Make usage of alpha channel for blending on a pixel per pixel basis.
#define MMSFB_LO_ALPHACHANNEL       "ALPHACHANNEL"
//! layer option: Enable flicker filtering.
#define MMSFB_LO_FLICKER_FILTERING  "FLICKER_FILTERING"
//! layer option: Enable deinterlacing of an interlaced (video) source.
#define MMSFB_LO_DEINTERLACING      "DEINTERLACING"
//! layer option: Enable source color key.
#define MMSFB_LO_SRC_COLORKEY       "SRC_COLORKEY"
//! layer option: Enable dest. color key.
#define MMSFB_LO_DST_COLORKEY       "DST_COLORKEY"
//! layer option: Make usage of the global alpha factor set by SetOpacity.
#define MMSFB_LO_OPACITY            "OPACITY"
//! layer option: Set field parity.
#define MMSFB_LO_FIELD_PARITY       "FIELD_PARITY"


// error logging routines
string MMSFB_ErrorString(const int rc, const string msg);
void MMSFB_SetError(const int rc, const string msg);

typedef struct {
	int bits;
	int red_length;
	int red_offset;
	int green_length;
	int green_offset;
	int blue_length;
	int blue_offset;
	int transp_length;
	int transp_offset;
} MMSFBPixelDef;

// pixelformat helpers
bool isAlphaPixelFormat(MMSFBSurfacePixelFormat pf);
bool isIndexedPixelFormat(MMSFBSurfacePixelFormat pf);
bool isRGBPixelFormat(MMSFBSurfacePixelFormat pf);
void getBitsPerPixel(MMSFBSurfacePixelFormat pf, MMSFBPixelDef *pixeldef);

#ifdef  __HAVE_DIRECTFB__
// dfb specific routines

// conversion routines for pixel formats
MMSFBSurfacePixelFormat getMMSFBPixelFormatFromDFBPixelFormat(DFBSurfacePixelFormat pf);
DFBSurfacePixelFormat getDFBPixelFormatFromMMSFBPixelFormat(MMSFBSurfacePixelFormat pf);

// conversion routines for layer buffer modes
string getDFBLayerBufferModeString(DFBDisplayLayerBufferMode bm);
DFBDisplayLayerBufferMode getDFBLayerBufferModeFromString(string bm);

// conversion routines for layer options
string getDFBLayerOptionsString(DFBDisplayLayerOptions opts);
DFBDisplayLayerOptions getDFBLayerOptionsFromString(string opts);


DFBSurfaceBlittingFlags getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(MMSFBBlittingFlags flags);
DFBSurfaceDrawingFlags getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(MMSFBDrawingFlags flags);
DFBSurfaceFlipFlags getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(MMSFBFlipFlags flags);
DFBSurfaceLockFlags getDFBSurfaceLockFlagsFromMMSFBLockFlags(MMSFBLockFlags flags);
#endif

void calcAspectRatio(int sw, int sh, int dw, int dh, MMSFBRectangle &dest, bool aspect_ratio, bool even_aligned);




#ifdef __ENABLE_ACTMON__

#define MMSFBPERF_START_MEASURING { \
	struct timeval perf_stime; \
	if (mmsperf) mmsperf->startMeasuring(&perf_stime);

#define MMSFBPERF_STOP_MEASURING_FILLRECT(surface, dw, dh) \
	if (mmsperf) mmsperf->stopMeasuringFillRectangle( \
					&perf_stime, surface, dw, dh); }

#define MMSFBPERF_STOP_MEASURING_DRAWLINE(surface, x1, y1, x2, y2) { \
	int w=x2-x1;if(w<0)w=-w;w++; \
	int h=y2-y1;if(h<0)h=-h;h++; \
	if (mmsperf) mmsperf->stopMeasuringDrawLine( \
					&perf_stime, surface, (w>=h)?w:h); }}

#define MMSFBPERF_STOP_MEASURING_DRAWSTRING(surface, clipreg, text, len, x, y) { \
	int w, h; \
	surface->config.font->getStringWidth(text, len, &w); \
	surface->config.font->getHeight(&h); \
	MMSFBRegion myreg = clipreg; \
	if (x+w-1 < myreg.x2) myreg.x2 = x+w-1; \
	if (y+h-1 < myreg.y2) myreg.y2 = y+h-1; \
	if (x > myreg.x1) myreg.x1 = x; \
	if (y > myreg.y1) myreg.y1 = y; \
	if ((myreg.x1 <= myreg.x2) && (myreg.y1 <= myreg.y2)) \
	if (mmsperf) mmsperf->stopMeasuringDrawString( \
					&perf_stime, surface, myreg.x2 - myreg.x1 + 1, myreg.y2 - myreg.y1 + 1); }}

#define MMSFBPERF_STOP_MEASURING_BLIT(surface, src_pixelformat, sw, sh) \
	if (mmsperf) mmsperf->stopMeasuringBlit( \
					&perf_stime, surface, src_pixelformat, sw, sh); }

#define MMSFBPERF_STOP_MEASURING_STRETCHBLIT(surface, source, sw, sh, dw, dh) \
	if (mmsperf) mmsperf->stopMeasuringStretchBlit( \
					&perf_stime, surface, src_pixelformat, sw, sh, dw, dh); }

#define MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(surface, sw, sh) \
	if (mmsperf) mmsperf->stopMeasuringXShmPutImage( \
					&perf_stime, surface, sw, sh); }

#define MMSFBPERF_STOP_MEASURING_XVSHMPUTIMAGE(surface, sw, sh, dw, dh) \
	if (mmsperf) mmsperf->stopMeasuringXvShmPutImage( \
					&perf_stime, surface, sw, sh, dw, dh); }

#define MMSFBPERF_STOP_MEASURING_VSYNC(surface) \
	if (mmsperf) mmsperf->stopMeasuringVSync( \
					&perf_stime, surface); }

#define MMSFBPERF_STOP_MEASURING_SWAPDISPLAY(surface) \
	if (mmsperf) mmsperf->stopMeasuringSwapDisplay( \
					&perf_stime, surface); }

#else

#define MMSFBPERF_START_MEASURING
#define MMSFBPERF_STOP_MEASURING_FILLRECT(surface, dw, dh)
#define MMSFBPERF_STOP_MEASURING_DRAWLINE(surface, x1, y1, x2, y2)
#define MMSFBPERF_STOP_MEASURING_DRAWSTRING(surface, clipreg, text, len, x, y)
#define MMSFBPERF_STOP_MEASURING_BLIT(surface, src_pixelformat, sw, sh)
#define MMSFBPERF_STOP_MEASURING_STRETCHBLIT(surface, source, sw, sh, dw, dh)
#define MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(surface, sw, sh)
#define MMSFBPERF_STOP_MEASURING_XVSHMPUTIMAGE(surface, sw, sh, dw, dh)
#define MMSFBPERF_STOP_MEASURING_VSYNC(surface)
#define MMSFBPERF_STOP_MEASURING_SWAPDISPLAY(surface)

#endif



#endif /*MMSFBBASE_H_*/
