/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#ifndef MMSTYPES_H_
#define MMSTYPES_H_

using namespace std;

#include <sigc++/sigc++.h>
#include <string>

// sigc++ accumulators.......................................................

//example from: http://libsigc.sourceforge.net/libsigc2/docs/reference/html/classsigc_1_1signal_1_1accumulated.html
//! this accumulator calculates the arithmetic mean value
struct arithmetic_mean_accumulator
{
  typedef double result_type;
  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const
  {
    result_type value_ = 0;
    int n_ = 0;
    for (; first != last; ++first, ++n_)
      value_ += *first;
    return value_ / n_;
  }
};

//example from: http://libsigc.sourceforge.net/libsigc2/docs/reference/html/classsigc_1_1signal_1_1accumulated.html
//! this accumulator stops signal emission when a slot returns zero
struct interruptable_accumulator
{
  typedef bool result_type;
  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const
  {
    int n_ = 0;
    for (; first != last; ++first, ++n_)
      if (!*first) return false;
    return true;
  }
};

//! bool accumulator
/*!
with this accumulator the emit() method of a callback ends with
 - true,  if the no callback methods are connected or all connected callback methods returns true
 - false, if at least one connected callback method returns false
*/
struct bool_accumulator
{
  typedef bool result_type;
  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const
  {
    bool ret_ = true;
    int n_ = 0;
    for (; first != last; ++first, ++n_)
      if (!*first) ret_ = false;
    return ret_;
  }
};


//! bool accumulator (not)
/*!
with this accumulator the emit() method of a callback ends with
 - false, if the no callback methods are connected or all connected callback methods returns false
 - true,  if at least one connected callback method returns true
*/
struct neg_bool_accumulator
{
  typedef bool result_type;
  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const
  {
    bool ret_ = false;
    int n_ = 0;
    for (; first != last; ++first, ++n_)
      if (*first) ret_ = true;
    return ret_;
  }
};



// plane description.........................................................

//! Describes up to 3 planes of an surface buffer.
class MMSFBSurfacePlanes {
	public:
	    //! buffer is a hardware buffer?
	    bool	hwbuffer;
	    //! the current pixel data describes a full opaque surface
	    bool	opaque;
	    //! the current pixel data describes a full transparent surface
	    bool	transparent;
		//! first plane
		void	*ptr;
		//! pitch of first plane
		int 	pitch;
		//! second plane or NULL if surface has only one plane
		void	*ptr2;
		//! pitch of second plane
		int 	pitch2;
		//! third plane or NULL if surface has only one or two planes
		void	*ptr3;
		//! pitch of third plane
		int 	pitch3;

		MMSFBSurfacePlanes(void *ptr = NULL, int pitch = 0, void *ptr2 = NULL, int pitch2 = 0, void *ptr3 = NULL, int pitch3 = 0) {
			this->hwbuffer		= false;
		    this->opaque		= false;
		    this->transparent	= false;
			this->ptr			= ptr;
			this->pitch			= pitch;
			this->ptr2			= ptr2;
			this->pitch2		= pitch2;
			this->ptr3			= ptr3;
			this->pitch3		= pitch3;
		}
};

//! for compatibility reason
#define MMSFBExternalSurfaceBuffer	MMSFBSurfacePlanes

//! max. number of buffers (3=TRIPLE buffering)
#define MMSFB_MAX_SURFACE_PLANES_BUFFERS	3

//! describes multiple buffers for backbuffer/triple buffer handling
typedef class MMSFBSurfacePlanes MMSFBSurfacePlanesBuffer[MMSFB_MAX_SURFACE_PLANES_BUFFERS];


// display backend types.....................................................

//! supported display backends
typedef enum {
	//! none
	MMSFB_BE_NONE = 0,
	//! directfb backend
	MMSFB_BE_DFB,
	//! X11 backend from disko framework
	MMSFB_BE_X11,
	//! FBDEV backend from disko framework
	MMSFB_BE_FBDEV,
	//! L4Re fb backend
	MMSFB_BE_L4RE,
} MMSFBBackend;

//! backend: none
#define MMSFB_BE_NONE_STR		""
//! backend: DFB
#define MMSFB_BE_DFB_STR		"DFB"
//! backend: X11
#define MMSFB_BE_X11_STR		"X11"
//! backend: FBDEV
#define MMSFB_BE_FBDEV_STR		"FBDEV"
//! backend: L4RE
#define MMSFB_BE_L4RE_STR		"L4RE"

//! list of valid backend types
#define MMSFB_BE_VALID_VALUES	"DFB, X11, FBDEV, L4RE"

//! list of valid backend types for output types MMSFB_OT_xxxFB
#define MMSFB_BE_VALID_VALUES_OT_FB	"DFB, FBDEV"

//! list of valid backend types for output type MMSFB_OT_X11
#define MMSFB_BE_VALID_VALUES_OT_X11	"DFB, X11"

//! list of valid backend types for output type MMSFB_OT_X
#define MMSFB_BE_VALID_VALUES_OT_X		"X11"

// conversion routines for backend types
string getMMSFBBackendString(MMSFBBackend be);
MMSFBBackend getMMSFBBackendFromString(string be);


// output types..............................................................

//! supported output types
typedef enum {
	//! none
	MMSFB_OT_NONE = 0,
	//! STDFB (backend: DFB and FBDEV)
	MMSFB_OT_STDFB,
	//! MATROXFB (backend: DFB and FBDEV)
	MMSFB_OT_MATROXFB,
	//! VIAFB (backend: DFB)
	MMSFB_OT_VIAFB,
	//! X11 (backend: DFB and X11)
	MMSFB_OT_X11,
	//! XSHM (backend: X11)
	MMSFB_OT_XSHM,
	//! XVSHM (backend: X11)
	MMSFB_OT_XVSHM,
	//! DAVINCIFB (backend: DFB and FBDEV)
	MMSFB_OT_DAVINCIFB,
	//! OMAPFB (backend: DFB and FBDEV)
	MMSFB_OT_OMAPFB,
	//! OGL (backend: X11 and FBDEV)
	MMSFB_OT_OGL,
    //! L4FB (backend: L4RE)
    MMSFB_OT_L4FB,
} MMSFBOutputType;

