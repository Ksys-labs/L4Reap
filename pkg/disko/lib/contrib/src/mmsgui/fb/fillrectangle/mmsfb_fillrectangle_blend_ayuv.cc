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

#ifdef __HAVE_PF_AYUV__

#include "mmstools/mmstools.h"

void mmsfb_fillrectangle_blend_ayuv(MMSFBSurfacePlanes *dst_planes, int dst_height,
									int dx, int dy, int dw, int dh, MMSFBColor color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend rectangle to AYUV.\n");
		firsttime = false;
	}

	// return immediately if alpha channel of the color is 0x00
	if (!color.a)
		return;

	// get the first destination ptr/pitch
	unsigned int *dst = (unsigned int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int dst_pitch_pix = dst_pitch >> 2;
	dst+= dx + dy * dst_pitch_pix;

	unsigned int OLDDST = (*dst) + 1;
	unsigned int *dst_end = dst + dst_pitch_pix * dh;
	int dst_pitch_diff = dst_pitch_pix - dw;
	register unsigned int d;

	// prepare the color
	register unsigned int A = color.a;
	register unsigned int SRC;
	SRC =     (A << 24)
			| ((MMSFB_CONV_RGB2Y(color.r, color.g, color.b)) << 16)
			| ((MMSFB_CONV_RGB2U(color.r, color.g, color.b)) << 8)
			| MMSFB_CONV_RGB2V(color.r, color.g, color.b);

	if (color.a == 0xff) {
		// source pixel is not transparent, copy it directly to the destination
		// for all lines
		while (dst < dst_end) {
			// for all pixels in the line
#ifdef __HAVE_SSE__
			// fill memory 4-byte-wise (much faster than loop see below)
//			__asm__ __volatile__ ( "\trep stosl\n" : : "D" (dst), "a" (SRC), "c" (dw));
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
	else {
		// source alpha is > 0x00 and < 0xff
		// for all lines
		while (dst < dst_end) {
			// for all pixels in the line
			unsigned int *line_end = dst + dw;
			while (dst < line_end) {
				// read the destination
				register unsigned int DST = *dst;
				if (DST==OLDDST) {
					// same pixel, use the previous value
					*dst = d;
				    dst++;
					continue;
				}
				OLDDST = DST;

				register int SA= 0x100 - A;
				unsigned int a = DST >> 24;
				int y = (DST << 8) >> 24;
				int u = (DST << 16) >> 24;
				int v = DST & 0xff;

				// we have to move the 0 point of the coordinate system
				// this make it a little slower than ARGB to ARGB blending
				MMSFB_CONV_PREPARE_YUVBLEND(y,u,v);

				// invert src alpha
			    a = (SA * a) >> 8;
			    y = (SA * y) >> 8;
			    u = (SA * u) >> 8;
			    v = (SA * v) >> 8;

			    // add src to dst
			    a += A;
			    y += (SRC << 8) >> 24;
			    u += (SRC << 16) >> 24;
			    v += SRC & 0xff;

			    // build destination pixel, have to check for negative values
				// this make it a little slower than ARGB to ARGB blending
			    d = ((a >> 8) ? 0xff000000 : (a << 24));
			    if (y > 0)
			    	d |= ((y >> 8) ? 0xff0000 : (y << 16));
			    if (u > 0)
			    	d |= ((u >> 8) ? 0xff00 : (u << 8));
			    if (v > 0)
			    	d |= ((v >> 8) ? 0xff : v);

			    *dst = d;

			    dst++;
			}

			// go to the next line
			dst+= dst_pitch_diff;
		}
	}
}

#endif
