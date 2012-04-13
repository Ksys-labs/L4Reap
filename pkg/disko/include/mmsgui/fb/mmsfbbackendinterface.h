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

#ifndef MMSFBBACKENDINTERFACE_H_
#define MMSFBBACKENDINTERFACE_H_

#include "mmstools/mmsthreadserver.h"
#include "mmsgui/fb/mmsfbsurface.h"
#include "mmsgui/fb/mmsfbgl.h"

#ifdef __HAVE_OPENGL__

#define __HAVE_MMS3D__

#endif

class MMSFBBackEndInterface : public MMSThreadServer {
private:
	typedef enum {
		BEI_REQUEST_TYPE_INIT = 0,
		BEI_REQUEST_TYPE_SWAP,
		BEI_REQUEST_TYPE_ALLOC,
		BEI_REQUEST_TYPE_FREE,
		BEI_REQUEST_TYPE_CLEAR,
		BEI_REQUEST_TYPE_FILLRECTANGLE,
		BEI_REQUEST_TYPE_FILLTRIANGLE,
		BEI_REQUEST_TYPE_DRAWLINE,
		BEI_REQUEST_TYPE_DRAWRECTANGLE,
		BEI_REQUEST_TYPE_DRAWTRIANGLE,
		BEI_REQUEST_TYPE_BLIT,
		BEI_REQUEST_TYPE_STRETCHBLIT,
		BEI_REQUEST_TYPE_STRETCHBLITBUFFER,
		BEI_REQUEST_TYPE_CREATEALPHATEXTURE,
		BEI_REQUEST_TYPE_DELETETEXTURE,
		BEI_REQUEST_TYPE_DRAWSTRING,
		BEI_REQUEST_TYPE_RENDERSCENE,
		BEI_REQUEST_TYPE_MERGE
	} BEI_REQUEST_TYPE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
#ifdef  __HAVE_XLIB__
		Display 			*x_display;
		int					x_screen;
		Window				x_window;
		MMSFBRectangle		x11_win_rect;
#endif
	} BEI_INIT;

	typedef struct {
		BEI_REQUEST_TYPE	type;
	} BEI_SWAP;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
	} BEI_ALLOC;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
	} BEI_FREE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBColor			color;
	} BEI_CLEAR;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBRectangle		rect;
		MMSFBDrawingFlags	drawingflags;
	} BEI_FILLRECTANGLE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBTriangle		triangle;
	} BEI_FILLTRIANGLE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBRegion			region;
	} BEI_DRAWLINE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBRectangle		rect;
	} BEI_DRAWRECTANGLE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBTriangle		triangle;
	} BEI_DRAWTRIANGLE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBSurface		*source;
		MMSFBRectangle		src_rect;
		int					x;
		int					y;
		MMSFBBlittingFlags	blittingflags;
	} BEI_BLIT;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBSurface		*source;
		MMSFBRectangle		src_rect;
		MMSFBRectangle		dst_rect;
		MMSFBBlittingFlags	blittingflags;
	} BEI_STRETCHBLIT;

	typedef struct {
		BEI_REQUEST_TYPE		type;
		MMSFBSurface			*surface;
		MMSFBSurfacePlanes		*src_planes;
		MMSFBSurfacePixelFormat	src_pixelformat;
		int						src_width;
		int						src_height;
		MMSFBRectangle			src_rect;
		MMSFBRectangle			dst_rect;
		MMSFBBlittingFlags		blittingflags;
	} BEI_STRETCHBLITBUFFER;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		unsigned int		*texture;
		unsigned char		*buffer;
		int					width;
		int					height;
	} BEI_CREATEALPHATEXTURE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		unsigned int		texture;
	} BEI_DELETETEXTURE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		string				text;
		int					len;
		int					x;
		int					y;
	} BEI_DRAWSTRING;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMS3D_VERTEX_ARRAY	**varrays;
		MMS3D_INDEX_ARRAY	**iarrays;
		MMS3D_MATERIAL		*materials;
		MMSFBSurface		**texsurfaces;
		MMS3D_OBJECT		**objects;
	} BEI_RENDERSCENE;

	typedef struct {
		BEI_REQUEST_TYPE	type;
		MMSFBSurface		*surface;
		MMSFBSurface		*source1;
		MMSFBSurface		*source2;
		MMSFBMergingMode	mergingmode;
	} BEI_MERGE;