//! output type: none
#define MMSFB_OT_NONE_STR		""
//! output type: STDFB (backend: DFB and FBDEV)
#define MMSFB_OT_STDFB_STR		"STDFB"
//! output type: MATROXFB (backend: DFB and FBDEV)
#define MMSFB_OT_MATROXFB_STR	"MATROXFB"
//! output type: VIAFB (backend: DFB)
#define MMSFB_OT_VIAFB_STR		"VIAFB"
//! output type: X11 (backend: DFB and X11)
#define MMSFB_OT_X11_STR		"X11"
//! output type: XSHM (backend: X11)
#define MMSFB_OT_XSHM_STR		"XSHM"
//! output type: XVSHM (backend: X11)
#define MMSFB_OT_XVSHM_STR		"XVSHM"
//! output type: DAVINCIFB (backend: DFB and FBDEV)
#define MMSFB_OT_DAVINCIFB_STR	"DAVINCIFB"
//! output type: OMAPFB (backend: DFB and FBDEV)
#define MMSFB_OT_OMAPFB_STR	"OMAPFB"
//! output type: OGL (backend: X11 and FBDEV)
#define MMSFB_OT_OGL_STR		"OGL"
//! output type: L4FB (backend: L4RE)
#define MMSFB_OT_L4FB_STR       "L4FB"

//! list of valid output types
#define MMSFB_OT_VALID_VALUES			"STDFB, MATROXFB, VIAFB, X11, XSHM, XVSHM, DAVINCIFB, OMAPFB, OGL, L4FB"

//! list of valid output types for backend MMSFB_BE_DFB
#define MMSFB_OT_VALID_VALUES_BE_DFB	"STDFB, MATROXFB, VIAFB, X11, DAVINCIFB, OMAPFB"

//! list of valid output types for backend MMSFB_BE_X11
#define MMSFB_OT_VALID_VALUES_BE_X11	"X11, XSHM, XVSHM, OGL"

//! list of valid output types for backend MMSFB_BE_FBDEV
#define MMSFB_OT_VALID_VALUES_BE_FBDEV	"STDFB, MATROXFB, DAVINCIFB, OMAPFB, OGL"

//! list of valid output types for backend MMSFB_BE_L4RE
#define MMSFB_OT_VALID_VALUES_BE_L4RE   "L4FB"

// conversion routines for output types
string getMMSFBOutputTypeString(MMSFBOutputType ot);
MMSFBOutputType getMMSFBOutputTypeFromString(string ot);


// full screen modes.........................................................

//! supported full screen modes
typedef enum {
	//! none
	MMSFB_FSM_NONE = 0,
	//! disabled
	MMSFB_FSM_FALSE,
	//! enabled
	MMSFB_FSM_TRUE,
	//! enabled, using the correct aspect ratio
	MMSFB_FSM_ASPECT_RATIO
} MMSFBFullScreenMode;

//! full screen mode: none
#define MMSFB_FSM_NONE_STR			""
//! full screen mode: disabled
#define MMSFB_FSM_FALSE_STR			"FALSE"
//! full screen mode: enabled
#define MMSFB_FSM_TRUE_STR			"TRUE"
//! full screen mode: enabled, using the correct aspect ratio
#define MMSFB_FSM_ASPECT_RATIO_STR	"ASPECT_RATIO"

//! list of valid full screen modes
#define MMSFB_FSM_VALID_VALUES		"FALSE, TRUE, ASPECT_RATIO"

// conversion routines for full screen modes
string getMMSFBFullScreenModeString(MMSFBFullScreenMode fsm);
MMSFBFullScreenMode getMMSFBFullScreenModeFromString(string fsm);


// pixelformats..............................................................

//! supported pixel formats
typedef enum {
	//! none
	MMSFB_PF_NONE = 0,
    //! 16 bit RGB (2 byte, red 5\@11, green 6\@5, blue 5\@0)
	MMSFB_PF_RGB16,
    //! 24 bit RGB (3 byte, red 8\@16, green 8\@8, blue 8\@0)
    MMSFB_PF_RGB24,
    //! 24 bit RGB (4 byte, nothing 8\@24, red 8\@16, green 8\@8, blue 8\@0)
    MMSFB_PF_RGB32,
    //! 32 bit ARGB (4 byte, alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
    MMSFB_PF_ARGB,
    //! 8 bit alpha (1 byte, alpha 8\@0), e.g. anti-aliased glyphs
    MMSFB_PF_A8,
    //! 16 bit YUV (4 byte/2 pixel, macropixel contains CbYCrY [31:0])
    MMSFB_PF_YUY2,
    //! 16 bit YUV (4 byte/2 pixel, macropixel contains YCbYCr [31:0])
    MMSFB_PF_UYVY,
    //! 12 bit YUV (8 bit Y plane followed by 8 bit quarter size U/V planes)
    MMSFB_PF_I420,
    //! 12 bit YUV (8 bit Y plane followed by 8 bit quarter size V/U planes)
    MMSFB_PF_YV12,
    //! 32 bit ARGB (4 byte, inv. alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
    MMSFB_PF_AiRGB,
    //! 1 bit alpha (1 byte/8 pixel, most significant bit used first)
    MMSFB_PF_A1,
    //! 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size CbCr [15:0] plane)
    MMSFB_PF_NV12,
    //! 16 bit YUV (8 bit Y plane followed by one 16 bit half width CbCr [15:0] plane)
    MMSFB_PF_NV16,
    //! 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size CrCb [15:0] plane)
    MMSFB_PF_NV21,
    //! 32 bit AYUV (4 byte, alpha 8\@24, Y 8\@16, Cb 8\@8, Cr 8\@0)
    MMSFB_PF_AYUV,
    //! 4 bit alpha (1 byte/2 pixel, more significant nibble used first)
    MMSFB_PF_A4,
    //! 19 bit ARGB (3 byte, nothing 5\@19, alpha 1\@18, red 6\@12, green 6\@6, blue 6\@0)
    MMSFB_PF_ARGB1666,
    //! 24 bit ARGB (3 byte, alpha 6\@18, red 6\@12, green 6\@6, blue 6\@0)
    MMSFB_PF_ARGB6666,
    //! 18 bit RGB (3 byte, nothing 6\@18, red 6\@12, green 6\@6, blue 6\@0)
    MMSFB_PF_RGB18,
    //! 2 bit LUT (1 byte/4 pixel, 2 bit color and alpha lookup from palette)
    MMSFB_PF_LUT2,
    //! 12 bit RGB (2 byte, nothing 4\@12, red 4\@8, green 4\@4, blue 4\@0)
    MMSFB_PF_RGB444,
    //! 15 bit RGB (2 byte, nothing 1\@15, red 5\@10, green 5\@5, blue 5\@0)
    MMSFB_PF_RGB555,
	//! 16 bit ARGB (2 byte, alpha 1\@15, red 5\@10, green 5\@5, blue 5\@0)
	MMSFB_PF_ARGB1555,
    //! 8 bit RGB (1 byte, red 3\@5, green 3\@2, blue 2\@0)
    MMSFB_PF_RGB332,
    //! 8 bit ALUT (1 byte, alpha 4\@4, color lookup 4\@0)
    MMSFB_PF_ALUT44,
    //! 8 bit LUT (8 bit color and alpha lookup from palette)
    MMSFB_PF_LUT8,
    //! 16 bit ARGB (2 byte, alpha 2\@14, red 5\@9, green 5\@4, blue 4\@0)
    MMSFB_PF_ARGB2554,
    //! 16 bit ARGB (2 byte, alpha 4\@12, red 4\@8, green 4\@4, blue 4\@0)
    MMSFB_PF_ARGB4444,
    //! 19 bit ARGB (16 bit RGB565 plane followed by 4 bit alpha plane (highest bit unused))
    MMSFB_PF_ARGB3565,
    //! 24 bit BGR (3 byte, blue 8\@16, green 8\@8, red 8\@0)
    MMSFB_PF_BGR24,
    //! 15 bit BGR (2 byte, nothing 1\@15, blue 5\@10, green 5\@5, red 5\@0)
    MMSFB_PF_BGR555,
    //! 32 bit ABGR (4 byte, alpha 8\@24, blue 8\@16, green 8\@8, red 8\@0)
    MMSFB_PF_ABGR,
    //! number of supported pixelformats
    MMSFB_PF_CNT
} MMSFBSurfacePixelFormat;

