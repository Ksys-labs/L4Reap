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

#ifndef MMSFBCONV_H_
#define MMSFBCONV_H_

#include "mmsgui/fb/mmsfbbase.h"
#include "mmsgui/fb/mmsfbfont.h"

// check which pixelformats we should support
#ifndef __HAVE_PF_ALL__
#ifndef __HAVE_PF_ARGB__
#ifndef __HAVE_PF_AiRGB__
#ifndef __HAVE_PF_ARGB4444__
#ifndef __HAVE_PF_ARGB3565__
#ifndef __HAVE_PF_RGB16__
#ifndef __HAVE_PF_RGB24__
#ifndef __HAVE_PF_RGB32__
#ifndef __HAVE_PF_BGR24__
#ifndef __HAVE_PF_BGR555__
#ifndef __HAVE_PF_AYUV__
#ifndef __HAVE_PF_YV12__
#ifndef __HAVE_PF_I420__
#ifndef __HAVE_PF_YUY2__
// no pixelformats defined
#ifndef __HAVE_PF_NONE__
#define __HAVE_PF_NONE__
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifdef __HAVE_PF_ALL__
// all pixelformats requested, enable all
#ifndef __HAVE_PF_ARGB__
#define __HAVE_PF_ARGB__
#endif
#ifndef __HAVE_PF_AiRGB__
#define __HAVE_PF_AiRGB__
#endif
#ifndef __HAVE_PF_ARGB4444__
#define __HAVE_PF_ARGB4444__
#endif
#ifndef __HAVE_PF_ARGB3565__
#define __HAVE_PF_ARGB3565__
#endif
#ifndef __HAVE_PF_RGB16__
#define __HAVE_PF_RGB16__
#endif
#ifndef __HAVE_PF_RGB24__
#define __HAVE_PF_RGB24__
#endif
#ifndef __HAVE_PF_RGB32__
#define __HAVE_PF_RGB32__
#endif
#ifndef __HAVE_PF_BGR24__
#define __HAVE_PF_BGR24__
#endif
#ifndef __HAVE_PF_BGR555__
#define __HAVE_PF_BGR555__
#endif
#ifndef __HAVE_PF_AYUV__
#define __HAVE_PF_AYUV__
#endif
#ifndef __HAVE_PF_YV12__
#define __HAVE_PF_YV12__
#endif
#ifndef __HAVE_PF_I420__
#define __HAVE_PF_I420__
#endif
#ifndef __HAVE_PF_YUY2__
#define __HAVE_PF_YUY2__
#endif
#endif

// always enable ARGB format which is basically needed
#ifndef __HAVE_PF_ARGB__
#define __HAVE_PF_ARGB__
#endif

// enable also YV12 if I420 format is requested
#ifdef __HAVE_PF_I420__
#ifndef __HAVE_PF_YV12__
#define __HAVE_PF_YV12__
#endif
#endif


//#define MMSFB_CONV_RGB2Y(r,g,b) ((((66*r+129*g+25*b+128)>>8)+16) & 0xff)
//#define MMSFB_CONV_RGB2U(r,g,b) ((((-38*r-74*g+112*b+128)>>8)+128) & 0xff)
//#define MMSFB_CONV_RGB2V(r,g,b) ((((112*r-94*g-18*b+128)>>8)+128) & 0xff)
#define MMSFB_CONV_RGB2Y(r,g,b) (((66*r+129*g+25*b+128)>>8)+16)
#define MMSFB_CONV_RGB2U(r,g,b) (((-38*r-74*g+112*b+128)>>8)+128)
#define MMSFB_CONV_RGB2V(r,g,b) (((112*r-94*g-18*b+128)>>8)+128)

#define MMSFB_CONV_PREPARE_YUV2RGB(y,u,v)  y=(int)y-16;u=(int)u-128;v=(int)v-128;
#define MMSFB_CONV_PREPARE_YUVBLEND(y,u,v) y=(int)y-16;u=(int)u-128;v=(int)v-128;
#define MMSFB_CONV_RESET_YUVBLEND(y,u,v) y=(int)y+16;u=(int)u+128;v=(int)v+128;

//#define MMSFB_CONV_YUV2R(y,u,v,r) if ((r=(298*(int)y+409*(int)v+128+0x200)>>8)>>8) r=0xff;
//#define MMSFB_CONV_YUV2G(y,u,v,g) if ((g=(298*(int)y-100*(int)u-208*(int)v+128+0x200)>>8)>>8) g=0xff;
//#define MMSFB_CONV_YUV2B(y,u,v,b) if ((b=(298*(int)y+516*(int)u+128+0x200)>>8)>>8) b=0xff;
#define MMSFB_CONV_YUV2R(y,u,v,r) if ((r=(298*(int)y+409*(int)v+128+0x200)>>8)<0)r=0;else if(r>0xff)r=0xff;
#define MMSFB_CONV_YUV2G(y,u,v,g) if ((g=(298*(int)y-100*(int)u-208*(int)v+128+0x200)>>8)<0)g=0;else if(g>0xff)g=0xff;
#define MMSFB_CONV_YUV2B(y,u,v,b) if ((b=(298*(int)y+516*(int)u+128+0x200)>>8)<0)b=0;else if(b>0xff)b=0xff;

#define MMSFB_CONV_YUV2RX(y,u,v,r) if ((r=298*(int)y+409*(int)v+128+0x200)>>16) r=0xff00;
#define MMSFB_CONV_YUV2GX(y,u,v,g) if ((g=298*(int)y-100*(int)u-208*(int)v+128+0x200)>>16) g=0xff00;
#define MMSFB_CONV_YUV2BX(y,u,v,b) if ((b=298*(int)y+516*(int)u+128+0x200)>>16) b=0xff00;


#define MMSFB_CONV_RGB24_TO_YV12_PUSHPTR \
	unsigned char *saved_src   = src;   \
	unsigned char *saved_dst_y = dst_y; \
	unsigned char *saved_dst_u = dst_u; \
	unsigned char *saved_dst_v = dst_v;

#define MMSFB_CONV_RGB24_TO_YV12_POPPTR \
	src   = saved_src;   \
	dst_y = saved_dst_y; \
	dst_u = saved_dst_u; \
	dst_v = saved_dst_v;

#define MMSFB_CONV_RGB24_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	{	register int r = *(src);				\
		register int g = *(src+1);				\
		register int b = *(src+2);				\
		dst_y = MMSFB_CONV_RGB2Y(r,g,b);		\
		d_u     MMSFB_CONV_RGB2U(r,g,b);		\
		d_v     MMSFB_CONV_RGB2V(r,g,b); }



#define MMSFB_CONV_ARGB_TO_YV12_PUSHPTR \
	unsigned int  *saved_src   = src;   \
	unsigned char *saved_dst_y = dst_y; \
	unsigned char *saved_dst_u = dst_u; \
	unsigned char *saved_dst_v = dst_v;

#define MMSFB_CONV_ARGB_TO_YV12_POPPTR \
	src   = saved_src;   \
	dst_y = saved_dst_y; \
	dst_u = saved_dst_u; \
	dst_v = saved_dst_v;

