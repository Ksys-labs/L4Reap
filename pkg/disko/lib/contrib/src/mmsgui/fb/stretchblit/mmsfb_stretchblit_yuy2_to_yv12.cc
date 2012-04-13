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

#ifdef __HAVE_PF_YUY2__
#ifdef __HAVE_PF_YV12__

#include "mmsgui/fb/mmsfbsurface.h"
#include "mmstools/mmstools.h"

#ifdef __HAVE_SWSCALE__
extern "C" {
#include <libswscale/swscale.h>
}
#endif

void mmsfb_stretchblit_yuy2_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated stretch YUY2 to YV12.\n");
		firsttime = false;
	}

//dy = 200;
//dh-= 400;

//sx+=100;
//sw-=200;

//dx+=100;
//dw-=200;

	static MMSFBSurface *interim = NULL;
#ifdef __HAVE_SWSCALE__
	static MMSFBSurface *interim2= NULL;
#endif

//////TESTONLY//////
#ifdef __HAVE_SWSCALE__
	static int fff = 0;
#endif
	static int testswitchtime = 0;
	static int testswitch = 0;
	if (!testswitchtime) {
		testswitchtime=time(NULL)+30;
#ifdef __HAVE_SWSCALE__
		printf(">>> using disko stretchblit YUY2\n");
#endif
	}
	else
	if (testswitchtime < time(NULL)) {
		testswitchtime=time(NULL)+30;
		testswitch++;
		if (testswitch > 11)
			testswitch = 0;
#ifdef __HAVE_SWSCALE__
		switch (testswitch) {
		case 0:
			printf(">>> using disko stretchblit YUY2\n");
			break;
		case 1:
			fff = SWS_POINT;
			printf(">>> using SWSCALE stretchblit flag=SWS_POINT\n");
			break;
		case 2:
			fff = SWS_AREA;
			printf(">>> using SWSCALE stretchblit flag=SWS_AREA\n");
			break;
		case 3:
			fff = SWS_BILINEAR;
			printf(">>> using SWSCALE stretchblit flag=SWS_BILINEAR\n");
			break;
		case 4:
			fff = SWS_FAST_BILINEAR;
			printf(">>> using SWSCALE stretchblit flag=SWS_FAST_BILINEAR\n");
			break;
		case 5:
			fff = SWS_BICUBIC;
			printf(">>> using SWSCALE stretchblit flag=SWS_BICUBIC\n");
			break;
		case 6:
			fff = SWS_X;
			printf(">>> using SWSCALE stretchblit flag=SWS_X\n");
			break;
		case 7:
			fff = SWS_GAUSS;
			printf(">>> using SWSCALE stretchblit flag=SWS_GAUSS\n");
			break;
		case 8:
			fff = SWS_LANCZOS;
			printf(">>> using SWSCALE stretchblit flag=SWS_LANCZOS\n");
			break;
		case 9:
			fff = SWS_SINC;
			printf(">>> using SWSCALE stretchblit flag=SWS_SINC\n");
			break;
		case 10:
			fff = SWS_SPLINE;
			printf(">>> using SWSCALE stretchblit flag=SWS_SPLINE\n");
			break;
		case 11:
			fff = SWS_BICUBLIN;
			printf(">>> using SWSCALE stretchblit flag=SWS_BICUBLIN\n");
			break;
		}

		/*
		02585     i= flags & ( SWS_POINT
		02586                 |SWS_AREA
		02587                 |SWS_BILINEAR
		02588                 |SWS_FAST_BILINEAR
		02589                 |SWS_BICUBIC
		02590                 |SWS_X
		02591                 |SWS_GAUSS
		02592                 |SWS_LANCZOS
		02593                 |SWS_SINC
		02594                 |SWS_SPLINE
		02595                 |SWS_BICUBLIN);
*/




#endif
	}
	if (testswitch > 0) {
/////TESTONLY///////
#ifdef __HAVE_SWSCALE__
	// this code is for libswscale support...
	MMSFBRectangle srect = MMSFBRectangle(sx,sy,sw,sh);
	MMSFBSurfacePlanes splanes = *src_planes;
	int sheight = src_height;
	MMSFBRectangle drect = MMSFBRectangle(dx,dy,dw,dh);
	MMSFBSurfacePlanes dplanes = *dst_planes;
	int dheight = dst_height;


//	if ((dx == 0) && (dst_planes->pitch == dw)) {
	{
// the width of the destination rectangle MUST fit the width of the destination surface!
		if ((sx != 0) || ((src_planes->pitch >> 1) != sw)) {
			// the width of the source rectangle is NOT equal to the width of the source surface!
			// have to use a interim surface
			if (mmsfb_create_cached_surface(&interim, sw, sh, MMSFB_PF_YUY2)) {
				// blit YUY2->YUY2
				interim->blitBuffer(&splanes, MMSFB_PF_YUY2, splanes.pitch >> 1, sheight,
									&srect, 0, 0);
				interim->lock(MMSFB_LOCK_READ, &splanes);
				interim->unlock();
				srect.x = 0;
				srect.y = 0;
				sheight = sh;
			}
		}
		else {
			// no interim surface needed
			if (interim) {
				delete interim;
				interim = NULL;
			}
		}

		if ((dx != 0) || (dst_planes->pitch != dw)) {
			// the width of the destination rectangle is NOT equal to the width of the destination surface!
			// have to use a interim surface
			if (mmsfb_create_cached_surface(&interim2, dw, dh, MMSFB_PF_YV12)) {
				interim2->lock(MMSFB_LOCK_READ, &dplanes);
				interim2->unlock();
				drect.x = 0;
				drect.y = 0;
				dheight = dh;
			}
		}
		else {
			// no interim surface needed
			if (interim2) {
				delete interim2;
				interim2 = NULL;
			}
		}



		// pixel width & height of source and destination
		int pix_src_width = splanes.pitch >> 1;
		int pix_src_height = srect.h;
		int pix_dst_width = dplanes.pitch;
		int pix_dst_height = drect.h;
		int src_yoffs = srect.y * splanes.pitch;
		int dst_yoffs = drect.y * dplanes.pitch;
		int flags = fff;

		// get an sws context with pixelformat PIX_FMT_YUYV422 -> PIX_FMT_YUV420P
		// note: if the parameters are changed, a new context will be created
		//       else the previous context will be used
		static SwsContext *sws_context = NULL;
		if (!sws_context) {
			const char *license = swscale_license();
			printf("DISKO: Using libswscale with license %s\n", license);
		}
		sws_context = sws_getCachedContext(	sws_context,
											pix_src_width, pix_src_height, PIX_FMT_YUYV422,
											pix_dst_width, pix_dst_height, PIX_FMT_YUV420P,
											flags, NULL, NULL, NULL);
		if (!sws_context) {
			// error, using disko YUY2->YV12 stretch blit fall back
			printf("DISKO: Failed to get SwsContext, using YUY2->YV12 stretch blit fall back\n");
		}
		else {
			// set source pitches
			int sws_src_stride[3];
			sws_src_stride[0] = splanes.pitch;
			sws_src_stride[1] = sws_src_stride[2] = 0;

			// set destination pitches
			int sws_dst_stride[3];
			sws_dst_stride[0] = dplanes.pitch;
			sws_dst_stride[1] = sws_dst_stride[2] = dplanes.pitch/2;

			// set source planes
			uint8_t* sws_src[3];
			sws_src[0] = (uint8_t*)splanes.ptr;
			sws_src[1] = NULL;
			sws_src[2] = NULL;

			// add source offset
			sws_src[0] = sws_src[0] + src_yoffs;

			// set destination planes (reverse order, because destination is YV12)
			uint8_t* sws_dst[3];
			sws_dst[0] = (uint8_t*)dplanes.ptr;
			if (!dplanes.ptr2) {
				// no plane pointers given, calculate it
				sws_dst[2] = sws_dst[0] + dplanes.pitch * dheight;
				sws_dst[1] = sws_dst[2] + dplanes.pitch * dheight / 4;
			}
			else {
				// set plane pointers
				sws_dst[2] = (uint8_t*)dplanes.ptr2;
				sws_dst[1] = (uint8_t*)dplanes.ptr3;
			}

			// add destination offset
			sws_dst[0] = sws_dst[0] + dst_yoffs;
			sws_dst[1] = sws_dst[1] + (dst_yoffs >> 2);
			sws_dst[2] = sws_dst[2] + (dst_yoffs >> 2);

			// scale the buffer
			sws_scale(  sws_context,
						sws_src, sws_src_stride, 0, pix_src_height,
						sws_dst, sws_dst_stride);

			if (interim2) {
				// final blit from iterim to destination
				mmsfb_blit_yv12_to_yv12(&dplanes, dheight, drect.x, drect.y, drect.w, drect.h,
										dst_planes, dst_height, dx, dy);
			}

			// finished :)
			return;
		}
	}
#endif

	}

	// here starts the disko YUY2->YV12 stretch blit...
	if (mmsfb_create_cached_surface(&interim, src_planes->pitch >> 1, src_height, MMSFB_PF_YV12)) {
		// blit YUY2->YV12
		interim->blitBuffer(src_planes, MMSFB_PF_YUY2, src_planes->pitch >> 1, src_height,
							NULL, 0, 0);

		// stretch YV12->YV12
		MMSFBSurfacePlanes sp;
		interim->lock(MMSFB_LOCK_READ, &sp);
		mmsfb_stretchblit_yv12_to_yv12(&sp, src_height, sx, sy, sw, sh,
									   dst_planes, dst_height, dx, dy, dw, dh,
									   antialiasing);
		interim->unlock();
	}
}

#endif
#endif