//! pixel format: none
#define MMSFB_PF_NONE_STR       ""
//! pixel format: 16 bit RGB (2 byte, red 5\@11, green 6\@5, blue 5\@0)
#define MMSFB_PF_RGB16_STR      "RGB16"
//! pixel format: 24 bit RGB (3 byte, red 8\@16, green 8\@8, blue 8\@0)
#define MMSFB_PF_RGB24_STR      "RGB24"
//! pixel format: 24 bit RGB (4 byte, nothing 8\@24, red 8\@16, green 8\@8, blue 8\@0)
#define MMSFB_PF_RGB32_STR      "RGB32"
//! pixel format: 32 bit ARGB (4 byte, alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
#define MMSFB_PF_ARGB_STR       "ARGB"
//! pixel format: 8 bit alpha (1 byte, alpha 8\@0), e.g. anti-aliased glyphs
#define MMSFB_PF_A8_STR         "A8"
//! pixel format: 16 bit YUV (4 byte/2 pixel, macropixel contains CbYCrY [31:0])
#define MMSFB_PF_YUY2_STR       "YUY2"
//! pixel format: 16 bit YUV (4 byte/2 pixel, macropixel contains YCbYCr [31:0])
#define MMSFB_PF_UYVY_STR       "UYVY"
//! pixel format: 12 bit YUV (8 bit Y plane followed by 8 bit quarter size U/V planes)
#define MMSFB_PF_I420_STR       "I420"
//! pixel format: 12 bit YUV (8 bit Y plane followed by 8 bit quarter size V/U planes)
#define MMSFB_PF_YV12_STR       "YV12"
//! pixel format: 32 bit ARGB (4 byte, inv. alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
#define MMSFB_PF_AiRGB_STR      "AiRGB"
//! pixel format: 1 bit alpha (1 byte/8 pixel, most significant bit used first)
#define MMSFB_PF_A1_STR         "A1"
//! pixel format: 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size CbCr [15:0] plane)
#define MMSFB_PF_NV12_STR       "NV12"
//! pixel format: 16 bit YUV (8 bit Y plane followed by one 16 bit half width CbCr [15:0] plane)
#define MMSFB_PF_NV16_STR       "NV16"
//! pixel format: 12 bit YUV (8 bit Y plane followed by one 16 bit quarter size CrCb [15:0] plane)
#define MMSFB_PF_NV21_STR       "NV21"
//! pixel format: 32 bit AYUV (4 byte, alpha 8\@24, Y 8\@16, Cb 8\@8, Cr 8\@0)
#define MMSFB_PF_AYUV_STR       "AYUV"
//! pixel format: 4 bit alpha (1 byte/2 pixel, more significant nibble used first)
#define	MMSFB_PF_A4_STR			"A4"
//! pixel format: 19 bit ARGB (3 byte, nothing 5\@19, alpha 1\@18, red 6\@12, green 6\@6, blue 6\@0)
#define	MMSFB_PF_ARGB1666_STR	"ARGB1666"
//! pixel format: 24 bit ARGB (3 byte, alpha 6\@18, red 6\@12, green 6\@6, blue 6\@0)
#define	MMSFB_PF_ARGB6666_STR	"ARGB6666"
//! pixel format: 18 bit RGB (3 byte, nothing 6\@18, red 6\@12, green 6\@6, blue 6\@0)
#define	MMSFB_PF_RGB18_STR		"RGB18"
//! pixel format: 2 bit LUT (1 byte/4 pixel, 2 bit color and alpha lookup from palette)
#define	MMSFB_PF_LUT2_STR		"LUT2"
//! pixel format: 12 bit RGB (2 byte, nothing 4\@12, red 4\@8, green 4\@4, blue 4\@0)
#define	MMSFB_PF_RGB444_STR		"RGB444"
//! pixel format: 15 bit RGB (2 byte, nothing 1\@15, red 5\@10, green 5\@5, blue 5\@0)
#define	MMSFB_PF_RGB555_STR		"RGB555"
//! pixel format: 16 bit ARGB (2 byte, alpha 1\@15, red 5\@10, green 5\@5, blue 5\@0)
#define MMSFB_PF_ARGB1555_STR   "ARGB1555"
//! pixel format: 8 bit RGB (1 byte, red 3\@5, green 3\@2, blue 2\@0)
#define MMSFB_PF_RGB332_STR     "RGB332"
//! pixel format: 8 bit ALUT (1 byte, alpha 4\@4, color lookup 4\@0)
#define MMSFB_PF_ALUT44_STR     "ALUT44"
//! pixel format: 8 bit LUT (8 bit color and alpha lookup from palette)
#define MMSFB_PF_LUT8_STR       "LUT8"
//! pixel format: 16 bit ARGB (2 byte, alpha 2\@14, red 5\@9, green 5\@4, blue 4\@0)
#define MMSFB_PF_ARGB2554_STR   "ARGB2554"
//! pixel format: 16 bit ARGB (2 byte, alpha 4\@12, red 4\@8, green 4\@4, blue 4\@0)
#define MMSFB_PF_ARGB4444_STR   "ARGB4444"
//! pixel format: 19 bit ARGB (16 bit RGB565 plane followed by 4 bit alpha plane (highest bit unused))
#define MMSFB_PF_ARGB3565_STR	"ARGB3565"
//! pixel format: 24 bit BGR (3 byte, blue 8\@16, green 8\@8, red 8\@0)
#define MMSFB_PF_BGR24_STR      "BGR24"
//! pixel format: 15 bit BGR (2 byte, nothing 1\@15, blue 5\@10, green 5\@5, red 5\@0)
#define MMSFB_PF_BGR555_STR     "BGR555"
//! pixel format: 32 bit ABGR (4 byte, alpha 8\@24, blue 8\@16, green 8\@8, red 8\@0)
#define MMSFB_PF_ABGR_STR       "ABGR"

