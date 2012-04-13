/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#ifndef MMSFBSURFACE_H_
#define MMSFBSURFACE_H_

#include "mmstools/mmslogger.h"
#include "mmstools/mmsthreadserver.h"
#include "mmsgui/fb/mmsfbbase.h"
#include "mmsgui/fb/mmsfbfont.h"
#include "mmsgui/fb/mmsfbconv.h"
#include "mmsgui/fb/mmsfbgl.h"

/* use DFB subsurfaces? */
//#define USE_DFB_SUBSURFACE

typedef enum {
	//! using directfb surfaces
	MMSFBSurfaceAllocMethod_dfb = 0,
	//! using malloc
	MMSFBSurfaceAllocMethod_malloc,
	//! using opengl surfaces
	MMSFBSurfaceAllocMethod_ogl
} MMSFBSurfaceAllocMethod;

typedef enum {
	//! allocated by directfb
	MMSFBSurfaceAllocatedBy_dfb = 0,
	//! allocated with malloc
	MMSFBSurfaceAllocatedBy_malloc,
	//! allocated by xv
	MMSFBSurfaceAllocatedBy_xvimage,
	//! allocated by x
	MMSFBSurfaceAllocatedBy_ximage,
	//! allocated by opengl
	MMSFBSurfaceAllocatedBy_ogl
} MMSFBSurfaceAllocatedBy;

//! dump mode
typedef enum {
	//! dump byte-by-byte hex values
	MMSFBSURFACE_DUMPMODE_BYTE = 0,
	//! dump pixels as 1, 2, 3 or 4 byte hex values
	MMSFBSURFACE_DUMPMODE_PIXEL
} MMSFBSurfaceDumpMode;

//! this is the maximum number of buffers for a surface (backbuffers + 1)
#define MMSFBSurfaceMaxBuffers		3

//! get access to the current write buffer
#define MMSFBSURFACE_WRITE_BUFFER(surface) surface->config.surface_buffer->buffers[surface->config.surface_buffer->currbuffer_write]

//! get access to the current read buffer
#define MMSFBSURFACE_READ_BUFFER(surface) surface->config.surface_buffer->buffers[surface->config.surface_buffer->currbuffer_read]

typedef struct {
	//! width
    int     sbw;
    //! height
    int     sbh;
    //! pixel format
    MMSFBSurfacePixelFormat pixelformat;
    //! the pixel format has alphachannel
    bool    alphachannel;
    //! premultiplied surface
    bool    premultiplied;
    //! number of backbuffers (e.g. 0 means FRONTONLY)
    int     backbuffer;
    //! true, if surface is stored in system memory
    bool	systemonly;
    //! real storage
    MMSFBSurfacePlanes buffers[MMSFBSurfaceMaxBuffers];
    //! real number of buffers allocated for a surface
    int 	numbuffers;
    //! index to the current read buffer (used if surface is the blit/stretchblit source)
    int 	currbuffer_read;
    //! index to the current write buffer (used as destination for all blitting/drawing routines)
    int 	currbuffer_write;
    //! surface buffer attached to this MMSFBSurface is externally allocated?
    bool	external_buffer;
#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
    //! interface to the fb layer surface
    class MMSFBSurface	*mmsfbdev_surface;
#endif
#ifdef __HAVE_XLIB__
    //! ximage of the x11 window
    XImage	*x_image[MMSFBSurfaceMaxBuffers];
#endif
#ifdef __HAVE_XV__
    //! xvimage of the x11 window
    XvImage	*xv_image[MMSFBSurfaceMaxBuffers];
#endif
#ifdef __HAVE_OPENGL__
	//! opengl framebuffer object (FBO), 0 means primary display buffer connected to the x-window or the fbdev
	GLuint	ogl_fbo;
	//! opengl texture attached to the FBO (this is the color buffer where normal 2D surface is stored)
	GLuint	ogl_tex;
	//! opengl renderbuffer attached to the FBO (this is the depth buffer needed for 3D rendering)
	GLuint	ogl_rbo;
	//! FBO initialized?
	bool ogl_fbo_initialized;
	//! texture initialized?
	bool ogl_tex_initialized;
	//! RBO initialized?
	bool ogl_rbo_initialized;
	//! content of depth buffer changed?
	bool ogl_unchanged_depth_buffer;
#endif
} MMSFBSurfaceBuffer;

