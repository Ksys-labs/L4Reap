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

#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_YV12__

#include "mmstools/mmstools.h"


#ifdef __HAVE_SSE__

	v4si X1 = { 0x00ff00ff, 0x00ff00ff };
	v4six Y_RBRB = { 25, 66, 25, 66 };
	v4six Y_GG   = { 129, 0, 129, 0 };
	v4six U_RBRB = { 112, -38, 112, -38 };
	v4six U_GG   = { -74, 0, -74, 0 };
	v4six V_RBRB = { -18, 112, -18, 112 };
	v4six V_GG   = { -94, 0, -94, 0 };

	v4six YY = { 16, 0, 16, 0 };
	v4six UV = { 128, 0, 128, 0 };



#define MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC_ALPHA					\
			__asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# load src: x1 -> mm0, x2 -> mm1, A -> mm2	\n\t"	\
					"movq		%[src],		%%mm0				\n\t"	\
					"movq		%%mm0,		%%mm1				\n\t"	\
					"pand		%[X1],		%%mm0				\n\t"	\
					"psrlw		$8,			%%mm1				\n\t"	\
					"movq		%%mm1,		%%mm2				\n\t"	\
					"psrld		$16,		%%mm2				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [src] "m" (*ssrc->i), [X1] "m" (*X1)				\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc Y in mm3							\n\t"	\
					"movq		%%mm0,		%%mm3				\n\t"	\
					"pmaddwd	%[Y_RBRB],	%%mm3				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[Y_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm3				\n\t"	\
					"psrld		$8,			%%mm3				\n\t"	\
					"paddd		%[YY],		%%mm3				\n\t"	\
					"pmullw		%%mm2,		%%mm3				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [Y_RBRB] "m" (*Y_RBRB), [Y_GG] "m" (*Y_GG), [YY] "m" (*YY)	\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc U in mm4							\n\t"	\
					"movq		%%mm0,		%%mm4				\n\t"	\
					"pmaddwd	%[U_RBRB],	%%mm4				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[U_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm4				\n\t"	\
					"psrld		$8,			%%mm4				\n\t"	\
					"paddd		%[UV],		%%mm4				\n\t"	\
					"pmullw		%%mm2,		%%mm4				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [U_RBRB] "m" (*U_RBRB), [U_GG] "m" (*U_GG), [UV] "m" (*UV)	\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc V in mm5							\n\t"	\
					"movq		%%mm0,		%%mm5				\n\t"	\
					"pmaddwd	%[V_RBRB],	%%mm5				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[V_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm5				\n\t"	\
					"psrld		$8,			%%mm5				\n\t"	\
					"paddd		%[UV],		%%mm5				\n\t"	\
					"pmullw		%%mm2,		%%mm5				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [V_RBRB] "m" (*V_RBRB), [V_GG] "m" (*V_GG), [UV] "m" (*UV)	\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc A in mm2							\n\t"	\
					"movq		%%mm2,		%%mm7				\n\t"	\
					"movq		%[TTTT],	%%mm2				\n\t"	\
					"psubd		%%mm7,		%%mm2				\n\t"	\
					"###########################################\n\t"	\
					"# important: clear mm7!!!					\n\t"	\
					"pxor		%%mm7,		%%mm7				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [TTTT] "m" (*TTTT)								\
					);




