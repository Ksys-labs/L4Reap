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

#ifdef __HAVE_PF_ARGB3565__

#include <cstring>
#include "mmstools/mmstools.h"

void mmsfb_blit_argb3565_to_argb3565(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated copy ARGB3565 to ARGB3565.\n");
		firsttime = false;
	}

	// SRC: point to the first plane (RGB16/RGB565 format)
	unsigned short int *src = (unsigned short int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// SRC: point to the second plane (3 bit alpha (2 pixels per byte, 4 bit per pixel))
	unsigned char *src_a;
	int src_a_pitch;
	if (src_planes->ptr2) {
		// plane pointer given
		src_a = (unsigned char *)src_planes->ptr2;
		src_a_pitch = src_planes->pitch2;
	}
	else {
		// calc plane pointer (after the first plane)
    	src_a = ((unsigned char *)src_planes->ptr) + src_planes->pitch * src_height;
    	src_a_pitch = src_planes->pitch / 4;
	}

	// DST: point to the first plane (RGB16/RGB565 format)
	unsigned short int *dst = (unsigned short int *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// DST: point to the second plane (3 bit alpha (2 pixels per byte, 4 bit per pixel))
	unsigned char *dst_a;
	int dst_a_pitch;
	if (dst_planes->ptr2) {
		// plane pointer given
		dst_a = (unsigned char *)dst_planes->ptr2;
		dst_a_pitch = dst_planes->pitch2;
	}
	else {
		// calc plane pointer (after the first plane)
    	dst_a = ((unsigned char *)dst_planes->ptr) + dst_planes->pitch * dst_height;
    	dst_a_pitch = dst_planes->pitch / 4;
	}

	// working for first plane (RGB16/RGB565 format)
	////////////////////////////////////////////////
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

	unsigned short int *src_end = src + src_pitch_pix * sh;

	// for all lines
	while (src < src_end) {
		// copy the line
		memcpy(dst, src, sw << 1);

		// go to the next line
		src+= src_pitch_pix;
		dst+= dst_pitch_pix;
	}


	// working for second plane (3 bit alpha (2 pixels per byte, 4 bit per pixel))
	//////////////////////////////////////////////////////////////////////////////
	src_a+= (sx >> 1) + sy * src_a_pitch;
	dst_a+= (dx >> 1) + dy * dst_a_pitch;

	// check odd/even
	bool odd_left 	= (dx & 0x01);
	bool odd_right 	= ((dx + sw) & 0x01);

	//TODO: not even...



	// calc even positions...
	if (odd_left) {
		// odd left
		dx++;
		sw--;
		src_a++;
		dst_a++;
	}

	if (odd_right) {
		// odd right
		sw--;
	}

	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////
	src_end = (unsigned short int *)(src_a + src_a_pitch * sh);

	// for all lines
	while (src_a < (unsigned char *)src_end) {
		// copy the line
		memcpy(dst_a, src_a, sw >> 1);

		// go to the next line
		src_a+= src_a_pitch;
		dst_a+= dst_a_pitch;
	}
}

#endif