//! list of valid pixelformats
#define MMSFB_PF_VALID_VALUES	"RGB16, RGB24, RGB32, ARGB, A8, YUY2, UYVY, I420, YV12, AiRGB, A1, NV12, NV16, NV21, AYUV, A4, ARGB1666, ARGB6666, RGB18, LUT2, RGB444, RGB555, ARGB1555, RGB332, ALUT44, LUT8, ARGB2554, ARGB4444, ARGB3565, BGR24, BGR555, ABGR"

//! list of valid pixelformats used for layer surfaces
#define MMSFB_PF_VALID_VALUES_LAYER	"RGB16, RGB24, RGB32, ARGB, YUY2, UYVY, I420, YV12, AiRGB, NV12, NV16, NV21, AYUV, ARGB1666, ARGB6666, RGB18, LUT2, RGB444, RGB555, ARGB1555, RGB332, LUT8, ARGB2554, ARGB4444, ARGB3565, BGR24, BGR555, ABGR"

//! list of valid pixelformats used for windows surfaces
#define MMSFB_PF_VALID_VALUES_WINDOWS	"ARGB, AiRGB, AYUV, ARGB4444, RGB16, ABGR, empty string for auto detection"

//! list of valid pixelformats used for worker surfaces
#define MMSFB_PF_VALID_VALUES_SURFACES	"ARGB, AiRGB, AYUV, ARGB4444, RGB16, ABGR, empty string for auto detection"

//! list of valid pixelformats for X11.XVSHM
#define MMSFB_PF_VALID_VALUES_BE_X11_OT_XVSHM	"YV12"

//! list of valid pixelformats for X11.XSHM
#define MMSFB_PF_VALID_VALUES_BE_X11_OT_XSHM	"RGB32, ARGB, YV12"

//! list of valid pixelformats for X11.OGL
#define MMSFB_PF_VALID_VALUES_BE_X11_OT_OGL	"RGB32, ARGB, ABGR"

//! list of valid pixelformats for DAVINCIFB, OSD Layer
#define MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_DAVINCIFB_LAYER_0	"ARGB3565, RGB16"

//! list of valid pixelformats for DAVINCIFB, Video Layer
#define MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_DAVINCIFB_LAYER_1	"YUY2"

//! list of valid pixelformats for OMAPFB, OSD Layer
#define MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OMAPFB_LAYER_0	"ARGB, RGB32, RGB16"

//! list of valid pixelformats for OMAPFB, Video Layer
#define MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OMAPFB_LAYER_1	"YUY2, RGB32"

//! list of valid pixelformats for FBDEV.OGL
#define MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OGL	"RGB32, ARGB, ABGR"

//! list of valid pixelformats used for layer surfaces
#define MMSFB_PF_VALID_BUFFERMODES "BACKVIDEO BACKSYSTEM FRONTONLY TRIPLE WINDOWS"
#define MMSFB_PF_VALID_BUFFERMODES_X11 "BACKVIDEO BACKSYSTEM TRIPLE WINDOWS"

// conversion routines for pixel formats
string getMMSFBPixelFormatString(MMSFBSurfacePixelFormat pf);
MMSFBSurfacePixelFormat getMMSFBPixelFormatFromString(string pf);


// color definition..........................................................

//! describes a color with alpha
class MMSFBColor {
	public:
		//! red
		unsigned char r;
		//! green
		unsigned char g;
		//! blue
		unsigned char b;
		//! alphachannel
		unsigned char a;

		MMSFBColor() {
			this->r = 0x00;
			this->g = 0x00;
			this->b = 0x00;
			this->a = 0x00;
		}

		MMSFBColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		MMSFBColor(unsigned int argb) {
			this->b = argb & 0xff;
			this->g = (argb << 16) >> 24;
			this->r = (argb << 8)  >> 24;
			this->a = argb >> 24;
		}

		unsigned int getARGB() {
			unsigned int argb;
			argb = (unsigned int)this->b;
			argb|= ((unsigned int)this->g) << 8;
			argb|= ((unsigned int)this->r) << 16;
			argb|= ((unsigned int)this->a) << 24;
			return argb;
		}

		bool operator==(const MMSFBColor &c) {
			return (this->r == c.r && this->g == c.g && this->b == c.b && this->a == c.a);
		}

		bool operator!=(const MMSFBColor &c) {
			return (this->r != c.r || this->g != c.g || this->b != c.b || this->a != c.a);
		}

};

//! Convert a color string into MMSFBColor.
/*!
The input string has the syntax "#rrggbbaa".

    rr - hex value for red
    gg - hex value for green
    bb - hex value for blue
    aa - hex value for alpha channel (value ff means full opaque)

\param input  the input string
\param color  pointer to the color to be returned
\return true if input is correct
\note If the function fails, the color is set to "#00000000".
*/
bool getMMSFBColorFromString(string input, MMSFBColor *color);


