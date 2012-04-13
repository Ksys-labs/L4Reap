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

#ifdef __HAVE_PF_BGR24__

#include "mmstools/mmstools.h"

void mmsfb_blit_coloralpha_bgr24_to_bgr24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
										  unsigned char alpha) {
	// check for full alpha value
	if (alpha == 0xff) {
		// max alpha is specified, so i can ignore it and use faster routine
		mmsfb_blit_bgr24_to_bgr24(src_planes, src_height, sx, sy, sw, sh,
								  dst_planes, dst_height, dx, dy);
		return;
	}

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blit coloralpha BGR24 to BGR24.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned char *src = (unsigned char *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned char *dst = (unsigned char *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
//	int src_pitch_pix = src_pitch / 3;
	int dst_pitch_pix = dst_pitch / 3;
	src+= sx*3 + sy * src_pitch;
	dst+= dx*3 + dy * dst_pitch;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	unsigned char *src_end = src + src_pitch * sh;
	int src_pitch_diff = src_pitch - sw * 3;
	int dst_pitch_diff = dst_pitch - sw * 3;

	register unsigned int A = alpha;
	register unsigned int SA= 0x100 - A;

	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned char *line_end = src + sw * 3;
		while (src < line_end) {
			// load source pixel
			unsigned int sr = *src;
			unsigned int sg = *(src+1);
			unsigned int sb = *(src+2);

			// load destination pixel
			unsigned int r = *dst;
			unsigned int g = *(dst+1);
			unsigned int b = *(dst+2);

			// invert src alpha
			r = (SA * r) >> 8;
			g = (SA * g) >> 8;
			b = (SA * b) >> 8;

			// add src to dst
			r += (A * sr) >> 8;
			g += (A * sg) >> 8;
			b += (A * sb) >> 8;
		    *dst     = (r >> 8) ? 0xff : r;
		    *(dst+1) = (g >> 8) ? 0xff : g;
		    *(dst+2) = (b >> 8) ? 0xff : b;

			/*
			// load pixel from memory and check if the previous pixel is the same
			register unsigned int SRC  = *src;

			// is the source alpha channel 0x00 or 0xff?
			register unsigned int A = SRC >> 24;
			if (A == 0xff) {
				// source pixel is not transparent, copy it directly to the destination
				*dst     = (unsigned char)(SRC >> 16);
				*(dst+1) = (unsigned char)(SRC >> 8);
				*(dst+2) = (unsigned char)SRC;
			}
			else
			if (A) {
				// source alpha is > 0x00 and < 0xff
				register unsigned int SA= 0x100 - A;
				unsigned int r = *dst;
				unsigned int g = *(dst+1);
				unsigned int b = *(dst+2);

				// invert src alpha
			    r = (SA * r) >> 8;
			    g = (SA * g) >> 8;
			    b = (SA * b) >> 8;

			    // add src to dst
			    r += (A*(SRC & 0xff0000)) >> 24;
			    g += (A*(SRC & 0xff00)) >> 16;
			    b += (A*(SRC & 0xff)) >> 8;
			    *dst     = (r >> 8) ? 0xff : r;
			    *(dst+1) = (g >> 8) ? 0xff : g;
			    *(dst+2) = (b >> 8) ? 0xff : b;
			}*/

		    dst+=3;
		    src+=3;
		}

		// go to the next line
		src+= src_pitch_diff;
		dst+= dst_pitch_diff;
	}
}

#endif