#define MMSFB_CONV_ARGB_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	SRC = src;													\
	A = SRC >> 24;												\
																\
	if (A == 0xff) {											\
		if (SRC!=OLDSRC) {										\
			int r = (SRC >> 16) & 0xff;							\
			int g = (SRC >> 8) & 0xff;							\
			int b = SRC  & 0xff;								\
			old_y = MMSFB_CONV_RGB2Y(r,g,b);					\
			old_u = MMSFB_CONV_RGB2U(r,g,b);					\
			old_v = MMSFB_CONV_RGB2V(r,g,b);					\
			OLDSRC = SRC;										\
		}														\
		dst_y = old_y;											\
		d_u     old_u;											\
		d_v     old_v;											\
	}															\
	else														\
	if (!A) {													\
		dst_y = 0;												\
		d_u 128;												\
		d_v 128;												\
	}															\
	else														\
	{															\
		if (SRC!=OLDSRC) {										\
			int r = (SRC >> 16) & 0xff;							\
			int g = (SRC >> 8) & 0xff;							\
			int b = SRC  & 0xff;								\
			old_y = MMSFB_CONV_RGB2Y(r,g,b);					\
			old_u = MMSFB_CONV_RGB2U(r,g,b);					\
			old_v = MMSFB_CONV_RGB2V(r,g,b);					\
			MMSFB_CONV_PREPARE_YUVBLEND(old_y, old_u, old_v); 	\
			old_y = (A * old_y) >> 8;  							\
			old_u = (A * old_u) >> 8; 							\
			old_v = (A * old_v) >> 8; 							\
			MMSFB_CONV_RESET_YUVBLEND(old_y, old_u, old_v); 	\
			OLDSRC = SRC;										\
		}														\
																\
	    dst_y = old_y;											\
		d_u     old_u;											\
		d_v     old_v;											\
	}


#define MMSFB_CONV_YV12_TO_RGB32_PIXEL(src_y, src_u, src_v, dst) \
	{ int y, u, v, r, g, b;										\
	y = src_y; u = src_u; v = src_v;							\
	MMSFB_CONV_PREPARE_YUV2RGB(y,u,v);							\
	MMSFB_CONV_YUV2R(y, u, v, r);								\
	MMSFB_CONV_YUV2G(y, u, v, g);								\
	MMSFB_CONV_YUV2B(y, u, v, b);								\
	dst = 0xff000000 | (r << 16) | (g << 8) | b; }



#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PUSHPTR \
	unsigned int  *saved_src   = src;   \
	unsigned char *saved_dst_y = dst_y; \
	unsigned char *saved_dst_u = dst_u; \
	unsigned char *saved_dst_v = dst_v;

#define MMSFB_CONV_BLEND_ARGB_TO_YV12_POPPTR \
	src   = saved_src;   \
	dst_y = saved_dst_y; \
	dst_u = saved_dst_u; \
	dst_v = saved_dst_v;



#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	SRC = src;											\
	A = SRC >> 24;										\
														\
	if (A == 0xff) {									\
		if (SRC!=OLDSRC) {								\
			int r = (SRC >> 16) & 0xff;					\
			int g = (SRC >> 8) & 0xff;					\
			int b = SRC  & 0xff;						\
			old_y = MMSFB_CONV_RGB2Y(r,g,b);			\
			old_u = MMSFB_CONV_RGB2U(r,g,b);			\
			old_v = MMSFB_CONV_RGB2V(r,g,b);			\
			OLDSRC = SRC;								\
		}												\
		dst_y = old_y;									\
		d_u     old_u;									\
		d_v     old_v;									\
	}													\
	else												\
	if (!A) {											\
		d_u dst_u;										\
		d_v dst_v;										\
	}													\
	else												\
	{													\
		if (SRC!=OLDSRC) {								\
			int r = (SRC >> 16) & 0xff;					\
			int g = (SRC >> 8) & 0xff;					\
			int b = SRC  & 0xff;						\
			old_y = A * MMSFB_CONV_RGB2Y(r,g,b);		\
			old_u = A * MMSFB_CONV_RGB2U(r,g,b);		\
			old_v = A * MMSFB_CONV_RGB2V(r,g,b);		\
			OLDSRC = SRC;								\
		}												\
														\
		register unsigned int SA = 0x100 - A;			\
		unsigned int y = dst_y;							\
		unsigned int u = dst_u;							\
		unsigned int v = dst_v;							\
														\
	    y = SA * y + old_y;								\
	    u = SA * u + old_u;								\
	    v = SA * v + old_v;								\
	    												\
	    dst_y = y >> 8;									\
		d_u     u >> 8;									\
		d_v     v >> 8;									\
	}








#define MMSFB_CONV_YUY2_TO_YV12_PUSHPTR  \
	unsigned short int *saved_src = src; \
	unsigned char *saved_dst_y = dst_y;  \
	unsigned char *saved_dst_u = dst_u;  \
	unsigned char *saved_dst_v = dst_v;

#define MMSFB_CONV_YUY2_TO_YV12_POPPTR \
	src   = saved_src;   \
	dst_y = saved_dst_y; \
	dst_u = saved_dst_u; \
	dst_v = saved_dst_v;


#if __BYTE_ORDER ==  __BIG_ENDIAN
	// e.g. ARM
#define MMSFB_CONV_YUY2_TO_YV12_PIXEL(src, dst_y, d_uv) \
	SRC = src;											\
	dst_y = SRC >> 8;									\
	d_uv    SRC & 0xff;

#define MMSFB_CONV_YUY2_TO_YV12_PIXEL_2(src, dst_y, d_uv) \
	SRC = src;											  \
	dst_y = SRC >> 8;									  \
	d_uv    (SRC & 0xff) << 1;

#else
	// e.g. Intel
#define MMSFB_CONV_YUY2_TO_YV12_PIXEL(src, dst_y, d_uv) \
	SRC = src;											\
	dst_y = SRC & 0xff;									\
	d_uv    SRC >> 8;

#define MMSFB_CONV_YUY2_TO_YV12_PIXEL_2(src, dst_y, d_uv) \
	SRC = src;											  \
	dst_y = SRC & 0xff;									  \
	d_uv    (SRC >> 8) << 1;
#endif




/*
typedef int v4si __attribute__ ((vector_size (8)));
typedef long long unsigned int v4sia __attribute__ ((vector_size (8)));
typedef short int v4six __attribute__ ((vector_size (8)));
typedef signed char v4siy __attribute__ ((vector_size (8)));

typedef union
{
v4si v;
v4sia va;
v4six vv;
v4siy vvv;
short s[4];
int i[2];
unsigned char c[8];
} _v4si;
*/

#ifdef __HAVE_SSE__

typedef int v4si[2];
typedef short int v4six[4];
typedef signed char v4siy[8];

typedef union
{
v4si v;
v4six vv;
v4siy vvv;
short s[4];
int i[2];
unsigned char c[8];
} _v4si;

#endif

//#ifdef __SSE2__
//#ifdef __MMX__