#define MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC							\
			__asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# load src: x1 -> mm0, x2 -> mm1			\n\t"	\
					"movq		%[src],		%%mm0				\n\t"	\
					"movq		%%mm0,		%%mm1				\n\t"	\
					"pand		%[X1],		%%mm0				\n\t"	\
					"psrlw		$8,			%%mm1				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [src] "m" (*ssrc->i), [X1] "m" (*X1)				\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc Y in mm3							\n\t"	\
					"movq		%%mm0,		%%mm3				\n\t"	\
					"pmaddwd	%[Y_RBRB],	%%mm3				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[Y_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm3				\n\t"	\
					"psrld		$8,			%%mm3				\n\t"	\
					"paddd		%[YY],		%%mm3				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [Y_RBRB] "m" (*Y_RBRB), [Y_GG] "m" (*Y_GG), [YY] "m" (*YY)	\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc U in mm4							\n\t"	\
					"movq		%%mm0,		%%mm4				\n\t"	\
					"pmaddwd	%[U_RBRB],	%%mm4				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[U_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm4				\n\t"	\
					"psrld		$8,			%%mm4				\n\t"	\
					"paddd		%[UV],		%%mm4				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [U_RBRB] "m" (*U_RBRB), [U_GG] "m" (*U_GG), [UV] "m" (*UV)	\
					);													\
		    __asm__ __volatile__ (										\
					"###########################################\n\t"	\
					"# calc V in mm5							\n\t"	\
					"movq		%%mm0,		%%mm5				\n\t"	\
					"pmaddwd	%[V_RBRB],	%%mm5				\n\t"	\
					"movq		%%mm1,		%%mm7				\n\t"	\
					"pmaddwd	%[V_GG],	%%mm7				\n\t"	\
					"paddd		%%mm7,		%%mm5				\n\t"	\
					"psrld		$8,			%%mm5				\n\t"	\
					"paddd		%[UV],		%%mm5				\n\t"	\
					"###########################################\n\t"	\
					"# important: clear mm7!!!					\n\t"	\
					"pxor		%%mm7,		%%mm7				\n\t"	\
					"###########################################\n\t"	\
					: /* no outputs */									\
					: [V_RBRB] "m" (*V_RBRB), [V_GG] "m" (*V_GG), [UV] "m" (*UV)	\
					);






#endif


void mmsfb_blit_blend_argb_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								   unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy) {

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend ARGB to YV12.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned int *src = (unsigned int *)extbuf->ptr;
	int src_pitch = extbuf->pitch;

	// prepare...
	int  src_pitch_pix 		= src_pitch >> 2;
	int dst_pitch_pix 		= dst_pitch;
	int dst_pitch_pix_half	= dst_pitch_pix >> 1;

	src+= sx + sy * src_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned int OLDSRC  = (*src) + 1;

	unsigned int old_y;
	unsigned int old_u;
	unsigned int old_v;

	int  src_pixels = src_pitch_pix * sh;

	// check odd/even
	bool odd_left 	= (dx & 0x01);
	bool odd_top 	= (dy & 0x01);
	bool odd_right 	= ((dx + sw) & 0x01);
	bool odd_bottom = ((dy + sh) & 0x01);

	// pointer to the pixel components of the first pixel
	unsigned char *dst_y = dst + dx + dy * dst_pitch_pix;
	unsigned char *dst_u = dst + dst_pitch_pix * dst_height + dst_pitch_pix_half * (dst_height >> 1) + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;
	unsigned char *dst_v = dst + dst_pitch_pix * dst_height                                          + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;

	// offsets to the other three pixels
	unsigned int dst_y2_offs = 1;
	unsigned int dst_y3_offs = dst_pitch;
	unsigned int src2_offs = 1;
	unsigned int src3_offs = src_pitch_pix;

	// arithmetic mean
	register unsigned int d_u;
	register unsigned int d_v;

	// draw odd pixels around the even rectangle
	if (odd_top && odd_left) {
		// odd top-left pixel
		register unsigned int SRC;
		register unsigned int A;

		// for arithmetic mean we have to set U and V from pixels outside the current rectangle
		d_u = (*dst_u) * 3;
		d_v = (*dst_v) * 3;

	    // calculate my pixel...
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;
	}

	if (odd_top && odd_right) {
		// odd top-right pixel
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom && odd_left) {
		// odd bottom-left pixel
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom && odd_right) {
		// odd bottom-right pixel
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);

		// calulate the arithmetic mean
		*dst_u = d_u >> 2;
		*dst_v = d_v >> 2;

		// restore the pointers
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_top) {
		// odd top line
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_bottom) {
		// odd bottom line
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_left) {
		// odd left line
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	if (odd_right) {
		// odd right line
		MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR;

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
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);

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
		MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR;
	}

	// calc even positions...
	if (odd_top) {
		// odd top
		dy++;
		sh--;
		src+=src_pitch_pix;
		src_pixels-=src_pitch_pix;
		dst_y+=dst_pitch;
		dst_u+=dst_pitch >> 1;
		dst_v+=dst_pitch >> 1;
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
		src++;
		dst_y++;
		dst_u++;
		dst_v++;
	}

	if (odd_right) {
		// odd right
		sw--;
	}

	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////

