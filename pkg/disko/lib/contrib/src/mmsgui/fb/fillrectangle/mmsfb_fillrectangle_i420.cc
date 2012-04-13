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

#include "mmstools/mmstools.h"

void mmsfb_fillrectangle_i420(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated fill rectangle to I420.\n");
		firsttime = false;
	}

	// prepare destination planes
	MMSFBSurfacePlanes destination_planes;
	if ((dst_planes->ptr2)&&(dst_planes->ptr3)) {
		destination_planes = *dst_planes;
	}
	else {
		destination_planes.ptr = dst_planes->ptr;
		destination_planes.pitch = dst_planes->pitch;
		destination_planes.ptr2 = (unsigned char *)destination_planes.ptr + destination_planes.pitch * dst_height;
		destination_planes.pitch2 = dst_planes->pitch;
		destination_planes.ptr3 = (unsigned char *)destination_planes.ptr + destination_planes.pitch * dst_height + (destination_planes.pitch >> 1) * (dst_height >> 1);
		destination_planes.pitch3 = dst_planes->pitch;
	}

	// now we can use the YV12 fill
	mmsfb_fillrectangle_yv12(&destination_planes, dst_height,
							 dx, dy, dw, dh, color);
}

#endif