/*AA = __builtin_ia32_psadbw(A, TTT); \
if (!AA) { \
*/
/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	if ((src->c[3]==0xff)&&(src->c[7]==0xff)) { \
		if ((src->i[0] != OLDSRC_MMX.i[0])&&(src->i[1] != OLDSRC_MMX.i[1])) { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			_v4si yy; \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(yy.vv, 8), YY); \
			old_y1 = yy.c[0]; \
			old_y2 = yy.c[4]; \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			old_u = __builtin_ia32_psadbw(ou, TTT); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			old_v = __builtin_ia32_psadbw(ov, TTT); \
			OLDSRC_MMX = *src;								\
		} \
		dst_y1 = old_y1; \
		dst_y2 = old_y2; \
		d_u old_u; \
		d_v old_v; \
	} \
	else \
	if ((!src->c[3])&&(!src->c[7])) { \
		d_u (dst_u << 1);										\
		d_v (dst_v << 1);										\
	} \
	else { \
		if ((src->i[0] != OLDSRC_MMX.i[0])&&(src->i[1] != OLDSRC_MMX.i[1])) { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			A  = __builtin_ia32_psrld(x2, 16); \
			oy = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			oy = __builtin_ia32_paddd(__builtin_ia32_psrld(oy, 8), YY); \
			oy = __builtin_ia32_pmullw(oy, A); \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			ou = __builtin_ia32_pmullw(ou, A); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			ov = __builtin_ia32_pmullw(ov, A); \
			A = __builtin_ia32_psubd(TTTT, A); \
			OLDSRC_MMX = *src;								\
		} \
		{ \
		_v4si yy; yy.i[0] = dst_y1; yy.i[1] = dst_y2; \
		_v4si uu; uu.i[0] = uu.i[1] = dst_u; \
		_v4si vv; vv.i[0] = vv.i[1] = dst_v; \
		\
		v4six qq; \
		qq = __builtin_ia32_pmullw(yy.vv, A); \
		qq = __builtin_ia32_paddw(qq, oy); \
		yy.vv = __builtin_ia32_psrlw(qq, 8); \
		dst_y1 = yy.c[0]; dst_y2 = yy.c[4]; \
		\
		qq = __builtin_ia32_pmullw(uu.vv, A); \
		qq = __builtin_ia32_paddw(qq, ou); \
		qq = __builtin_ia32_psrlw(qq, 8); \
		d_u  __builtin_ia32_psadbw(qq, TTT); \
		\
		qq = __builtin_ia32_pmullw(vv.vv, A); \
		qq = __builtin_ia32_paddw(qq, ov); \
		qq = __builtin_ia32_psrlw(qq, 8); \
		d_v  __builtin_ia32_psadbw(qq, TTT); \
		} \
	}

*/
//	     __asm__ __volatile__ ( "mov %%eax,%%ebx" : [x] "=m" (x));

/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	if ((src->c[3]==0xff)&&(src->c[7]==0xff)) { \
		if ((src->i[0] != OLDSRC_MMX.i[0])&&(src->i[1] != OLDSRC_MMX.i[1])) { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			_v4si yy; \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(yy.vv, 8), YY); \
			old_y1 = yy.c[0]; \
			old_y2 = yy.c[4]; \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			old_u = __builtin_ia32_psadbw(ou, TTT); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			old_v = __builtin_ia32_psadbw(ov, TTT); \
			OLDSRC_MMX = *src;								\
		} \
		dst_y1 = old_y1; \
		dst_y2 = old_y2; \
		d_u old_u; \
		d_v old_v; \
	} \
	else \
	if ((!src->c[3])&&(!src->c[7])) { \
		d_u (dst_u << 1);										\
		d_v (dst_v << 1);										\
	} \
	else { \
		if ((src->i[0] != OLDSRC_MMX.i[0])&&(src->i[1] != OLDSRC_MMX.i[1])) { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			A  = __builtin_ia32_psrld(x2, 16); \
			oy = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			oy = __builtin_ia32_paddd(__builtin_ia32_psrld(oy, 8), YY); \
			oy = __builtin_ia32_pmullw(oy, A); \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			ou = __builtin_ia32_pmullw(ou, A); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			ov = __builtin_ia32_pmullw(ov, A); \
			A = __builtin_ia32_psubd(TTTT, A); \
			OLDSRC_MMX = *src;								\
		} \
		{ \
		_v4si yy; yy.i[0] = dst_y1; yy.i[1] = dst_y2; \
		_v4si uu; uu.i[0] = uu.i[1] = dst_u; \
		_v4si vv; vv.i[0] = vv.i[1] = dst_v; \
		\
		v4six qq; \
		qq = __builtin_ia32_pmullw(yy.vv, A); \
		qq = __builtin_ia32_paddw(qq, oy); \
		yy.vv = __builtin_ia32_psrlw(qq, 8); \
		dst_y1 = yy.c[0]; dst_y2 = yy.c[4]; \
		\
		qq = __builtin_ia32_pmullw(uu.vv, A); \
		qq = __builtin_ia32_paddw(qq, ou); \
		qq = __builtin_ia32_psrlw(qq, 8); \
		d_u  __builtin_ia32_psadbw(qq, TTT); \
		\
		qq = __builtin_ia32_pmullw(vv.vv, A); \
		qq = __builtin_ia32_paddw(qq, ov); \
		qq = __builtin_ia32_psrlw(qq, 8); \
		d_v  __builtin_ia32_psadbw(qq, TTT); \
		} \
	}
*/
/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	if ((src->c[3]==0xff)&&(src->c[7]==0xff)) { \
		if ((src->i[0] != OLDSRC_MMX.i[0])&&(src->i[1] != OLDSRC_MMX.i[1])) { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			_v4si yy; \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(yy.vv, 8), YY); \
			old_y1 = yy.c[0]; \
			old_y2 = yy.c[4]; \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			old_u = __builtin_ia32_psadbw(ou, TTT); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			old_v = __builtin_ia32_psadbw(ov, TTT); \
			OLDSRC_MMX = *src;								\
		} \
		dst_y1 = old_y1; \
		dst_y2 = old_y2; \
		d_u old_u; \
		d_v old_v; \
	} \
	else \
	if ((!src->c[3])&&(!src->c[7])) { \
		d_u (dst_u << 1);										\
		d_v (dst_v << 1);										\
	} \
	else { \
			x1 = __builtin_ia32_pand(src->v, X1); \
			x2 = __builtin_ia32_psrlw(src->v, 8); \
			A  = __builtin_ia32_psrld(x2, 16); \
			oy = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			oy = __builtin_ia32_paddd(__builtin_ia32_psrld(oy, 8), YY); \
			oy = __builtin_ia32_pmullw(oy, A); \
			ou = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			ou = __builtin_ia32_paddd(__builtin_ia32_psrld(ou, 8), UV); \
			ou = __builtin_ia32_pmullw(ou, A); \
			ov = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			ov = __builtin_ia32_paddd(__builtin_ia32_psrld(ov, 8), UV); \
			ov = __builtin_ia32_pmullw(ov, A); \
			A = __builtin_ia32_psubd(TTTT, A); \
		{ \
		_v4si yy; yy.i[0] = dst_y1; yy.i[1] = dst_y2; \
		_v4si uu; uu.i[0] = dst_u; \
		_v4si vv; vv.i[0] = dst_v; \
		\
		yy.vv = __builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(yy.vv, A), oy), 8); \
		dst_y1 = yy.c[0]; dst_y2 = yy.c[4]; \
		\
		d_u  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(uu.vv, 68), A), ou), 8), TTT); \
		d_v  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(vv.vv, 68), A), ov), 8), TTT); \
		} \
	}
*/

