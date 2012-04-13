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
#include "mmstools/mmstools.h"

#define MMSFB_DRAWLINE_BLEND_PIXEL \
	if ((x >= clipreg.x1)&&(x <= clipreg.x2)&&(y >= clipreg.y1)&&(y <= clipreg.y2)) { \
		register unsigned int DST = dst[x+y*dst_pitch_pix]; \
		if (DST==OLDDST) dst[x+y*dst_pitch_pix] = d;  else { \
		OLDDST = DST; \
		register int SA= 0x100 - A; \
		unsigned int a = DST >> 24; \
		unsigned int r = (DST << 8) >> 24; \
		unsigned int g = (DST << 16) >> 24; \
		unsigned int b = DST & 0xff; \
	    a = (SA * a) >> 8; \
	    r = (SA * r) >> 8; \
	    g = (SA * g) >> 8; \
	    b = (SA * b) >> 8; \
	    a += A; \
	    r += (SRC << 8) >> 24; \
	    g += (SRC << 16) >> 24; \
	    b += SRC & 0xff; \
	    d =   ((a >> 8) ? 0xff000000 : (a << 24)) \
			| ((r >> 8) ? 0xff0000   : (r << 16)) \
			| ((g >> 8) ? 0xff00     : (g << 8)) \
	    	| ((b >> 8) ? 0xff 		 :  b); \
	    dst[x+y*dst_pitch_pix] = d; } }

void mmsfb_drawline_blend_argb(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       MMSFBRegion &clipreg, int x1, int y1, int x2, int y2, MMSFBColor &color) {
	if (color.a == 0xff) {
		// source pixel is not transparent
		mmsfb_drawline_argb(dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
		return;
	}

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend line to ARGB.\n");
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
	unsigned int OLDDST = 0;
	register unsigned int d = 0;

	// prepare the color
	register unsigned int A = color.a;
	register unsigned int SRC;
	SRC =     (A << 24)
			| (color.r << 16)
			| (color.g << 8)
			| color.b;
	d = SRC;

	// draw a line with Bresenham-Algorithm
	MMSFB_DRAWLINE_BRESENHAM(MMSFB_DRAWLINE_BLEND_PIXEL);
}