#ifdef __HAVE_OPENGL__

	//! access to the OpenGL wrapper class
	MMSFBGL	mmsfbgl;

	//! indicate that the matrix should be reset next time used
	bool	reset_matrix;

	//! if true the central projection will be used, else parallel projection is used
	bool	matrix_central_projection;

	//! specify the coordinate for the left vertical clipping plane
	int		matrix_left;

	//! specify the coordinate for the right vertical clipping plane
	int		matrix_right;

	//! specify the coordinate for the bottom horizontal clipping plane
	int		matrix_bottom;

	//! specify the coordinate for the top horizontal clipping plane
	int		matrix_top;

	//! specify the distance to the nearer depth clipping plane
	int		matrix_nearZ;

	//! specify the distance to the farther depth clipping plane
	int		matrix_farZ;

	//! internal: init or update the current matrix
	void oglMatrix(bool central_projection, int left, int right, int bottom, int top, int nearZ = 0, int farZ = 1);

	//! internal: allocate a new MMSFBSurface which uses OpenGL memory
	void oglAlloc(MMSFBSurface *surface, bool rbo_required = false);

	//! internal: bind a MMSFBSurface and it's OpenGL handle(s) to the OpenGL context
	void oglBindSurface(MMSFBSurface *surface);

	//! internal: see oglBindSurface(MMSFBSurface *surface), additionally using nearer and farther depth in conjunction with central_projection flag
	void oglBindSurface(MMSFBSurface *surface, int nearZ, int farZ, bool central_projection = false);

#endif


	void processData(void *in_data, int in_data_len, void **out_data, int *out_data_len);
	void processInit(BEI_INIT *req);
	void processSwap(BEI_SWAP *req);
	void processAlloc(BEI_ALLOC *req);
	void processFree(BEI_FREE *req);
	void processClear(BEI_CLEAR *req);
	void processFillRectangle(BEI_FILLRECTANGLE *req);
	void processFillTriangle(BEI_FILLTRIANGLE *req);
	void processDrawLine(BEI_DRAWLINE *req);
	void processDrawRectangle(BEI_DRAWRECTANGLE *req);
	void processDrawTriangle(BEI_DRAWTRIANGLE *req);
	void processBlit(BEI_BLIT *req);
	void processStretchBlit(BEI_STRETCHBLIT *req);
	void processStretchBlitBuffer(BEI_STRETCHBLITBUFFER *req);
	void processCreateAlphaTexture(BEI_CREATEALPHATEXTURE *req);
	void processDeleteTexture(BEI_DELETETEXTURE *req);
	void processDrawString(BEI_DRAWSTRING *req);

	void processRenderScene(BEI_RENDERSCENE *req);
	void processMerge(BEI_MERGE *req);


public:

	MMSFBBackEndInterface(int queue_size = 1000);

#ifdef  __HAVE_XLIB__
	void init(Display *x_display, int x_screen, Window x_window, MMSFBRectangle x11_win_rect);
#else
	void init();
#endif

	void swap();
	void alloc(MMSFBSurface *surface);
	void free(MMSFBSurface *surface);
	void clear(MMSFBSurface *surface, MMSFBColor &color);
	void fillRectangle(MMSFBSurface *surface, MMSFBRectangle &rect, MMSFBDrawingFlags drawingflags);
	void fillTriangle(MMSFBSurface *surface, MMSFBTriangle &triangle);
	void drawLine(MMSFBSurface *surface, MMSFBRegion &region);
	void drawRectangle(MMSFBSurface *surface, MMSFBRectangle &rect);
	void drawTriangle(MMSFBSurface *surface, MMSFBTriangle &triangle);
	void blit(MMSFBSurface *surface, MMSFBSurface *source, MMSFBRectangle &src_rect, int x, int y, MMSFBBlittingFlags blittingflags);
	void stretchBlit(MMSFBSurface *surface, MMSFBSurface *source, MMSFBRectangle &src_rect, MMSFBRectangle &dst_rect, MMSFBBlittingFlags blittingflags);
	void blitBuffer(MMSFBSurface *surface, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
				    int src_width, int src_height, MMSFBRectangle &src_rect, int x, int y, MMSFBBlittingFlags blittingflags);
	void stretchBlitBuffer(MMSFBSurface *surface, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
						   int src_width, int src_height, MMSFBRectangle &src_rect, MMSFBRectangle &dst_rect, MMSFBBlittingFlags blittingflags);

	void createAlphaTexture(unsigned int *texture, unsigned char *buffer, int width, int height);
	void deleteTexture(unsigned int texture);
	void drawString(MMSFBSurface *surface, string &text, int len, int x, int y);

	void renderScene(MMSFBSurface *surface,
					 MMS3D_VERTEX_ARRAY	**varrays,
					 MMS3D_INDEX_ARRAY	**iarrays,
					 MMS3D_MATERIAL		*materials,
					 MMSFBSurface		**texsurfaces,
					 MMS3D_OBJECT		**objects);

	void merge(MMSFBSurface *surface, MMSFBSurface *source1, MMSFBSurface *source2, MMSFBMergingMode mergingmode);
};

#endif /* MMSFBBACKENDINTERFACE_H_ */
