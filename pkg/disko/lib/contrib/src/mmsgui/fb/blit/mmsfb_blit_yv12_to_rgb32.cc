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

#include "mmsgui/fb/mmsfbconv.h"

#ifdef __HAVE_PF_YV12__
#ifdef __HAVE_PF_RGB32__

#include "mmstools/mmstools.h"

void mmsfb_blit_yv12_to_rgb32(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							  unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated conversion YV12 to RGB32.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned char *src = (unsigned char *)extbuf->ptr;
	int src_pitch = extbuf->pitch;

	// prepare...
	int dst_pitch_pix 		= dst_pitch >> 2;
	int src_pitch_pix 		= src_pitch;
	int src_pitch_pix_half	= src_pitch_pix >> 1;

	dst+= dx + dy * dst_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

/*	unsigned int OLDSRC  = (*src) + 1;
	unsigned int old_y;
	unsigned int old_u;
	unsigned int old_v;*/

	int  src_pixels = src_pitch_pix * sh;

	// check odd/even
	bool odd_left 	= (sx & 0x01);
	bool odd_top 	= (sy & 0x01);
	bool odd_right 	= ((sx + sw) & 0x01);
	bool odd_bottom = ((sy + sh) & 0x01);

	// pointer to the pixel components of the first pixel
	unsigned char *src_y = src + sx + sy * src_pitch_pix;
	unsigned char *src_u = src + src_pitch_pix * src_height + src_pitch_pix_half * (src_height >> 1) + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;
	unsigned char *src_v = src + src_pitch_pix * src_height                                          + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;

	// offsets to the other three pixels
	unsigned int src_y2_offs = 1;
	unsigned int src_y3_offs = src_pitch;
	unsigned int src_y4_offs = src_y3_offs + 1;
	unsigned int dst2_offs = 1;
	unsigned int dst3_offs = dst_pitch_pix;
	unsigned int dst4_offs = dst3_offs + 1;

	// arithmetic mean
/*	register unsigned int d_u;
	register unsigned int d_v;
*/
#ifdef sfsgsgsgsg
	// draw odd pixels around the even rectangle
	if (odd_top && odd_left) {
		// odd top-left pixel
		register unsigned int SRC;
		register unsigned int A;

		// for arithmetic mean we have to set U and V from pixels outside the current rectangle
		d_u = (*dst_u) * 3;
		d_v = (*dst_v) * 3;

	    // calculate my pixel...
		MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;
	}

	if (odd_top && odd_right) {
		// odd top-right pixel
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// go to the pixel in the current line
		src   += sw - 1;
		dst_y += sw - 1;
		if (odd_left) {
			dst_u += sw >> 1;
			dst_v += sw >> 1;
		}
		else {
			dst_u += (sw - 1) >> 1;
			dst_v += (sw - 1) >> 1;
		}

		register unsigned int SRC;
		register unsigned int A;

		// for arithmetic mean we have to set U and V from pixels outside the current rectangle
		d_u = (*dst_u) * 3;
		d_v = (*dst_v) * 3;

	    // calculate my pixel...
		MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom && odd_left) {
		// odd bottom-left pixel
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// go to the line
		src   += src_pitch_pix * (sh-1);
		dst_y += dst_pitch_pix * (sh-1);
		if (odd_top) {
			dst_u += dst_pitch_pix_half * (sh >> 1);
			dst_v += dst_pitch_pix_half * (sh >> 1);
		}
		else {
			dst_u += dst_pitch_pix_half * ((sh-1) >> 1);
			dst_v += dst_pitch_pix_half * ((sh-1) >> 1);
		}

		register unsigned int SRC;
		register unsigned int A;

		// for arithmetic mean we have to set U and V from pixels outside the current rectangle
		d_u = (*dst_u) * 3;
		d_v = (*dst_v) * 3;

	    // calculate my pixel...
		MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom && odd_right) {
		// odd bottom-right pixel
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// go to the line
		src   += src_pitch_pix * (sh-1);
		dst_y += dst_pitch_pix * (sh-1);
		if (odd_top) {
			dst_u += dst_pitch_pix_half * (sh >> 1);
			dst_v += dst_pitch_pix_half * (sh >> 1);
		}
		else {
			dst_u += dst_pitch_pix_half * ((sh-1) >> 1);
			dst_v += dst_pitch_pix_half * ((sh-1) >> 1);
		}

		// go to the pixel in the current line
		src   += sw - 1;
		dst_y += sw - 1;
		if (odd_left) {
			dst_u += sw >> 1;
			dst_v += sw >> 1;
		}
		else {
			dst_u += (sw - 1) >> 1;
			dst_v += (sw - 1) >> 1;
		}

		register unsigned int SRC;
		register unsigned int A;

		// for arithmetic mean we have to set U and V from pixels outside the current rectangle
		d_u = (*dst_u) * 3;
		d_v = (*dst_v) * 3;

	    // calculate my pixel...
		MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_top) {
		// odd top line
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// calculate start and end
		unsigned int *line_end = src + sw;
		if (odd_left) {
			src++;
			dst_y++;
			dst_u++;
			dst_v++;
			line_end--;
		}
		if (odd_right)
			line_end--;

		// through the line
		while (src < line_end) {
			register unsigned int SRC;
			register unsigned int A;

			// for arithmetic mean we have to set U and V from pixels outside the current rectangle
			d_u = (*dst_u) << 1;
			d_v = (*dst_v) << 1;

		    // calculate my two pixels...
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);

			// calulate the arithmetic mean
			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;

			// go to the next two pixels
		    src+=2;
		    dst_y+=2;
		    dst_u++;
		    dst_v++;
		}

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom) {
		// odd bottom line
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// calculate start and end
		src   += src_pitch_pix * (sh-1);
		dst_y += dst_pitch_pix * (sh-1);
		if (odd_top) {
			dst_u += dst_pitch_pix_half * (sh >> 1);
			dst_v += dst_pitch_pix_half * (sh >> 1);
		}
		else {
			dst_u += dst_pitch_pix_half * ((sh-1) >> 1);
			dst_v += dst_pitch_pix_half * ((sh-1) >> 1);
		}

		unsigned int *line_end = src + sw;
		if (odd_left) {
			src++;
			dst_y++;
			dst_u++;
			dst_v++;
			line_end--;
		}
		if (odd_right)
			line_end--;

		// through the line
		while (src < line_end) {
			register unsigned int SRC;
			register unsigned int A;

			// for arithmetic mean we have to set U and V from pixels outside the current rectangle
			d_u = (*dst_u) << 1;
			d_v = (*dst_v) << 1;

		    // calculate my two pixels...
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);

			// calulate the arithmetic mean
			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;

			// go to the next two pixels
		    src+=2;
		    dst_y+=2;
		    dst_u++;
		    dst_v++;
		}

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_left) {
		// odd left line
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// calculate start and end
		unsigned int *src_end = src + src_pixels;
		int src_pitch_diff    = src_pitch_pix << 1;
		int dst_pitch_diff    = dst_pitch_pix << 1;
		int dst_pitch_uvdiff  = dst_pitch_pix_half;
		if (odd_top) {
			src     += src_pitch_pix;
			src_end -= src_pitch_pix;
			dst_y   += dst_pitch_pix;
			dst_u   += dst_pitch_pix_half;
			dst_v   += dst_pitch_pix_half;
		}
		if (odd_bottom)
			src_end -= src_pitch_pix;

		// through all lines
		while (src < src_end) {
			// for the first pixel in the line
			register unsigned int SRC;
			register unsigned int A;

			// for arithmetic mean we have to set U and V from pixels outside the current rectangle
			d_u = (*dst_u) << 1;
			d_v = (*dst_v) << 1;

		    // calculate my two pixels...
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);

			// calulate the arithmetic mean
			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;

			// go to the next two lines
			src   += src_pitch_diff;
			dst_y += dst_pitch_diff;
			dst_u += dst_pitch_uvdiff;
			dst_v += dst_pitch_uvdiff;
		}

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}

	if (odd_right) {
		// odd right line
		MMSFB_CONV_ARGB_TO_YV12_PUSHPTR;

		// calculate start and end
		unsigned int *src_end = src + src_pixels;
		int src_pitch_diff    = src_pitch_pix << 1;
		int dst_pitch_diff    = dst_pitch_pix << 1;
		int dst_pitch_uvdiff  = dst_pitch_pix_half;
		src   += sw - 1;
		dst_y += sw - 1;
		if (odd_left) {
			dst_u += sw >> 1;
			dst_v += sw >> 1;
		}
		else {
			dst_u += (sw - 1) >> 1;
			dst_v += (sw - 1) >> 1;
		}
		if (odd_top) {
			src     += src_pitch_pix;
			src_end -= src_pitch_pix;
			dst_y   += dst_pitch_pix;
			dst_u   += dst_pitch_pix_half;
			dst_v   += dst_pitch_pix_half;
		}
		if (odd_bottom)
			src_end -= src_pitch_pix;

		// through all lines
		while (src < src_end) {
			// for the first pixel in the line
			register unsigned int SRC;
			register unsigned int A;

			// for arithmetic mean we have to set U and V from pixels outside the current rectangle
			d_u = (*dst_u) << 1;
			d_v = (*dst_v) << 1;

		    // calculate my two pixels...
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);

			// calulate the arithmetic mean
			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;

			// go to the next two lines
			src   += src_pitch_diff;
			dst_y += dst_pitch_diff;
			dst_u += dst_pitch_uvdiff;
			dst_v += dst_pitch_uvdiff;
		}

		// restore the pointers
		MMSFB_CONV_ARGB_TO_YV12_POPPTR;
	}