//! Convert MMSFBColor to a color string.
/*!
The output string has the syntax "#rrggbbaa".

    rr - hex value for red
    gg - hex value for green
    bb - hex value for blue
    aa - hex value for alpha channel (value ff means full opaque)

\param color  color to be converted
\return color string
*/
string getMMSFBColorString(MMSFBColor color);


// rectangles, regions, etc..................................................

//! describes a rectangle
class MMSFBRectangle {
	public:
		//! x
		int x;
		//! y
		int y;
		//! width
		int w;
		//! height
		int h;

		MMSFBRectangle(int x = 0, int y = 0, int w = 0, int h = 0) {
			this->x = x;
			this->y = y;
			this->w = w;
			this->h = h;
		}
};

//! describes a region
class MMSFBRegion {
	public:
		//! x1
		int x1;
		//! y1
		int y1;
		//! x2
		int x2;
		//! y2
		int y2;

		MMSFBRegion(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0) {
			this->x1 = x1;
			this->y1 = y1;
			this->x2 = x2;
			this->y2 = y2;
		}
};

//! describes a triangle
class MMSFBTriangle {
	public:
		//! x1
		int x1;
		//! y1
		int y1;
		//! x2
		int x2;
		//! y2
		int y2;
		//! x3
		int x3;
		//! y3
		int y3;

		MMSFBTriangle(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0, int x3 = 0, int y3 = 0) {
			this->x1 = x1;
			this->y1 = y1;
			this->x2 = x2;
			this->y2 = y2;
			this->x3 = x3;
			this->y3 = y3;
		}
};




// pointer mode..............................................................

//! supported pointer modes
typedef enum {
	//! none
	MMSFB_PM_NONE = 0,
	//! disabled
	MMSFB_PM_FALSE,
	//! enabled, using internal mouse pointer
	MMSFB_PM_TRUE,
	//! enabled, using external mouse pointer e.g. from X11, if using the X11 backend
	MMSFB_PM_EXTERNAL
} MMSFBPointerMode;

//! pointer mode: none
#define MMSFB_PM_NONE_STR			""
//! pointer mode: disabled
#define MMSFB_PM_FALSE_STR			"FALSE"
//! pointer mode: enabled, using internal mouse pointer
#define MMSFB_PM_TRUE_STR			"TRUE"
//! pointer mode: enabled, using external mouse pointer e.g. from X11, if using the X11 backend
#define MMSFB_PM_EXTERNAL_STR		"EXTERNAL"

//! list of valid pointer modes
#define MMSFB_PM_VALID_VALUES		"FALSE, TRUE, EXTERNAL"


// conversion routines for pointer modes
string getMMSFBPointerModeString(MMSFBPointerMode pm);
MMSFBPointerMode getMMSFBPointerModeFromString(string pm);

// media backend types.......................................................

//! supported media backends
typedef enum {
	//! none
	MMSMEDIA_BE_NONE = 0,
	//! xine
	MMSMEDIA_BE_XINE,
	//! gstreamer
	MMSMEDIA_BE_GST
} MMSMEDIABackend;



// key symbols...............................................................