/*

#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	if ((src->c[3]==0xff)&&(src->c[7]==0xff)) { \
		SRC = src->v; \
		if (SRC!=OLDSRC_MMX) {								\
			x1 = __builtin_ia32_pand(SRC, X1); \
			x2 = __builtin_ia32_psrlw(SRC, 8); \
			_v4si yy; \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
			yy.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(yy.vv, 8), YY); \
			old_y1 = yy.c[0]; \
			old_y2 = yy.c[4]; \
			u = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
			u = __builtin_ia32_paddd(__builtin_ia32_psrld(u, 8), UV); \
			old_u = __builtin_ia32_psadbw(u, TTT); \
			v = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
			v = __builtin_ia32_paddd(__builtin_ia32_psrld(v, 8), UV); \
			old_v = d_v __builtin_ia32_psadbw(v, TTT); \
			OLDSRC_MMX = SRC;								\
		} \
		dst_y1 = old_y1; \
		dst_y2 = old_y2; \
		d_u old_u; \
		d_v old_v; \
	} \
	else \
	if ((!src->c[3])&&(!src->c[7])) { \
		d_u (dst_u << 1);										\
		d_v (dst_v << 1);										\
	} \
	else { \
		SRC = src->v; \
		x1 = __builtin_ia32_pand(SRC, X1); \
		x2 = __builtin_ia32_psrlw(SRC, 8); \
		A  = __builtin_ia32_psrld(x2, 16); \
		y = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
		y = __builtin_ia32_paddd(__builtin_ia32_psrld(y, 8), YY); \
		y = __builtin_ia32_pmullw(y, A); \
		u = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
		u = __builtin_ia32_paddd(__builtin_ia32_psrld(u, 8), UV); \
		u = __builtin_ia32_pmullw(u, A); \
		v = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
		v = __builtin_ia32_paddd(__builtin_ia32_psrld(v, 8), UV); \
		v = __builtin_ia32_pmullw(v, A); \
		{ \
		_v4si yy; yy.i[0] = dst_y1; yy.i[1] = dst_y2; \
		_v4si uu; uu.i[0] = dst_u; \
		_v4si vv; vv.i[0] = dst_v; \
		\
		A = __builtin_ia32_psubd(TTTT, A); \
		yy.vv = __builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(yy.vv, A), y), 8); \
		dst_y1 = yy.c[0]; dst_y2 = yy.c[4]; \
		\
		d_u  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(uu.vv, 68), A), u), 8), TTT); \
		d_v  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(vv.vv, 68), A), v), 8), TTT); \
		} \
	}
*/

/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	SRC = src->v; \
	x1 = __builtin_ia32_pand(SRC, X1); \
	x2 = __builtin_ia32_psrlw(SRC, 8); \
	A  = __builtin_ia32_psrld(x2, 16); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(y.vv, 8), YY); \
	y.vv = __builtin_ia32_pmullw(y.vv, A); \
	u = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
	u = __builtin_ia32_paddd(__builtin_ia32_psrld(u, 8), UV); \
	u = __builtin_ia32_pmullw(u, A); \
	v = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
	v = __builtin_ia32_paddd(__builtin_ia32_psrld(v, 8), UV); \
	v = __builtin_ia32_pmullw(v, A); \
	{ \
	_v4si yy; \
	yy.i[0] = dst_y1; \
	yy.i[1] = dst_y2; \
	_v4si uu; \
	uu.i[0] = dst_u; \
	_v4si vv; \
	vv.i[0] = dst_v; \
	\
	A = __builtin_ia32_psubd(TTTT, A); \
	yy.vv = __builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(yy.vv, A), y.vv), 8); \
	dst_y1 = yy.c[0]; \
	dst_y2 = yy.c[4]; \
	\
	d_u  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(uu.vv, 68), A), u), 8), TTT); \
	d_v  __builtin_ia32_psadbw(__builtin_ia32_psrlw(__builtin_ia32_paddw(__builtin_ia32_pmullw(__builtin_ia32_pshufw(vv.vv, 68), A), v), 8), TTT); \
    }
*/


/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	SRC = src->v; \
	x1 = __builtin_ia32_pand(SRC, X1); \
	x2 = __builtin_ia32_psrlw(SRC, 8); \
	A.vv = __builtin_ia32_psrld(x2, 16); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(y.vv, 8), YY); \
	y.vv = __builtin_ia32_pmullw(y.vv, A.vv); \
	u = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
	u = __builtin_ia32_paddd(__builtin_ia32_psrld(u, 8), UV); \
	u = __builtin_ia32_pmullw(u, A.vv); \
	v = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
	v = __builtin_ia32_paddd(__builtin_ia32_psrld(v, 8), UV); \
	v = __builtin_ia32_pmullw(v, A.vv); \
	{ \
	_v4si yy; \
	yy.i[0] = dst_y1; \
	yy.i[1] = dst_y2; \
	_v4si uu; \
	uu.i[0] = dst_u; \
	uu.i[1] = dst_u; \
	_v4si vv; \
	vv.i[0] = dst_v; \
	vv.i[1] = dst_v; \
	_v4si SA; \
	SA.vv = __builtin_ia32_psubd(TTTT, A.vv); \
	yy.vv = __builtin_ia32_pmullw(yy.vv, SA.vv); \
	uu.vv = __builtin_ia32_pmullw(uu.vv, SA.vv); \
	vv.vv = __builtin_ia32_pmullw(vv.vv, SA.vv); \
	yy.vv = __builtin_ia32_paddw(yy.vv, y.vv); \
	uu.vv = __builtin_ia32_paddw(uu.vv, u); \
	vv.vv = __builtin_ia32_paddw(vv.vv, v); \
	yy.vv = __builtin_ia32_psrlw(yy.vv, 8); \
	uu.vv = __builtin_ia32_psrlw(uu.vv, 8); \
	vv.vv = __builtin_ia32_psrlw(vv.vv, 8); \
	dst_y1 = yy.c[0]; \
	dst_y2 = yy.c[4]; \
	d_u  __builtin_ia32_psadbw(uu.vv, TTT); \
	d_v  __builtin_ia32_psadbw(vv.vv, TTT); \
    }
*/

/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
	SRC = src->v; \
	x1 = __builtin_ia32_pand(SRC, X1); \
	x2 = __builtin_ia32_psrlw(SRC, 8); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_psrld(y.vv, 8), YY); \
	dst_y1 = y.c[0]; \
	dst_y2 = y.c[4]; \
	uv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
	uv = __builtin_ia32_paddd(__builtin_ia32_psrld(uv, 8), UV); \
	d_u __builtin_ia32_psadbw(uv, TTT); \
	uv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
	uv = __builtin_ia32_paddd(__builtin_ia32_psrld(uv, 8), UV); \
	d_v __builtin_ia32_psadbw(uv, TTT);

 */