typedef struct {
	//! width
    int     w;
    //! height
    int     h;
    //! color for drawing/blitting
    MMSFBColor  	color;
    //! is a clip region set?
    bool			clipped;
    //! current clip region
    MMSFBRegion		clip;
    //! the surface is a window surface
    bool        	iswinsurface;
    //! the surface is the layer surface
    //! note: for example it is possible to have a window surface in combination with this layer flag
    bool        	islayersurface;
    //! drawing flags
    MMSFBDrawingFlags 	drawingflags;
    //! blitting flags
    MMSFBBlittingFlags 	blittingflags;
    //! font
    MMSFBFont			*font;
    //! the surface buffer(s)
    MMSFBSurfaceBuffer	*surface_buffer;
    //! color of the shadow on the top, no shadow is drawn if shadow_top_color.a is set to 0 (default)
	MMSFBColor  		shadow_top_color;
    //! color of the shadow on the bottom, no shadow is drawn if shadow_bottom_color.a is set to 0 (default)
	MMSFBColor  		shadow_bottom_color;
    //! color of the shadow on the left, no shadow is drawn if shadow_left_color.a is set to 0 (default)
	MMSFBColor  		shadow_left_color;
    //! color of the shadow on the right, no shadow is drawn if shadow_right_color.a is set to 0 (default)
	MMSFBColor  		shadow_right_color;
    //! color of the shadow on the top-left, no shadow is drawn if shadow_top_left_color.a is set to 0 (default)
	MMSFBColor  		shadow_top_left_color;
    //! color of the shadow on the top-right, no shadow is drawn if shadow_top_right_color.a is set to 0 (default)
	MMSFBColor  		shadow_top_right_color;
    //! color of the shadow on the bottom-left, no shadow is drawn if shadow_bottom_left_color.a is set to 0 (default)
	MMSFBColor  		shadow_bottom_left_color;
    //! color of the shadow on the bottom-right, no shadow is drawn if shadow_bottom_right_color.a is set to 0 (default)
	MMSFBColor  		shadow_bottom_right_color;
} MMSFBSurfaceConfig;



//! This class describes a surface.
/*!
\author Jens Schneider
*/
class MMSFBSurface {
    private:
#ifdef  __HAVE_DIRECTFB__
		//! dfb surface for drawing/blitting
		IDirectFBSurface	*dfb_surface;
#endif

#ifdef __ENABLE_ACTMON__
		//! mmsfb performance collector
		class MMSPerf *mmsperf;
#endif

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
	    //! separate thread used for display panning
	    MMSThreadServer		*fbdev_ts;
#endif

		//! which system has allocated the memory?
		MMSFBSurfaceAllocatedBy	allocated_by;

		//! surface initialized?
		bool	initialized;


        bool				surface_read_locked;
        int					surface_read_lock_cnt;
        bool				surface_write_locked;
        int					surface_write_lock_cnt;
        bool				surface_invert_lock;

#ifdef __HAVE_XLIB__
        MMSFBSurface 		*scaler;
#endif
        class MMSFBLayer *layer;
        //! surface configuration
        MMSFBSurfaceConfig  config;

        // if set to true, a few self-coded blend/stretch methods will be used instead of the according DFB functions
        static bool			extendedaccel;

        // how surface memory should be allocated?
        static MMSFBSurfaceAllocMethod	allocmethod;

        void createSurfaceBuffer();
        void freeSurfaceBuffer();

