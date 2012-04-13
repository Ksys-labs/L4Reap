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
#ifdef __HAVE_PF_RGB32__

#include "mmstools/mmstools.h"

void mmsfb_stretchblit_blend_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated stretch & blend ARGB to RGB32.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned int *src = (unsigned int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned int *dst = (unsigned int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int  src_pitch_pix = src_pitch >> 2;
	int  dst_pitch_pix = dst_pitch >> 2;
	unsigned int *src_end = src + sx + src_pitch_pix * (sy + sh);
	if (src_end > src + src_pitch_pix * src_height)
		src_end = src + src_pitch_pix * src_height;
	unsigned int *dst_end = dst + dst_pitch_pix * dst_height;
	src+=sx + sy * src_pitch_pix;
	dst+=dx + dy * dst_pitch_pix;


//	printf("sw=%d,sh=%d\n", sw,sh);

	int horifact = (dw<<16)/sw;
	int vertfact = (dh<<16)/sh;



	// for all lines
	int vertcnt = 0x8000;
	while ((src < src_end)&&(dst < dst_end)) {
		// for all pixels in the line
		vertcnt+=vertfact;
		if (vertcnt & 0xffff0000) {
			unsigned int *line_end = src + sw;
			unsigned int *old_dst = dst;

			do {
				int horicnt = 0x8000;
				while (src < line_end) {
					// load pixel from memory and check if the previous pixel is the same

					horicnt+=horifact;

					if (horicnt & 0xffff0000) {
						register unsigned int SRC  = *src;
						register unsigned int A = SRC >> 24;

						if (A == 0xff) {
							// source pixel is not transparent, copy it directly to the destination
							do {
								*dst = SRC | 0xff000000;
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}
						else
						if (!A) {
							// source pixel is full transparent, do not change the destination
							do {
								dst++;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}
						else
						{
							// source alpha is > 0x00 and < 0xff
							register unsigned int SA= 0x100 - A;
							register unsigned int DST = *dst;
							unsigned int OLDDST = DST + 1;
							register unsigned int d;

							do {
								if (DST==OLDDST) {
									// same pixel, use the previous value
									if (A) {
										// source has an alpha
										*dst = d;
									}
								    dst++;
								    DST = *dst;
									horicnt-=0x10000;
									continue;
								}
								OLDDST = DST;

								// extract destination
								unsigned int r = (DST << 8) >> 24;
								unsigned int g = (DST << 16) >> 24;
								unsigned int b = DST & 0xff;

								// invert src alpha
							    r = (SA * r) >> 8;
							    g = (SA * g) >> 8;
							    b = (SA * b) >> 8;

							    // add src to dst
							    r += (A*(SRC & 0xff0000)) >> 24;
							    g += (A*(SRC & 0xff00)) >> 16;
							    b += (A*(SRC & 0xff)) >> 8;
							    d =   0xff000000
									| ((r >> 8) ? 0xff0000   : (r << 16))
									| ((g >> 8) ? 0xff00     : (g << 8))
							    	| ((b >> 8) ? 0xff 		 :  b);
								*dst = d;
								dst++;
								DST = *dst;
								horicnt-=0x10000;
							} while (horicnt & 0xffff0000);
						}
					}

				    src++;
				}
				src-=sw;
				vertcnt-=0x10000;
				dst = old_dst + dst_pitch_pix;
				old_dst = dst;
			} while (vertcnt & 0xffff0000);
		}

		// next line
		src+=src_pitch_pix;
	}
}

#endif
#endif