/*
#define MMSFB_CONV_BLEND_ARGB_TO_YV12_PIXEL_MMX(src, dst_y1, dst_y2, dst_u, dst_v, d_u, d_v) \
 { \
	register v4si t = src->v; \
	register v4six x1, x2; \
	x1 = __builtin_ia32_pand(t, X1); \
	x2 = __builtin_ia32_psrlw(t, 8); \
	_v4si y; \
	y.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, Y_RBRB), __builtin_ia32_pmaddwd(x2, Y_GG)); \
	y.vv = __builtin_ia32_pand(__builtin_ia32_paddd(__builtin_ia32_psrld(y.vv, 8), YY), X1); \
	dst_y1 = y.c[0]; \
	dst_y2 = y.c[4]; \
	_v4si u; \
	u.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, U_RBRB), __builtin_ia32_pmaddwd(x2, U_GG)); \
	u.vv = __builtin_ia32_pand(__builtin_ia32_paddd(__builtin_ia32_psrld(u.vv, 8), UV), X1); \
	d_u (u.s[0] + u.s[2]); \
	_v4si v; \
	v.vv = __builtin_ia32_paddd(__builtin_ia32_pmaddwd(x1, V_RBRB), __builtin_ia32_pmaddwd(x2, V_GG)); \
	v.vv = __builtin_ia32_pand(__builtin_ia32_paddd(__builtin_ia32_psrld(v.vv, 8), UV), X1); \
	d_v (v.s[0] + v.s[2]); }
*/

/*

 \
	SRC = src;											\
	A = SRC >> 24;										\
														\
	if (A == 0xff) {									\
		if (SRC!=OLDSRC) {								\
			int r = (SRC >> 16) & 0xff;					\
			int g = (SRC >> 8) & 0xff;					\
			int b = SRC  & 0xff;						\
			old_y = MMSFB_CONV_RGB2Y(r,g,b);			\
			old_u = MMSFB_CONV_RGB2U(r,g,b);			\
			old_v = MMSFB_CONV_RGB2V(r,g,b);			\
			OLDSRC = SRC;								\
		}												\
		dst_y = old_y;									\
		d_u     old_u;									\
		d_v     old_v;									\
	}													\
	else												\
	if (!A) {											\
		d_u dst_u;										\
		d_v dst_v;										\
	}													\
	else												\
	{													\
		if (SRC!=OLDSRC) {								\
			int r = (SRC >> 16) & 0xff;					\
			int g = (SRC >> 8) & 0xff;					\
			int b = SRC  & 0xff;						\
			old_y = A * MMSFB_CONV_RGB2Y(r,g,b);		\
			old_u = A * MMSFB_CONV_RGB2U(r,g,b);		\
			old_v = A * MMSFB_CONV_RGB2V(r,g,b);		\
			OLDSRC = SRC;								\
		}												\
														\
		register unsigned int SA = 0x100 - A;			\
		unsigned int y = dst_y;							\
		unsigned int u = dst_u;							\
		unsigned int v = dst_v;							\
														\
	    y = SA * y + old_y;								\
	    u = SA * u + old_u;								\
	    v = SA * v + old_v;								\
	    												\
	    dst_y = y >> 8;									\
		d_u     u >> 8;									\
		d_v     v >> 8;									\
	}
*/


#define MMSFB_CONV_BLEND_COLORALPHA_ARGB_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	SRC = src;											\
	A = SRC >> 24;										\
														\
	if (!A) {											\
		d_u dst_u;										\
		d_v dst_v;										\
	}													\
	else												\
	{													\
		if (SRC!=OLDSRC) {								\
			int r = (ALPHA * (SRC & 0xff0000)) >> 24;	\
			int g = (ALPHA * (SRC & 0xff00)) >> 16;		\
			int b = (ALPHA * (SRC & 0xff)) >> 8;		\
			old_y = A * MMSFB_CONV_RGB2Y(r,g,b);		\
			A = (ALPHA * A) >> 8;						\
			old_u = A * MMSFB_CONV_RGB2U(r,g,b);		\
			old_v = A * MMSFB_CONV_RGB2V(r,g,b);		\
			OLDSRC = SRC;								\
		}												\
		else											\
			A = (ALPHA * A) >> 8;						\
														\
		register unsigned int SA = 0x100 - A;			\
		unsigned int y = dst_y;							\
		unsigned int u = dst_u;							\
		unsigned int v = dst_v;							\
														\
	    y = SA * y + old_y;								\
	    u = SA * u + old_u;								\
	    v = SA * v + old_v;								\
	    												\
	    dst_y = y >> 8;									\
		d_u     u >> 8;									\
		d_v     v >> 8;									\
	}






#define MMSFB_CONV_BLEND_AYUV_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	SRC = src;											\
	A = SRC >> 24;										\
														\
	if (A == 0xff) {									\
		if (SRC!=OLDSRC) {								\
			old_y = (SRC >> 16) & 0xff;					\
			old_u = (SRC >> 8) & 0xff;					\
			old_v = SRC  & 0xff;						\
			OLDSRC = SRC;								\
		}												\
		dst_y = old_y;									\
		d_u     old_u;									\
		d_v     old_v;									\
	}													\
	else												\
	if (!A) {											\
		d_u dst_u;										\
		d_v dst_v;										\
	}													\
	else												\
	{													\
		if (SRC!=OLDSRC) {								\
			old_y = A * ((SRC >> 16) & 0xff);			\
			old_u = A * ((SRC >> 8) & 0xff);			\
			old_v = A * (SRC  & 0xff);					\
			OLDSRC = SRC;								\
		}												\
														\
		register unsigned int SA = 0x100 - A;			\
		unsigned int y = dst_y;							\
		unsigned int u = dst_u;							\
		unsigned int v = dst_v;							\
														\
	    y = SA * y + old_y;								\
	    u = SA * u + old_u;								\
	    v = SA * v + old_v;								\
	    												\
	    dst_y = y >> 8;									\
		d_u     u >> 8;									\
		d_v     v >> 8;									\
	}


#define MMSFB_CONV_BLEND_COLORALPHA_AYUV_TO_YV12_PIXEL(src, dst_y, dst_u, dst_v, d_u, d_v) \
	SRC = src;											\
	A = SRC >> 24;										\
														\
	if (!A) {											\
		d_u dst_u;										\
		d_v dst_v;										\
	}													\
	else												\
	{													\
		if (SRC!=OLDSRC) {								\
			int sy = (SRC << 8) >> 24;					\
			int su = (SRC << 16) >> 24;					\
			int sv = SRC & 0xff;						\
			MMSFB_CONV_PREPARE_YUVBLEND(sy,su,sv);	\
			sy = (ALPHA * sy) >> 8;						\
			su = (ALPHA * su) >> 8;						\
			sv = (ALPHA * sv) >> 8;						\
			MMSFB_CONV_RESET_YUVBLEND(sy,su,sv);		\
			old_y = A * sy;								\
			A = (ALPHA * A) >> 8;						\
			old_u = A * su;								\
			old_v = A * sv;								\
			OLDSRC = SRC;								\
		}												\
		else 											\
			A = (ALPHA * A) >> 8;						\
														\
		register unsigned int SA = 0x100 - A;			\
		unsigned int y = dst_y;							\
		unsigned int u = dst_u;							\
		unsigned int v = dst_v;							\
														\
	    y = SA * y + old_y;								\
	    u = SA * u + old_u;								\
	    v = SA * v + old_v;								\
	    												\
	    dst_y = y >> 8;									\
		d_u     u >> 8;									\
		d_v     v >> 8;									\
	}





