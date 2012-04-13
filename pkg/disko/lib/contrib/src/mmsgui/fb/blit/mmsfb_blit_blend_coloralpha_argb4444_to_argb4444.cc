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

#include "mmstools/mmstools.h"

void mmsfb_blit_blend_coloralpha_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
													  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
													  unsigned char alpha) {
	// check for full alpha value
	if (alpha == 0xff) {
		// max alpha is specified, so i can ignore it and use faster routine
		mmsfb_blit_blend_argb4444_to_argb4444(src_planes, src_height, sx, sy, sw, sh,
											  dst_planes, dst_height, dx, dy);
		return;
	}

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend coloralpha ARGB4444 to ARGB4444.\n");
		firsttime = false;
	}

	// something to do?
	if (!alpha)
		// source should blitted full transparent, so leave destination as is
		return;

	// get the first source ptr/pitch
	unsigned short int *src = (unsigned short int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned short int *dst = (unsigned short int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int src_pitch_pix = src_pitch >> 1;
	int dst_pitch_pix = dst_pitch >> 1;
	src+= sx + sy * src_pitch_pix;
	dst+= dx + dy * dst_pitch_pix;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned short int OLDDST = (*dst) + 1;
	unsigned short int OLDSRC  = (*src) + 1;
	unsigned short int *src_end = src + src_pitch_pix * sh;
	int src_pitch_diff = src_pitch_pix - sw;
	int dst_pitch_diff = dst_pitch_pix - sw;
	register unsigned short int d;

	register unsigned int ALPHA = alpha;
	ALPHA++;

	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned short int *line_end = src + sw;
		while (src < line_end) {
			// load pixel from memory and check if the previous pixel is the same
			register unsigned short int SRC = *src;

			// is the source alpha channel 0x00 or 0x0f?
			register unsigned int A = SRC >> 12;
			if (A) {
				// source alpha is > 0x00 and <= 0x0f
				register unsigned short int DST = *dst;

				if ((DST==OLDDST)&&(SRC==OLDSRC)) {
					// same pixel, use the previous value
					*dst = d;
				    dst++;
				    src++;
					continue;
				}
				OLDDST = DST;
				OLDSRC = SRC;

				// load source pixel and multiply it with given ALPHA
			    A = (ALPHA * A) >> 4;
				unsigned int sr = (ALPHA * (SRC & 0x0f00)) >> 12;
				unsigned int sg = (ALPHA * (SRC & 0x00f0)) >> 8;
				unsigned int sb = (ALPHA * (SRC & 0x000f)) >> 4;
				register unsigned int SA= 0x100 - A;

				unsigned int a = DST >> 12;
				unsigned int r = DST & 0x0f00;
				unsigned int g = DST & 0x00f0;
				unsigned int b = DST & 0x000f;

				// invert src alpha
			    a = (SA * a) >> 4;
			    r = (SA * r) >> 12;
			    g = (SA * g) >> 8;
			    b = (SA * b) >> 4;

			    // add src to dst
			    a += A;
			    r += sr;
			    g += sg;
			    b += sb;
			    d =   ((a >> 8) ? 0xf000 : ((a >> 4) << 12))
					| ((r >> 8) ? 0x0f00 : ((r >> 4) << 8))
					| ((g >> 8) ? 0xf0   : ((g >> 4) << 4))
			    	| ((b >> 8) ? 0x0f   :  (b >> 4));
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
