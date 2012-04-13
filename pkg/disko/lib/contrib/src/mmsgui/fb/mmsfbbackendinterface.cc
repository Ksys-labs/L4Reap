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
 *   Lesser General Public Licen
 *   se for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#include "mmsgui/fb/mmsfbbackendinterface.h"
#include "mmstools/tools.h"

#define BEI_SOURCE_WIDTH	((!req->source->is_sub_surface)?req->source->config.w:req->source->root_parent->config.w)
#define BEI_SOURCE_HEIGHT	((!req->source->is_sub_surface)?req->source->config.h:req->source->root_parent->config.h)

#define BEI_SURFACE_WIDTH		((!req->surface->is_sub_surface)?req->surface->config.w:req->surface->root_parent->config.w)
#define BEI_SURFACE_HEIGHT		((!req->surface->is_sub_surface)?req->surface->config.h:req->surface->root_parent->config.h)
#define BEI_SURFACE_BOTTOM		(BEI_SURFACE_HEIGHT - 1)
#define BEI_SURFACE_BOTTOM_F	(float)BEI_SURFACE_BOTTOM


#define GET_OFFS(surface) \
		int xoff = 0; int yoff = 0; \
		if (surface->is_sub_surface) { \
			xoff = surface->sub_surface_xoff; \
			yoff = surface->sub_surface_yoff; }

#define GET_OFFS_SRC(surface) \
		int src_xoff = 0; int src_yoff = 0; \
		if (surface->is_sub_surface) { \
			src_xoff = surface->sub_surface_xoff; \
			src_yoff = surface->sub_surface_yoff; }





#ifdef __HAVE_OPENGL__


#define OGL_SCISSOR(surface, X, Y, W, H) \
	if (surface->config.surface_buffer->ogl_fbo) mmsfbgl.setScissor(X, Y, W, H); \
	else mmsfbgl.setScissor(X, BEI_SURFACE_HEIGHT - (H) - (Y), W, H);


#define INIT_OGL_DRAWING(surface, drawingflags) { \
		switch (drawingflags) { \
		case MMSFB_DRAW_BLEND: \
			mmsfbgl.enableBlend(); \
			mmsfbgl.setDrawingMode(); \
			break; \
		default: \
			mmsfbgl.disableBlend(); \
			mmsfbgl.setDrawingMode(); \
			break; } \
		mmsfbgl.setColor(surface->config.color.r, surface->config.color.g, surface->config.color.b, surface->config.color.a); }



#define INIT_OGL_BLITTING(surface, blittingflags) \
		switch (blittingflags) { \
		case MMSFB_BLIT_BLEND_ALPHACHANNEL:	\
			mmsfbgl.enableBlend(); \
			mmsfbgl.setTexEnvReplace(GL_RGBA); \
			break; \
		case MMSFB_BLIT_BLEND_COLORALPHA: \
			mmsfbgl.disableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(255, 255, 255, surface->config.color.a); \
			break; \
		case MMSFB_BLIT_BLEND_ALPHACHANNEL + MMSFB_BLIT_BLEND_COLORALPHA: \
			mmsfbgl.enableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(255, 255, 255, surface->config.color.a); \
			break; \
		case MMSFB_BLIT_COLORIZE: \
			mmsfbgl.disableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(surface->config.color.r, surface->config.color.g, surface->config.color.b, 0xff); \
			break; \
		case MMSFB_BLIT_BLEND_ALPHACHANNEL + MMSFB_BLIT_COLORIZE: \
			mmsfbgl.enableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(surface->config.color.r, surface->config.color.g, surface->config.color.b, 0xff); \
			break; \
		case MMSFB_BLIT_BLEND_COLORALPHA + MMSFB_BLIT_COLORIZE: \
			mmsfbgl.disableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(surface->config.color.r, surface->config.color.g, surface->config.color.b, surface->config.color.a); \
			break; \
		case MMSFB_BLIT_BLEND_ALPHACHANNEL + MMSFB_BLIT_BLEND_COLORALPHA + MMSFB_BLIT_COLORIZE: \
			mmsfbgl.enableBlend(); \
			mmsfbgl.setTexEnvModulate(GL_RGBA); \
			mmsfbgl.setColor(surface->config.color.r, surface->config.color.g, surface->config.color.b, surface->config.color.a); \
			break; \
		default: \
			mmsfbgl.disableBlend(); \
			mmsfbgl.setTexEnvReplace(GL_RGBA); \
			break; }


#define ENABLE_OGL_DEPTHTEST(surface, readonly) \
		if (!readonly && surface->config.surface_buffer) \
			if (!surface->is_sub_surface) \
				surface->config.surface_buffer->ogl_unchanged_depth_buffer = false; \
			else \
				surface->root_parent->config.surface_buffer->ogl_unchanged_depth_buffer = surface->config.surface_buffer->ogl_unchanged_depth_buffer = false; \
		mmsfbgl.enableDepthTest(readonly);


#define DISABLE_OGL_DEPTHTEST(surface, mark_as_unchanged) \
		if (mark_as_unchanged && surface->config.surface_buffer) \
			if (!surface->is_sub_surface) \
				surface->config.surface_buffer->ogl_unchanged_depth_buffer = true; \
			else \
				surface->root_parent->config.surface_buffer->ogl_unchanged_depth_buffer = surface->config.surface_buffer->ogl_unchanged_depth_buffer = true; \
		mmsfbgl.disableDepthTest();


#define IS_OGL_DEPTH_BUFFER_UNCHANGED(surface) \
		((!surface->is_sub_surface && surface->config.surface_buffer && surface->config.surface_buffer->ogl_unchanged_depth_buffer) \
		||(surface->is_sub_surface && surface->root_parent->config.surface_buffer && surface->root_parent->config.surface_buffer->ogl_unchanged_depth_buffer))


#ifdef __HAVE_GL2__
#define OGL_DRAW_POINT(x, y) { glBegin(GL_POINTS); glVertex2f((float)(x) + 0.5, (float)(BEI_SURFACE_BOTTOM - (y)) + 0.5); glEnd(); }
#endif

#ifdef __HAVE_GLES2__
#define OGL_DRAW_POINT(x, y) {}
#endif

#define OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) \
		if (x1 == x2 && y1 == y2) OGL_DRAW_POINT(x1, y1) else

//TODO: am dreieck nochwas???
#define OGL_SINGLE_POINT_FALLBACK2(x1, y1, x2, y2, x3, y3) \
		if (x1 == x2 && x1 == x3 && y1 == y2 && y1 == y3) OGL_DRAW_POINT(x1, y1) else


