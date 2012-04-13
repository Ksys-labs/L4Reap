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

#include "mmsgui/fb/mmsfbbase.h"
#include "mmstools/tools.h"
#include <string.h>

// stores the last error text
string MMSFB_LastErrorString;

// screen should be rotated by 180°?
bool MMSFBBase_rotate180 = false;


string MMSFB_ErrorString(const int rc, const string msg) {
    if (rc)
    {
#ifdef  __HAVE_DIRECTFB__
        string s1 = msg;
        string s2 = DirectFBErrorString((DFBResult)rc);
        return s1 + " [" + s2 + "]";
#else
        return msg;
#endif
    }
    else
        return msg;
}

void MMSFB_SetError(const int rc, const string msg) {
    MMSFB_LastErrorString = MMSFB_ErrorString(rc, msg);
    DEBUGMSG("MMSGUI", MMSFB_LastErrorString);
}

bool isAlphaPixelFormat(MMSFBSurfacePixelFormat pf) {
    if   ((pf == MMSFB_PF_RGB16)
        ||(pf == MMSFB_PF_RGB24)
        ||(pf == MMSFB_PF_RGB32)
		||(pf == MMSFB_PF_YV12)
		||(pf == MMSFB_PF_YUY2)
		||(pf == MMSFB_PF_UYVY)
		||(pf == MMSFB_PF_LUT8)
		||(pf == MMSFB_PF_NV12)
		||(pf == MMSFB_PF_NV16)
		||(pf == MMSFB_PF_NV21)
		||(pf == MMSFB_PF_I420)
		||(pf == MMSFB_PF_RGB18)
		||(pf == MMSFB_PF_LUT2)
		||(pf == MMSFB_PF_RGB444)
        ||(pf == MMSFB_PF_RGB555)
        ||(pf == MMSFB_PF_BGR24)
        ||(pf == MMSFB_PF_BGR555))
        return false;
    return true;
}

bool isIndexedPixelFormat(MMSFBSurfacePixelFormat pf) {
    if   ((pf == MMSFB_PF_ALUT44)
    	||(pf == MMSFB_PF_LUT8))
        return true;
    return false;
}

bool isRGBPixelFormat(MMSFBSurfacePixelFormat pf) {
    if   ((pf == MMSFB_PF_YV12)
		||(pf == MMSFB_PF_AYUV)
		||(pf == MMSFB_PF_YUY2)
        ||(pf == MMSFB_PF_UYVY)
        ||(pf == MMSFB_PF_I420)
		||(pf == MMSFB_PF_NV12)
		||(pf == MMSFB_PF_NV16)
		||(pf == MMSFB_PF_NV21)
		||(pf == MMSFB_PF_LUT2))
        return false;
    return true;
}