//! supported keys
typedef enum {
	MMSKEY_UNKNOWN=0,
	MMSKEY_BACKSPACE=1,
	MMSKEY_TAB=2,
	MMSKEY_RETURN=3,
	MMSKEY_CANCEL=4,
	MMSKEY_ESCAPE=5,
	MMSKEY_SPACE=6,
	MMSKEY_EXCLAMATION_MARK=7,
	MMSKEY_QUOTATION=8,
	MMSKEY_NUMBER_SIGN=9,
	MMSKEY_DOLLAR_SIGN=10,
	MMSKEY_PERCENT_SIGN=11,
	MMSKEY_AMPERSAND=12,
	MMSKEY_APOSTROPHE=13,
	MMSKEY_PARENTHESIS_LEFT=14,
	MMSKEY_PARENTHESIS_RIGHT=15,
	MMSKEY_ASTERISK=16,
	MMSKEY_PLUS_SIGN=17,
	MMSKEY_COMMA=18,
	MMSKEY_MINUS_SIGN=19,
	MMSKEY_PERIOD=20,
	MMSKEY_SLASH=21,
	MMSKEY_0=22,
	MMSKEY_1=23,
	MMSKEY_2=24,
	MMSKEY_3=25,
	MMSKEY_4=26,
	MMSKEY_5=27,
	MMSKEY_6=28,
	MMSKEY_7=29,
	MMSKEY_8=30,
	MMSKEY_9=31,
	MMSKEY_COLON=32,
	MMSKEY_SEMICOLON=33,
	MMSKEY_LESS_THAN_SIGN=34,
	MMSKEY_EQUALS_SIGN=35,
	MMSKEY_GREATER_THAN_SIGN=36,
	MMSKEY_QUESTION_MARK=37,
	MMSKEY_AT=38,
	MMSKEY_CAPITAL_A=39,
	MMSKEY_CAPITAL_B=40,
	MMSKEY_CAPITAL_C=41,
	MMSKEY_CAPITAL_D=42,
	MMSKEY_CAPITAL_E=43,
	MMSKEY_CAPITAL_F=44,
	MMSKEY_CAPITAL_G=45,
	MMSKEY_CAPITAL_H=46,
	MMSKEY_CAPITAL_I=47,
	MMSKEY_CAPITAL_J=48,
	MMSKEY_CAPITAL_K=49,
	MMSKEY_CAPITAL_L=50,
	MMSKEY_CAPITAL_M=51,
	MMSKEY_CAPITAL_N=52,
	MMSKEY_CAPITAL_O=53,
	MMSKEY_CAPITAL_P=54,
	MMSKEY_CAPITAL_Q=55,
	MMSKEY_CAPITAL_R=56,
	MMSKEY_CAPITAL_S=57,
	MMSKEY_CAPITAL_T=58,
	MMSKEY_CAPITAL_U=59,
	MMSKEY_CAPITAL_V=60,
	MMSKEY_CAPITAL_W=61,
	MMSKEY_CAPITAL_X=62,
	MMSKEY_CAPITAL_Y=63,
	MMSKEY_CAPITAL_Z=64,
	MMSKEY_SQUARE_BRACKET_LEFT=65,
	MMSKEY_BACKSLASH=66,
	MMSKEY_SQUARE_BRACKET_RIGHT=67,
	MMSKEY_CIRCUMFLEX_ACCENT=68,
	MMSKEY_UNDERSCORE=69,
	MMSKEY_GRAVE_ACCENT=70,
	MMSKEY_SMALL_A=71,
	MMSKEY_SMALL_B=72,
	MMSKEY_SMALL_C=73,
	MMSKEY_SMALL_D=74,
	MMSKEY_SMALL_E=75,
	MMSKEY_SMALL_F=76,
	MMSKEY_SMALL_G=77,
	MMSKEY_SMALL_H=78,
	MMSKEY_SMALL_I=79,
	MMSKEY_SMALL_J=80,
	MMSKEY_SMALL_K=81,
	MMSKEY_SMALL_L=82,
	MMSKEY_SMALL_M=83,
	MMSKEY_SMALL_N=84,
	MMSKEY_SMALL_O=85,
	MMSKEY_SMALL_P=86,
	MMSKEY_SMALL_Q=87,
	MMSKEY_SMALL_R=88,
	MMSKEY_SMALL_S=89,
	MMSKEY_SMALL_T=90,
	MMSKEY_SMALL_U=91,
	MMSKEY_SMALL_V=92,
	MMSKEY_SMALL_W=93,
	MMSKEY_SMALL_X=94,
	MMSKEY_SMALL_Y=95,
	MMSKEY_SMALL_Z=96,
	MMSKEY_CURLY_BRACKET_LEFT=97,
	MMSKEY_VERTICAL_BAR=98,
	MMSKEY_CURLY_BRACKET_RIGHT=99,
	MMSKEY_TILDE=100,
	MMSKEY_DELETE=101,
	MMSKEY_CURSOR_LEFT=102,
	MMSKEY_CURSOR_RIGHT=103,
	MMSKEY_CURSOR_UP=104,
	MMSKEY_CURSOR_DOWN=105,
	MMSKEY_INSERT=106,
	MMSKEY_HOME=107,
	MMSKEY_END=108,
	MMSKEY_PAGE_UP=109,
	MMSKEY_PAGE_DOWN=110,
	MMSKEY_PRINT=111,
	MMSKEY_PAUSE=112,
	MMSKEY_OK=113,
	MMSKEY_SELECT=114,
	MMSKEY_GOTO=115,
	MMSKEY_CLEAR=116,
	MMSKEY_POWER=117,
	MMSKEY_POWER2=118,
	MMSKEY_OPTION=119,
	MMSKEY_MENU=120,
	MMSKEY_HELP=121,
	MMSKEY_INFO=122,
	MMSKEY_TIME=123,
	MMSKEY_VENDOR=124,
	MMSKEY_ARCHIVE=125,
	MMSKEY_PROGRAM=126,
	MMSKEY_CHANNEL=127,
	MMSKEY_FAVORITES=128,
	MMSKEY_EPG=129,
	MMSKEY_PVR=130,
	MMSKEY_MHP=131,
	MMSKEY_LANGUAGE=132,
	MMSKEY_TITLE=133,
	MMSKEY_SUBTITLE=134,
	MMSKEY_ANGLE=135,
	MMSKEY_ZOOM=136,
	MMSKEY_MODE=137,
	MMSKEY_KEYBOARD=138,
	MMSKEY_PC=139,
	MMSKEY_SCREEN=140,
	MMSKEY_TV=141,
	MMSKEY_TV2=142,
	MMSKEY_VCR=143,
	MMSKEY_VCR2=144,
	MMSKEY_SAT=145,
	MMSKEY_SAT2=146,
	MMSKEY_CD=147,
	MMSKEY_TAPE=148,
	MMSKEY_RADIO=149,
	MMSKEY_TUNER=150,
	MMSKEY_PLAYER=151,
	MMSKEY_TEXT=152,
	MMSKEY_DVD=153,
	MMSKEY_AUX=154,
	MMSKEY_MP3=155,
	MMSKEY_PHONE=156,
	MMSKEY_AUDIO=157,
	MMSKEY_VIDEO=158,
	MMSKEY_INTERNET=159,
	MMSKEY_MAIL=160,
	MMSKEY_NEWS=161,
	MMSKEY_DIRECTORY=162,
	MMSKEY_LIST=163,
	MMSKEY_CALCULATOR=164,
	MMSKEY_MEMO=165,
	MMSKEY_CALENDAR=166,
	MMSKEY_EDITOR=167,
	MMSKEY_RED=168,
	MMSKEY_GREEN=169,
	MMSKEY_YELLOW=170,
	MMSKEY_BLUE=171,
	MMSKEY_CHANNEL_UP=172,
	MMSKEY_CHANNEL_DOWN=173,
	MMSKEY_BACK=174,
	MMSKEY_FORWARD=175,
	MMSKEY_FIRST=176,
	MMSKEY_LAST=177,
	MMSKEY_VOLUME_UP=178,
	MMSKEY_VOLUME_DOWN=179,
	MMSKEY_MUTE=180,
	MMSKEY_AB=181,
	MMSKEY_PLAYPAUSE=182,
	MMSKEY_PLAY=183,
	MMSKEY_STOP=184,
	MMSKEY_RESTART=185,
	MMSKEY_SLOW=186,
	MMSKEY_FAST=187,
	MMSKEY_RECORD=188,
	MMSKEY_EJECT=189,
	MMSKEY_SHUFFLE=190,
	MMSKEY_REWIND=191,
	MMSKEY_FASTFORWARD=192,
	MMSKEY_PREVIOUS=193,
	MMSKEY_NEXT=194,
	MMSKEY_BEGIN=195,
	MMSKEY_DIGITS=196,
	MMSKEY_TEEN=197,
	MMSKEY_TWEN=198,
	MMSKEY_BREAK=199,
	MMSKEY_EXIT=200,
	MMSKEY_SETUP=201,
	MMSKEY_CURSOR_LEFT_UP=202,
	MMSKEY_CURSOR_LEFT_DOWN=203,
	MMSKEY_CURSOR_UP_RIGHT=204,
	MMSKEY_CURSOR_DOWN_RIGHT=205,
	MMSKEY_F1=206,
	MMSKEY_F2=207,
	MMSKEY_F3=208,
	MMSKEY_F4=209,
	MMSKEY_F5=210,
	MMSKEY_F6=211,
	MMSKEY_F7=212,
	MMSKEY_F8=213,
	MMSKEY_F9=214,
	MMSKEY_F10=215,
	MMSKEY_F11=216,
	MMSKEY_F12=217,
	MMSKEY_SHIFT=218,
	MMSKEY_CONTROL=219,
	MMSKEY_ALT=220,
	MMSKEY_ALTGR=221,
	MMSKEY_META=222,
	MMSKEY_SUPER=223,
	MMSKEY_HYPER=224,
	MMSKEY_CAPS_LOCK=225,
	MMSKEY_NUM_LOCK=226,
	MMSKEY_SCROLL_LOCK=227,
	MMSKEY_DEAD_ABOVEDOT=228,
	MMSKEY_DEAD_ABOVERING=229,
	MMSKEY_DEAD_ACUTE=230,
	MMSKEY_DEAD_BREVE=231,
	MMSKEY_DEAD_CARON=232,
	MMSKEY_DEAD_CEDILLA=233,
	MMSKEY_DEAD_CIRCUMFLEX=234,
	MMSKEY_DEAD_DIAERESIS=235,
	MMSKEY_DEAD_DOUBLEACUTE=236,
	MMSKEY_DEAD_GRAVE=237,
	MMSKEY_DEAD_IOTA=238,
	MMSKEY_DEAD_MACRON=239,
	MMSKEY_DEAD_OGONEK=240,
	MMSKEY_DEAD_SEMIVOICED_SOUND=241,
	MMSKEY_DEAD_TILDE=242,
	MMSKEY_DEAD_VOICED_SOUND=243,
	MMSKEY_CUSTOM0=244,
	MMSKEY_CUSTOM1=245,
	MMSKEY_CUSTOM2=246,
	MMSKEY_CUSTOM3=247,
	MMSKEY_CUSTOM4=248,
	MMSKEY_CUSTOM5=249,
	MMSKEY_CUSTOM6=250,
	MMSKEY_CUSTOM7=251,
	MMSKEY_CUSTOM8=252,
	MMSKEY_CUSTOM9=253,
	MMSKEY_CUSTOM10=254,
	MMSKEY_CUSTOM11=255,
	MMSKEY_CUSTOM12=256,
	MMSKEY_CUSTOM13=257,
	MMSKEY_CUSTOM14=258,
	MMSKEY_CUSTOM15=259,
	MMSKEY_CUSTOM16=260,
	MMSKEY_CUSTOM17=261,
	MMSKEY_CUSTOM18=262,
	MMSKEY_CUSTOM19=263,
	MMSKEY_CUSTOM20=264,
	MMSKEY_CUSTOM21=265,
	MMSKEY_CUSTOM22=266,
	MMSKEY_CUSTOM23=267,
	MMSKEY_CUSTOM24=268,
	MMSKEY_CUSTOM25=269,
	MMSKEY_CUSTOM26=270,
	MMSKEY_CUSTOM27=271,
	MMSKEY_CUSTOM28=272,
	MMSKEY_CUSTOM29=273,
	MMSKEY_CUSTOM30=274,
	MMSKEY_CUSTOM31=275,
	MMSKEY_CUSTOM32=276,
	MMSKEY_CUSTOM33=277,
	MMSKEY_CUSTOM34=278,
	MMSKEY_CUSTOM35=279,
	MMSKEY_CUSTOM36=280,
	MMSKEY_CUSTOM37=281,
	MMSKEY_CUSTOM38=282,
	MMSKEY_CUSTOM39=283,
	MMSKEY_CUSTOM40=284,
	MMSKEY_CUSTOM41=285,
	MMSKEY_CUSTOM42=286,
	MMSKEY_CUSTOM43=287,
	MMSKEY_CUSTOM44=288,
	MMSKEY_CUSTOM45=289,
	MMSKEY_CUSTOM46=290,
	MMSKEY_CUSTOM47=291,
	MMSKEY_CUSTOM48=292,
	MMSKEY_CUSTOM49=293,
	MMSKEY_CUSTOM50=294,
	MMSKEY_CUSTOM51=295,
	MMSKEY_CUSTOM52=296,
	MMSKEY_CUSTOM53=297,
	MMSKEY_CUSTOM54=298,
	MMSKEY_CUSTOM55=299,
	MMSKEY_CUSTOM56=300,
	MMSKEY_CUSTOM57=301,
	MMSKEY_CUSTOM58=302,
	MMSKEY_CUSTOM59=303,
	MMSKEY_CUSTOM60=304,
	MMSKEY_CUSTOM61=305,
	MMSKEY_CUSTOM62=306,
	MMSKEY_CUSTOM63=307,
	MMSKEY_CUSTOM64=308,
	MMSKEY_CUSTOM65=309,
	MMSKEY_CUSTOM66=310,
	MMSKEY_CUSTOM67=311,
	MMSKEY_CUSTOM68=312,
	MMSKEY_CUSTOM69=313,
	MMSKEY_CUSTOM70=314,
	MMSKEY_CUSTOM71=315,
	MMSKEY_CUSTOM72=316,
	MMSKEY_CUSTOM73=317,
	MMSKEY_CUSTOM74=318,
	MMSKEY_CUSTOM75=319,
	MMSKEY_CUSTOM76=320,
	MMSKEY_CUSTOM77=321,
	MMSKEY_CUSTOM78=322,
	MMSKEY_CUSTOM79=323,
	MMSKEY_CUSTOM80=324,
	MMSKEY_CUSTOM81=325,
	MMSKEY_CUSTOM82=326,
	MMSKEY_CUSTOM83=327,
	MMSKEY_CUSTOM84=328,
	MMSKEY_CUSTOM85=329,
	MMSKEY_CUSTOM86=330,
	MMSKEY_CUSTOM87=331,
	MMSKEY_CUSTOM88=332,
	MMSKEY_CUSTOM89=333,
	MMSKEY_CUSTOM90=334,
	MMSKEY_CUSTOM91=335,
	MMSKEY_CUSTOM92=336,
	MMSKEY_CUSTOM93=337,
	MMSKEY_CUSTOM94=338,
	MMSKEY_CUSTOM95=339,
	MMSKEY_CUSTOM96=340,
	MMSKEY_CUSTOM97=341,
	MMSKEY_CUSTOM98=342,
	MMSKEY_CUSTOM99=343,
	MMSKEY_NULL=344
} MMSKeySymbol;