//! bresenham algorithm
#define MMSFB_DRAWLINE_BRESENHAM(putpixel) { \
	int x = x1; int y = y1; \
	int dx = x2 - x1; int dy = y2 - y1; \
	int ix = (dx > 0) ? 1 : (dx < 0) ? -1 : 0; int iy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0; \
	if (!dx && !dy) { putpixel; } else { \
	if (dx < 0) dx = -dx; if (dy < 0) dy = -dy; \
	int pdx, pdy, ddx, ddy, es, el; \
	if (dx > dy) { pdx=ix; pdy=0; ddx=ix; ddy=iy; es=dy; el=dx; } else { pdx=0;  pdy=iy; ddx=ix; ddy=iy; es=dx; el=dy; } \
	int err = el >> 1; putpixel; \
	for(int i = 0; i < el; ++i) { err-=es; if (err < 0) { err+=el; x+=ddx; y+=ddy; } else { x+=pdx; y+=pdy; } putpixel; } } }

//! put pixel macro will be used e.g. as parameter for the bresenham algorithm
#define MMSFB_DRAWLINE_PUT_PIXEL \
	if ((x >= clipreg.x1)&&(x <= clipreg.x2)&&(y >= clipreg.y1)&&(y <= clipreg.y2)) \
		dst[x+y*dst_pitch_pix]=SRC;


//! used for text output
#define MMSFBSURFACE_BLIT_TEXT_INIT(pw) \
	int FH = 0;   font->getHeight(&FH); \
	int desc = 0; font->getDescender(&desc); \
	int DY = FH - (desc + 1); \
	int dst_pitch_pix = dst_pitch >> pw;

//! used for text output
#define MMSFBSURFACE_BLIT_TEXT_LOAD_GLYPH(font, character) \
	int			  src_pitch_pix; \
	int 		  src_w; \
	int 		  src_h; \
	unsigned char *src; \
	MMSFBFont_Glyph glyph; \
	bool glyph_loaded = font->getGlyph(character, &glyph); \
	if (glyph_loaded) { \
		src_pitch_pix = glyph.pitch; \
		src_w         = glyph.width; \
		src_h         = glyph.height; \
		src           = glyph.buffer; \
	}

//! used for text output
#define MMSFBSURFACE_BLIT_TEXT_START_RENDER(pt) \
	if (glyph_loaded) { \
		int dx, dy; \
		if (!MMSFBBase_rotate180) { \
			dx = x + glyph.left; \
			dy = y + DY - glyph.top; \
		} else { \
			dx = x - glyph.left - glyph.width + 1; \
			dy = y - (DY - glyph.top) - glyph.height + 1; \
		} \
		if (dx < clipreg.x1) { \
			src_w -= clipreg.x1 - dx; \
			src   += clipreg.x1 - dx; \
			dx     = clipreg.x1; } \
		if (dx + src_w - 1 > clipreg.x2) src_w = clipreg.x2 - dx + 1; \
		if (dy < clipreg.y1) { \
			src_h -= clipreg.y1 - dy; \
			src   +=(clipreg.y1 - dy) * src_pitch_pix; \
			dy     = clipreg.y1; } \
		if (dy + src_h - 1 > clipreg.y2) src_h = clipreg.y2 - dy + 1; \
		unsigned char *src_end = src + src_h * src_pitch_pix; \
		unsigned char *line_end = src + src_w; \
		int src_pitch_pix_diff = src_pitch_pix - src_w; \
		int dst_pitch_pix_diff = dst_pitch_pix - src_w; \
		pt *dst = ((pt *)dst_ptr) + dx + dy * dst_pitch_pix;

//! used for text output
#define MMSFBSURFACE_BLIT_TEXT_END_RENDER \
	if (!MMSFBBase_rotate180) \
		x+=glyph.advanceX >> 6; \
	else \
		x-=glyph.advanceX >> 6;	}


//! calculate region if screen is rotated by 180°
#define MMSFB_ROTATE_180_REGION(SURFACE, X1, Y1, X2, Y2) \
	if (MMSFBBase_rotate180) { \
		int tmp; \
		tmp = X2; \
		X2 = ((!(SURFACE)->root_parent)?(SURFACE)->config.w:(SURFACE)->root_parent->config.w) - X1 - 1; \
		X1 = ((!(SURFACE)->root_parent)?(SURFACE)->config.w:(SURFACE)->root_parent->config.w) - tmp - 1; \
		tmp = Y2; \
		Y2 = ((!(SURFACE)->root_parent)?(SURFACE)->config.h:(SURFACE)->root_parent->config.h) - Y1 - 1; \
		Y1 = ((!(SURFACE)->root_parent)?(SURFACE)->config.h:(SURFACE)->root_parent->config.h) - tmp - 1; \
	}

//! calculate rectangle if screen is rotated by 180°
#define MMSFB_ROTATE_180_RECT(SURFACE, X, Y, W, H) \
	if (MMSFBBase_rotate180) { \
		X = ((!(SURFACE)->root_parent)?(SURFACE)->config.w:(SURFACE)->root_parent->config.w) - X - (W); \
		Y = ((!(SURFACE)->root_parent)?(SURFACE)->config.h:(SURFACE)->root_parent->config.h) - Y - (H); \
	}

//! calculate rectangle if screen is rotated by 180°
#define MMSFB_ROTATE_180_RECT_WH(WIDTH, HEIGHT, X, Y, W, H) \
	if (MMSFBBase_rotate180) { \
		X = WIDTH - X - (W); \
		Y = HEIGHT - Y - (H); \
	}




//! Stretching the source byte buffer to a destination.
/*!
\note src and dst have to point to the first pixel which is to process
\author Jens Schneider
*/
void stretch_byte_buffer(bool h_antialiasing, bool v_antialiasing,
						 unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
					     unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh);

//! Compressing a 2x2 matrix (arithmetic mean), e.g. used for U/V components (YV12 pixel format).
/*!
\note src and dst have to point to the first pixel which is to process
\author Jens Schneider
*/
void compress_2x2_matrix(unsigned char *src, int src_pitch, int src_pitch_pix, int src_height, int sw, int sh,
						 unsigned char *dst, int dst_pitch, int dst_pitch_pix, int dst_height, int dw, int dh);



//! Stretching the source unsigned int buffer to a destination.
/*!
\author Jens Schneider
*/
void stretch_uint_buffer(bool h_antialiasing, bool v_antialiasing,
						 unsigned int *src, int src_pitch, int src_pitch_pix, int src_height,
						 int sx, int sy, int sw, int sh,
						 unsigned int *dst, int dst_pitch, int dst_pitch_pix, int dst_height,
						 int dx, int dy, int dw, int dh);

//! Stretching the source unsigned short int buffer to a destination.
/*!
\author Jens Schneider
*/
void stretch_usint_buffer(bool h_antialiasing, bool v_antialiasing,
						  unsigned short int *src, int src_pitch, int src_pitch_pix,
						  int src_height, int sx, int sy, int sw, int sh,
					      unsigned short int *dst, int dst_pitch, int dst_pitch_pix,
					      int dst_height, int dx, int dy, int dw, int dh);

//! Stretching the source 3-byte-buffer to a unsigned int destination.
/*!
\author Jens Schneider
*/
void stretch_324byte_buffer(bool h_antialiasing, bool v_antialiasing,
							unsigned char *src, int src_pitch, int src_pitch_pix,
							int src_height, int sx, int sy, int sw, int sh,
							unsigned int *dst, int dst_pitch, int dst_pitch_pix,
							int dst_height, int dx, int dy, int dw, int dh);

