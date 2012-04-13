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

#ifdef __HAVE_PF_RGB16__

#include "mmstools/mmstools.h"

void mmsfb_fillrectangle_rgb16(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       int dx, int dy, int dw, int dh, MMSFBColor color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated fill rectangle to RGB16.\n");
		firsttime = false;
	}

	// get the first destination ptr/pitch
	unsigned short int *dst = (unsigned short int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int dst_pitch_pix = dst_pitch >> 1;
	dst+= dx + dy * dst_pitch_pix;

	unsigned short int *dst_end = dst + dst_pitch_pix * dh;
#ifndef __HAVE_SSE__
	int dst_pitch_diff = dst_pitch_pix - dw;
#endif

	// prepare the color
	register unsigned short int SRC;
	unsigned int r = color.r >> 3;
	unsigned int g = color.g >> 2;
	unsigned int b = color.b >> 3;
	SRC =     (r << 11)
			| (g << 5)
			| b;

	if ((SRC >> 8) != (SRC & 0xff)) {
		// different values for hi and lo byte
		// copy pixel directly to the destination
		// for all lines
		while (dst < dst_end) {
			// for all pixels in the line
#ifdef __HAVE_SSE__
			// fill memory 2-byte-wise (much faster than loop see below)
//		__asm__ __volatile__ ( "\trep stosw\n" : : "D" (dst), "a" (SRC), "c" (dw));
			short d0, d1, d2;
			__asm__ __volatile__ ( "\tcld\n\trep stosw" \
					: "=&D" (d0), "=&a" (d1), "=&c" (d2) \
					: "0" (dst), "1" (SRC), "2" (dw) \
					: "memory", "cc");

			// go to the next line
			dst+= dst_pitch_pix;
#else
			unsigned short int *line_end = dst + dw;
			while (dst < line_end) {
				*dst = SRC;
				dst++;
			}

			// go to the next line
			dst+= dst_pitch_diff;
#endif
		}
	}
	else {
		// hi and lo byte are equal, so clear it with value from lo byte
		register int mw = dw << 1;
		register unsigned char lo = SRC & 0xff;
		if (mw != dst_pitch) {
			// for all lines
			while (dst < dst_end) {
				// reset all pixels in the line
				memset(dst, lo, mw);

				// go to the next line
				dst+= dst_pitch_pix;
			}
		}
		else {
			// clear one big block
			memset(dst, lo, (int)((char*)dst_end - (char*)dst));
		}
	}
}

#endif