// returned strings must be equal to XKeysymToString()
// we use GStreamer function gst_element_send_event() which uses "application/x-gst-navigation"
const char *convertMMSKeySymbolToXKeysymString(MMSKeySymbol key);


// Special State Type .......................................................

//! special state type
typedef enum {
	//! false
	MMSSTATE_FALSE = 0,
	//! true
	MMSSTATE_TRUE,
	//! auto
	MMSSTATE_AUTO
} MMSSTATE;



// Sequence mode ............................................................

//! sequence mode
typedef enum {
	//! no sequence
	MMSSEQUENCEMODE_NONE = 0,
	//! linear sequence
	MMSSEQUENCEMODE_LINEAR,
	//! logarithmical sequence, soft start and stop of the sequence
	MMSSEQUENCEMODE_LOG,
	//! logarithmical sequence, soft start
	MMSSEQUENCEMODE_LOG_SOFT_START,
	//! logarithmical sequence, soft end
	MMSSEQUENCEMODE_LOG_SOFT_END
} MMSSEQUENCEMODE;



// known languages...........................................................

//! known languages
typedef enum {
	//! none
	MMSLANG_NONE = 0,
	//! german
	MMSLANG_DE,
	//! english
	MMSLANG_EN,
	//! denmark
	MMSLANG_DK,
	//! spanish
	MMSLANG_ES,
	//! finnish
	MMSLANG_FI,
	//! french
	MMSLANG_FR,
	//! italian
	MMSLANG_IT,
	//! dutch
	MMSLANG_NL,
	//! norwegian
	MMSLANG_NO,
	//! swedish
	MMSLANG_SE,
	//! turkish
	MMSLANG_TR,
	//! chinese
	MMSLANG_CN,
	//! israeli
	MMSLANG_IL,
	//! number of languages
	MMSLANG_SIZE
} MMSLanguage;

