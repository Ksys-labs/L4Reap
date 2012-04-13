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

#ifdef __HAVE_PF_AiRGB__

#include "mmstools/mmstools.h"

void mmsfb_blit_blend_coloralpha_airgb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
												unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy,
												unsigned char alpha) {
	// check for full alpha value
	if (alpha == 0xff) {
		// max alpha is specified, so i can ignore it and use faster routine
		mmsfb_blit_blend_airgb_to_airgb(extbuf, src_height, sx, sy, sw, sh,
										dst, dst_pitch, dst_height, dx, dy);
		return;
	}

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend coloralpha AiRGB to AiRGB.\n");
		firsttime = false;
	}

	// something to do?
	if (!alpha)
		// source should blitted full transparent, so leave destination as is
		return;

	// get the first source ptr/pitch
	unsigned int *src = (unsigned int *)extbuf->ptr;
	int src_pitch = extbuf->pitch;

	// prepare...
	int src_pitch_pix = src_pitch >> 2;
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
	unsigned int OLDSRC  = (*src) + 1;
	unsigned int *src_end = src + src_pitch_pix * sh;
	int src_pitch_diff = src_pitch_pix - sw;
	int dst_pitch_diff = dst_pitch_pix - sw;
	register unsigned int d;

	register unsigned int ALPHA = alpha;
	ALPHA++;

	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned int *line_end = src + sw;
		while (src < line_end) {
			// load pixel from memory and check if the previous pixel is the same
			register unsigned int SRC = *src;

			// is the source alpha channel 0x00 or 0xff?
			register unsigned int A = SRC >> 24;
			if (A < 0xff) {
				// source alpha is >= 0x00 and < 0xff
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

				// load source pixel and multiply it with given ALPHA
			    A = 0x100 - ((ALPHA * (0x100 - A)) >> 8);
				unsigned int sr = (ALPHA * (SRC & 0xff0000)) >> 24;
				unsigned int sg = (ALPHA * (SRC & 0xff00)) >> 16;
				unsigned int sb = (ALPHA * (SRC & 0xff)) >> 8;

				unsigned int a = DST >> 24;
				unsigned int r = (DST << 8) >> 24;
				unsigned int g = (DST << 16) >> 24;
				unsigned int b = DST & 0xff;

				// invert src alpha
			    a = (A * (0x100 - a)) >> 8;
			    r = (A * r) >> 8;
			    g = (A * g) >> 8;
			    b = (A * b) >> 8;

			    // add src to dst
			    a += 0x100 - A;
			    r += sr;
			    g += sg;
			    b += sb;
				d =	  ((r >> 8) ? 0xff0000   : (r << 16))
					| ((g >> 8) ? 0xff00     : (g << 8))
			    	| ((b >> 8) ? 0xff 		 :  b);
			    if (!(a >> 8)) d |= (0x100 - a) << 24;
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