        void deleteSubSurface(MMSFBSurface *surface);

        int  calcPitch(int width);
        int  calcSize(int pitch, int height);
        void initPlanePointers(MMSFBSurfacePlanes *planes, int height);

        void getRealSubSurfacePos(MMSFBSurface *surface = NULL, bool refreshChilds = false);

        bool clipSubSurface(MMSFBRegion *region, bool regionset, MMSFBRegion *tmp, bool *tmpset);

        bool setWinSurface(bool iswinsurface = true);
        bool setLayerSurface(bool islayersurface = true);

        bool checkDrawingStatus(int x, int y, int w, int h,
								MMSFBRectangle &crect, MMSFBDrawingFlags &drawingflags);
        bool checkBlittingStatus(bool src_opaque, bool src_transparent, int x, int y, int w, int h,
        						 MMSFBRectangle &crect, MMSFBBlittingFlags &blittingflags);
        bool checkBlittingStatus(MMSFBSurface *source, int x, int y, int w, int h,
								 MMSFBRectangle &crect, MMSFBBlittingFlags &blittingflags);


        bool extendedLock(MMSFBSurface *src, MMSFBSurfacePlanes *src_planes,
        				  MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes);
        void extendedUnlock(MMSFBSurface *src, MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes = NULL);

        bool printMissingCombination(string method, MMSFBSurface *source = NULL, MMSFBSurfacePlanes *src_planes = NULL,
									 MMSFBSurfacePixelFormat src_pixelformat = MMSFB_PF_NONE, int src_width = 0, int src_height = 0,
									 MMSFBBlittingFlags blittingflags = MMSFB_BLIT_NOFX);


        bool extendedAccelBlitEx(MMSFBSurface *source,
								 MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
        						 MMSFBRectangle *src_rect, int x, int y, MMSFBBlittingFlags blittingflags);
        bool extendedAccelBlit(MMSFBSurface *source, MMSFBRectangle *src_rect,
								  int x, int y, MMSFBBlittingFlags blittingflags);
        bool extendedAccelBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									 MMSFBRectangle *src_rect, int x, int y, MMSFBBlittingFlags blittingflags);

        bool extendedAccelStretchBlitEx(MMSFBSurface *source,
										MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
										MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
										MMSFBRectangle *real_dest_rect, bool calc_dest_rect);
        bool extendedAccelStretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
									  MMSFBRectangle *real_dest_rect, bool calc_dest_rect);
        bool extendedAccelStretchBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
											MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
											MMSFBRectangle *real_dest_rect, bool calc_dest_rect);

        bool extendedAccelFillRectangleEx(int x, int y, int w, int h, MMSFBDrawingFlags drawingflags);
        bool extendedAccelFillRectangle(int x, int y, int w, int h, MMSFBDrawingFlags drawingflags);

        bool extendedAccelDrawLineEx(int x1, int y1, int x2, int y2);
        bool extendedAccelDrawLine(int x1, int y1, int x2, int y2);

        bool blit_text(string &text, int len, int x, int y);
        bool blit_text_with_shadow(string &text, int len, int x, int y);


        //! flags which are used when flipping
        MMSFBFlipFlags			flipflags;

        //! to make it thread-safe
        MMSMutex  				Lock;
        //! save the id of the thread which has locked the surface
        unsigned long       	TID;
        //! count the number of times the thread has call lock()
        int       				Lock_cnt;

        //! is it a sub surface?
        bool					is_sub_surface;
        //! parent surface in case of subsurface
        MMSFBSurface    		*parent;
        //! root parent surface in case of subsurface
        MMSFBSurface    		*root_parent;
        //! sub surface position and size relative to the parent
        MMSFBRectangle 			sub_surface_rect;
        //! x offset which is added to sub_surface_rect
        int						sub_surface_xoff;
        //! y offset which is added to sub_surface_rect
        int						sub_surface_yoff;
        //! list of sub surfaces connected to this surface
        vector<MMSFBSurface *>  children;


        typedef struct {
        	bool			set;
        	bool			clipped;
        	MMSFBSurface	*surface;
        	MMSFBRegion 	clip;
        	MMSFBColor		color;
        	MMSFBRegion		real_region;
        } CLEAR_REQUEST;

        CLEAR_REQUEST	clear_request;


        void init(MMSFBSurfaceAllocatedBy allocated_by, MMSFBSurface *parent,
				  MMSFBRectangle *sub_surface_rect);


        void lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes, bool pthread_lock);
        void unlock(bool pthread_unlock);


        void processSwapDisplay(void *in_data, int in_data_len, void **out_data, int *out_data_len);
        void swapDisplay(bool vsync);

        bool doClear(unsigned char r = 0, unsigned char g = 0,
                     unsigned char b = 0, unsigned char a = 0);
        void finClear(MMSFBRectangle *check_rect = NULL);

    public:
        MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer=0, bool systemonly=true);