#ifndef __HAVE_SSE__
	unsigned int dst_y4_offs = dst_y3_offs + 1;
	unsigned int src4_offs = src3_offs + 1;

	// without mmx/sse
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
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(*src, *dst_y, *dst_u, *dst_v, d_u=, d_v=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src2_offs], dst_y[dst_y2_offs], *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src3_offs], dst_y[dst_y3_offs], *dst_u, *dst_v, d_u+=, d_v+=);
			MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src[src4_offs], dst_y[dst_y4_offs], *dst_u, *dst_v, d_u+=, d_v+=);

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

#else

	// with mmx/sse
//	static v4six TTT = { 0,0,0,0 };
	static v4six TTTT = { 0x100,0,0x100,0 };





	_v4si *src_end = (_v4si *)(src + src_pixels);
	_v4si *ssrc = (_v4si *)src;
	int src_pitch_diff = (src_pitch_pix << 1) - sw;
	int dst_pitch_diff = (dst_pitch_pix << 1) - sw;
	int dst_pitch_uvdiff = (dst_pitch_pix - sw) >> 1;


	src3_offs = src3_offs>>1;
	sw = sw >> 1;
	src_pitch_diff = src_pitch_diff >> 1;



	int src3_offsX = src3_offs-1;
	int dst_y3_offsX = dst_y3_offs-2;

	_v4si	OLDSRC_MMX;
	OLDSRC_MMX.i[0] = ssrc->i[0]+1;
	OLDSRC_MMX.i[1] = ssrc->i[1]+1;

	// for all lines
	while (ssrc < src_end) {
		// for all pixels in the line
		_v4si *line_end = ssrc + sw;

		// go through two lines in parallel (square 2x2 pixel)
		while (ssrc < line_end) {
			if ((ssrc->c[3]==0xff)&&(ssrc->c[7]==0xff)) {
				// alpha channel == 0xff for both pixels
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// convert argb source to yv12
					MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC;
					OLDSRC_MMX = *ssrc;
				}

				__asm__ __volatile__ (
						"###########################################\n\t"
						"# save the two Y values					\n\t"
						"pextrw		$0, 		%%mm3,		%%eax	\n\t"
						"pextrw		$2, 		%%mm3,		%%ecx	\n\t"
						"mov		%%cl, 		%%ah				\n\t"
						"mov		%%ax,		%[dst_y]			\n\t"
						"###########################################\n\t"
						"# load reg mm0 with the U value			\n\t"
						"movq		%%mm4,		%%mm0				\n\t"
						"psadbw		%%mm7,		%%mm0				\n\t"
						"# save the U result in mm6					\n\t"
						"movq		%%mm0,		%%mm6				\n\t"
						"###########################################\n\t"
						"# load reg mm0 with the V value			\n\t"
						"movq		%%mm5,		%%mm0				\n\t"
						"psadbw		%%mm7,		%%mm0				\n\t"
						"pextrw		$0, 		%%mm0,		%%eax	\n\t"
						"# save the V result in mm6					\n\t"
						"pinsrw		$2, 		%%eax,		%%mm6	\n\t"
						"###########################################\n\t"
						: [dst_y] "=m" (*dst_y)				// outputs
						: 									// inputs
						: "cc", "%eax", "%ecx"				// clobbers
						);

			}
			else
			if ((!ssrc->c[3])&&(!ssrc->c[7])) {
				// alpha channel == 0x00 for both pixels
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// pixel value has changed
					OLDSRC_MMX = *ssrc;
				}

				// calculate U/V values, do it because we don't know if the next too pixels have also alpha 0x00
				__asm__ __volatile__ (
						"###########################################\n\t"
						"# load reg eax with the U value			\n\t"
						"xor		%%eax,		%%eax				\n\t"
						"mov		%[dst_u], 	%%al				\n\t"
						"# calc U * 2								\n\t"
						"shl		$1,			%%ax				\n\t"
						"# save the U result in mm6					\n\t"
						"pinsrw		$0, 		%%eax,		%%mm6	\n\t"
						"###########################################\n\t"
						"# load reg eax with the V value			\n\t"
						"xor		%%eax,		%%eax				\n\t"
						"mov		%[dst_v], 	%%al				\n\t"
						"# calc V * 2								\n\t"
						"shl		$1,			%%ax				\n\t"
						"# save the V result in mm6					\n\t"
						"pinsrw		$2, 		%%eax,		%%mm6	\n\t"
						"###########################################\n\t"
						: 												// outputs
						: [dst_u] "m" (*dst_u), [dst_v] "m" (*dst_v)	// inputs
						: "cc", "%eax"									// clobbers
						);
			}
			else {
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// convert argb source to yv12
					MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC_ALPHA;
					OLDSRC_MMX = *ssrc;
				}
				__asm__ __volatile__ (
						"###########################################\n\t"
						"# load reg mm0 with the two Y values		\n\t"
						"pxor		%%mm0,		%%mm0				\n\t"
						"mov		%[dst_y], 	%%ax				\n\t"
						"mov		%%ax,		%%cx				\n\t"
						"xor		%%ah,		%%ah				\n\t"
						"shr		$8,			%%cx				\n\t"
						"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
						"pinsrw		$2, 		%%ecx,		%%mm0	\n\t"
						"# calc Y									\n\t"
						"pmullw		%%mm2,		%%mm0				\n\t"
						"paddw		%%mm3,		%%mm0				\n\t"
						"psrlw		$8,			%%mm0				\n\t"
						"# save the two Y results					\n\t"
						"pextrw		$0, 		%%mm0,		%%eax	\n\t"
						"pextrw		$2, 		%%mm0,		%%ecx	\n\t"
						"mov		%%cl, 		%%ah				\n\t"
						"mov		%%ax,		%[dst_y]			\n\t"
						"###########################################\n\t"
						: [dst_y] "+m" (*dst_y)				// outputs
						: 									// inputs
						: "cc", "%eax", "%ecx"				// clobbers
						);

				__asm__ __volatile__ (
						"###########################################\n\t"
						"# load reg mm0 with the U value			\n\t"
						"xor		%%eax,		%%eax				\n\t"
						"mov		%[dst_u], 	%%al				\n\t"
						"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
						"pinsrw		$2, 		%%eax,		%%mm0	\n\t"
						"# calc U									\n\t"
						"pmullw		%%mm2,		%%mm0				\n\t"
						"paddw		%%mm4,		%%mm0				\n\t"
						"psrlw		$8,			%%mm0				\n\t"
						"psadbw		%%mm7,		%%mm0				\n\t"
						"# save the U result in mm6					\n\t"
						"movq		%%mm0,		%%mm6				\n\t"
						"###########################################\n\t"
						"# load reg mm0 with the V value			\n\t"
						"xor		%%eax,		%%eax				\n\t"
						"mov		%[dst_v], 	%%al				\n\t"
						"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
						"pinsrw		$2, 		%%eax,		%%mm0	\n\t"
						"# calc V									\n\t"
						"pmullw		%%mm2,		%%mm0				\n\t"
						"paddw		%%mm5,		%%mm0				\n\t"
						"psrlw		$8,			%%mm0				\n\t"
						"psadbw		%%mm7,		%%mm0				\n\t"
						"# save the V result in mm6					\n\t"
						"pextrw		$0, 		%%mm0,		%%eax	\n\t"
						"pinsrw		$2, 		%%eax,		%%mm6	\n\t"
						"###########################################\n\t"
						: 												// outputs
						: [dst_u] "m" (*dst_u), [dst_v] "m" (*dst_v)	// inputs
						: "cc", "%eax"									// clobbers
						);
			}

			ssrc+=src3_offs;
			dst_y+=dst_y3_offs;

			if ((ssrc->c[3]==0xff)&&(ssrc->c[7]==0xff)) {
				// alpha channel == 0xff for both pixels
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// pixel value has changed, convert argb source to yv12
					MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC;
					OLDSRC_MMX = *ssrc;

					// calculate the U/V values
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# load reg mm0 with the U value			\n\t"
							"movq		%%mm4,		%%mm0				\n\t"
							"psadbw		%%mm7,		%%mm0				\n\t"
							"# save the U result to memory				\n\t"
							"paddw		%%mm6, 		%%mm0				\n\t"
							"pextrw		$0, 		%%mm0,		%%eax	\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_u]			\n\t"
							"###########################################\n\t"
							"# load reg mm0 with the V value			\n\t"
							"movq		%%mm5,		%%mm0				\n\t"
							"psadbw		%%mm7,		%%mm0				\n\t"
							"# save the V result to memory				\n\t"
							"pextrw		$0, 		%%mm0,		%%eax	\n\t"
							"pextrw		$2, 		%%mm6,		%%ecx	\n\t"
							"add		%%ecx, 		%%eax				\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_v]			\n\t"
							"###########################################\n\t"
							: [dst_u] "=m" (*dst_u), [dst_v] "=m" (*dst_v)	// outputs
							: 												// inputs
							: "cc", "%eax", "%ecx"							// clobbers
							);
				}
				else {
					// pixel value has NOT changed, so we can use a optimized calculation of U and V

					// calculate the U/V values
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# save the U result to memory				\n\t"
							"pextrw		$0, 		%%mm6,		%%eax	\n\t"
							"shr		$1, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_u]			\n\t"
							"###########################################\n\t"
							"# save the V result to memory				\n\t"
							"pextrw		$2, 		%%mm6,		%%eax	\n\t"
							"shr		$1, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_v]			\n\t"
							"###########################################\n\t"
							: [dst_u] "=m" (*dst_u), [dst_v] "=m" (*dst_v)	// outputs
							: 												// inputs
							: "cc", "%eax"									// clobbers
							);
				}

				// calculate the two Y values
				__asm__ __volatile__ (
						"###########################################\n\t"
						"# save the two Y values					\n\t"
						"pextrw		$0, 		%%mm3,		%%eax	\n\t"
						"pextrw		$2, 		%%mm3,		%%ecx	\n\t"
						"mov		%%cl, 		%%ah				\n\t"
						"mov		%%ax,		%[dst_y]			\n\t"
						"###########################################\n\t"
						: [dst_y] "=m" (*dst_y)				// outputs
						: 									// inputs
						: "cc", "%eax", "%ecx"				// clobbers
						);
			}
			else
			if ((!ssrc->c[3])&&(!ssrc->c[7])) {
				// alpha channel == 0x00 for both pixels
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// pixel value has changed, calculate U/V values
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# load reg eax with the U value			\n\t"
							"xor		%%eax,		%%eax				\n\t"
							"mov		%[dst_u], 	%%al				\n\t"
							"# calc U * 2								\n\t"
							"shl		$1,			%%ax				\n\t"
							"# save the U result to memory				\n\t"
							"pextrw		$0, 		%%mm6,		%%ecx	\n\t"
							"add		%%ecx, 		%%eax				\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_u]			\n\t"
							"###########################################\n\t"
							: [dst_u] "+m" (*dst_u)				// outputs
							: 									// inputs
							: "cc", "%eax", "%ecx"				// clobbers
							);

					__asm__ __volatile__ (
							"###########################################\n\t"
							"# load reg eax with the V value			\n\t"
							"xor		%%eax,		%%eax				\n\t"
							"mov		%[dst_v], 	%%al				\n\t"
							"# calc V * 2								\n\t"
							"shl		$1,			%%ax				\n\t"
							"# save the V result to memory				\n\t"
							"pextrw		$2, 		%%mm6,		%%ecx	\n\t"
							"add		%%ecx, 		%%eax				\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_v]			\n\t"
							"###########################################\n\t"
							: [dst_v] "+m" (*dst_v)				// outputs
							: 									// inputs
							: "cc", "%eax", "%ecx"				// clobbers
							);
				}
			}
			else {
				// alpha channel > 0x00 and < 0xff for both pixels
				if ((ssrc->i[0] != OLDSRC_MMX.i[0])||(ssrc->i[1] != OLDSRC_MMX.i[1])) {
					// pixel value has changed, convert argb source to yv12
					MMSFB_BLIT_BLEND_ARGB_TO_YV12_LOAD_SRC_ALPHA;
					OLDSRC_MMX = *ssrc;

					// calculate the U value
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# load reg mm0 with the U value			\n\t"
							"xor		%%eax,		%%eax				\n\t"
							"mov		%[dst_u], 	%%al				\n\t"
							"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
							"pinsrw		$2, 		%%eax,		%%mm0	\n\t"
							"# calc U									\n\t"
							"pmullw		%%mm2,		%%mm0				\n\t"
							"paddw		%%mm4,		%%mm0				\n\t"
							"psrlw		$8,			%%mm0				\n\t"
							"psadbw		%%mm7,		%%mm0				\n\t"
							"# save the U result to memory				\n\t"
							"paddw		%%mm6,		%%mm0				\n\t"
							"pextrw		$0, 		%%mm0,		%%eax	\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_u]			\n\t"
							"###########################################\n\t"
							: [dst_u] "+m" (*dst_u)				// outputs
							: 									// inputs
							: "cc", "%eax"						// clobbers
							);

					// calculate the V value
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# load reg mm0 with the V value			\n\t"
							"xor		%%eax,		%%eax				\n\t"
							"mov		%[dst_v], 	%%al				\n\t"
							"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
							"pinsrw		$2, 		%%eax,		%%mm0	\n\t"
							"# calc V									\n\t"
							"pmullw		%%mm2,		%%mm0				\n\t"
							"paddw		%%mm5,		%%mm0				\n\t"
							"psrlw		$8,			%%mm0				\n\t"
							"psadbw		%%mm7,		%%mm0				\n\t"
							"# save the V result to memory				\n\t"
							"pextrw		$0, 		%%mm0,		%%eax	\n\t"
							"pextrw		$2, 		%%mm6,		%%ecx	\n\t"
							"add		%%ecx, 		%%eax				\n\t"
							"shr		$2, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_v]			\n\t"
							"###########################################\n\t"
							: [dst_v] "+m" (*dst_v)				// outputs
							: 									// inputs
							: "cc", "%eax", "%ecx"				// clobbers
							);
				}
				else {
					// pixel value has NOT changed, so we can use a optimized calculation of U and V

					// calculate the U/V values, we do not need to load the destination because of same pixel value
					__asm__ __volatile__ (
							"###########################################\n\t"
							"# save the U result to memory				\n\t"
							"pextrw		$0, 		%%mm6,		%%eax	\n\t"
							"shr		$1, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_u]			\n\t"
							"###########################################\n\t"
							"# save the V result to memory				\n\t"
							"pextrw		$2, 		%%mm6,		%%eax	\n\t"
							"shr		$1, 		%%eax				\n\t"
							"mov		%%al, 		%[dst_v]			\n\t"
							"###########################################\n\t"
							: [dst_u] "=m" (*dst_u), [dst_v] "=m" (*dst_v)	// outputs
							: 												// inputs
							: "cc", "%eax"									// clobbers
							);
				}

				// calculate the two Y values
				__asm__ __volatile__ (
						"###########################################\n\t"
						"# load reg mm0 with the two Y values		\n\t"
						"pxor		%%mm0,		%%mm0				\n\t"
						"mov		%[dst_y], 	%%ax				\n\t"
						"mov		%%ax,		%%cx				\n\t"
						"xor		%%ah,		%%ah				\n\t"
						"shr		$8,			%%cx				\n\t"
						"pinsrw		$0, 		%%eax,		%%mm0	\n\t"
						"pinsrw		$2, 		%%ecx,		%%mm0	\n\t"
						"# calc Y									\n\t"
						"pmullw		%%mm2,		%%mm0				\n\t"
						"paddw		%%mm3,		%%mm0				\n\t"
						"psrlw		$8,			%%mm0				\n\t"
						"# save the two Y results					\n\t"
						"pextrw		$0, 		%%mm0,		%%eax	\n\t"
						"pextrw		$2, 		%%mm0,		%%ecx	\n\t"
						"mov		%%cl, 		%%ah				\n\t"
						"mov		%%ax,		%[dst_y]			\n\t"
						: [dst_y] "+m" (*dst_y)				// outputs
						: 									// inputs
						: "cc", "%eax", "%ecx"				// clobbers
						);

			}



			// go to the next two pixels
			ssrc-=src3_offsX;
			dst_y-=dst_y3_offsX;
		    dst_u++;
		    dst_v++;
		}

		// go to the next two lines
		ssrc  += src_pitch_diff;
		dst_y += dst_pitch_diff;
		dst_u += dst_pitch_uvdiff;
		dst_v += dst_pitch_uvdiff;
	}


    __asm__ __volatile__ (
			"###########################################\n\t"
			"# clear the MMX state						\n\t"
    		"emms										\n\t"
			"###########################################\n\t"
    		);
#endif

}

#endif
#endif