//! language: none
#define MMSLANG_NONE_STR		""
//! language: german
#define MMSLANG_DE_STR			"DE"
//! language: english
#define MMSLANG_EN_STR			"EN"
//! language: denmark
#define MMSLANG_DK_STR			"DK"
//! language: spanish
#define MMSLANG_ES_STR			"ES"
//! language: finnish
#define MMSLANG_FI_STR			"FI"
//! language: french
#define MMSLANG_FR_STR			"FR"
//! language: italian
#define MMSLANG_IT_STR			"IT"
//! language: dutch
#define MMSLANG_NL_STR			"NL"
//! language: norwegian
#define MMSLANG_NO_STR			"NO"
//! language: swedish
#define MMSLANG_SE_STR			"SE"
//! language: turkish
#define MMSLANG_TR_STR			"TR"
//! language: chinese
#define MMSLANG_CN_STR			"CN"
//! language: israeli
#define MMSLANG_IL_STR			"IL"

// conversion routines for languages
string getMMSLanguageString(MMSLanguage lang);
MMSLanguage getMMSLanguageFromString(string lang);










typedef struct {
	float	*buf;
	int		eSize;
	int		eNum;
} MMS3D_VERTEX_ARRAY;

typedef enum {
	MMS3D_INDEX_ARRAY_TYPE_TRIANGLES = 0,
	MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP,
	MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_FAN
} MMS3D_INDEX_ARRAY_TYPE;

typedef struct {
	MMS3D_INDEX_ARRAY_TYPE	type;
	unsigned int			*buf;
	int						eNum;
} MMS3D_INDEX_ARRAY;


typedef struct {
	float r;
	float g;
	float b;
	float a;
} MMS3D_RGBA;

typedef struct {
	MMS3D_RGBA	emission;
	MMS3D_RGBA	ambient;
	MMS3D_RGBA	diffuse;
	MMS3D_RGBA	specular;
	float		shininess;
} MMS3D_MATERIAL_S;

typedef float MMS3D_MATERIAL_A[17];

typedef union {
	//! structure access
	MMS3D_MATERIAL_S	s;
	//! array access
	MMS3D_MATERIAL_A	a;
} MMS3D_MATERIAL;






#define MMS3D_PI 3.1415926535897932384626433832795f

typedef float MMS3DMatrix[4][4];

void multiplyMatrix(MMS3DMatrix result, MMS3DMatrix srcA, MMS3DMatrix srcB);
void copyMatrix(MMS3DMatrix result, MMS3DMatrix src);
bool equalMatrix(MMS3DMatrix result, MMS3DMatrix src);
void loadIdentityMatrix(MMS3DMatrix result);
void scaleMatrix(MMS3DMatrix result, float sx, float sy, float sz);
void translateMatrix(MMS3DMatrix result, float tx, float ty, float tz);
void rotateMatrix(MMS3DMatrix result, float angle, float x, float y, float z);
void frustumMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ);
void perspectiveMatrix(MMS3DMatrix result, float fovy, float aspect, float nearZ, float farZ);
void orthoMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ);


//! decribes a 3D object which can be rendered
typedef struct _bei_object {
	//! parent of object or NULL
	_bei_object *parent;

	//! index to available vertices, else negative
	int		vertices;

	//! index to available normals, else negative
	int		normals;

	//! index to available texture coordinates, else negative
	int		texcoords;

	//! index to available indices, else negative
	int		indices;

	//! index to available material, else negative
	int		material;

	//! index to available texture, else negative
	int		texture;

	//! object is shown?
	bool	shown;

	//! cull face?
	bool	cullface;

	//! matrix of the object
	MMS3DMatrix 	matrix;
} MMS3D_OBJECT;


bool isMMS3DObjectShown(MMS3D_OBJECT *object);



struct TsCalibration {
    TsCalibration(float ax=0, float bx=0, float dx=0, float ay=0, float by=0, float dy=0):
        aX(ax), bX(bx), dX(dx), aY(ay), bY(by), dY(dy) {}
    float aX, bX, dX;
    float aY, bY, dY;
};


#endif /* MMSTYPES_H_ */