#endif


	// calc even positions...
	if (odd_top) {
		// odd top
		dy++;
		sh--;
/*		src+=src_pitch_pix;
		src_pixels-=src_pitch_pix;
		dst_y+=dst_pitch;
		dst_u+=dst_pitch >> 1;
		dst_v+=dst_pitch >> 1;
*/

		dst+=dst_pitch_pix;
		src_pixels-=src_pitch_pix;
		src_y+=src_pitch;
		src_u+=src_pitch >> 1;
		src_v+=src_pitch >> 1;

	}

	if (odd_bottom) {
		// odd bottom
		src_height--;
		src_pixels-=src_pitch_pix;
	}

	if (odd_left) {
		// odd left
		dx++;
		sw--;
		dst++;
		src_y++;
		src_u++;
		src_v++;
	}

	if (odd_right) {
		// odd right
		sw--;
	}

/*
	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////
	unsigned int *src_end = src + src_pixels;
	int src_pitch_diff = (src_pitch_pix << 1) - sw;
	int dst_pitch_diff = (dst_pitch_pix << 1) - sw;
	int dst_pitch_uvdiff = (dst_pitch_pix - sw) >> 1;

	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned int *line_end = src + sw;

		// go through two lines in parallel (square 2x2 pixel)
		while (src < line_end) {
			register unsigned int SRC;
			register unsigned int A;

		    // calculate the four pixels...
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u=, d_v=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_ARGB_TO_YV12_PIXEL(src[src4_offs], dst_y[dst_y4_offs], *dst_u, *dst_v, d_u+=, d_v+=);

			// calulate the arithmetic mean
			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;

			// go to the next two pixels
		    src  +=2;
		    dst_y+=2;
		    dst_u++;
		    dst_v++;
		}

		// go to the next two lines
		src   += src_pitch_diff;
		dst_y += dst_pitch_diff;
		dst_u += dst_pitch_uvdiff;
		dst_v += dst_pitch_uvdiff;
	}
*/

	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////
	unsigned char *src_end = src_y + src_pixels;
	int dst_pitch_diff = (dst_pitch_pix << 1) - sw;
	int src_pitch_diff = (src_pitch_pix << 1) - sw;
	int src_pitch_uvdiff = (src_pitch_pix - sw) >> 1;

	// for all lines
	while (src_y < src_end) {
		// for all pixels in the line
		unsigned char *line_end = src_y + sw;

		// go through two lines in parallel (square 2x2 pixel)
		while (src_y < line_end) {
/*			register unsigned int SRC;
			register unsigned int A;
*/
		    // calculate the four pixels...
			MMSFB_CONV_YV12_TO_RGB32_PIXEL(*src_y, *src_u, *src_v, *dst);
			MMSFB_CONV_YV12_TO_RGB32_PIXEL(src_y[src_y2_offs], *src_u, *src_v, dst[dst2_offs]);
			MMSFB_CONV_YV12_TO_RGB32_PIXEL(src_y[src_y3_offs], *src_u, *src_v, dst[dst3_offs]);
			MMSFB_CONV_YV12_TO_RGB32_PIXEL(src_y[src_y4_offs], *src_u, *src_v, dst[dst4_offs]);

			// calulate the arithmetic mean
/*			*dst_u = d_u >> 2;
			*dst_v = d_v >> 2;
*/
			// go to the next two pixels
		    dst  +=2;
		    src_y+=2;
		    src_u++;
		    src_v++;
		}

		// go to the next two lines
		dst   += dst_pitch_diff;
		src_y += src_pitch_diff;
		src_u += src_pitch_uvdiff;
		src_v += src_pitch_uvdiff;
	}
}

#endif
#endif
