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
#ifdef __HAVE_PF_ARGB3565__

#include "mmstools/mmstools.h"

#define MMSFB_BLIT_BLEND_ARGB_TO_ARGB3565(src, dst, dst_a, alpha)		\
			SRC  = src;													\
			A    = SRC >> 24;											\
			if (A == 0xff) {											\
				if (SRC==OLDSRC) {										\
					dst = d;											\
					alpha da;											\
				}														\
				else {													\
					OLDSRC = SRC;										\
					unsigned int r = (SRC << 8) >> 27;					\
					unsigned int g = (SRC << 16) >> 26;					\
					unsigned int b = (SRC & 0xff) >> 3;					\
					d =   (r << 11)										\
						| (g << 5)										\
						| b;											\
					da = A >> 5;										\
					dst = d;											\
					alpha da;											\
				}														\
			}															\
			else														\
			if (!A) {													\
				alpha dst_a;											\
			}															\
			else {														\
				register unsigned short int DST = dst;					\
				if ((DST==OLDDST)&&(dst_a==OLDDSTA)&&(SRC==OLDSRC)) {	\
					dst = d;											\
					alpha da;											\
				}														\
				else  {													\
					OLDDST  = DST;										\
					OLDDSTA = dst_a;									\
					OLDSRC = SRC;										\
					register unsigned int SA= 0x100 - A;				\
					unsigned int a = dst_a;								\
					unsigned int r = DST >> 11;							\
					unsigned int g = (DST << 5) >> 10;					\
					unsigned int b = DST & 0x1f;						\
					a = (SA * a) >> 3;									\
					r = (SA * r) >> 5;									\
					g = (SA * g) >> 6;									\
					b = (SA * b) >> 5;									\
					a += A >> 5;										\
					r += (SRC << 8) >> 27;								\
					g += (SRC << 16) >> 26;								\
					b += (SRC << 24) >> 27;								\
					d =   ((r >> 5) ? 0xf800   	: (r << 11))			\
						| ((g >> 6) ? 0x07e0	: (g << 5))				\
						| ((b >> 5) ? 0x1f 		: b);					\
					da = (a >> 8) ? 0x07 : a;							\
					dst = d;											\
					alpha da;											\
				}														\
			}


void mmsfb_blit_blend_argb_to_argb3565(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated blend ARGB to ARGB3565.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned int *src = (unsigned int *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

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

	// prepare...
	int src_pitch_pix = src_pitch >> 2;
	int dst_pitch_pix = dst_pitch >> 1;
	src+= sx + sy * src_pitch_pix;
	dst+= dx + dy * dst_pitch_pix;
	dst_a+= (dx >> 1) + dy * dst_a_pitch;

	// check the surface range
	if (dst_pitch_pix - dx < sw - sx)
		sw = dst_pitch_pix - dx - sx;
	if (dst_height - dy < sh - sy)
		sh = dst_height - dy - sy;
	if ((sw <= 0)||(sh <= 0))
		return;

	// check odd/even
	bool odd_left 	= (dx & 0x01);
	bool odd_right 	= ((dx + sw) & 0x01);


	//TODO: not even...



	// calc even positions...
	if (odd_left) {
		// odd left
		dx++;
		sw--;
		src++;
		dst++;
		dst_a++;
	}

	if (odd_right) {
		// odd right
		sw--;
	}

	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////
	unsigned short int OLDDST  = (*dst) + 1;
	unsigned char 	   OLDDSTA = (*dst_a) + 1;
	unsigned int OLDSRC = (*src) + 1;
	unsigned int *src_end = src + src_pitch_pix * sh;
	int src_pitch_diff = src_pitch_pix - sw;
	int dst_pitch_diff = dst_pitch_pix - sw;
	int dst_a_pitch_diff = dst_a_pitch - (sw >> 1);
	register unsigned short int d;
	register unsigned char da;


	// for all lines
	while (src < src_end) {
		// for all pixels in the line
		unsigned int *line_end = src + sw;
		while (src < line_end) {
			register unsigned int SRC;
			register unsigned int A;
			unsigned char alpha;

			// process two pixels
			MMSFB_BLIT_BLEND_ARGB_TO_ARGB3565(*src, *dst, ((*dst_a) >> 4), alpha=);
			MMSFB_BLIT_BLEND_ARGB_TO_ARGB3565(*(src+1), *(dst+1), ((*dst_a)&0x0f), alpha=(alpha<<4)|);
			dst+=2;
			src+=2;

			// set alpha value
			*dst_a = alpha;
			dst_a++;
		}

		// go to the next line
		src+= src_pitch_diff;
		dst+= dst_pitch_diff;
		dst_a+= dst_a_pitch_diff;
	}
}

#endif
#endif
