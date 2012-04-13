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

#ifdef __HAVE_PF_RGB24__

#include <cstring>
#include "mmstools/mmstools.h"

void mmsfb_blit_rgb24_to_rgb24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated copy RGB24 to RGB24.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned char *src = (unsigned char *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned char *dst = (unsigned char *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
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
	int line_len = sw * 3;

	// for all lines
	while (src < src_end) {
		// copy the line
		memcpy(dst, src, line_len);

		// go to the next line
		src+= src_pitch;
		dst+= dst_pitch;
	}
}

#endif
