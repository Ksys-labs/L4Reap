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

#ifdef __HAVE_PF_YV12__

#include "mmstools/mmstools.h"

#ifdef __HAVE_SWSCALE__
extern "C" {
#include <libswscale/swscale.h>
}
#endif

void mmsfb_stretchblit_yv12_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated stretch YV12 to YV12.\n");
		firsttime = false;
	}

//dy = 200;
//dh-= 400;


//////TESTONLY//////
#ifdef __HAVE_SWSCALE__
	static int fff = 0;
#endif
	static int testswitchtime = 0;
	static int testswitch = 0;
	if (!testswitchtime) {
		testswitchtime=time(NULL)+30;
#ifdef __HAVE_SWSCALE__
		printf(">>> using disko stretchblit YV12\n");
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
			printf(">>> using disko stretchblit YV12\n");
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
	if   ((sx == 0 && dx == 0)
		&&(src_planes->pitch == sw && src_height >= (sy + sh))
		&&(dst_planes->pitch == dw && dst_height >= (dy + dh))) {
		// stretchblit with libswscale
		// note: the width of the source and destination rectangle MUST
		//		 fit the width of the surfaces!!!
		// note: the position and the height of the source and destination rectangle
		//       should be multiple of two
		static SwsContext *sws_context = NULL;
		if (!sws_context) {
			const char *license = swscale_license();
			printf("DISKO: Using libswscale with license %s\n", license);
		}

		// multiple of two check
		if (sy & 0x01) {
			sy++;
			sh--;
		}
		if (dy & 0x01) {
			dy++;
			dh--;
		}
		sh = sh & ~0x01;
		dh = dh & ~0x01;

		// pixel width & height of source and destination
		int pix_src_width = src_planes->pitch;
		int pix_src_height = sh;
		int pix_dst_width = dst_planes->pitch;
		int pix_dst_height = dh;
		int src_yoffs = sy * src_planes->pitch;
		int dst_yoffs = dy * dst_planes->pitch;
		int flags = fff;

		// get an sws context with pixelformat PIX_FMT_YUV420P
		// which is equal to MMSFB_PF_I420 (reverse UV planes as against MMSFB_PF_YV12)
		// note: if the parameters are changed, a new context will be created
		//       else the previous context will be used
		sws_context = sws_getCachedContext(	sws_context,
											pix_src_width, pix_src_height, PIX_FMT_YUV420P,
											pix_dst_width, pix_dst_height, PIX_FMT_YUV420P,
											flags, NULL, NULL, NULL);
		if (!sws_context) {
			// error, using disko YV12 stretch blit fall back
			printf("DISKO: Failed to get SwsContext, using YV12 stretch blit fall back\n");
		}
		else {
			// set source pitches
			int sws_src_stride[3];
			sws_src_stride[0] = src_planes->pitch;
			sws_src_stride[1] = sws_src_stride[2] = src_planes->pitch/2;

			// set destination pitches
			int sws_dst_stride[3];
			sws_dst_stride[0] = dst_planes->pitch;
			sws_dst_stride[1] = sws_dst_stride[2] = dst_planes->pitch/2;

			// set source planes (reverse order, because source is YV12)
			uint8_t* sws_src[3];
			sws_src[0] = (uint8_t*)src_planes->ptr;
			if (!src_planes->ptr2) {
				// no plane pointers given, calculate it
				sws_src[2] = sws_src[0] + src_planes->pitch * src_height;
				sws_src[1] = sws_src[2] + src_planes->pitch * src_height / 4;
			}
			else {
				// set plane pointers
				sws_src[2] = (uint8_t*)src_planes->ptr2;
				sws_src[1] = (uint8_t*)src_planes->ptr3;
			}

			// add source offset
			sws_src[0] = sws_src[0] + src_yoffs;
			sws_src[1] = sws_src[1] + (src_yoffs >> 2);
			sws_src[2] = sws_src[2] + (src_yoffs >> 2);

			// set destination planes (reverse order, because destination is YV12)
			uint8_t* sws_dst[3];
			sws_dst[0] = (uint8_t*)dst_planes->ptr;
			if (!dst_planes->ptr2) {
				// no plane pointers given, calculate it
				sws_dst[2] = sws_dst[0] + dst_planes->pitch * dst_height;
				sws_dst[1] = sws_dst[2] + dst_planes->pitch * dst_height / 4;
			}
			else {
				// set plane pointers
				sws_dst[2] = (uint8_t*)dst_planes->ptr2;
				sws_dst[1] = (uint8_t*)dst_planes->ptr3;
			}

			// add destination offset
			sws_dst[0] = sws_dst[0] + dst_yoffs;
			sws_dst[1] = sws_dst[1] + (dst_yoffs >> 2);
			sws_dst[2] = sws_dst[2] + (dst_yoffs >> 2);

			// scale the buffer
			sws_scale(  sws_context,
						sws_src, sws_src_stride, 0, pix_src_height,
						sws_dst, sws_dst_stride);

			// finished :)
			return;
		}
	}
#endif

	}


	// here starts the disko YV12 stretch blit...

	// get the first source ptr/pitch
	unsigned char *src = (unsigned char *)src_planes->ptr;
	int src_pitch = src_planes->pitch;

	// get the first destination ptr/pitch
	unsigned char *dst = (unsigned char *)dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// prepare...
	int src_pitch_pix 		= src_pitch;
	int src_pitch_pix_half	= src_pitch_pix >> 1;
	int dst_pitch_pix 		= dst_pitch;
	int dst_pitch_pix_half	= dst_pitch_pix >> 1;

	// check the surface range
	if ((sw <= 0)||(sh <= 0))
		return;

	// check odd/even
	bool src_odd_left 	= (sx & 0x01);
	bool src_odd_top 	= (sy & 0x01);
	bool src_odd_right 	= ((sx + sw) & 0x01);
	bool src_odd_bottom = ((sy + sh) & 0x01);
	bool dst_odd_left 	= (dx & 0x01);
	bool dst_odd_top 	= (dy & 0x01);
	bool dst_odd_right 	= ((dx + dw) & 0x01);
	bool dst_odd_bottom = ((dy + dh) & 0x01);

	// pointer to the pixel components of the first source pixel
	unsigned char *src_y = src + sx + sy * src_pitch_pix;
	unsigned char *src_u;
	unsigned char *src_v;
	if ((src_planes->ptr2)&&(src_planes->ptr3)) {
		src_u = (unsigned char *)src_planes->ptr3 + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;
		src_v = (unsigned char *)src_planes->ptr2 + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;
	}
	else {
		src_u = src + src_pitch_pix * src_height + src_pitch_pix_half * (src_height >> 1) + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;
		src_v = src + src_pitch_pix * src_height                                          + (sx >> 1) + (sy >> 1) * src_pitch_pix_half;
	}

	// pointer to the pixel components of the first destination pixel
	unsigned char *dst_y = dst + dx + dy * dst_pitch_pix;
	unsigned char *dst_u;
	unsigned char *dst_v;
	if ((dst_planes->ptr2)&&(dst_planes->ptr3)) {
		dst_u = (unsigned char *)dst_planes->ptr2 + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;
		dst_v = (unsigned char *)dst_planes->ptr3 + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;
	}
	else {
		dst_u = dst + dst_pitch_pix * dst_height + dst_pitch_pix_half * (dst_height >> 1) + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;
		dst_v = dst + dst_pitch_pix * dst_height                                          + (dx >> 1) + (dy >> 1) * dst_pitch_pix_half;
	}

	// antialiasing horizontal/vertical/both
	bool h_antialiasing = false;
	bool v_antialiasing = false;
	if (antialiasing) {
		if (sw != dw)
			h_antialiasing = true;
		if (sh != dh)
			v_antialiasing = true;
	}

	// calc even positions for src...
	if (src_odd_top) {
		// odd top
		sy++;
		sh--;
		src_y+=src_pitch;
		src_u+=src_pitch >> 1;
		src_v+=src_pitch >> 1;
	}
	if (src_odd_bottom) {
		// odd bottom
		src_height--;
	}
	if (src_odd_left) {
		// odd left
		sx++;
		sw--;
		src_y++;
		src_u++;
		src_v++;
	}
	if (src_odd_right) {
		// odd right
		sw--;
	}

	// calc even positions for dst...
	if (dst_odd_top) {
		// odd top
		dy++;
		dh--;
		dst_y+=dst_pitch;
		dst_u+=dst_pitch >> 1;
		dst_v+=dst_pitch >> 1;
	}
	if (dst_odd_bottom) {
		// odd bottom
		dst_height--;
	}
	if (dst_odd_left) {
		// odd left
		dx++;
		dw--;
		dst_y++;
		dst_u++;
		dst_v++;
	}
	if (dst_odd_right) {
		// odd right
		dw--;
	}

	// check if something to do :)
	if ((sw < 2)||(sh < 2)||(dw < 2)||(dh < 2))
		return;

	// now we are even aligned and can go through a optimized loop
	////////////////////////////////////////////////////////////////////////

