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

#ifdef __HAVE_PF_YUY2__

#include "mmstools/mmstools.h"

void mmsfb_fillrectangle_yuy2(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated fill rectangle to YUY2.\n");
		firsttime = false;
	}

	// get the first destination ptr/pitch
	unsigned int *dst = (unsigned int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// dest x and w must be multiple of two
	// reason: YUY2 codes U/V values separated in two horizontal pixels
	if (dx & 0x01) {
		dx++;
		dw--;
	}
	dw/=2;

	// prepare...
	int dst_pitch_pix = dst_pitch >> 2;
	dst+= dx + dy * dst_pitch_pix;

	unsigned int *dst_end = dst + dst_pitch_pix * dh;
#ifndef __HAVE_SSE__
	int dst_pitch_diff = dst_pitch_pix - dw;
#endif

	// prepare the color
	unsigned int SRC_Y = MMSFB_CONV_RGB2Y(color.r, color.g, color.b);
	unsigned int SRC_U = MMSFB_CONV_RGB2U(color.r, color.g, color.b);
	unsigned int SRC_V = MMSFB_CONV_RGB2V(color.r, color.g, color.b);
	register unsigned int SRC;
#if __BYTE_ORDER == __BIG_ENDIAN
	// e.g. ARM
	SRC = 	(SRC_Y << 24)
		  | (SRC_V << 16)
		  | (SRC_Y << 8)
		  |  SRC_U;
#else
	// e.g. Intel
	SRC = 	(SRC_U << 24)
		  | (SRC_Y << 16)
		  | (SRC_V << 8)
		  |  SRC_Y;
#endif

	// copy pixel directly to the destination
	// for all lines
	while (dst < dst_end) {
		// for all pixels in the line
#ifdef __HAVE_SSE__
		// fill memory 4-byte-wise (much faster than loop see below)
//		__asm__ __volatile__ ( "\trep stosl\n" : : "D" (dst), "a" (SRC), "c" (dw));
		short d0, d1, d2;
		__asm__ __volatile__ ( "\tcld\n\trep stosl" \
				: "=&D" (d0), "=&a" (d1), "=&c" (d2) \
				: "0" (dst), "1" (SRC), "2" (dw) \
				: "memory", "cc");

		// go to the next line
		dst+= dst_pitch_pix;
#else
		unsigned int *line_end = dst + dw;
		while (dst < line_end) {
			*dst = SRC;
			dst++;
		}

		// go to the next line
		dst+= dst_pitch_diff;
#endif
	}
}

#endif