//! Blitting unsigned int (4 byte) source to unsigned int destination.
/*!
\author Jens Schneider
*/
void mmsfb_blit_uint(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
					 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

//! Blitting unsigned short int (2 byte) source to unsigned short int destination.
/*!
\author Jens Schneider
*/
void mmsfb_blit_usint(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
					  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);



// --- BLITTING TO ARGB -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
								   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
											  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
											  unsigned char alpha);

//! Blit RGB16 to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb16_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

//! Blit RGB24 to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb24_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- BLITTING TO ARGB ---


// --- BLITTING TO RGB32 ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit RGB32 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb32_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit ARGB to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
											   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
											   unsigned char alpha);


//! Blit with alpha blending with alpha from color ARGB to RGB32, ignoring alpha channel from source.
/*!
\author Jens Schneider
*/
void mmsfb_blit_coloralpha_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
											   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
											   unsigned char alpha);

//! Blit with alpha from color RGB32 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_coloralpha_rgb32_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
										  unsigned char alpha);

//! Blit RGB16 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb16_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

//! Blit RGB24 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb24_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit YV12 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_yv12_to_rgb32(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							  unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB4444 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb4444_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB4444 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb4444_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
												   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
												   unsigned char alpha);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ BLITTING TO RGB32 ---


// --- BLITTING TO AiRGB ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_airgb_to_airgb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_airgb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								  	 unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_airgb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
												unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy,
												unsigned char alpha);


//! Blit with alpha blending ARGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								    unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ BLITTING TO AiRGB ---



// --- BLITTING TO RGB24 ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit RGB24 to RGB24.
/*!
\note RGB24 byte order: blue@0, green@1, red@2
\author Jens Schneider
*/
void mmsfb_blit_rgb24_to_rgb24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to RGB24.
/*!
\note RGB24 byte order: blue@0, green@1, red@2
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_rgb24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

//! Blit with alpha blending ARGB to RGB24.
/*!
\note RGB24 byte order: blue@0, green@1, red@2
\author Jens Schneider
*/
void mmsfb_blit_argb_to_rgb24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ BLITTING TO RGB24 ---


// --- BLITTING TO BGR24 ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit BGR24 to BGR24.
/*!
\note BGR24 byte order: red@0, green@1, blue@2
\author Jens Schneider
*/
void mmsfb_blit_bgr24_to_bgr24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to BGR24.
/*!
\note BGR24 byte order: red@0, green@1, blue@2
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_bgr24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB to BGR24.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb_to_bgr24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
											   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
											   unsigned char alpha);


//! Blit with alpha from color BGR24 to BGR24.
/*!
\author Jens Schneider
*/
void mmsfb_blit_coloralpha_bgr24_to_bgr24(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
										  unsigned char alpha);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ BLITTING TO BGR24 ---


// --- BLITTING TO RGB16 ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit RGB16 to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb16_to_rgb16(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit ARGB to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb_to_rgb16(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							  unsigned short int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_rgb16(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit AiRGB to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_airgb_to_rgb16(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							   unsigned short int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending AiRGB to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_airgb_to_rgb16(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
									 unsigned short int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit AYUV to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_ayuv_to_rgb16(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							  unsigned short int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending AYUV to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_ayuv_to_rgb16(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								    unsigned short int *dst, int dst_pitch, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ BLITTING TO RGB16 ---


// --- BLITTING TO AYUV -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_blit_ayuv_to_ayuv(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_ayuv_to_ayuv(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								   unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_ayuv_to_ayuv(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
											  unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy,
											  unsigned char alpha);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- BLITTING TO AYUV ---


// --- BLITTING TO YV12 -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit YV12 to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_yv12_to_yv12(MMSFBSurfacePlanes *extbuf, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit I420 to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_i420_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit YUY2 to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_yuy2_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit RGB24 to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_rgb24_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							  unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit ARGB to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
							 unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
								   unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
											  unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy,
											  unsigned char alpha);


//! Blit with alpha blending AYUV to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_ayuv_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
				 			 	   unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color AYUV to YV12.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_ayuv_to_yv12(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
											  unsigned char *dst, int dst_pitch, int dst_height, int dx, int dy,
											  unsigned char alpha);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- BLITTING TO YV12 ---


// --- BLITTING TO I420 -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit I420 to I420.
/*!
\author Jens Schneider
*/
void mmsfb_blit_i420_to_i420(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- BLITTING TO I420 ---


// --- BLITTING TO YUY2 -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit YUY2 to YUY2.
/*!
\author Jens Schneider
*/
void mmsfb_blit_yuy2_to_yuy2(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- BLITTING TO YUY2 ---



// --- BLITTING TO ARGB3565 ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit ARGB3565 to ARGB3565.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb3565_to_argb3565(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit ARGB to ARGB3565.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb_to_argb3565(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
								 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB to ARGB3565.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_argb3565(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// --------------------------------------------------- BLITTING TO ARGB3565 ---


// --- BLITTING TO ARGB4444 ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit ARGB4444 to ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_blit_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
								     MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending ARGB4444 to ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										   MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);


//! Blit with alpha blending with alpha from color ARGB4444 to ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_coloralpha_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
													  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy,
													  unsigned char alpha);

// ----------------------------------------------------------------------------
// --------------------------------------------------- BLITTING TO ARGB4444 ---


// --- BLITTING TO BGR555 -----------------------------------------------------
// ----------------------------------------------------------------------------

//! Blit BGR555 to BGR555.
/*!
\author Jens Schneider
*/
void mmsfb_blit_bgr555_to_bgr555(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
							     MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

//! Blit with alpha blending ARGB to BGR555.
/*!
\author Jens Schneider
*/
void mmsfb_blit_blend_argb_to_bgr555(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy);

// ----------------------------------------------------------------------------
// ----------------------------------------------------- BLITTING TO BGR555 ---



// --- STRETCH TO ARGB --------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing);


//! Stretch blit with alpha blending ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh);


//! Stretch blit with alpha blending with alpha from color ARGB to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_coloralpha_argb_to_argb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
													 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
													 unsigned char alpha);


//! Stretch blit RGB24 to ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_rgb24_to_argb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
									 unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh,
									 bool antialiasing);

// ----------------------------------------------------------------------------
// -------------------------------------------------------- STRETCH TO ARGB ---


// --- STRETCH TO RGB32 -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit with alpha blending ARGB to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_argb_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
										  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh);

//! Stretch blit RGB32 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_rgb32_to_rgb32(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									  bool antialiasing);

//! Stretch blit RGB24 to RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_rgb24_to_rgb32(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
									  unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh,
									  bool antialiasing);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- STRETCH TO RGB32 ---


// --- STRETCH TO AiRGB -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_airgb_to_airgb(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									  bool antialiasing);


//! Stretch blit with alpha blending AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_airgb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
										    unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh);


//! Stretch blit with alpha blending with alpha from color AiRGB to AiRGB.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_coloralpha_airgb_to_airgb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
													   unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh,
													   unsigned char alpha);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- STRETCH TO AiRGB ---


// --- STRETCH TO AYUV --------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_ayuv_to_ayuv(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing);


//! Stretch blit with alpha blending AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_ayuv_to_ayuv(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
										  unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh);