//printf("sw=%d,dw=%d,sh=%d,dh=%d\n", sw,dw, sh,dh);

	// calc U plane (use Y plane as temp buffer)
	// note: concerning performance we use vertical antialiasing only in combination with horizontal antialiasing
	// note: the stretch and the subsequent 2x2 matrix conversion is needed to calculate the correct arithmetic mean
	stretch_byte_buffer(h_antialiasing, h_antialiasing,
						src_u, src_pitch >> 1, src_pitch_pix >> 1, src_height >> 1, sw >> 1, sh >> 1,
						dst_y, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
	compress_2x2_matrix(dst_y, dst_pitch, dst_pitch_pix, dst_height, dw, dh,
						dst_u, dst_pitch >> 1, dst_pitch_pix >> 1, dst_height >> 1, dw >> 1, dh >> 1);

	// calc V plane (use Y plane as temp buffer)
	// note: concerning performance we use vertical antialiasing only in combination with horizontal antialiasing
	// note: the stretch and the subsequent 2x2 matrix conversion is needed to calculate the correct arithmetic mean
	stretch_byte_buffer(h_antialiasing, h_antialiasing,
						src_v, src_pitch >> 1, src_pitch_pix >> 1, src_height >> 1, sw >> 1, sh >> 1,
						dst_y, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
	compress_2x2_matrix(dst_y, dst_pitch, dst_pitch_pix, dst_height, dw, dh,
						dst_v, dst_pitch >> 1, dst_pitch_pix >> 1, dst_height >> 1, dw >> 1, dh >> 1);

	// calc Y plane
	stretch_byte_buffer(h_antialiasing, v_antialiasing,
						src_y, src_pitch, src_pitch_pix, src_height, sw, sh,
					    dst_y, dst_pitch, dst_pitch_pix, dst_height, dw, dh);
}

#endif
