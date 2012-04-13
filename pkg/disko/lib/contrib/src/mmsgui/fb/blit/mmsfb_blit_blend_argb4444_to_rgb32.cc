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

#ifdef __HAVE_PF_ARGB4444__
#ifdef __HAVE_PF_RGB32__

#include "mmstools/mmstools.h"

void mmsfb_blit_blend_argb4444_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend ARGB4444 to RGB32.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned short int *src = (unsigned short int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned int *dst = (unsigned int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int src_pitch_pix = src_pitch >> 1;
	int dst_pitch_pix = dst_pitch >> 2;
	src+= sx + sy * src_pitch_pix;
	dst+= dx + dy * dst_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned int OLDDST = (*dst) + 1;
	unsigned short int OLDSRC = (*src) + 1;
	unsigned short int *src_end = src + src_pitch_pix * sh;
	int src_pitch_diff = src_pitch_pix - sw;
	int dst_pitch_diff = dst_pitch_pix - sw;
	register unsigned int d;

	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned short int *line_end = src + sw;
		while (src < line_end) {
			// load pixel from memory and check if the previous pixel is the same
			register unsigned short int SRC  = *src;

			// is the source alpha channel 0x00 or 0x0f?
			register unsigned int A = SRC >> 12;
			if (A == 0x0f) {
				// source pixel is not transparent, copy it directly to the destination
				if (SRC==OLDSRC) {
					// same pixel, use the previous value
					*dst = d;
				    dst++;
				    src++;
					continue;
				}

				// calc pixel and store it to destination
				d =	  0xff000000
					| ((SRC & 0x0f00) << 12)
					| ((SRC & 0x00f0) << 8)
					| ((SRC & 0x000f) << 4);
				*dst = d;

				// next pixel
				dst++;
			    src++;
				OLDDST = (*dst) + 1;
				OLDSRC = SRC;
				continue;
			}
			else
			if (A) {
				// source alpha is > 0x00 and < 0x0f
				register unsigned int DST = *dst;

				if ((DST==OLDDST)&&(SRC==OLDSRC)) {
					// same pixel, use the previous value
					*dst = d;
				    dst++;
				    src++;
					continue;
				}
				OLDDST = DST;
				OLDSRC = SRC;

				register unsigned int SA= 0x10 - A;
				unsigned int r = (DST << 8) >> 24;
				unsigned int g = (DST << 16) >> 24;
				unsigned int b = DST & 0xff;

				// invert src alpha
			    r = (SA * r) >> 4;
			    g = (SA * g) >> 4;
			    b = (SA * b) >> 4;

			    // add src to dst
			    r += (A*(SRC & 0x0f00)) >> 8;
			    g += (A*(SRC & 0x00f0)) >> 4;
			    b +=  A*(SRC & 0x000f);
			    d =   0xff000000
					| ((r >> 8) ? 0xff0000   : (r << 16))
					| ((g >> 8) ? 0xff00     : (g << 8))
			    	| ((b >> 8) ? 0xff 		 :  b);
				*dst = d;
			}

		    dst++;
		    src++;
		}

		// go to the next line
		src+= src_pitch_diff;
		dst+= dst_pitch_diff;
	}
}

#endif
#endif
