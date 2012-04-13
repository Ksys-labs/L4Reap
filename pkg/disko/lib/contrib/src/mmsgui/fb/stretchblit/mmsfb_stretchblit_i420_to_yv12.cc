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

#ifdef __HAVE_PF_I420__
#ifdef __HAVE_PF_YV12__

#include "mmstools/mmstools.h"

void mmsfb_stretchblit_i420_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated stretch I420 to YV12.\n");
		firsttime = false;
	}

	// prepare source planes
	MMSFBSurfacePlanes source_planes;
	if ((src_planes->ptr2)&&(src_planes->ptr3)) {
		source_planes = *src_planes;
	}
	else {
		source_planes.ptr = src_planes->ptr;
		source_planes.pitch = src_planes->pitch;
		source_planes.ptr2 = (unsigned char *)source_planes.ptr + source_planes.pitch * src_height + (source_planes.pitch >> 1) * (src_height >> 1);
		source_planes.pitch2 = src_planes->pitch;
		source_planes.ptr3 = (unsigned char *)source_planes.ptr + source_planes.pitch * src_height;
		source_planes.pitch3 = src_planes->pitch;
	}

	// now we can use the YV12 to YV12 stretch blit
	mmsfb_stretchblit_yv12_to_yv12(&source_planes, src_height, sx, sy, sw, sh,
								   dst_planes, dst_height, dx, dy, dw, dh,
								   antialiasing);
}

#endif
#endif