void getBitsPerPixel(MMSFBSurfacePixelFormat pf, MMSFBPixelDef *pixeldef) {

	if (!pixeldef)
		return;

	pixeldef->bits = 0;
	pixeldef->red_length = 0;
	pixeldef->red_offset = 0;
	pixeldef->green_length = 0;
	pixeldef->green_offset = 0;
	pixeldef->blue_length = 0;
	pixeldef->blue_offset = 0;
	pixeldef->transp_length = 0;
	pixeldef->transp_offset = 0;

	if(pf == MMSFB_PF_RGB16) {
		pixeldef->bits			= 2*8;
		pixeldef->red_length	= 5;
		pixeldef->green_length	= 6;
		pixeldef->blue_length	= 5;
		pixeldef->red_offset	= 11;
		pixeldef->green_offset	= 5;
		pixeldef->blue_offset	= 0;
		return;
    }
	else
    if(pf == MMSFB_PF_RGB24) {
		pixeldef->bits			= 3*8;
		pixeldef->red_length	= 8;
		pixeldef->green_length	= 8;
		pixeldef->blue_length	= 8;
		pixeldef->red_offset	= 16;
		pixeldef->green_offset	= 8;
		pixeldef->blue_offset	= 0;
        return;
    }
	else
    if(pf == MMSFB_PF_RGB32) {
		pixeldef->bits			= 4*8;
		pixeldef->red_length	= 8;
		pixeldef->green_length	= 8;
		pixeldef->blue_length	= 8;
		pixeldef->red_offset	= 16;
		pixeldef->green_offset	= 8;
		pixeldef->blue_offset	= 0;
        return;
    }
	else
    if(pf == MMSFB_PF_ARGB) {
		pixeldef->bits			= 4*8;
		pixeldef->transp_length	= 8;
		pixeldef->red_length	= 8;
		pixeldef->green_length	= 8;
		pixeldef->blue_length	= 8;
		pixeldef->transp_offset	= 24;
		pixeldef->red_offset	= 16;
		pixeldef->green_offset	= 8;
		pixeldef->blue_offset	= 0;
        return;
    }
	else
    if(pf == MMSFB_PF_A8) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if(pf == MMSFB_PF_YUY2) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_UYVY) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_I420) {
		pixeldef->bits = 12;
        return;
    }
	else
    if(pf == MMSFB_PF_YV12) {
		pixeldef->bits = 12;
		return;
    }
	else
    if(pf == MMSFB_PF_AiRGB) {
		pixeldef->bits			= 4*8;
    	pixeldef->transp_length	= 8;
    	pixeldef->red_length	= 8;
    	pixeldef->green_length	= 8;
    	pixeldef->blue_length	= 8;
    	pixeldef->transp_offset	= 24;
    	pixeldef->red_offset	= 16;
    	pixeldef->green_offset	= 8;
    	pixeldef->blue_offset	= 0;
        return;
    }
	else
    if(pf == MMSFB_PF_A1) {
		pixeldef->bits = 1;
        return;
    }
	else
    if(pf == MMSFB_PF_NV12) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if(pf == MMSFB_PF_NV16) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if (pf == MMSFB_PF_NV21) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if (pf == MMSFB_PF_AYUV) {
		pixeldef->bits = 4*8;
        return;
    }
	else
    if (pf == MMSFB_PF_A4) {
		pixeldef->bits = 4;
        return;
    }
	else
    if (pf == MMSFB_PF_ARGB1666) {
		pixeldef->bits = 3*8;
        return;
    }
	else
    if (pf == MMSFB_PF_ARGB6666) {
		pixeldef->bits = 3*8;
        return;
    }
	else
    if (pf == MMSFB_PF_RGB18) {
		pixeldef->bits = 3*8;
        return;
    }
	else
    if (pf == MMSFB_PF_LUT2) {
		pixeldef->bits = 2;
        return;
    }
	else
    if (pf == MMSFB_PF_RGB444) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if (pf == MMSFB_PF_RGB555) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_ARGB1555) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_RGB332) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if(pf == MMSFB_PF_ALUT44) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if(pf == MMSFB_PF_LUT8) {
		pixeldef->bits = 1*8;
        return;
    }
	else
    if(pf == MMSFB_PF_ARGB2554) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_ARGB4444) {
		pixeldef->bits = 2*8;
        return;
    }
	else
    if(pf == MMSFB_PF_ARGB3565) {
		pixeldef->bits			= 20;
    	pixeldef->transp_length	= 3;
    	pixeldef->red_length	= 5;
    	pixeldef->green_length	= 6;
    	pixeldef->blue_length	= 5;
    	pixeldef->transp_offset	= 16;
    	pixeldef->red_offset	= 11;
    	pixeldef->green_offset	= 5;
    	pixeldef->blue_offset	= 0;
        return;
    }
	else
    if(pf == MMSFB_PF_BGR24) {
		pixeldef->bits			= 3*8;
    	pixeldef->red_length	= 8;
    	pixeldef->green_length	= 8;
    	pixeldef->blue_length	= 8;
    	pixeldef->red_offset	= 0;
    	pixeldef->green_offset	= 8;
    	pixeldef->blue_offset	= 16;
        return;
    }
	else
    if(pf == MMSFB_PF_BGR555) {
		pixeldef->bits			= 2*8;
    	pixeldef->red_length	= 5;
    	pixeldef->green_length	= 5;
    	pixeldef->blue_length	= 5;
    	pixeldef->red_offset	= 0;
    	pixeldef->green_offset	= 5;
    	pixeldef->blue_offset	= 10;
        return;
    }
	else
    if(pf == MMSFB_PF_ABGR) {
		pixeldef->bits			= 4*8;
    	pixeldef->transp_length	= 8;
    	pixeldef->red_length	= 8;
    	pixeldef->green_length	= 8;
    	pixeldef->blue_length	= 8;
    	pixeldef->transp_offset	= 24;
    	pixeldef->red_offset	= 0;
    	pixeldef->green_offset	= 8;
    	pixeldef->blue_offset	= 16;
        return;
    }
}