#ifdef  __HAVE_DIRECTFB__
        MMSFBSurface(IDirectFBSurface *dfb_surface, MMSFBSurface *parent = NULL,
					 MMSFBRectangle *sub_surface_rect = NULL);
#endif
        MMSFBSurface(MMSFBSurface *parent = NULL, MMSFBRectangle *sub_surface_rect = NULL);
        MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, MMSFBSurfacePlanes *planes);
        MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, MMSFBSurfacePlanes *planes);
#ifdef __HAVE_XLIB__
        MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XImage *x_image1, XImage *x_image2, MMSFBSurface *scaler);
#endif
#ifdef __HAVE_XV__
        MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XvImage *xv_image1, XvImage *xv_image2);
#endif

#ifdef __HAVE_OPENGL__
        MMSFBSurface(int w, int h, MMSFBSurfaceAllocatedBy allocated_by);
#endif

        virtual ~MMSFBSurface();

        bool isInitialized();

        void *getDFBSurface();

        bool getConfiguration(MMSFBSurfaceConfig *config = NULL);

        void setExtendedAcceleration(bool extendedaccel);
        bool getExtendedAcceleration();

        void setAllocMethod(MMSFBSurfaceAllocMethod allocmethod);
        MMSFBSurfaceAllocMethod getAllocMethod();

        bool isWinSurface();
        bool isLayerSurface();
        bool isSubSurface();
        MMSFBSurface *getParent();
        MMSFBSurface *getRootParent();

        bool isOpaque();

        bool getPixelFormat(MMSFBSurfacePixelFormat *pf);
        bool getSize(int *w, int *h);
        bool getNumberOfBuffers(int *num);
        bool getMemSize(int *size);

        bool setFlipFlags(MMSFBFlipFlags flags);

        //! Get the clipping rectangle.
        /*!
        \return true, if clipping rectangle (crect) is set
        \return false, if the rectangle described with x,y,w,h is outside of the surface or clipping rectangle
        */
        bool calcClip(int x, int y, int w, int h, MMSFBRectangle *crect);

        bool clear(unsigned char r = 0, unsigned char g = 0,
                   unsigned char b = 0, unsigned char a = 0);

        bool setColor(unsigned char r, unsigned char g,
                      unsigned char b, unsigned char a);
        bool setColor(MMSFBColor &color);
        bool getColor(MMSFBColor *color);

        bool setShadowColor(MMSFBColor &shadow_top_color, MMSFBColor &shadow_bottom_color,
							MMSFBColor &shadow_left_color, MMSFBColor &shadow_right_color,
							MMSFBColor &shadow_top_left_color, MMSFBColor &shadow_top_right_color,
							MMSFBColor &shadow_bottom_left_color, MMSFBColor &shadow_bottom_right_color);

        bool setClip(MMSFBRegion *clip);
        bool setClip(int x1, int y1, int x2, int y2);
        bool getClip(MMSFBRegion *clip);

        bool setDrawingFlags(MMSFBDrawingFlags flags);
        bool drawLine(int x1, int y1, int x2, int y2);
        bool drawRectangle(int x, int y, int w, int h);
        bool fillRectangle(int x = 0, int y = 0, int w = 0, int h = 0);
        bool drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
        bool fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
        bool drawCircle(int x, int y, int radius, int start_octant = 0, int end_octant = 7);

        bool setBlittingFlags(MMSFBBlittingFlags flags);
        bool getBlittingFlags(MMSFBBlittingFlags *flags);
        bool blit(MMSFBSurface *source, MMSFBRectangle *src_rect = NULL, int x = 0, int y = 0);
        bool blitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
						MMSFBRectangle *src_rect = NULL, int x = 0, int y = 0,
						bool opaque = false);
        bool blitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
						MMSFBRectangle *src_rect = NULL, int x = 0, int y = 0,
						bool opaque = false);
        bool stretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect = NULL, MMSFBRectangle *dest_rect = NULL,
						 MMSFBRectangle *real_dest_rect = NULL, bool calc_dest_rect = false);
        bool stretchBlitBuffer(MMSFBExternalSurfaceBuffer *extbuf, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							   MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
							   MMSFBRectangle *real_dest_rect = NULL, bool calc_dest_rect = false);
        bool stretchBlitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							   MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
							   MMSFBRectangle *real_dest_rect = NULL, bool calc_dest_rect = false);

        bool renderScene(MMS3D_VERTEX_ARRAY	**varrays,
						 MMS3D_INDEX_ARRAY	**iarrays,
						 MMS3D_MATERIAL		*materials,
						 MMSFBSurface		**textures,
						 MMS3D_OBJECT		**objects);

        bool merge(MMSFBSurface *source1, MMSFBSurface *source2, MMSFBMergingMode mergingmode);

        bool flip(MMSFBRegion *region = NULL);
        bool flip(int x1, int y1, int x2, int y2);
        bool refresh();

        bool createCopy(MMSFBSurface **dstsurface, int w = 0, int h = 0,
                        bool copycontent = false, bool withbackbuffer = false,
                        MMSFBSurfacePixelFormat pixelformat = MMSFB_PF_NONE);
        bool resize(int w = 0, int h = 0);

        void modulateBrightness(MMSFBColor *color, unsigned char brightness);
        void modulateOpacity(MMSFBColor *color, unsigned char opacity);

        bool setBlittingFlagsByBrightnessAlphaAndOpacity(
                    unsigned char brightness, unsigned char alpha, unsigned char opacity);
        bool setBlittingFlagsByBrightnessAlphaAndOpacityAndSource(
                    unsigned char brightness, unsigned char alpha, unsigned char opacity,
                    MMSFBSurface *source);
        bool setDrawingFlagsByAlpha(unsigned char alpha);
        bool setDrawingColorAndFlagsByBrightnessAndOpacity(
                    MMSFBColor color, unsigned char brightness, unsigned char opacity);
        bool setDrawingColorAndFlagsByBrightnessAndOpacity(
                    MMSFBColor color,
                    MMSFBColor shadow_top_color, MMSFBColor shadow_bottom_color,
					MMSFBColor shadow_left_color, MMSFBColor shadow_right_color,
                    MMSFBColor shadow_top_left_color, MMSFBColor shadow_top_right_color,
					MMSFBColor shadow_bottom_left_color, MMSFBColor shadow_bottom_right_color,
					unsigned char brightness, unsigned char opacity);

        bool setFont(MMSFBFont *font);
        bool drawString(string text, int len, int x, int y);

        void lock(MMSFBLockFlags flags = MMSFB_LOCK_NONE, void **ptr = NULL, int *pitch = NULL);
        void lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes);
        void unlock();

        unsigned int getNumberOfSubSurfaces();
        MMSFBSurface *getSubSurface(MMSFBRectangle *rect);
        bool setSubSurface(MMSFBRectangle *rect);
        bool setSubSurface(MMSFBRegion *region);
        bool moveTo(int x, int y);
        bool move(int x, int y);

        bool dump2fcb(bool (*fcb)(char *, int, void *, int *), void *argp, int *argi,
					  int x, int y, int w, int h, MMSFBSurfaceDumpMode dumpmode);
        int  dump2buffer(char *buffer, int buffer_len, int x = 0, int y = 0, int w = 0, int h = 0,
						 MMSFBSurfaceDumpMode dumpmode = MMSFBSURFACE_DUMPMODE_BYTE);
        bool dump2file(string filename, int x = 0, int y = 0, int w = 0, int h = 0,
					   MMSFBSurfaceDumpMode dumpmode = MMSFBSURFACE_DUMPMODE_BYTE);
        bool dump2file(string filename, MMSFBSurfaceDumpMode dumpmode);
        bool dump(int x = 0, int y = 0, int w = 0, int h = 0,
				  MMSFBSurfaceDumpMode dumpmode = MMSFBSURFACE_DUMPMODE_BYTE);
        bool dump(MMSFBSurfaceDumpMode dumpmode);

    friend class MMSFBLayer;
    friend class MMSFBSurfaceManager;
    friend class MMSFBWindowManager;
    friend class MMSFBBackEndInterface;
    friend class MMSPerf;

    private:

        bool blitARGBtoARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoARGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoARGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoRGB32_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoRGB32_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoARGB3565(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoARGB3565_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoYV12_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoYV12_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoRGB24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoRGB24_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoBGR24_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGBtoBGR24_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGBtoBGR555_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB32toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitRGB32toRGB32_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB16toRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB16toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB16toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitAiRGBtoAiRGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAiRGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAiRGBtoAiRGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitAiRGBtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAiRGBtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitAYUVtoAYUV(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAYUVtoAYUV_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAYUVtoAYUV_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitAYUVtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAYUVtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitAYUVtoYV12_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitAYUVtoYV12_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitYV12toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitYV12toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitI420toI420(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitI420toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitYUY2toYUY2(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitYUY2toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB24toRGB24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB24toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB24toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitRGB24toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitBGR24toBGR24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitBGR24toBGR24_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGB3565toARGB3565(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGB4444toARGB4444(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGB4444toARGB4444_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGB4444toARGB4444_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitARGB4444toRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);
        bool blitARGB4444toRGB32_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);

        bool blitBGR555toBGR555(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int x, int y);


        bool stretchBlitARGBtoARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitARGBtoARGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitARGBtoARGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitARGBtoRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitRGB32toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitRGB24toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitRGB24toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitAiRGBtoAiRGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitAiRGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitAiRGBtoAiRGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitAYUVtoAYUV(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitAYUVtoAYUV_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitAYUVtoAYUV_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitYV12toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitI420toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitYUY2toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitARGB4444toARGB4444_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);
        bool stretchBlitARGB4444toARGB4444_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);

        bool stretchBlitRGB16toRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
								int src_width, int src_height, int sx, int sy, int sw, int sh,
								int dx, int dy, int dw, int dh,
								bool antialiasing);


        bool fillRectangleARGB(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);
        bool fillRectangleARGB_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleAYUV(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);
        bool fillRectangleAYUV_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleRGB32(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleRGB24(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleRGB16(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);
        bool fillRectangleRGB16_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleYV12(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleI420(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleYUY2(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleARGB3565(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleARGB4444(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);
        bool fillRectangleARGB4444_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleBGR24(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);

        bool fillRectangleBGR555(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color);
};

bool mmsfb_create_cached_surface(MMSFBSurface **cs, int width, int height,
								 MMSFBSurfacePixelFormat pixelformat);

#endif /*MMSFBSURFACE_H_*/