#define OGL_CALC_COORD(v1, v2) (((v1)<(v2)) ? (float)(v1) : (float)(v1) + 0.99)

#define OGL_CALC_2X_N(v1, v2, width)	(OGL_CALC_COORD(v1, v2) / (width))
#define OGL_CALC_2Y_N(v1, v2, height)	(OGL_CALC_COORD(v1, v2) / (height))


#define OGL_CALC_3X(v1, v2, v3) (((v1)<=(v2) && (v1)<=(v3)) ? (float)(v1) : ((v1)>(v2) && (v1)>(v3)) ? (float)(v1) + 0.99 : (float)(v1) + 0.5)
#define OGL_CALC_3Y(v1, v2, v3) (((v1)<=(v2) && (v1)<=(v3)) ? BEI_SURFACE_BOTTOM_F - (float)(v1) + 0.99 : ((v1)>(v2) && (v1)>(v3)) ? BEI_SURFACE_BOTTOM_F - (float)(v1) : BEI_SURFACE_BOTTOM_F - (float)(v1) + 0.5)



#define OGL_FILL_RECTANGLE(x1, y1, x2, y2) \
		OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) \
		mmsfbgl.fillRectangle2Di(x1, y1, x2, y2);


#define OGL_DRAW_RECTANGLE(x1, y1, x2, y2) \
		OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) \
		mmsfbgl.drawRectangle2Di(x1, y1, x2, y2);

#endif


#ifdef __HAVE_GL2__



/*
#define OGL_DRAW_POINT(x, y) { glBegin(GL_POINTS); glVertex2f((float)(x) + 0.5, (float)(BEI_SURFACE_BOTTOM - (y)) + 0.5); glEnd(); }

#define OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) \
		if (x1 == x2 && y1 == y2) OGL_DRAW_POINT(x1, y1) else

//TODO: am dreieck nochwas???
#define OGL_SINGLE_POINT_FALLBACK2(x1, y1, x2, y2, x3, y3) \
		if (x1 == x2 && x1 == x3 && y1 == y2 && y1 == y3) OGL_DRAW_POINT(x1, y1) else



#define OGL_CALC_2X(v1, v2) (((v1)<(v2)) ? (float)(v1) : (float)(v1) + 0.99)
#define OGL_CALC_2Y(v1, v2) OGL_CALC_2Y_H(v1, v2, BEI_SURFACE_HEIGHT)
#define OGL_CALC_2Y_H(v1, v2, height) (((v1)<(v2)) ? (float)(height-1) - (float)(v1) + 0.99 : (float)(height-1) - (float)(v1))

#define OGL_CALC_3X(v1, v2, v3) (((v1)<=(v2) && (v1)<=(v3)) ? (float)(v1) : ((v1)>(v2) && (v1)>(v3)) ? (float)(v1) + 0.99 : (float)(v1) + 0.5)
#define OGL_CALC_3Y(v1, v2, v3) (((v1)<=(v2) && (v1)<=(v3)) ? BEI_SURFACE_BOTTOM_F - (float)(v1) + 0.99 : ((v1)>(v2) && (v1)>(v3)) ? BEI_SURFACE_BOTTOM_F - (float)(v1) : BEI_SURFACE_BOTTOM_F - (float)(v1) + 0.5)

#define OGL_CALC_2X_N(v1, v2, width)	(OGL_CALC_2X(v1, v2) / (width))
#define OGL_CALC_2Y_N(v1, v2, height)	(OGL_CALC_2Y_H(v1, v2, height) / (height))

#define OGL_FILL_RECTANGLE(x1, y1, x2, y2) \
		OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) \
		glRectf(OGL_CALC_2X(x1, x2), OGL_CALC_2Y(y1, y2), OGL_CALC_2X(x2, x1), OGL_CALC_2Y(y2, y1));
*/

#define OGL_DRAW_LINE(x1, y1, x2, y2) \
		OGL_SINGLE_POINT_FALLBACK(x1, y1, x2, y2) { \
		glBegin(GL_LINES); \
			glVertex2f(OGL_CALC_COORD(x1, x2), OGL_CALC_COORD(y1, y2)); \
			glVertex2f(OGL_CALC_COORD(x2, x1), OGL_CALC_COORD(y2, y1)); \
		glEnd(); }


#define OGL_FILL_TRIANGLE(x1, y1, x2, y2, x3, y3) \
		OGL_SINGLE_POINT_FALLBACK2(x1, y1, x2, y2, x3, y3) { \
		glBegin(GL_TRIANGLES); \
			glVertex2f(OGL_CALC_3X(x1, x2, x3), OGL_CALC_3Y(y1, y2, y3)); \
			glVertex2f(OGL_CALC_3X(x2, x3, x1), OGL_CALC_3Y(y2, y3, y1)); \
			glVertex2f(OGL_CALC_3X(x3, x1, x2), OGL_CALC_3Y(y3, y1, y2)); \
		glEnd(); }

#define OGL_DRAW_TRIANGLE(x1, y1, x2, y2, x3, y3) \
		OGL_SINGLE_POINT_FALLBACK2(x1, y1, x2, y2, x3, y3) { \
		glBegin(GL_LINE_STRIP); \
			glVertex2f(OGL_CALC_3X(x1, x2, x3), OGL_CALC_3Y(y1, y2, y3)); \
			glVertex2f(OGL_CALC_3X(x2, x3, x1), OGL_CALC_3Y(y2, y3, y1)); \
			glVertex2f(OGL_CALC_3X(x3, x1, x2), OGL_CALC_3Y(y3, y1, y2)); \
			glVertex2f(OGL_CALC_3X(x1, x2, x3), OGL_CALC_3Y(y1, y2, y3)); \
		glEnd(); }


#endif

MMSFBBackEndInterface::MMSFBBackEndInterface(int queue_size) : MMSThreadServer(queue_size, "MMSFBBackEndInterface") {

}