#ifdef  __HAVE_DIRECTFB__
MMSFBSurfacePixelFormat getMMSFBPixelFormatFromDFBPixelFormat(DFBSurfacePixelFormat pf) {
	switch (pf) {
    case DSPF_RGB16:    return MMSFB_PF_RGB16;
    case DSPF_RGB24:    return MMSFB_PF_RGB24;
    case DSPF_RGB32:    return MMSFB_PF_RGB32;
    case DSPF_ARGB:     return MMSFB_PF_ARGB;
    case DSPF_A8:       return MMSFB_PF_A8;
    case DSPF_YUY2:     return MMSFB_PF_YUY2;
    case DSPF_UYVY:     return MMSFB_PF_UYVY;
    case DSPF_I420:     return MMSFB_PF_I420;
    case DSPF_YV12:     return MMSFB_PF_YV12;
    case DSPF_AiRGB:    return MMSFB_PF_AiRGB;
    case DSPF_A1:       return MMSFB_PF_A1;
    case DSPF_NV12:     return MMSFB_PF_NV12;
    case DSPF_NV16:     return MMSFB_PF_NV16;
    case DSPF_NV21:     return MMSFB_PF_NV21;
    case DSPF_AYUV:     return MMSFB_PF_AYUV;
    case DSPF_A4:       return MMSFB_PF_A4;
    case DSPF_ARGB1666: return MMSFB_PF_ARGB1666;
    case DSPF_ARGB6666: return MMSFB_PF_ARGB6666;
    case DSPF_RGB18:    return MMSFB_PF_RGB18;
    case DSPF_LUT2:     return MMSFB_PF_LUT2;
    case DSPF_RGB444:   return MMSFB_PF_RGB444;
    case DSPF_RGB555:   return MMSFB_PF_RGB555;
    case DSPF_ARGB1555: return MMSFB_PF_ARGB1555;
    case DSPF_RGB332:   return MMSFB_PF_RGB332;
    case DSPF_ALUT44:   return MMSFB_PF_ALUT44;
    case DSPF_LUT8:     return MMSFB_PF_LUT8;
    case DSPF_ARGB2554: return MMSFB_PF_ARGB2554;
    case DSPF_ARGB4444: return MMSFB_PF_ARGB4444;
    default:            return MMSFB_PF_NONE;
	}
}

DFBSurfacePixelFormat getDFBPixelFormatFromMMSFBPixelFormat(MMSFBSurfacePixelFormat pf) {
	switch (pf) {
    case MMSFB_PF_RGB16:    return DSPF_RGB16;
    case MMSFB_PF_RGB24:    return DSPF_RGB24;
    case MMSFB_PF_RGB32:    return DSPF_RGB32;
    case MMSFB_PF_ARGB:     return DSPF_ARGB;
    case MMSFB_PF_A8:       return DSPF_A8;
    case MMSFB_PF_YUY2:     return DSPF_YUY2;
    case MMSFB_PF_UYVY:     return DSPF_UYVY;
    case MMSFB_PF_I420:     return DSPF_I420;
    case MMSFB_PF_YV12:     return DSPF_YV12;
    case MMSFB_PF_AiRGB:    return DSPF_AiRGB;
    case MMSFB_PF_A1:       return DSPF_A1;
    case MMSFB_PF_NV12:     return DSPF_NV12;
    case MMSFB_PF_NV16:     return DSPF_NV16;
    case MMSFB_PF_NV21:     return DSPF_NV21;
    case MMSFB_PF_AYUV:     return DSPF_AYUV;
    case MMSFB_PF_A4:       return DSPF_A4;
    case MMSFB_PF_ARGB1666: return DSPF_ARGB1666;
    case MMSFB_PF_ARGB6666: return DSPF_ARGB6666;
    case MMSFB_PF_RGB18:    return DSPF_RGB18;
    case MMSFB_PF_LUT2:     return DSPF_LUT2;
    case MMSFB_PF_RGB444:   return DSPF_RGB444;
    case MMSFB_PF_RGB555:   return DSPF_RGB555;
    case MMSFB_PF_ARGB1555: return DSPF_ARGB1555;
    case MMSFB_PF_RGB332:   return DSPF_RGB332;
    case MMSFB_PF_ALUT44:   return DSPF_ALUT44;
    case MMSFB_PF_LUT8:     return DSPF_LUT8;
    case MMSFB_PF_ARGB2554: return DSPF_ARGB2554;
    case MMSFB_PF_ARGB4444: return DSPF_ARGB4444;
    default:                return DSPF_UNKNOWN;
	}
}