//! Stretch blit with alpha blending with alpha from color AYUV to AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_coloralpha_ayuv_to_ayuv(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
												     unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh,
												     unsigned char alpha);

// ----------------------------------------------------------------------------
// -------------------------------------------------------- STRETCH TO AYUV ---


// --- STRETCH TO YV12 --------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit YV12 to YV12 with antialiasing.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_yv12_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing);


//! Stretch blit I420 to YV12 with antialiasing.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_i420_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing);

//! Stretch blit YUY2 to YV12 with antialiasing.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_yuy2_to_yv12(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									bool antialiasing);

// ----------------------------------------------------------------------------
// -------------------------------------------------------- STRETCH TO YV12 ---


// --- STRETCH TO ARGB4444 ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit with alpha blending ARGB4444 to ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
												  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh);


//! Stretch blit with alpha blending with alpha from color ARGB4444 to ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_blend_coloralpha_argb4444_to_argb4444(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
															 MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
															 unsigned char alpha);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- STRETCH TO ARGB4444 ---


// --- STRETCH TO RGB16 -------------------------------------------------------
// ----------------------------------------------------------------------------

//! Stretch blit RGB16 to RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_stretchblit_rgb16_to_rgb16(MMSFBSurfacePlanes *src_planes, int src_height, int sx, int sy, int sw, int sh,
									  MMSFBSurfacePlanes *dst_planes, int dst_height, int dx, int dy, int dw, int dh,
									  bool antialiasing);

// ----------------------------------------------------------------------------
// ------------------------------------------------------- STRETCH TO RGB16 ---


// --- FILL ARGB RECTANGLE ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_argb(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color);


//! Fill rectangle with alpha blending ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_blend_argb(MMSFBSurfacePlanes *dst_planes, int dst_height,
									int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- FILL ARGB RECTANGLE ---


// --- FILL RGB32 RECTANGLE ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle RGB32.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_rgb32(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// --------------------------------------------------- FILL RGB32 RECTANGLE ---


// --- FILL RGB24 RECTANGLE ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle RGB24.
/*!
\note RGB24 byte order: blue@0, green@1, red@2
\author Jens Schneider
*/
void mmsfb_fillrectangle_rgb24(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// --------------------------------------------------- FILL RGB24 RECTANGLE ---


// --- FILL RGB16 RECTANGLE ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_rgb16(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       int dx, int dy, int dw, int dh, MMSFBColor color);

//! Fill rectangle with alpha blending RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_blend_rgb16(MMSFBSurfacePlanes *dst_planes, int dst_height,
									 int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// --------------------------------------------------- FILL RGB16 RECTANGLE ---


// --- FILL AYUV RECTANGLE ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_ayuv(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color);


//! Fill rectangle with alpha blending AYUV.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_blend_ayuv(MMSFBSurfacePlanes *dst_planes, int dst_height,
									int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- FILL AYUV RECTANGLE ---


// --- FILL YV12 RECTANGLE ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle YV12.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_yv12(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- FILL YV12 RECTANGLE ---


// --- FILL I420 RECTANGLE ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle I420.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_i420(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- FILL I420 RECTANGLE ---


// --- FILL YUY2 RECTANGLE ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle YUY2.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_yuy2(MMSFBSurfacePlanes *dst_planes, int dst_height,
						      int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- FILL YUY2 RECTANGLE ---


// --- FILL ARGB3565 RECTANGLE ------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle ARGB3565.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_argb3565(MMSFBSurfacePlanes *dst_planes, int dst_height,
						          int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ------------------------------------------------ FILL ARGB3565 RECTANGLE ---


// --- FILL ARGB4444 RECTANGLE ------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle ARGB3565.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_argb4444(MMSFBSurfacePlanes *dst_planes, int dst_height,
						          int dx, int dy, int dw, int dh, MMSFBColor color);


//! Fill rectangle with alpha blending ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_blend_argb4444(MMSFBSurfacePlanes *dst_planes, int dst_height,
										int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// ------------------------------------------------ FILL ARGB4444 RECTANGLE ---


// --- FILL BGR24 RECTANGLE ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle BGR24.
/*!
\note BGR24 byte order: red@0, green@1, blue@2
\author Jens Schneider
*/
void mmsfb_fillrectangle_bgr24(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// --------------------------------------------------- FILL BGR24 RECTANGLE ---


// --- FILL BGR555 RECTANGLE --------------------------------------------------
// ----------------------------------------------------------------------------

//! Fill rectangle BGR555.
/*!
\author Jens Schneider
*/
void mmsfb_fillrectangle_bgr555(MMSFBSurfacePlanes *dst_planes, int dst_height,
						        int dx, int dy, int dw, int dh, MMSFBColor color);

// ----------------------------------------------------------------------------
// -------------------------------------------------- FILL BGR555 RECTANGLE ---


// --- DRAW LINE TO ARGB ------------------------------------------------------
// ----------------------------------------------------------------------------

//! Draw line ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_drawline_argb(MMSFBSurfacePlanes *dst_planes, int dst_height,
						 MMSFBRegion &clipreg, int x1, int y1, int x2, int y2, MMSFBColor &color);


//! Draw line with alpha blending ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_drawline_blend_argb(MMSFBSurfacePlanes *dst_planes, int dst_height,
						       MMSFBRegion &clipreg, int x1, int y1, int x2, int y2, MMSFBColor &color);

// ----------------------------------------------------------------------------
// ------------------------------------------------------ DRAW LINE TO ARGB ---


// --- DRAW LINE TO ARGB4444 --------------------------------------------------
// ----------------------------------------------------------------------------

//! Draw line ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_drawline_argb4444(MMSFBSurfacePlanes *dst_planes, int dst_height,
							 MMSFBRegion &clipreg, int x1, int y1, int x2, int y2, MMSFBColor &color);

// ----------------------------------------------------------------------------
// -------------------------------------------------- DRAW LINE TO ARGB4444 ---


// --- DRAW STRING TO ARGB ----------------------------------------------------
// ----------------------------------------------------------------------------

//! Draw string with alpha blending ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_drawstring_blend_argb(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
								 MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color);


//! Draw string with alpha blending with alpha from color ARGB.
/*!
\author Jens Schneider
*/
void mmsfb_drawstring_blend_coloralpha_argb(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
											MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color);

// ----------------------------------------------------------------------------
// ---------------------------------------------------- DRAW STRING TO ARGB ---


// --- DRAW STRING TO ARGB4444 ------------------------------------------------
// ----------------------------------------------------------------------------

//! Draw string with alpha blending ARGB4444.
/*!
\author Jens Schneider
*/
void mmsfb_drawstring_blend_argb4444(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
									 MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color);

// ----------------------------------------------------------------------------
// ------------------------------------------------ DRAW STRING TO ARGB4444 ---


// --- DRAW STRING TO RGB16 ---------------------------------------------------
// ----------------------------------------------------------------------------

//! Draw string with alpha blending RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_drawstring_blend_rgb16(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
								  MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color);

//! Draw string with alpha blending with alpha from color RGB16.
/*!
\author Jens Schneider
*/
void mmsfb_drawstring_blend_coloralpha_rgb16(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
											MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color);

// ----------------------------------------------------------------------------
// --------------------------------------------------- DRAW STRING TO RGB16 ---


#endif /* MMSFBCONV_H_ */