void MMSFBBackEndInterface::processData(void *in_data, int in_data_len, void **out_data, int *out_data_len) {
	if (!in_data) return;

	BEI_REQUEST_TYPE *type = (BEI_REQUEST_TYPE *)in_data;

	switch (*type) {
	case BEI_REQUEST_TYPE_INIT:
		processInit((BEI_INIT *)in_data);
		break;
	case BEI_REQUEST_TYPE_SWAP:
		processSwap((BEI_SWAP *)in_data);
		break;
	case BEI_REQUEST_TYPE_ALLOC:
		processAlloc((BEI_ALLOC *)in_data);
		break;
	case BEI_REQUEST_TYPE_FREE:
		processFree((BEI_FREE *)in_data);
		break;
	case BEI_REQUEST_TYPE_CLEAR:
		processClear((BEI_CLEAR *)in_data);
		break;
	case BEI_REQUEST_TYPE_FILLRECTANGLE:
		processFillRectangle((BEI_FILLRECTANGLE *)in_data);
		break;
	case BEI_REQUEST_TYPE_FILLTRIANGLE:
		processFillTriangle((BEI_FILLTRIANGLE *)in_data);
		break;
	case BEI_REQUEST_TYPE_DRAWLINE:
		processDrawLine((BEI_DRAWLINE *)in_data);
		break;
	case BEI_REQUEST_TYPE_DRAWRECTANGLE:
		processDrawRectangle((BEI_DRAWRECTANGLE *)in_data);
		break;
	case BEI_REQUEST_TYPE_DRAWTRIANGLE:
		processDrawTriangle((BEI_DRAWTRIANGLE *)in_data);
		break;
	case BEI_REQUEST_TYPE_BLIT:
		processBlit((BEI_BLIT *)in_data);
		break;
	case BEI_REQUEST_TYPE_STRETCHBLIT:
		processStretchBlit((BEI_STRETCHBLIT *)in_data);
		break;
	case BEI_REQUEST_TYPE_STRETCHBLITBUFFER:
		processStretchBlitBuffer((BEI_STRETCHBLITBUFFER *)in_data);
		break;
	case BEI_REQUEST_TYPE_CREATEALPHATEXTURE:
		processCreateAlphaTexture((BEI_CREATEALPHATEXTURE *)in_data);
		break;
	case BEI_REQUEST_TYPE_DELETETEXTURE:
		processDeleteTexture((BEI_DELETETEXTURE *)in_data);
		break;
	case BEI_REQUEST_TYPE_DRAWSTRING:
		processDrawString((BEI_DRAWSTRING *)in_data);
		break;
	case BEI_REQUEST_TYPE_RENDERSCENE:
		processRenderScene((BEI_RENDERSCENE *)in_data);
		break;
	case BEI_REQUEST_TYPE_MERGE:
		processMerge((BEI_MERGE *)in_data);
		break;
	default:
		break;
	}
}

#ifdef  __HAVE_XLIB__

void MMSFBBackEndInterface::init(Display *x_display, int x_screen, Window x_window, MMSFBRectangle x11_win_rect) {
	// start the server thread
	start();

	// trigger the init request
	BEI_INIT req;
	req.type		= BEI_REQUEST_TYPE_INIT;
	req.x_display	= x_display;
	req.x_screen	= x_screen;
	req.x_window	= x_window;
	req.x11_win_rect= x11_win_rect;
	trigger((void*)&req, sizeof(req));
}

#else

void MMSFBBackEndInterface::init() {
	// start the server thread
	start();

	// trigger the init request
	BEI_INIT req;
	req.type = BEI_REQUEST_TYPE_INIT;
	trigger((void*)&req, sizeof(req));
}

#endif


void MMSFBBackEndInterface::processInit(BEI_INIT *req) {
#ifdef __HAVE_OPENGL__

#ifdef __HAVE_XLIB__

	mmsfbgl.init(req->x_display, req->x_screen, req->x_window, req->x11_win_rect.w, req->x11_win_rect.h);

#else

	mmsfbgl.init();

#endif

	// set the coordinate system, here we use the parallel projection as default
	this->reset_matrix = true;
	int w, h;
	mmsfbgl.getResolution(&w, &h);
	oglMatrix(false, 0, w, h, 0);

#endif
}


#ifdef __HAVE_OPENGL__
void MMSFBBackEndInterface::oglMatrix(bool central_projection, int left, int right, int bottom, int top, int nearZ, int farZ) {
	if (this->reset_matrix || (central_projection != this->matrix_central_projection)
	  || (left != this->matrix_left) || (right != this->matrix_right)
	  || (bottom != this->matrix_bottom) || (top != this->matrix_top)
	  || (nearZ != this->matrix_nearZ) || (farZ != this->matrix_farZ)) {
		this->reset_matrix = false;
		this->matrix_central_projection = central_projection;
		this->matrix_left = left;
		this->matrix_right = right;
		this->matrix_bottom = bottom;
		this->matrix_top = top;
		this->matrix_nearZ = nearZ;
		this->matrix_farZ = farZ;
		if (!central_projection) {
			// init parallel projection
			mmsfbgl.setParallelProjection(left, right, bottom, top, nearZ, farZ);
		}
		else {
			// init central projection
			mmsfbgl.setCentralProjection(left, right, bottom, top, nearZ, farZ);
		}
	}
}




void MMSFBBackEndInterface::oglAlloc(MMSFBSurface *surface, bool rbo_required) {

	if (surface->is_sub_surface) {
		// surface is a subsurface without own memory
		// so set surface to root parents surface for allocation check
		surface = surface->root_parent;
	}

	MMSFBSurfaceBuffer *sb = surface->config.surface_buffer;

#ifdef __HAVE_GL2__
	if (!sb->ogl_fbo_initialized) {
		//TODO: GL2 needs always a renderbuffer???
		if (!sb->ogl_tex_initialized) {
			// texture is also not initialized
			mmsfbgl.allocFBOandRBO(sb->ogl_fbo, sb->ogl_tex, sb->ogl_rbo, surface->config.w, surface->config.h);
		}
		else {
			// texture is already initialized, so we can use it for FBO
			mmsfbgl.attachTexture2FrameBuffer(sb->ogl_fbo, sb->ogl_tex);
			mmsfbgl.attachRenderBuffer2FrameBuffer(sb->ogl_fbo, sb->ogl_rbo, surface->config.w, surface->config.h);
		}

		sb->ogl_fbo_initialized = true;
		sb->ogl_tex_initialized = true;
		sb->ogl_rbo_initialized = true;

	}
#endif

#ifdef __HAVE_GLES2__
	if (!sb->ogl_fbo_initialized) {
		// per default we do NOT attach a renderbuffer to the FBO (not needed for 2D, reduce memory foot print)
		sb->ogl_rbo_initialized = false;

		// allocate a texture (color buffer) and bind it to a new FBO
		if (!sb->ogl_tex_initialized) {
			// texture is also not initialized
			if (rbo_required) {
				// additionally have to initialize RBO
				// so, initialize FBO, RBO and texture
				mmsfbgl.allocFBOandRBO(sb->ogl_fbo, sb->ogl_tex, sb->ogl_rbo, surface->config.w, surface->config.h);
				sb->ogl_rbo_initialized = true;
			}
			else {
				// RBO not needed
				// so, initialize FBO and texture
				mmsfbgl.allocFBO(sb->ogl_fbo, sb->ogl_tex, surface->config.w, surface->config.h);
			}
		}
		else {
			// texture is already initialized, so we can use it for FBO
			mmsfbgl.attachTexture2FrameBuffer(sb->ogl_fbo, sb->ogl_tex);
		}

		sb->ogl_fbo_initialized = true;
		sb->ogl_tex_initialized = true;

	}
#endif

	// it can be, that the decision to have an RBO comes later
	// so it is possible to have a initialized FBO here without an RBO
	if (sb->ogl_fbo_initialized) {
		// FBO is already initialized
		if (!sb->ogl_rbo_initialized) {
			// RBO is not initialized
			if (rbo_required) {
				// additionally have to initialize RBO
				mmsfbgl.attachRenderBuffer2FrameBuffer(sb->ogl_fbo, sb->ogl_rbo, surface->config.w, surface->config.h);
				sb->ogl_rbo_initialized = true;
			}
		}
	}
}