string getDFBLayerBufferModeString(DFBDisplayLayerBufferMode bm) {
    string ret = MMSFB_BM_NONE;

    if(bm & DLBM_FRONTONLY)
        ret = ret + "|" + MMSFB_BM_FRONTONLY;

    if(bm & DLBM_BACKVIDEO)
        ret = ret + "|" + MMSFB_BM_BACKVIDEO;
    if(bm & DLBM_BACKSYSTEM)
        ret = ret + "|" + MMSFB_BM_BACKSYSTEM;
    if(bm & DLBM_TRIPLE)
        ret = ret + "|" + MMSFB_BM_TRIPLE;
    if(bm & DLBM_WINDOWS)
        ret = ret + "|" + MMSFB_BM_WINDOWS;

    if (ret!=MMSFB_BM_NONE)
        return ret.substr(1);
    else
        return MMSFB_BM_NONE;
}

DFBDisplayLayerBufferMode getDFBLayerBufferModeFromString(string bm) {
    DFBDisplayLayerBufferMode b = DLBM_UNKNOWN;

    if (bm == MMSFB_BM_NONE)
        return b;
    if(strstr(bm.c_str(),MMSFB_BM_FRONTONLY))
        b = (DFBDisplayLayerBufferMode)(b | DLBM_FRONTONLY);

    if(strstr(bm.c_str(),MMSFB_BM_BACKVIDEO))
        b = (DFBDisplayLayerBufferMode)(b | DLBM_BACKVIDEO);
    if(strstr(bm.c_str(),MMSFB_BM_BACKSYSTEM))
        b = (DFBDisplayLayerBufferMode)(b | DLBM_BACKSYSTEM);
    if(strstr(bm.c_str(),MMSFB_BM_TRIPLE))
        b = (DFBDisplayLayerBufferMode)(b | DLBM_TRIPLE);
    if(strstr(bm.c_str(),MMSFB_BM_WINDOWS))
        b = (DFBDisplayLayerBufferMode)(b | DLBM_WINDOWS);
    return b;
}

string getDFBLayerOptionsString(DFBDisplayLayerOptions opts) {
    string ret = MMSFB_LO_NONE;

    if(opts & DLOP_ALPHACHANNEL)
        ret = ret + "|" + MMSFB_LO_ALPHACHANNEL;
    if(opts & DLOP_FLICKER_FILTERING)
        ret = ret + "|" + MMSFB_LO_FLICKER_FILTERING;
    if(opts & DLOP_DEINTERLACING)
        ret = ret + "|" + MMSFB_LO_DEINTERLACING;
    if(opts & DLOP_SRC_COLORKEY)
        ret = ret + "|" + MMSFB_LO_SRC_COLORKEY;
    if(opts & DLOP_DST_COLORKEY)
        ret = ret + "|" + MMSFB_LO_DST_COLORKEY;
    if(opts & DLOP_OPACITY)
        ret = ret + "|" + MMSFB_LO_OPACITY;
    if(opts & DLOP_FIELD_PARITY)
        ret = ret + "|" + MMSFB_LO_FIELD_PARITY;

    if (ret!=MMSFB_LO_NONE)
        return ret.substr(1);
    else
        return MMSFB_LO_NONE;
}

DFBDisplayLayerOptions getDFBLayerOptionsFromString(string opts) {
    DFBDisplayLayerOptions  o = DLOP_NONE;

    if (opts == MMSFB_LO_NONE)
        return o;
    if(strstr(opts.c_str(),MMSFB_LO_ALPHACHANNEL))
        o = (DFBDisplayLayerOptions)(o | DLOP_ALPHACHANNEL);
    if(strstr(opts.c_str(),MMSFB_LO_FLICKER_FILTERING))
        o = (DFBDisplayLayerOptions)(o | DLOP_FLICKER_FILTERING);
    if(strstr(opts.c_str(),MMSFB_LO_DEINTERLACING))
        o = (DFBDisplayLayerOptions)(o | DLOP_DEINTERLACING);
    if(strstr(opts.c_str(),MMSFB_LO_SRC_COLORKEY))
        o = (DFBDisplayLayerOptions)(o | DLOP_SRC_COLORKEY);
    if(strstr(opts.c_str(),MMSFB_LO_DST_COLORKEY))
        o = (DFBDisplayLayerOptions)(o | DLOP_DST_COLORKEY);
    if(strstr(opts.c_str(),MMSFB_LO_OPACITY))
        o = (DFBDisplayLayerOptions)(o | DLOP_OPACITY);
    if(strstr(opts.c_str(),MMSFB_LO_FIELD_PARITY))
        o = (DFBDisplayLayerOptions)(o | DLOP_FIELD_PARITY);
    return o;
}