void MMSFBBackEndInterface::oglBindSurface(MMSFBSurface *surface) {
	// allocate FBO, RBO and texture
	oglAlloc(surface);

	// bind FBO
	mmsfbgl.bindFrameBuffer(surface->config.surface_buffer->ogl_fbo);

	if (surface->config.surface_buffer->ogl_fbo) {
		// set the matrix for off-screen FBO's
		if (!surface->is_sub_surface) {
			oglMatrix(false, 0, surface->config.w, 0, surface->config.h);
		}
		else {
			oglMatrix(false, 0, surface->root_parent->config.w, 0, surface->root_parent->config.h);
		}
	} else {
		// set the matrix for primary FBO
		if (!surface->is_sub_surface) {
			oglMatrix(false, 0, surface->config.w, surface->config.h, 0);
		}
		else {
			oglMatrix(false, 0, surface->root_parent->config.w, surface->root_parent->config.h, 0);
		}
	}
}

void MMSFBBackEndInterface::oglBindSurface(MMSFBSurface *surface, int nearZ, int farZ, bool central_projection) {

	// allocate FBO, RBO and texture
	oglAlloc(surface, true);

	// bind FBO
	mmsfbgl.bindFrameBuffer(surface->config.surface_buffer->ogl_fbo);

	if (surface->config.surface_buffer->ogl_fbo) {
		// set the matrix for off-screen FBO's
		if (!surface->is_sub_surface) {
			oglMatrix(central_projection,
					  -(int)surface->config.w/2, (int)surface->config.w/2,
					  -(int)surface->config.h/2, (int)surface->config.h/2,
					  nearZ, farZ);
		}
		else {
			oglMatrix(central_projection,
					  -(int)surface->root_parent->config.w/2, (int)surface->root_parent->config.w/2,
					  -(int)surface->root_parent->config.h/2, (int)surface->root_parent->config.h/2,
					  nearZ, farZ);
		}
	} else {
		// set the matrix for primary FBO
		if (!surface->is_sub_surface) {
			oglMatrix(central_projection,
					  -(int)surface->config.w/2, (int)surface->config.w/2,
					   (int)surface->config.h/2,-(int)surface->config.h/2,
					   nearZ, farZ);
		}
		else {
			oglMatrix(central_projection,
					  -(int)surface->root_parent->config.w/2, (int)surface->root_parent->config.w/2,
					   (int)surface->root_parent->config.h/2,-(int)surface->root_parent->config.h/2,
					   nearZ, farZ);
		}
	}
}



#endif