DFBSurfaceBlittingFlags getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(MMSFBBlittingFlags flags) {
	DFBSurfaceBlittingFlags retflags = DSBLIT_NOFX;
	if (flags & MMSFB_BLIT_BLEND_ALPHACHANNEL)
		retflags = (DFBSurfaceBlittingFlags)(retflags | DSBLIT_BLEND_ALPHACHANNEL);
	if (flags & MMSFB_BLIT_BLEND_COLORALPHA)
		retflags = (DFBSurfaceBlittingFlags)(retflags | DSBLIT_BLEND_COLORALPHA);
	if (flags & MMSFB_BLIT_COLORIZE)
		retflags = (DFBSurfaceBlittingFlags)(retflags | DSBLIT_COLORIZE);
	if (flags & MMSFB_BLIT_SRC_PREMULTIPLY)
		retflags = (DFBSurfaceBlittingFlags)(retflags | DSBLIT_SRC_PREMULTIPLY);
	if (flags & MMSFB_BLIT_SRC_PREMULTCOLOR)
		retflags = (DFBSurfaceBlittingFlags)(retflags | DSBLIT_SRC_PREMULTCOLOR);
	return retflags;
}

DFBSurfaceDrawingFlags getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(MMSFBDrawingFlags flags) {
	DFBSurfaceDrawingFlags retflags = DSDRAW_NOFX;
	if (flags & MMSFB_DRAW_BLEND)
		retflags = (DFBSurfaceDrawingFlags)(retflags | DSDRAW_BLEND);
	if (flags & MMSFB_DRAW_SRC_PREMULTIPLY)
		retflags = (DFBSurfaceDrawingFlags)(retflags | DSDRAW_SRC_PREMULTIPLY);
	return retflags;
}

DFBSurfaceFlipFlags getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(MMSFBFlipFlags flags) {
	DFBSurfaceFlipFlags retflags = DSFLIP_NONE;
	if (flags & MMSFB_FLIP_WAIT)
		retflags = (DFBSurfaceFlipFlags)(retflags | DSFLIP_WAIT);
	if (flags & MMSFB_FLIP_ONSYNC)
		retflags = (DFBSurfaceFlipFlags)(retflags | DSFLIP_ONSYNC);
	if (flags & MMSFB_FLIP_WAITFORSYNC)
		retflags = (DFBSurfaceFlipFlags)(retflags | DSFLIP_WAITFORSYNC);
	return retflags;
}

DFBSurfaceLockFlags getDFBSurfaceLockFlagsFromMMSFBLockFlags(MMSFBLockFlags flags) {
	DFBSurfaceLockFlags retflags = DSLF_READ;
	if (flags & MMSFB_LOCK_WRITE)
		retflags = (DFBSurfaceLockFlags)(retflags | DSLF_WRITE);
	return retflags;
}


#endif


void calcAspectRatio(int sw, int sh, int dw, int dh, MMSFBRectangle &dest, bool aspect_ratio, bool even_aligned) {
	if (aspect_ratio) {
		// calc aspect ratio
		dest.h = dw * sh / sw;
		if (dest.h <= dh) {
			dest.w = dw;
		}
		else {
			dest.w = dh * sw / sh;
			dest.h = dh;
		}
	}
	else {
		// using full dest dimension
		dest.w = dw;
		dest.h = dh;
	}

	// calc pos
	dest.x = (dw - dest.w) / 2;
	dest.y = (dh - dest.h) / 2;

	if (even_aligned) {
		// even pos and dimension
		dest.x &= ~0x01;
		dest.y &= ~0x01;
		dest.w &= ~0x01;
		dest.h &= ~0x01;
	}
}