void MMSFBBackEndInterface::swap() {
	BEI_SWAP req;
	req.type	= BEI_REQUEST_TYPE_SWAP;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processSwap(BEI_SWAP *req) {
#ifdef __HAVE_OPENGL__
	// swap screen
	mmsfbgl.swap();
#endif
}

void MMSFBBackEndInterface::alloc(MMSFBSurface *surface) {
	BEI_ALLOC req;
	req.type	= BEI_REQUEST_TYPE_ALLOC;
	req.surface	= surface;
	trigger((void*)&req, sizeof(req));
}


void MMSFBBackEndInterface::processAlloc(BEI_ALLOC *req) {
#ifdef  __HAVE_OPENGL__

	MMSFBSurfaceBuffer *sb = req->surface->config.surface_buffer;

	// generate id's for texture, fbo and rbo - but do NOT initialize it
	mmsfbgl.genTexture(&sb->ogl_tex);
	mmsfbgl.genFrameBuffer(&sb->ogl_fbo);
	mmsfbgl.genRenderBuffer(&sb->ogl_rbo);
	sb->ogl_fbo_initialized = false;
	sb->ogl_tex_initialized = false;
	sb->ogl_rbo_initialized = false;

#endif
}


void MMSFBBackEndInterface::free(MMSFBSurface *surface) {
	BEI_FREE req;
	req.type	= BEI_REQUEST_TYPE_FREE;
	req.surface	= surface;
	trigger((void*)&req, sizeof(req));
}


void MMSFBBackEndInterface::processFree(BEI_FREE *req) {
#ifdef  __HAVE_OPENGL__
	MMSFBSurfaceBuffer *sb = req->surface->config.surface_buffer;
	mmsfbgl.freeFBO(sb->ogl_fbo, sb->ogl_tex, sb->ogl_rbo);
#endif
}


void MMSFBBackEndInterface::clear(MMSFBSurface *surface, MMSFBColor &color) {
	BEI_CLEAR req;
	req.type	= BEI_REQUEST_TYPE_CLEAR;
	req.surface	= surface;
	req.color	= color;
	trigger((void*)&req, sizeof(req));
}


void MMSFBBackEndInterface::processClear(BEI_CLEAR *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	if (req->surface->calcClip(0 + xoff, 0 + yoff, req->surface->config.w, req->surface->config.h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

		mmsfbgl.clear(req->color.r, req->color.g, req->color.b, req->color.a);
	}
#endif
}

void MMSFBBackEndInterface::fillRectangle(MMSFBSurface *surface, MMSFBRectangle &rect, MMSFBDrawingFlags drawingflags) {
	BEI_FILLRECTANGLE req;
	req.type		= BEI_REQUEST_TYPE_FILLRECTANGLE;
	req.surface		= surface;
	req.rect		= rect;
	req.drawingflags= drawingflags;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processFillRectangle(BEI_FILLRECTANGLE *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);

	// setup drawing
	INIT_OGL_DRAWING(req->surface, req->drawingflags);

	// set ogl clip
	OGL_SCISSOR(req->surface, req->rect.x, req->rect.y, req->rect.w, req->rect.h);

	// fill rectangle
	OGL_FILL_RECTANGLE(req->rect.x,
					   req->rect.y,
					   req->rect.x + req->rect.w - 1,
					   req->rect.y + req->rect.h - 1);
#endif
}

void MMSFBBackEndInterface::fillTriangle(MMSFBSurface *surface, MMSFBTriangle &triangle) {
	BEI_FILLTRIANGLE req;
	req.type	= BEI_REQUEST_TYPE_FILLTRIANGLE;
	req.surface	= surface;
	req.triangle= triangle;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processFillTriangle(BEI_FILLTRIANGLE *req) {
#ifdef  __HAVE_GL2__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// setup drawing
	INIT_OGL_DRAWING(req->surface, req->surface->config.drawingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	int x, y, w, h;
	if (req->triangle.x2 >= req->triangle.x1) {
		x = req->triangle.x1;
		w = req->triangle.x2 - req->triangle.x1 + 1;
	}
	else {
		x = req->triangle.x2;
		w = req->triangle.x1 - req->triangle.x2 + 1;
	}
	if (req->triangle.y2 >= req->triangle.y1) {
		y = req->triangle.y1;
		h = req->triangle.y2 - req->triangle.y1 + 1;
	}
	else {
		y = req->triangle.y2;
		h = req->triangle.y1 - req->triangle.y2 + 1;
	}
	if (req->triangle.x3 < x) x = req->triangle.x3;
	if (req->triangle.x3 > x + w - 1) w = req->triangle.x3 - x + 1;
	if (req->triangle.y3 < y) y = req->triangle.y3;
	if (req->triangle.y3 > y + h - 1) h = req->triangle.y3 - y + 1;

	if (req->surface->calcClip(x + xoff, y + yoff, w, h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);
		glEnable(GL_SCISSOR_TEST);

		// fill triangle
		OGL_FILL_TRIANGLE(req->triangle.x1 + xoff, req->triangle.y1 + yoff,
						  req->triangle.x2 + xoff, req->triangle.y2 + yoff,
						  req->triangle.x3 + xoff, req->triangle.y3 + yoff);
	}
#endif
}

void MMSFBBackEndInterface::drawLine(MMSFBSurface *surface, MMSFBRegion &region) {
	BEI_DRAWLINE req;
	req.type	= BEI_REQUEST_TYPE_DRAWLINE;
	req.surface	= surface;
	req.region	= region;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processDrawLine(BEI_DRAWLINE *req) {
#ifdef  __HAVE_GL2__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

/*	printf("%08x, %dx%d, %d,%d,%d,%d DISKO: processDrawLine\n",
			req->surface,
			req->surface->config.color.r,req->surface->config.color.g,req->surface->config.color.b,req->surface->config.color.a);
*/
	// setup drawing
	INIT_OGL_DRAWING(req->surface, req->surface->config.drawingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	int x, y, w, h;
	if (req->region.x2 >= req->region.x1) {
		x = req->region.x1;
		w = req->region.x2 - req->region.x1 + 1;
	}
	else {
		x = req->region.x2;
		w = req->region.x1 - req->region.x2 + 1;
	}
	if (req->region.y2 >= req->region.y1) {
		y = req->region.y1;
		h = req->region.y2 - req->region.y1 + 1;
	}
	else {
		y = req->region.y2;
		h = req->region.y1 - req->region.y2 + 1;
	}

	if (req->surface->calcClip(x + xoff, y + yoff, w, h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);
		glEnable(GL_SCISSOR_TEST);

		// draw line
		OGL_DRAW_LINE(req->region.x1 + xoff,
					  req->region.y1 + yoff,
					  req->region.x2 + xoff,
					  req->region.y2 + yoff);
	}
#endif
}

void MMSFBBackEndInterface::drawRectangle(MMSFBSurface *surface, MMSFBRectangle &rect) {
	BEI_DRAWRECTANGLE req;
	req.type	= BEI_REQUEST_TYPE_DRAWRECTANGLE;
	req.surface	= surface;
	req.rect	= rect;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processDrawRectangle(BEI_DRAWRECTANGLE *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);

	// setup drawing
	INIT_OGL_DRAWING(req->surface, req->surface->config.drawingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	if (req->surface->calcClip(req->rect.x + xoff, req->rect.y + yoff, req->rect.w, req->rect.h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

		// draw rectangle
		OGL_DRAW_RECTANGLE(req->rect.x						+ xoff,
						   req->rect.y						+ yoff,
						   req->rect.x + req->rect.w - 1	+ xoff,
						   req->rect.y + req->rect.h - 1	+ yoff);
	}
#endif
}

void MMSFBBackEndInterface::drawTriangle(MMSFBSurface *surface, MMSFBTriangle &triangle) {
	BEI_DRAWTRIANGLE req;
	req.type	= BEI_REQUEST_TYPE_DRAWTRIANGLE;
	req.surface	= surface;
	req.triangle= triangle;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processDrawTriangle(BEI_DRAWTRIANGLE *req) {
#ifdef  __HAVE_GL2__
	// lock destination fbo and prepare it
	oglBindSurface(req->surface);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// setup drawing
	INIT_OGL_DRAWING(req->surface, req->surface->config.drawingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	int x, y, w, h;
	if (req->triangle.x2 >= req->triangle.x1) {
		x = req->triangle.x1;
		w = req->triangle.x2 - req->triangle.x1 + 1;
	}
	else {
		x = req->triangle.x2;
		w = req->triangle.x1 - req->triangle.x2 + 1;
	}
	if (req->triangle.y2 >= req->triangle.y1) {
		y = req->triangle.y1;
		h = req->triangle.y2 - req->triangle.y1 + 1;
	}
	else {
		y = req->triangle.y2;
		h = req->triangle.y1 - req->triangle.y2 + 1;
	}
	if (req->triangle.x3 < x) x = req->triangle.x3;
	if (req->triangle.x3 > x + w - 1) w = req->triangle.x3 - x + 1;
	if (req->triangle.y3 < y) y = req->triangle.y3;
	if (req->triangle.y3 > y + h - 1) h = req->triangle.y3 - y + 1;

	if (req->surface->calcClip(x + xoff, y + yoff, w, h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);
		glEnable(GL_SCISSOR_TEST);

		// draw triangle
		OGL_DRAW_TRIANGLE(req->triangle.x1 + xoff, req->triangle.y1 + yoff,
						  req->triangle.x2 + xoff, req->triangle.y2 + yoff,
						  req->triangle.x3 + xoff, req->triangle.y3 + yoff);
	}
#endif
}

void MMSFBBackEndInterface::blit(MMSFBSurface *surface, MMSFBSurface *source, MMSFBRectangle &src_rect,
								 int x, int y, MMSFBBlittingFlags blittingflags) {
	BEI_BLIT req;
	req.type			= BEI_REQUEST_TYPE_BLIT;
	req.surface			= surface;
	req.source			= source;
	req.src_rect		= src_rect;
	req.x				= x;
	req.y				= y;
	req.blittingflags	= blittingflags;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processBlit(BEI_BLIT *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface);

	// setup blitting
	INIT_OGL_BLITTING(req->surface, req->blittingflags);

	// set ogl clip
	OGL_SCISSOR(req->surface, req->x, req->y, req->src_rect.w, req->src_rect.h);

	if (req->source->config.surface_buffer->ogl_tex_initialized) {
		// blit source texture to the destination
		mmsfbgl.stretchBliti(req->source->config.surface_buffer->ogl_tex,
					req->src_rect.x, req->src_rect.y, req->src_rect.x + req->src_rect.w - 1, req->src_rect.y + req->src_rect.h - 1,
					req->source->config.w, req->source->config.h,
					req->x, req->y, req->x + req->src_rect.w - 1, req->y + req->src_rect.h - 1);
	}
	else {
		printf("skip blitting from texture which is not initialized\n");
	}
#endif
}

void MMSFBBackEndInterface::stretchBlit(MMSFBSurface *surface, MMSFBSurface *source, MMSFBRectangle &src_rect,
										MMSFBRectangle &dst_rect, MMSFBBlittingFlags blittingflags) {
	BEI_STRETCHBLIT req;
	req.type			= BEI_REQUEST_TYPE_STRETCHBLIT;
	req.surface			= surface;
	req.source			= source;
	req.src_rect		= src_rect;
	req.dst_rect		= dst_rect;
	req.blittingflags	= blittingflags;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processStretchBlit(BEI_STRETCHBLIT *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface);

	// setup blitting
	INIT_OGL_BLITTING(req->surface, req->blittingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);
	GET_OFFS_SRC(req->source);

	// set the clip to ogl
	MMSFBRectangle crect;
	if (req->surface->calcClip(req->dst_rect.x + xoff, req->dst_rect.y + yoff, req->dst_rect.w, req->dst_rect.h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

		// get source region
		int sx1 = req->src_rect.x + src_xoff;
		int sy1 = req->src_rect.y + src_yoff;
		int sx2 = req->src_rect.x + req->src_rect.w - 1 + src_xoff;
		int sy2 = req->src_rect.y + req->src_rect.h - 1 + src_yoff;

		// get destination region
		int dx1 = req->dst_rect.x + xoff;
		int dy1 = req->dst_rect.y + yoff;
		int dx2 = req->dst_rect.x + req->dst_rect.w - 1 + xoff;
		int dy2 = req->dst_rect.y + req->dst_rect.h - 1 + yoff;

		if (req->source->config.surface_buffer->ogl_tex_initialized) {
			// blit source texture to the destination
			mmsfbgl.stretchBliti(req->source->config.surface_buffer->ogl_tex,
						sx1, sy1, sx2, sy2, req->source->config.w, req->source->config.h,
						dx1, dy1, dx2, dy2);
		}
		else {
			printf("skip blitting from texture which is not initialized\n");
		}
	}
#endif
}

void MMSFBBackEndInterface::blitBuffer(MMSFBSurface *surface, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									   int src_width, int src_height, MMSFBRectangle &src_rect, int x, int y,
									   MMSFBBlittingFlags blittingflags) {
	BEI_STRETCHBLITBUFFER req;
	req.type			= BEI_REQUEST_TYPE_STRETCHBLITBUFFER;
	req.surface			= surface;
	req.src_planes		= src_planes;
	req.src_pixelformat	= src_pixelformat;
	req.src_width		= src_width;
	req.src_height		= src_height;
	req.src_rect		= src_rect;
	req.dst_rect.x		= x;
	req.dst_rect.y		= y;
	req.dst_rect.w		= src_rect.w;
	req.dst_rect.h		= src_rect.h;
	req.blittingflags	= blittingflags;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::stretchBlitBuffer(MMSFBSurface *surface, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
											  int src_width, int src_height,MMSFBRectangle &src_rect, MMSFBRectangle &dst_rect,
											  MMSFBBlittingFlags blittingflags) {
	BEI_STRETCHBLITBUFFER req;
	req.type			= BEI_REQUEST_TYPE_STRETCHBLITBUFFER;
	req.surface			= surface;
	req.src_planes		= src_planes;
	req.src_pixelformat	= src_pixelformat;
	req.src_width		= src_width;
	req.src_height		= src_height;
	req.src_rect		= src_rect;
	req.dst_rect		= dst_rect;
	req.blittingflags	= blittingflags;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processStretchBlitBuffer(BEI_STRETCHBLITBUFFER *req) {
#ifdef  __HAVE_OPENGL__

	// check if we can blit to texture without fbo
	bool blit2texture = (!req->surface->config.surface_buffer->ogl_fbo_initialized);
	if (blit2texture) {
		if (req->blittingflags != MMSFB_BLIT_NOFX) {
			if (req->blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				if (!MMSFBSURFACE_WRITE_BUFFER(req->surface).opaque) {
					// alphachannel blending and opaque flag not set, so we cannot blit to texture directly
					// note: the opaque flag means, that surface should be opaque AFTER the blit
					blit2texture = false;
				}
			}
			else {
				// blitting flags does not allow to blit to texture directly
				blit2texture = false;
			}
		}
	}

	if (blit2texture) {
		// FBO and it's texture is not initialized and we only have to COPY the buffer in a texture
		mmsfbgl.blitBuffer2Texture(req->surface->config.surface_buffer->ogl_tex,
								   (!req->surface->config.surface_buffer->ogl_tex_initialized),
								   req->src_planes->ptr, req->src_width, req->src_height);

		// now we have a texture allocated
		req->surface->config.surface_buffer->ogl_tex_initialized = true;

		return;
	}

	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface);

	// setup blitting
	INIT_OGL_BLITTING(req->surface, req->blittingflags);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	if (req->surface->calcClip(req->dst_rect.x + xoff, req->dst_rect.y + yoff, req->dst_rect.w, req->dst_rect.h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

		// get source region
		int sx1 = req->src_rect.x;
		int sy1 = req->src_rect.y;
		int sx2 = req->src_rect.x + req->src_rect.w - 1;
		int sy2 = req->src_rect.y + req->src_rect.h - 1;

		// get destination region
		int dx1 = req->dst_rect.x + xoff;
		int dy1 = req->dst_rect.y + yoff;
		int dx2 = req->dst_rect.x + req->dst_rect.w - 1 + xoff;
		int dy2 = req->dst_rect.y + req->dst_rect.h - 1 + yoff;

		// blit source texture to the destination
		mmsfbgl.stretchBlitBufferi(req->src_planes->ptr,
					sx1, sy1, sx2, sy2, req->src_width, req->src_height,
					dx1, dy1, dx2, dy2);
	}
#endif
}



void MMSFBBackEndInterface::createAlphaTexture(unsigned int *texture, unsigned char *buffer, int width, int height) {
	BEI_CREATEALPHATEXTURE req;
	req.type	= BEI_REQUEST_TYPE_CREATEALPHATEXTURE;
	req.texture	= texture;
	req.buffer	= buffer;
	req.width	= width;
	req.height	= height;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processCreateAlphaTexture(BEI_CREATEALPHATEXTURE *req) {
#ifdef  __HAVE_OPENGL__

	// alloc a texture
	mmsfbgl.genTexture(req->texture);

	// specify two-dimensional glyph texture
	mmsfbgl.initTexture2D(*(req->texture), GL_ALPHA, req->buffer, GL_ALPHA, req->width, req->height);

#endif
}


void MMSFBBackEndInterface::deleteTexture(unsigned int texture) {
	BEI_DELETETEXTURE req;
	req.type	= BEI_REQUEST_TYPE_DELETETEXTURE;
	req.texture	= texture;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processDeleteTexture(BEI_DELETETEXTURE *req) {
#ifdef  __HAVE_OPENGL__

	// delete a texture
	mmsfbgl.deleteTexture(req->texture);

#endif
}


void MMSFBBackEndInterface::drawString(MMSFBSurface *surface, string &text, int len, int x, int y) {
	BEI_DRAWSTRING req;
	req.type	= BEI_REQUEST_TYPE_DRAWSTRING;
	req.surface	= surface;
	req.text	= text;
	req.len		= len;
	req.x		= x;
	req.y		= y;
	trigger((void*)&req, sizeof(req));
}

void MMSFBBackEndInterface::processDrawString(BEI_DRAWSTRING *req) {
#ifdef  __HAVE_OPENGL__

	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface);

	// setup blitting
	mmsfbgl.enableBlend();
	if (req->surface->config.color.a == 0xff)
		mmsfbgl.setTexEnvReplace(GL_ALPHA);
	else
		mmsfbgl.setTexEnvModulate(GL_ALPHA);
	mmsfbgl.setColor(req->surface->config.color.r,
					req->surface->config.color.g,
					req->surface->config.color.b,
					req->surface->config.color.a);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// get vertical font settings
	int DY = 0, desc = 0;
	req->surface->config.font->getHeight(&DY);
	req->surface->config.font->getDescender(&desc);
	DY -= desc + 1;

	// for all characters
	MMSFBFONT_GET_UNICODE_CHAR(req->text, req->len) {

		// load the glyph
		MMSFBSURFACE_BLIT_TEXT_LOAD_GLYPH(req->surface->config.font, character);

		// start rendering of glyph to destination
		if (glyph_loaded) {
			// calc destination position of the character
			int dx = req->x + glyph.left;
			int dy = req->y + DY - glyph.top;

			// set the clip to ogl
			MMSFBRectangle crect;
			if (req->surface->calcClip(dx + xoff, dy + yoff, src_w, src_h, &crect)) {
				// inside clipping region
				OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

				// get source region
				int sx1 = 0;
				int sy1 = 0;
				int sx2 = src_w - 1;
				int sy2 = src_h - 1;

				// get destination region
				int dx1 = dx + xoff;
				int dy1 = dy + yoff;
				int dx2 = dx + src_w - 1 + xoff;
				int dy2 = dy + src_h - 1 + yoff;

				// blit glyph texture to the destination
				mmsfbgl.stretchBliti(glyph.texture,
										sx1, sy1, sx2, sy2, src_pitch_pix, src_h,
										dx1, dy1, dx2, dy2);
			}

			// prepare for next loop
			req->x+=glyph.advanceX >> 6;
		}
	}}

#endif
}

void MMSFBBackEndInterface::renderScene(MMSFBSurface *surface,
										MMS3D_VERTEX_ARRAY	**varrays,
										MMS3D_INDEX_ARRAY	**iarrays,
										MMS3D_MATERIAL		*materials,
										MMSFBSurface		**texsurfaces,
										MMS3D_OBJECT		**objects) {
	BEI_RENDERSCENE req;
	req.type		= BEI_REQUEST_TYPE_RENDERSCENE;
	req.surface		= surface;
	req.varrays		= varrays;
	req.iarrays		= iarrays;
	req.materials	= materials;
	req.texsurfaces	= texsurfaces;
	req.objects		= objects;
	trigger((void*)&req, sizeof(req));
}




void setLight() {
#ifdef  __HAVE_GL2__

	glDisable(GL_LIGHT0);

	glEnable(GL_LIGHTING);

	GLfloat am[] = {0.4,0.4,0.4,1};
	glLightfv(GL_LIGHT0,GL_AMBIENT,am);

	GLfloat di[] = {0.5,0.5,0.5,1};
	glLightfv(GL_LIGHT0,GL_DIFFUSE,di);

	GLfloat sp[] = {0.3,0.3,0.3,1};
	glLightfv(GL_LIGHT0,GL_SPECULAR,sp);

	glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,0);

	GLfloat lightPos[] = {40000,10000,-100,1 };
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);


	glEnable(GL_LIGHT0);
#endif
}




void MMSFBBackEndInterface::processRenderScene(BEI_RENDERSCENE *req) {
#ifdef  __HAVE_OPENGL__

	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface, 200, 1700, true);
//	oglBindSurface(req->surface);

	// set light
/*	mmsfbgl.rotateCurrentMatrix(0, 1, 0, 0);
	mmsfbgl.rotateCurrentMatrix(-45, 0, 0, 1);
	mmsfbgl.rotateCurrentMatrix(90, 0, 1, 0);*/
	setLight();

//	oglBindSurface(req->surface, 200, 1700, true);

	// setup blitting
//	mmsfbgl.enableBlend();
	mmsfbgl.disableBlend();
	mmsfbgl.setTexEnvReplace(GL_RGBA);

	// get subsurface offsets
	GET_OFFS(req->surface);

	// set the clip to ogl
	MMSFBRectangle crect;
	if (req->surface->calcClip(0 + xoff, 0 + yoff, req->surface->config.w, req->surface->config.h, &crect)) {
		// inside clipping region
		OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

		// clear surface
		ENABLE_OGL_DEPTHTEST(req->surface, false);
		mmsfbgl.clear();

		// draw objects...
		int cnt = 0;
		MMS3D_OBJECT *object;

		while ((object = req->objects[cnt])) {

			if (!isMMS3DObjectShown(object) || object->indices < 0) {
				// skip not shown objects
				cnt++;
				continue;
			}

			// cull facing, do not draw hidden pixels
			if (object->cullface) {
#ifdef  __HAVE_GL2__
				glFrontFace(GL_CCW);
				glCullFace(GL_BACK);
				glEnable(GL_CULL_FACE);
#endif
			}
			else {
#ifdef  __HAVE_GL2__
				glDisable(GL_CULL_FACE);
#endif
			}

			// set matrix
			mmsfbgl.setCurrentMatrix(object->matrix);

			// set material
			if (object->material >= 0) {
#ifdef  __HAVE_GL2__

				MMS3D_MATERIAL material = req->materials[object->material];

				// material emits light
				glMaterialfv(GL_FRONT, GL_EMISSION, (float*)&material.s.emission);

				// material absorbs ambient light
				glMaterialfv(GL_FRONT, GL_AMBIENT, (float*)&material.s.ambient);

				// material absorbs diffuse light
				glMaterialfv(GL_FRONT, GL_DIFFUSE, (float*)&material.s.diffuse);

				// material absorbs specular light
				glMaterialfv(GL_FRONT, GL_SPECULAR, (float*)&material.s.specular);

				// specular-light exponent
				glMaterialf(GL_FRONT, GL_SHININESS, material.s.shininess * 128);
#endif
			}

			// bind object's texture
			if (object->texcoords >= 0 && object->texture >= 0) {
				mmsfbgl.enableTexture2D(req->texsurfaces[object->texture]->config.surface_buffer->ogl_tex);
			}
			else {
				mmsfbgl.disableTexture2D();
			}

			// draw object
			mmsfbgl.drawElements(
					(object->vertices >= 0) ? req->varrays[object->vertices] : NULL,
					(object->normals >= 0) ? req->varrays[object->normals] : NULL,
					(object->texcoords >= 0 && object->texture >= 0) ? req->varrays[object->texcoords] : NULL,
					(object->indices >= 0) ? req->iarrays[object->indices] : NULL);

			// next object
			cnt++;
		}

#ifdef  __HAVE_GL2__
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
#endif


		this->reset_matrix = true;

	}


#endif
}



void MMSFBBackEndInterface::merge(MMSFBSurface *surface, MMSFBSurface *source1, MMSFBSurface *source2,
									   MMSFBMergingMode mergingmode) {
	BEI_MERGE req;
	req.type		= BEI_REQUEST_TYPE_MERGE;
	req.surface		= surface;
	req.source1		= source1;
	req.source2		= source2;
	req.mergingmode	= mergingmode;
	trigger((void*)&req, sizeof(req));
}


void MMSFBBackEndInterface::processMerge(BEI_MERGE *req) {
#ifdef  __HAVE_OPENGL__
	// lock destination fbo and bind source texture to it
	oglBindSurface(req->surface);

	// get destination subsurface offset
	GET_OFFS(req->surface);

	// merging the two source surfaces
	for (int i = 0; i < 2; i++) {
		// setup blitting
		switch (req->mergingmode) {
		case MMSFB_MM_ANAGLYPH_RED_CYAN:
			if (!i) {
				// red channel from first source surface
				mmsfbgl.disableBlend();
				mmsfbgl.setTexEnvModulate(GL_RGBA);
				mmsfbgl.setColor(0xff, 0x00, 0x00, 0xff);
			}
			else {
				// green and blue channels (cyan) from second source surface
				mmsfbgl.enableBlend(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
				mmsfbgl.setTexEnvModulate(GL_RGBA);
				mmsfbgl.setColor(0x00, 0xff, 0xff, 0xff);
			}
			break;

		case MMSFB_MM_LINE_INTERLEAVED:
			if (!i) {
				// even lines from first source surface
				mmsfbgl.disableBlend();
				mmsfbgl.setTexEnvReplace(GL_RGBA);

				if (IS_OGL_DEPTH_BUFFER_UNCHANGED(req->surface)) {
					// depth buffer has not changed, so we do not have to re-build it
					mmsfbgl.disableDepthTest();
				}
				else {
					// we have to build an interleaved depth buffer needed for blitting of the second source surface
					ENABLE_OGL_DEPTHTEST(req->surface, false);
					mmsfbgl.clear();
					mmsfbgl.setColor(0xff,0xff,0xff,0xff);
					for (int i = 0; i < req->surface->config.h; i+=2) {
						mmsfbgl.fillRectangle2D(0, (float)i, (float)req->surface->config.w - 0.1f, (float)i+0.9);
					}

					// we mark the depth buffer as "unchanged", so we can save
					// a lot of GPU time next time processMerge() is called
					DISABLE_OGL_DEPTHTEST(req->surface, true);
				}
			}
			else {
				// odd lines from second source surface
				mmsfbgl.disableBlend();
				mmsfbgl.setTexEnvReplace(GL_RGBA);

				// we enable the depth test in "readonly" mode, so we leave the depth buffer unchanged
				ENABLE_OGL_DEPTHTEST(req->surface, true);
			}
			break;
		}

		// get source surface
		MMSFBSurface *source = (!i)?req->source1:req->source2;

		// get source subsurface offset
		GET_OFFS_SRC(source);

		// set the clip to ogl
		MMSFBRectangle crect;
		if (req->surface->calcClip(0 + xoff, 0 + yoff, req->surface->config.w, req->surface->config.h, &crect)) {
			// inside clipping region
			OGL_SCISSOR(req->surface, crect.x, crect.y, crect.w, crect.h);

			// get source region
			int sx1 = 0 + src_xoff;
			int sy1 = 0 + src_yoff;
			int sx2 = 0 + source->config.w - 1 + src_xoff;
			int sy2 = 0 + source->config.h - 1 + src_yoff;

			// get destination region
			int dx1 = 0 + xoff;
			int dy1 = 0 + yoff;
			int dx2 = 0 + req->surface->config.w - 1 + xoff;
			int dy2 = 0 + req->surface->config.h - 1 + yoff;

			if (source->config.surface_buffer->ogl_tex_initialized) {
				// blit source texture to the destination
				mmsfbgl.stretchBliti(source->config.surface_buffer->ogl_tex,
							sx1, sy1, sx2, sy2, source->config.w, source->config.h,
							dx1, dy1, dx2, dy2);
			}
			else {
				printf("skip blitting from texture which is not initialized\n");
			}
		}
	}
#endif
}


