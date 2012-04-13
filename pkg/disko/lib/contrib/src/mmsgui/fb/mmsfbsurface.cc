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

#include "mmsgui/fb/mmsfbsurface.h"
#include "mmsgui/fb/mmsfb.h"
#include "mmsgui/fb/mmsfbsurfacemanager.h"

#ifdef __ENABLE_ACTMON__
#include "mmscore/mmsperf.h"
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifdef __HAVE_XLIB__
#include <ft2build.h>
#include FT_GLYPH_H
#endif


#ifdef  __HAVE_DIRECTFB__
D_DEBUG_DOMAIN( MMS_Surface, "MMS/Surface", "MMS FB Surface" );
#endif

// static variables
bool MMSFBSurface::extendedaccel								= false;
MMSFBSurfaceAllocMethod MMSFBSurface::allocmethod				= MMSFBSurfaceAllocMethod_malloc;

#define INITCHECK  if((!mmsfb->isInitialized())||(!this->initialized)){MMSFB_SetError(0,"MMSFBSurface is not initialized");return false;}

#define CLIPSUBSURFACE \
	MMSFBRegion reg, tmp; \
	bool tmpset; \
	if (clipSubSurface(&reg, false, &tmp, &tmpset)) {

#define UNCLIPSUBSURFACE \
	clipSubSurface(NULL, false, &tmp, &tmpset); }

#define SETSUBSURFACE_DRAWINGFLAGS \
	MMSFBColor ccc = this->config.color; \
	this->dfb_surface->SetColor(this->dfb_surface, ccc.r, ccc.g, ccc.b, ccc.a); \
	this->dfb_surface->SetDrawingFlags(this->dfb_surface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(this->config.drawingflags));

#define RESETSUBSURFACE_DRAWINGFLAGS \
    ccc = this->root_parent->config.color; \
    this->dfb_surface->SetColor(this->dfb_surface, ccc.r, ccc.g, ccc.b, ccc.a); \
    this->dfb_surface->SetDrawingFlags(this->dfb_surface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(this->root_parent->config.drawingflags));

#define SETSUBSURFACE_BLITTINGFLAGS \
	MMSFBColor ccc = this->config.color; \
	this->dfb_surface->SetColor(this->dfb_surface, ccc.r, ccc.g, ccc.b, ccc.a); \
	this->dfb_surface->SetBlittingFlags(this->dfb_surface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(this->config.blittingflags));

#define RESETSUBSURFACE_BLITTINGFLAGS \
    ccc = this->root_parent->config.color; \
    this->dfb_surface->SetColor(this->dfb_surface, ccc.r, ccc.g, ccc.b, ccc.a); \
    this->dfb_surface->SetBlittingFlags(this->dfb_surface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(this->root_parent->config.blittingflags));



MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, bool systemonly) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

    if (this->allocmethod == MMSFBSurfaceAllocMethod_dfb) {
#ifdef  __HAVE_DIRECTFB__
		// create surface description
		DFBSurfaceDescription   surface_desc;
		surface_desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
		surface_desc.width = w;
		surface_desc.height = h;
		surface_desc.pixelformat = getDFBPixelFormatFromMMSFBPixelFormat(pixelformat);

		if (surface_desc.pixelformat==DSPF_UNKNOWN)
			surface_desc.flags = (DFBSurfaceDescriptionFlags)(surface_desc.flags & ~DSDESC_PIXELFORMAT);

		// we use premultiplied surfaces because of alphachannel blitting with better performance
		surface_desc.flags = (DFBSurfaceDescriptionFlags)(surface_desc.flags | DSDESC_CAPS);
		surface_desc.caps = DSCAPS_PREMULTIPLIED;

		switch (backbuffer) {
			case 1: // front + one back buffer (double)
				surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_DOUBLE);
				break;
			case 2: // front + two back buffer (triple)
				surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_TRIPLE);
				break;
		}

		// surface should stored in system memory only?
		if (systemonly)
			surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_SYSTEMONLY);

		// create the surface
		DFBResult dfbres;
		if ((dfbres=mmsfb->dfb->CreateSurface(mmsfb->dfb, &surface_desc, &this->dfb_surface)) != DFB_OK) {
			this->dfb_surface = NULL;
			DEBUGMSG("MMSGUI", "ERROR");
			MMSFB_SetError(dfbres, "IDirectFB::CreateSurface(" + iToStr(w) + "x" + iToStr(h) + ") failed");
			return;
		}

	    init(MMSFBSurfaceAllocatedBy_dfb, NULL, NULL);
#endif
	}
	else
	if (this->allocmethod == MMSFBSurfaceAllocMethod_ogl) {
#ifdef  __HAVE_OPENGL__
		// setup surface attributes
		// if we allocate an fbo, backbuffers are not supported
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		this->config.w = sb->sbw = w;
		this->config.h = sb->sbh = h;
		sb->pixelformat = MMSFB_PF_ABGR;
		sb->alphachannel = true;
		sb->premultiplied = false;
		sb->backbuffer = 0;
		sb->numbuffers = 1;
		sb->systemonly = false;

		// setup plane buffer
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 0;
		sb->buffers[0].hwbuffer = true;
		sb->buffers[0].opaque = false;
		sb->buffers[0].transparent = false;

		mmsfb->bei->alloc(this);

	    init(MMSFBSurfaceAllocatedBy_ogl, NULL, NULL);
#endif
	}
	else {
		// setup surface attributes
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		this->config.w = sb->sbw = w;
		this->config.h = sb->sbh = h;
		sb->pixelformat = pixelformat;
		sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
		sb->premultiplied = true;
		sb->backbuffer = backbuffer;
		sb->systemonly = systemonly;

		// allocate my surface buffers
		sb->numbuffers = backbuffer + 1;
		if (sb->numbuffers > MMSFBSurfaceMaxBuffers) {
			sb->numbuffers = MMSFBSurfaceMaxBuffers;
			sb->backbuffer = sb->numbuffers - 1;
		}
		sb->currbuffer_read = 0;
		if (sb->numbuffers > 1)
			// using backbuffer(s)
			sb->currbuffer_write = 1;
		else
			// using only a single buffer for read/write
			sb->currbuffer_write = 0;
		DEBUGMSG("MMSGUI", "start allocating surface buffer");
		memset(sb->buffers, 0, sizeof(sb->buffers));
		for (int i = 0; i < sb->numbuffers; i++) {
			sb->buffers[i].pitch = calcPitch(w);
			int size = calcSize(sb->buffers[i].pitch, sb->sbh);
			DEBUGMSG("MMSGUI", ">allocating surface buffer #%d, %d bytes (pitch=%d, h=%d)", i, size, sb->buffers[i].pitch, sb->sbh);
			sb->buffers[i].ptr = malloc(size);
			sb->buffers[i].hwbuffer = false;

			// few internally pixelformats supports planes and therefore we must init the pointers
			initPlanePointers(&sb->buffers[i], sb->sbh);
		}
		DEBUGMSG("MMSGUI", "allocating surface buffer finished");

		init(MMSFBSurfaceAllocatedBy_malloc, NULL, NULL);
	}
}


#ifdef  __HAVE_DIRECTFB__

MMSFBSurface::MMSFBSurface(IDirectFBSurface *dfb_surface, MMSFBSurface *parent,
						   MMSFBRectangle *sub_surface_rect) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = dfb_surface;
#endif
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

	init(MMSFBSurfaceAllocatedBy_dfb, parent, sub_surface_rect);
}

#endif


MMSFBSurface::MMSFBSurface(MMSFBSurface *parent, MMSFBRectangle *sub_surface_rect) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif

	if ((!parent)||(this->allocmethod == MMSFBSurfaceAllocMethod_dfb)) {
	    // create the surfacebuffer where additional infos are stored
	    createSurfaceBuffer();
	}
    else {
    	// != DFB and parent set
    	this->config.surface_buffer = NULL;
    }

	this->layer = NULL;

	init(parent->allocated_by, parent, sub_surface_rect);
}



MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, MMSFBSurfacePlanes *planes) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
	this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

    // setup surface attributes
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = backbuffer;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	sb->numbuffers = backbuffer+1;
	if (sb->numbuffers > MMSFBSurfaceMaxBuffers) sb->numbuffers = MMSFBSurfaceMaxBuffers;
	sb->buffers[0] = *planes;
	if (sb->numbuffers >= 2) {
		if (planes[1].ptr)
			sb->buffers[1] = planes[1];
		else
			sb->numbuffers = 1;
	}
	if (sb->numbuffers >= 3) {
		if (planes[2].ptr)
			sb->buffers[2] = planes[2];
		else
			sb->numbuffers = 2;
	}
	sb->backbuffer = sb->numbuffers - 1;
	sb->currbuffer_read = 0;
	if (sb->numbuffers <= 1)
		sb->currbuffer_write = 0;
	else
		sb->currbuffer_write = 1;
	sb->external_buffer = true;

	init(MMSFBSurfaceAllocatedBy_malloc, NULL, NULL);
}

MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, MMSFBSurfacePlanes *planes) {
	MMSFBSurface(w, h, pixelformat, 0, planes);
}


#ifdef __HAVE_XV__
MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XvImage *xv_image1, XvImage *xv_image2) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
	this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
    this->scaler = NULL;

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

    // setup surface attributes
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = 1;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	sb->numbuffers = 2;
	sb->xv_image[0] = xv_image1;
	sb->buffers[0].ptr = sb->xv_image[0]->data;
	sb->buffers[0].pitch = *(sb->xv_image[0]->pitches);
	sb->buffers[0].hwbuffer = false;
	sb->xv_image[1] = xv_image2;
	sb->buffers[1].ptr = sb->xv_image[1]->data;
	sb->buffers[1].pitch = *(sb->xv_image[1]->pitches);
	sb->buffers[1].hwbuffer = false;
	sb->currbuffer_read = 0;
	sb->currbuffer_write = 1;
	sb->external_buffer = true;

	init(MMSFBSurfaceAllocatedBy_xvimage, NULL, NULL);
}
#endif

#ifdef __HAVE_XLIB__
MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XImage *x_image1, XImage *x_image2, MMSFBSurface *scaler) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
	this->scaler = scaler;

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

    // setup surface attributes
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = 0;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	if (x_image2) {
		// two ximages
		sb->backbuffer = 1;
		sb->numbuffers = 2;
		sb->x_image[0] = x_image1;
		sb->buffers[0].ptr = sb->x_image[0]->data;
		sb->buffers[0].pitch = sb->x_image[0]->bytes_per_line;
		sb->buffers[0].hwbuffer = false;
		sb->x_image[1] = x_image2;
		sb->buffers[1].ptr = sb->x_image[1]->data;
		sb->buffers[1].pitch = sb->x_image[1]->bytes_per_line;
		sb->buffers[1].hwbuffer = false;
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 1;
		sb->external_buffer = true;
	}
	else {
		// only one buffer
		sb->backbuffer = 0;
		sb->numbuffers = 1;
		sb->x_image[0] = x_image1;
		sb->buffers[0].ptr = sb->x_image[0]->data;
		sb->buffers[0].pitch = sb->x_image[0]->bytes_per_line;
		sb->buffers[0].hwbuffer = false;
		sb->x_image[1] = NULL;
		sb->buffers[1].ptr = NULL;
		sb->buffers[1].hwbuffer = false;
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 0;
		sb->external_buffer = true;
	}

	init(MMSFBSurfaceAllocatedBy_ximage, NULL, NULL);
}
#endif



#ifdef __HAVE_OPENGL__
MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfaceAllocatedBy allocated_by) {
    // init me
	this->initialized = false;
#ifdef  __HAVE_DIRECTFB__
	this->dfb_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif

    // currently only for ogl
	if (allocated_by != MMSFBSurfaceAllocatedBy_ogl)
		return;

    // create the surfacebuffer where additional infos are stored
    createSurfaceBuffer();

    // setup surface attributes
	this->config.w = this->config.surface_buffer->sbw = w;
	this->config.h = this->config.surface_buffer->sbh = h;

	if (this->config.surface_buffer) {
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		memset(sb->buffers, 0, sizeof(sb->buffers));
		sb->numbuffers = 0;
		sb->external_buffer = false;

		// setup plane buffer
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 0;
		sb->buffers[0].hwbuffer = true;
		sb->buffers[0].opaque = false;
		sb->buffers[0].transparent = false;

		// this surface is the primary opengl framebuffer
		sb->ogl_fbo = 0;
		sb->ogl_fbo_initialized = true;
	}

	init(MMSFBSurfaceAllocatedBy_ogl, NULL, NULL);
}
#endif



MMSFBSurface::~MMSFBSurface() {

#ifdef __ENABLE_ACTMON__
	if (this->mmsperf)
		delete this->mmsperf;
#endif

    if (!mmsfb->isInitialized()) return;

    // release memory - only if not the layer surface
    if (this->initialized) {
		if (!this->is_sub_surface) {
#ifndef USE_DFB_SUBSURFACE
			// delete all sub surfaces
			deleteSubSurface(NULL);
#endif
			mmsfbsurfacemanager->releaseSurface(this);
		}
		else {
#ifdef USE_DFB_SUBSURFACE
			if (this->dfb_surface) {
				this->dfb_surface->Release(this->dfb_surface);
			}
#endif

			if (this->parent)
				this->parent->deleteSubSurface(this);
		}
	}
}



void MMSFBSurface::init(MMSFBSurfaceAllocatedBy allocated_by,
	        		    MMSFBSurface *parent,
						MMSFBRectangle *sub_surface_rect) {
    // init me
#ifdef __ENABLE_ACTMON__
	this->mmsperf = new MMSPerf();
#endif

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
	this->fbdev_ts = NULL;
#endif

	this->allocated_by = allocated_by;
	this->initialized = true;

    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
    this->flipflags = MMSFB_FLIP_NONE;
    this->TID = 0;
    this->Lock_cnt = 0;

    this->clear_request.set = false;

    // init subsurface
    this->parent = parent;
    this->root_parent =  NULL;
    this->sub_surface_xoff = 0;
    this->sub_surface_yoff = 0;
    if (this->parent) {
    	if (!this->parent->is_sub_surface)
			this->root_parent = this->parent;
    	else
    		this->root_parent = this->parent->root_parent;

    	this->is_sub_surface = true;

    	this->sub_surface_rect = *sub_surface_rect;

		this->config.surface_buffer = this->root_parent->config.surface_buffer;

		this->layer = parent->layer;

#ifndef USE_DFB_SUBSURFACE

#ifdef __HAVE_DIRECTFB__
    	this->dfb_surface = this->root_parent->dfb_surface;
#endif

    	getRealSubSurfacePos();
#endif

    }
    else {
    	this->is_sub_surface = false;
    	this->sub_surface_rect.x = 0;
    	this->sub_surface_rect.y = 0;
    	this->sub_surface_rect.w = 0;
    	this->sub_surface_rect.h = 0;
    }


    // get current config
    if (this->initialized) {
        getConfiguration();

        // init color
        this->config.color.r = 0;
        this->config.color.g = 0;
        this->config.color.b = 0;
        this->config.color.a = 0;
        this->config.shadow_top_color = this->config.color;
        this->config.shadow_bottom_color = this->config.color;
        this->config.shadow_left_color = this->config.color;
        this->config.shadow_right_color = this->config.color;
        this->config.clipped = false;
        this->config.iswinsurface = false;
        this->config.islayersurface = (this->parent && this->parent->isLayerSurface());
        this->config.drawingflags = MMSFB_DRAW_NOFX;
        this->config.blittingflags = MMSFB_BLIT_NOFX;
        this->config.font = NULL;
    }
}



bool MMSFBSurface::isInitialized() {
	return this->initialized;
}


void MMSFBSurface::createSurfaceBuffer() {

	// create the surfacebuffer where additional infos are stored
	this->config.surface_buffer = new MMSFBSurfaceBuffer;
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
    if (!sb) return;

	memset(sb->buffers, 0, sizeof(sb->buffers));
	sb->numbuffers = 0;
	sb->external_buffer = false;
#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
	sb->mmsfbdev_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
	sb->x_image[0] = NULL;
#endif
#ifdef __HAVE_XV__
	sb->xv_image[0] = NULL;
#endif
#ifdef __HAVE_OPENGL__
	sb->ogl_fbo = 0;
	sb->ogl_tex = 0;
	sb->ogl_rbo = 0;
	sb->ogl_fbo_initialized = false;
	sb->ogl_tex_initialized = false;
	sb->ogl_rbo_initialized = false;
	sb->ogl_unchanged_depth_buffer = false;
#endif
}


void MMSFBSurface::freeSurfaceBuffer() {

	if (!this->initialized)
		return;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
		if (this->dfb_surface) {
			this->dfb_surface->Release(this->dfb_surface);
			this->dfb_surface = NULL;
		}
#endif
	}
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		//free my surface buffers
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (!sb->external_buffer) {
			// buffer which is internally allocated
			if (!this->is_sub_surface) {
				// no subsurface
				// free all buffers (front and back buffers)
				mmsfb->bei->free(this);
				delete sb;
				sb=NULL;
			}
		}

		if(sb) {
			sb->numbuffers = 0;
		}
#endif
	}
	else {
		//free my surface buffers
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (!sb->external_buffer) {
			// buffer which is internally allocated
			if (!this->is_sub_surface) {
				// no subsurface
				// free all buffers (front and back buffers)
				for (int i = 0; i < sb->numbuffers; i++) {
					// free only first plane of each buffer, because it points to memory for all used planes
					if (sb->buffers[i].ptr) {
						free(sb->buffers[i].ptr);
						sb->buffers[i].ptr = NULL;
					}
				}
				delete sb;
				sb=NULL;
			}
		}
		if(sb) {
			sb->numbuffers = 0;
		}
	}

	this->initialized = false;
}

void MMSFBSurface::deleteSubSurface(MMSFBSurface *surface) {
	if (surface) {
		// remove a sub surface from the list
		for (unsigned int i = 0; i < this->children.size(); i++)
			if (this->children.at(i) == surface) {
	            this->children.erase(this->children.begin()+i);
				break;
			}
	}
	else {
		// delete all sub surfaces
		for (unsigned int i = 0; i < this->children.size(); i++) {
			this->children.at(i)->deleteSubSurface(NULL);
			delete this->children.at(i);
		}
	}
}

int MMSFBSurface::calcPitch(int width) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;
    int    pitch = width;

    switch (pf) {
    case MMSFB_PF_ARGB1555:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB16:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB24:
    	pitch = width * 3;
    	break;
    case MMSFB_PF_RGB32:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_ARGB:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_A8:
    	pitch = width;
    	break;
    case MMSFB_PF_YUY2:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB332:
    	pitch = width;
    	break;
    case MMSFB_PF_UYVY:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_I420:
    	pitch = width;
    	break;
    case MMSFB_PF_YV12:
    	pitch = width;
    	break;
    case MMSFB_PF_LUT8:
    	pitch = width;
    	break;
    case MMSFB_PF_ALUT44:
    	pitch = width;
    	break;
    case MMSFB_PF_AiRGB:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_A1:
    	pitch = width / 8;
    	break;
    case MMSFB_PF_NV12:
    	pitch = width;
    	break;
    case MMSFB_PF_NV16:
    	pitch = width;
    	break;
    case MMSFB_PF_ARGB2554:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_ARGB4444:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_NV21:
    	pitch = width;
    	break;
    case MMSFB_PF_AYUV:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_ARGB3565:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_BGR24:
    	pitch = width * 3;
    	break;
    case MMSFB_PF_BGR555:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_ABGR:
    	pitch = width * 4;
    	break;
    default:
    	break;
    }

    if (pitch <= 0) pitch = 1;
    if (pitch % 4)
    	pitch += 4 - pitch % 4;

    return pitch;
}

int MMSFBSurface::calcSize(int pitch, int height) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;
    int size = pitch * height;
    int diff;

    if (pf == MMSFB_PF_I420) {
    	// increase size for U/V planes
    	size += size / 2;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }
    else
    if (pf == MMSFB_PF_YV12) {
    	// increase size for U/V planes
    	size += size / 2;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }
    else
    if (pf == MMSFB_PF_ARGB3565) {
    	// increase size for alpha plane (4 bit for each pixel)
    	size += size / 4;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }

    return size;
}

void MMSFBSurface::initPlanePointers(MMSFBSurfacePlanes *planes, int height) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;

	switch (pf) {
	case MMSFB_PF_YV12:
    	planes->ptr3 = ((unsigned char *)planes->ptr) + planes->pitch * height;
    	planes->pitch3 = planes->pitch / 4;
    	planes->ptr2 = ((unsigned char *)planes->ptr3) + planes->pitch3 * height;
    	planes->pitch2 = planes->pitch3;
    	break;
	case MMSFB_PF_ARGB3565:
    	planes->ptr2 = ((unsigned char *)planes->ptr) + planes->pitch * height;
    	planes->pitch2 = planes->pitch / 4;
    	planes->ptr3 = NULL;
    	planes->pitch3 = 0;
    	break;
    default:
    	break;
	}
}

void MMSFBSurface::getRealSubSurfacePos(MMSFBSurface *surface, bool refreshChilds) {
	if (this->is_sub_surface) {
		this->sub_surface_xoff = this->sub_surface_rect.x + this->parent->sub_surface_xoff;
		this->sub_surface_yoff = this->sub_surface_rect.y + this->parent->sub_surface_yoff;

		if (refreshChilds)
			for (unsigned int i = 0; i < this->children.size(); i++)
				this->children.at(i)->getRealSubSurfacePos(NULL, refreshChilds);
	}
	else {
		this->sub_surface_xoff = 0;
		this->sub_surface_yoff = 0;
	}
}


bool MMSFBSurface::clipSubSurface(MMSFBRegion *region, bool regionset, MMSFBRegion *tmp, bool *tmpset) {
	MMSFBRegion myregion;

	if (!region) {
		if (*tmpset)
			this->root_parent->setClip(tmp);
		else
			this->root_parent->setClip(NULL);
		this->root_parent->unlock();
		return true;
	}

	/* get my region */
	getClip(&myregion);

	if (this->is_sub_surface) {
	    myregion.x1+=sub_surface_xoff;
	    myregion.y1+=sub_surface_yoff;
	    myregion.x2+=sub_surface_xoff;
	    myregion.y2+=sub_surface_yoff;
	}

	if (!regionset) {
		/* init region */
		*region = myregion;
		if(this->parent)
    		return this->parent->clipSubSurface(region, true, tmp, tmpset);
	}

    /* check if input region is within my region */
    if (region->x1 < myregion.x1)
    	region->x1 = myregion.x1;
    else
    if (region->x1 > myregion.x2)
    	return false;

    if (region->y1 < myregion.y1)
    	region->y1 = myregion.y1;
    else
    if (region->y1 > myregion.y2)
    	return false;

    if (region->x2 > myregion.x2)
    	region->x2 = myregion.x2;
    else
    if (region->x2 < myregion.x1)
    	return false;

    if (region->y2 > myregion.y2)
    	region->y2 = myregion.y2;
    else
    if (region->y2 < myregion.y1)
    	return false;

    /* have a parent, call recursive */
	if (this->is_sub_surface)
		return this->parent->clipSubSurface(region, true, tmp, tmpset);

	/* i am the root, set clip now */
	lock();
	if (this->config.clipped) {
		getClip(tmp);
		*tmpset=true;
	}
	else
		*tmpset=false;
	setClip(region);
	return true;
}

void *MMSFBSurface::getDFBSurface() {
	if (!initialized)
		return NULL;

#ifdef  __HAVE_DIRECTFB__
	return this->dfb_surface;
#endif

	return NULL;
}

bool MMSFBSurface::getConfiguration(MMSFBSurfaceConfig *config) {

    /* check if initialized */
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
		DFBSurfaceCapabilities  caps;
		DFBResult               dfbres;
		DFBSurfacePixelFormat   mypf;

		/* get size */
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->GetSize(this->dfb_surface, &(this->config.w), &(this->config.h))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::GetSize() failed");
				return false;
			}
			this->config.surface_buffer->sbw = this->config.w;
			this->config.surface_buffer->sbh = this->config.h;
		}
		else {
#ifdef USE_DFB_SUBSURFACE
			if ((dfbres=this->dfb_surface->GetSize(this->dfb_surface, &(this->config.w), &(this->config.h))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::GetSize() failed");
				return false;
			}
#else
			this->config.w = this->sub_surface_rect.w;
			this->config.h = this->sub_surface_rect.h;
#endif
		}

		// get the surface pitch
		void *ptr;
		if (this->dfb_surface->Lock(this->dfb_surface, DSLF_READ, &ptr, &this->config.surface_buffer->buffers[0].pitch) == DFB_OK) {
			this->dfb_surface->Unlock(this->dfb_surface);
		}

		/* get pixelformat */
		if ((dfbres=this->dfb_surface->GetPixelFormat(this->dfb_surface, &mypf)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::GetPixelFormat() failed");
			return false;
		}

		/* build a format string */
		this->config.surface_buffer->pixelformat = getMMSFBPixelFormatFromDFBPixelFormat(mypf);
		this->config.surface_buffer->alphachannel = isAlphaPixelFormat(this->config.surface_buffer->pixelformat);

		/* get capabilities */
		if ((dfbres=this->dfb_surface->GetCapabilities(this->dfb_surface, &caps)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::GetCapabilities() failed");
			return false;
		}

	    /* is it a premultiplied surface? */
		this->config.surface_buffer->premultiplied = caps & DSCAPS_PREMULTIPLIED;

	    /* get the buffer mode */
		this->config.surface_buffer->backbuffer = 0;
	    if (caps & DSCAPS_DOUBLE)
	    	this->config.surface_buffer->backbuffer = 1;
	    else
	    if (caps & DSCAPS_TRIPLE)
	    	this->config.surface_buffer->backbuffer = 2;

	    /* system only? */
	    this->config.surface_buffer->systemonly = false;
	    if (caps & DSCAPS_SYSTEMONLY)
	    	this->config.surface_buffer->systemonly = true;

	    /* fill return config */
	    if (config)
	        *config = this->config;

	    /* log some infos */
	    if ((!config)&&(!this->is_sub_surface)) {
	    	DEBUGMSG("MMSGUI", "Surface properties:");

	    	DEBUGMSG("MMSGUI", " type:         DFB");
	    	DEBUGMSG("MMSGUI", " size:         " + iToStr(this->config.w) + "x" + iToStr(this->config.h));
			DEBUGMSG("MMSGUI", " pitch:        " + iToStr(this->config.surface_buffer->buffers[0].pitch));

		    if (this->config.surface_buffer->alphachannel)
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat) + ",ALPHACHANNEL");
		    else
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat));

		    DEBUGMSG("MMSGUI", " capabilities:");

		    if (caps & DSCAPS_PRIMARY)
		    	DEBUGMSG("MMSGUI", "  PRIMARY");
		    if (caps & DSCAPS_SYSTEMONLY)
		    	DEBUGMSG("MMSGUI", "  SYSTEMONLY");
		    if (caps & DSCAPS_VIDEOONLY)
		    	DEBUGMSG("MMSGUI", "  VIDEOONLY");
		    if (caps & DSCAPS_DOUBLE)
		    	DEBUGMSG("MMSGUI", "  DOUBLE");
		    if (caps & DSCAPS_TRIPLE)
		    	DEBUGMSG("MMSGUI", "  TRIPLE");
		    if (caps & DSCAPS_PREMULTIPLIED)
		    	DEBUGMSG("MMSGUI", "  PREMULTIPLIED");
	    }

	    return true;
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (this->config.surface_buffer->ogl_fbo == 0) {
			// this surface is the primary display buffer connected to the x-window
			int val;

			// get size
			if (this->is_sub_surface) {
				this->config.w = this->sub_surface_rect.w;
				this->config.h = this->sub_surface_rect.h;
			}
			this->config.surface_buffer->buffers[0].pitch = this->config.w * 4;

			this->config.surface_buffer->pixelformat = MMSFB_PF_ABGR;
			this->config.surface_buffer->alphachannel = true;
			this->config.surface_buffer->premultiplied = false;

			// get double buffering status
/*printf("getconfig\n");fflush(stdout);
			LOCK_OGL(0);
printf("getconfig2\n");fflush(stdout);
			int glxres;
			if ((glxres = glXGetConfig(this->x_display, this->xvi, GLX_DOUBLEBUFFER, &val))) {
				MMSFB_SetError(glxres, "glXGetConfig() failed");
				UNLOCK_OGL;
				return false;
			}
			UNLOCK_OGL;*/
// TODO
			val = 1;
			this->config.surface_buffer->backbuffer = (val)?1:0;
			this->config.surface_buffer->numbuffers = this->config.surface_buffer->backbuffer + 1;

		    this->config.surface_buffer->systemonly = false;
		}
		else {
			// get size
			if (this->is_sub_surface) {
				this->config.w = this->sub_surface_rect.w;
				this->config.h = this->sub_surface_rect.h;
			}
			this->config.surface_buffer->buffers[0].pitch = this->config.w * 4;

			this->config.surface_buffer->pixelformat = MMSFB_PF_ABGR;
			this->config.surface_buffer->alphachannel = true;
			this->config.surface_buffer->premultiplied = false;

			this->config.surface_buffer->backbuffer = 0;
			this->config.surface_buffer->numbuffers = this->config.surface_buffer->backbuffer + 1;

		    this->config.surface_buffer->systemonly = false;
		}

	    // fill return config
	    if (config)
	        *config = this->config;

	    // log some infos
	    if ((!config)&&(!this->is_sub_surface)) {
	    	DEBUGMSG("MMSGUI", "Surface properties:");

	    	DEBUGMSG("MMSGUI", " type:         OGL");
	    	DEBUGMSG("MMSGUI", " size:         " + iToStr(this->config.w) + "x" + iToStr(this->config.h));
			DEBUGMSG("MMSGUI", " pitch:        " + iToStr(this->config.surface_buffer->buffers[0].pitch));

		    if (this->config.surface_buffer->alphachannel)
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat) + ",ALPHACHANNEL");
		    else
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat));

		    DEBUGMSG("MMSGUI", " capabilities:");

		    if (this->config.surface_buffer->ogl_fbo == 0)
		    	DEBUGMSG("MMSGUI", "  PRIMARY");
			if (this->config.surface_buffer->backbuffer == 1)
				DEBUGMSG("MMSGUI", "  DOUBLE");
		}
	    return true;
#endif
	}
	else {
		// get size
		if (this->is_sub_surface) {
			this->config.w = this->sub_surface_rect.w;
			this->config.h = this->sub_surface_rect.h;
		}

		// fill return config
		if (config)
			*config = this->config;

		// log some infos
		if ((!config)&&(!this->is_sub_surface)) {
			DEBUGMSG("MMSGUI", "Surface properties:");

	    	DEBUGMSG("MMSGUI", " type:         MMS");
			DEBUGMSG("MMSGUI", " size:         " + iToStr(this->config.w) + "x" + iToStr(this->config.h));
			DEBUGMSG("MMSGUI", " pitch:        " + iToStr(this->config.surface_buffer->buffers[0].pitch));

			if (this->config.surface_buffer->alphachannel)
				DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat) + ",ALPHACHANNEL");
			else
				DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat));

			DEBUGMSG("MMSGUI", " capabilities:");

			if (this->config.surface_buffer->systemonly)
				DEBUGMSG("MMSGUI", "  SYSTEMONLY");
			if (this->config.surface_buffer->backbuffer == 1)
				DEBUGMSG("MMSGUI", "  DOUBLE");
			if (this->config.surface_buffer->backbuffer == 2)
				DEBUGMSG("MMSGUI", "  TRIPLE");
			if (this->config.surface_buffer->premultiplied)
				DEBUGMSG("MMSGUI", "  PREMULTIPLIED");
		}
	    return true;
	}
	return false;
}

void MMSFBSurface::setExtendedAcceleration(bool extendedaccel) {
	this->extendedaccel = extendedaccel;
}

bool MMSFBSurface::getExtendedAcceleration() {
	return this->extendedaccel;
}

void MMSFBSurface::setAllocMethod(MMSFBSurfaceAllocMethod allocmethod) {
	this->allocmethod = allocmethod;
	if (this->allocmethod == MMSFBSurfaceAllocMethod_malloc)
		printf("DISKO: Using own surface memory management.\n");
}

MMSFBSurfaceAllocMethod MMSFBSurface::getAllocMethod() {
	return this->allocmethod;
}

bool MMSFBSurface::isWinSurface() {
    return this->config.iswinsurface;
}

bool MMSFBSurface::isLayerSurface() {
    return this->config.islayersurface;
}

bool MMSFBSurface::isSubSurface() {
    return this->is_sub_surface;
}

MMSFBSurface *MMSFBSurface::getParent() {
    return this->parent;
}

MMSFBSurface *MMSFBSurface::getRootParent() {
    return this->root_parent;
}

bool MMSFBSurface::setWinSurface(bool iswinsurface) {

    /* check if initialized */
    INITCHECK;

    /* set the flag */
    this->config.iswinsurface = iswinsurface;

    return true;
}

bool MMSFBSurface::setLayerSurface(bool islayersurface) {

    /* check if initialized */
    INITCHECK;

    /* set the flag */
    this->config.islayersurface = islayersurface;

    return true;
}


bool MMSFBSurface::isOpaque() {
	if (MMSFBSURFACE_READ_BUFFER(this).opaque) {
		return true;
	}
	else {
		return (!isAlphaPixelFormat(this->config.surface_buffer->pixelformat));
	}
}


bool MMSFBSurface::getPixelFormat(MMSFBSurfacePixelFormat *pixelformat) {

    /* check if initialized */
    INITCHECK;

    /* return the pixelformat */
    *pixelformat = this->config.surface_buffer->pixelformat;

    return true;
}

bool MMSFBSurface::getSize(int *w, int *h) {

    /* check if initialized */
    INITCHECK;

    /* return values */
    *w = this->config.w;
    *h = this->config.h;

    return true;
}

bool MMSFBSurface::getNumberOfBuffers(int *num) {

    // check if initialized
    INITCHECK;

    // return value
    *num = this->config.surface_buffer->backbuffer + 1;

    return true;
}

bool MMSFBSurface::getMemSize(int *size) {

	/* check if initialized */
    INITCHECK;

    /* init size */
    if (!size)
    	return false;
    *size = 0;

    *size = calcSize(this->config.surface_buffer->buffers[0].pitch, this->config.h);

    return true;
}


bool MMSFBSurface::setFlipFlags(MMSFBFlipFlags flags) {
	this->flipflags = flags;
	return true;
}



bool MMSFBSurface::calcClip(int x, int y, int w, int h, MMSFBRectangle *crect) {
	MMSFBRegion clipreg;

#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else {
			clipreg = this->config.clip;
		}
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else {
			clipreg = this->root_parent->config.clip;
		}
	}
#endif

	if (x < clipreg.x1) {
		// left outside
		w-= clipreg.x1 - x;
		if (w <= 0) {
			return false;
		}
		x = clipreg.x1;
	}
	else
	if (x > clipreg.x2) {
		// right outside
		return false;
	}

	if (y < clipreg.y1) {
		// top outside
		h-= clipreg.y1 - y;
		if (h <= 0) {
			return false;
		}
		y = clipreg.y1;
	}
	else
	if (y > clipreg.y2) {
		// bottom outside
		return false;
	}

	if (x + w - 1 > clipreg.x2) {
		// to width
		w = clipreg.x2 - x + 1;
	}

	if (y + h - 1 > clipreg.y2) {
		// to height
		h = clipreg.y2 - y + 1;
	}

	// set crect and return
	if (crect) {
		crect->x = x;
		crect->y = y;
		crect->w = w;
		crect->h = h;
	}
	return true;
}

bool MMSFBSurface::doClear(unsigned char r, unsigned char g,
                           unsigned char b, unsigned char a) {
    bool ret = false;

    // check if initialized
    INITCHECK;
/*
MMSFBRegion clip;
getClip(&clip);
printf("doClear with clip %d,%d,%d,%d (%d,%d,%d,%d)\n", clip.x1, clip.y1, clip.x2, clip.y2, r,g,b,a);
*/

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__

	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
		MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

		DFBResult   dfbres;
	    D_DEBUG_AT( MMS_Surface, "clear( argb %02x %02x %02x %02x ) <- %dx%d\n",
	                a, r, g, b, this->config.surface_buffer->sbw, this->config.surface_buffer->sbh );
	    MMSFB_TRACE();

	    if ((a < 0xff)&&(this->config.surface_buffer->premultiplied)) {
			// premultiplied surface, have to premultiply the color
			register int aa = a + 1;
			r = (aa * r) >> 8;
			g = (aa * g) >> 8;
			b = (aa * b) >> 8;
		}

		if (!this->is_sub_surface) {
			// clear surface
			if ((dfbres=this->dfb_surface->Clear(this->dfb_surface, r, g, b, a)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::Clear() failed");
				return false;
			}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE
#endif

			// clear surface
			if (this->dfb_surface->Clear(this->dfb_surface, r, g, b, a) == DFB_OK)
				ret = true;

#ifndef USE_DFB_SUBSURFACE
			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
		MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	    if (!this->is_sub_surface) {

			MMSFBColor color = MMSFBColor(r, g, b, a);
			mmsfb->bei->clear(this, color);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			MMSFBColor color = MMSFBColor(r, g, b, a);
			mmsfb->bei->clear(this, color);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}
	else {
		MMSFBColor *col = &this->config.color;
		MMSFBColor savedcol = *col;
		col->r = r;
		col->g = g;
		col->b = b;
		col->a = a;
		MMSFBDrawingFlags saveddf = this->config.drawingflags;
		this->config.drawingflags = MMSFB_DRAW_SRC_PREMULTIPLY;

		ret = fillRectangle(0, 0, this->config.w, this->config.h);

		*col = savedcol;
		this->config.drawingflags = saveddf;
	}


    return ret;
}


void MMSFBSurface::finClear(MMSFBRectangle *check_rect) {

	// block other threads
	lock();

	CLEAR_REQUEST *clear_req = &this->clear_request;
	if (this->is_sub_surface) clear_req = &this->root_parent->clear_request;

	if (!clear_req->set) {
		unlock();
		return;
	}

	clear_req->set = false;

//printf("finClear\n");


	if (check_rect) {
		// we have to check if clear request can be skipped
//printf(">>>>>finClear rect %d,%d,%d,%d\n", check_rect->x, check_rect->y, check_rect->w, check_rect->h);
		if (this->is_sub_surface) {
//			check_rect->x+=sub_surface_xoff;
//			check_rect->y+=sub_surface_yoff;
		}
//printf(">>X>>finClear rect %d,%d,%d,%d\n", check_rect->x, check_rect->y, check_rect->w, check_rect->h);

		if (check_rect->x <= clear_req->real_region.x1 &&
			check_rect->y <= clear_req->real_region.y1 &&
			check_rect->x + check_rect->w - 1 >= clear_req->real_region.x2 &&
			check_rect->y + check_rect->h - 1 >= clear_req->real_region.y2) {
			// skip clear request
//printf(">>>>>finClear rect %d,%d,%d,%d  >>> skipped\n", check_rect->x, check_rect->y, check_rect->w, check_rect->h);

			unlock();
			return;
		}
	}

	// we have to clear, set clip
	MMSFBRegion clip;
	bool clipped = clear_req->surface->config.clipped;
	if (clipped) {
		clear_req->surface->getClip(&clip);
		if (clear_req->clipped)
			clear_req->surface->setClip(&clear_req->clip);
		else
			clear_req->surface->setClip(NULL);
	}
	else {
		if (clear_req->clipped)
			clear_req->surface->setClip(&clear_req->clip);
	}

	// do it
	clear_req->surface->doClear(clear_req->color.r,
								clear_req->color.g,
								clear_req->color.b,
								clear_req->color.a);

	// reset clip
	if (clipped) {
		clear_req->surface->setClip(&clip);
	}
	else {
		if (clear_req->clipped)
			clear_req->surface->setClip(NULL);
	}

	unlock();
}

bool MMSFBSurface::clear(unsigned char r, unsigned char g,
                         unsigned char b, unsigned char a) {

	// check if initialized
    INITCHECK;
/*
MMSFBRegion clip;
getClip(&clip);
printf("clear with clip %d,%d,%d,%d (%d,%d,%d,%d)  dest opaque = %d\n", clip.x1, clip.y1, clip.x2, clip.y2, r,g,b,a,MMSFBSURFACE_WRITE_BUFFER(this).opaque);
printf("------real %d,%d,%d,%d\n",clip.x1+sub_surface_xoff, clip.y1+sub_surface_yoff, clip.x2+sub_surface_xoff, clip.y2+sub_surface_yoff);
*/

    // block other threads
    lock();

	// get access to previous clear request
	CLEAR_REQUEST *clear_req = &this->clear_request;
	if (this->is_sub_surface) clear_req = &this->root_parent->clear_request;

	if (clear_req->set) {
		// finalize previous clear
		if (r != clear_req->color.r || g != clear_req->color.g || b != clear_req->color.b || a != clear_req->color.a) {
			// we cannot skip previous clear, because requested color is different
			finClear();
		}
		else {
			// it can be possible to skip previous clear
			// so we have to put affected rectangle to finClear() method, preparing the decision
			MMSFBRegion clip;
			getClip(&clip);
			MMSFBRectangle rect = MMSFBRectangle(clip.x1, clip.y1, clip.x1 + clip.x2 + 1, clip.y1 + clip.y2 + 1);
			finClear(&rect);
		}
	}

	// save clear request
	clear_req->set = true;
	clear_req->surface = this;
	clear_req->clipped = this->config.clipped;
	if (clear_req->clipped)
		getClip(&clear_req->clip);
	clear_req->color = MMSFBColor(r, g, b, a);

	// get affected region on the real existing surface buffer
	clear_req->real_region = (clear_req->clipped) ? clear_req->clip : MMSFBRegion(0, 0, this->config.w-1, this->config.h-1);
	if (this->is_sub_surface) {
		clear_req->real_region.x1+= this->sub_surface_xoff;
		clear_req->real_region.y1+= this->sub_surface_yoff;
		clear_req->real_region.x2+= this->sub_surface_xoff;
		clear_req->real_region.y2+= this->sub_surface_yoff;
	}

	// all right
	unlock();
	return true;
}

bool MMSFBSurface::setColor(unsigned char r, unsigned char g,
                            unsigned char b, unsigned char a) {

    // check if initialized
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    // set color
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->dfb_surface->SetColor(this->dfb_surface, r, g, b, a)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetColor() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->SetColor(this->dfb_surface, r, g, b, a)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetColor() failed");
				return false;
			}
		}
#endif
#endif
	}

    // save the color
	MMSFBColor *col = &this->config.color;
	col->r = r;
	col->g = g;
	col->b = b;
	col->a = a;

    // set the default drawing flags
    // reason a): if it is an PREMULTIPLIED surface, the given color has to
    //            premultiplied internally before using it
    // reason b): if an alpha value is specified, the next draw function
    //            should blend over the surface
	this->setDrawingFlagsByAlpha(a);

    return true;
}

bool MMSFBSurface::setColor(MMSFBColor &color) {
	return setColor(color.r, color.g, color.b, color.a);
}

bool MMSFBSurface::getColor(MMSFBColor *color) {

    // check if initialized
    INITCHECK;

    // return the color
    *color = this->config.color;

    return true;
}

bool MMSFBSurface::setShadowColor(MMSFBColor &shadow_top_color, MMSFBColor &shadow_bottom_color,
								  MMSFBColor &shadow_left_color, MMSFBColor &shadow_right_color,
								  MMSFBColor &shadow_top_left_color, MMSFBColor &shadow_top_right_color,
								  MMSFBColor &shadow_bottom_left_color, MMSFBColor &shadow_bottom_right_color) {

	// check if initialized
    INITCHECK;

    // save the new shadow colors
    // note: if the alphachannel of a color is 0, the respective shadow is disabled
    this->config.shadow_top_color = shadow_top_color;
    this->config.shadow_bottom_color = shadow_bottom_color;
    this->config.shadow_left_color = shadow_left_color;
    this->config.shadow_right_color = shadow_right_color;
    this->config.shadow_top_left_color = shadow_top_left_color;
    this->config.shadow_top_right_color = shadow_top_right_color;
    this->config.shadow_bottom_left_color = shadow_bottom_left_color;
    this->config.shadow_bottom_right_color = shadow_bottom_right_color;

    return true;
}

bool MMSFBSurface::setClip(MMSFBRegion *clip) {

    // check if initialized
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    // set clip
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->dfb_surface->SetClip(this->dfb_surface, (DFBRegion*)clip)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetClip() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->SetClip(this->dfb_surface, (DFBRegion*)clip)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetClip() failed");
				return false;
			}
		}
#endif
#endif
	}

    // save the region
    if (clip) {
    	this->config.clipped = true;
	    this->config.clip = *clip;
    }
    else {
    	this->config.clipped = false;
    }

    return true;
}

bool MMSFBSurface::setClip(int x1, int y1, int x2, int y2) {
	MMSFBRegion clip;
	clip.x1=x1;
	clip.y1=y1;
	clip.x2=x2;
	clip.y2=y2;
	return setClip(&clip);
}

bool MMSFBSurface::getClip(MMSFBRegion *clip) {

	/* check if initialized */
    INITCHECK;

    /* return the clip region */
    if (this->config.clipped) {
    	*clip = this->config.clip;
    }
    else {
    	clip->x1 = 0;
    	clip->y1 = 0;
    	clip->x2 = this->config.w - 1;
    	clip->y2 = this->config.h - 1;
    }

	return true;
}


bool MMSFBSurface::setDrawingFlags(MMSFBDrawingFlags flags) {

    /* check if initialized */
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set the drawing flags */
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->dfb_surface->SetDrawingFlags(this->dfb_surface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(flags))) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetDrawingFlags() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->SetDrawingFlags(this->dfb_surface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(flags))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetDrawingFlags() failed");
				return false;
			}
		}
#endif
#endif
	}

    /* save the flags */
    this->config.drawingflags = flags;

    return true;
}



bool MMSFBSurface::drawLine(int x1, int y1, int x2, int y2) {
    bool ret = false;

    // check if initialized
    INITCHECK;

	// check if we can use fill rectangle
	if (x1 == x2) {
		return fillRectangle(x1, y1, 1, y2-y1+1);
	}
	else
	if (y1 == y2) {
		return fillRectangle(x1, y1, x2-x1+1, 1);
	}

    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
	MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    MMSFB_BREAK();

	    // draw a line
		if (!this->is_sub_surface) {
			if (!extendedAccelDrawLine(x1, y1, x2, y2))
				if ((dfbres=this->dfb_surface->DrawLine(this->dfb_surface, x1, y1, x2, y2)) != DFB_OK) {
					MMSFB_SetError(dfbres, "IDirectFBSurface::DrawLine() failed");
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			if (extendedAccelDrawLine(x1, y1, x2, y2))
				ret = true;
			else
				if (this->dfb_surface->DrawLine(this->dfb_surface, x1, y1, x2, y2) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}

#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (!this->is_sub_surface) {

			MMSFBRegion region = MMSFBRegion(x1, y1, x2, y2);
			mmsfb->bei->drawLine(this, region);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			MMSFBRegion region = MMSFBRegion(x1, y1, x2, y2);
			mmsfb->bei->drawLine(this, region);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}
	else {

		if (!this->is_sub_surface) {
			ret = extendedAccelDrawLine(x1, y1, x2, y2);
		}
		else {
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;

			ret = extendedAccelDrawLine(x1, y1, x2, y2);

			UNCLIPSUBSURFACE
		}

	}

    return ret;
}

bool MMSFBSurface::drawRectangle(int x, int y, int w, int h) {
	bool ret = false;

    // check if initialized
    INITCHECK;
    if (w < 1 || h < 1)
    	return false;

    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
	MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (!this->is_sub_surface) {

			MMSFBRectangle rect = MMSFBRectangle(x, y, w, h);
			mmsfb->bei->drawRectangle(this, rect);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			MMSFBRectangle rect = MMSFBRectangle(x, y, w, h);
			mmsfb->bei->drawRectangle(this, rect);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}
	else {
		// draw lines...
		if (w==1)
			ret = drawLine(x, y, x, y+h-1);
		else
		if (h==1)
			ret = drawLine(x, y, x+w-1, y);
		else {
			ret = drawLine(x, y, x+w-1, y);
			ret = drawLine(x, y+h-1, x+w-1, y+h-1);
			if (h>2) {
				ret = drawLine(x, y+1, x, y+h-2);
				ret = drawLine(x+w-1, y+1, x+w-1, y+h-2);
			}
		}
	}

    return ret;
}

bool MMSFBSurface::checkDrawingStatus(int x, int y, int w, int h,
									  MMSFBRectangle &crect, MMSFBDrawingFlags &drawingflags) {

	if (this->config.color.a == 0x00) {
    	// fill color is full transparent
    	if (this->config.drawingflags & MMSFB_DRAW_BLEND) {
    		// nothing to draw
    		return false;
    	}
    }

    // check clipping region and calculate final rectangle
	if (!this->is_sub_surface) {
	    if (!calcClip(x, y, w, h, &crect)) {
	    	// rectangle described with x, y, w, h is outside of the surface or clipping rectangle
	    	return false;
	    }
	}
	else {
		bool outside = false;
		CLIPSUBSURFACE
		if (!calcClip(x + this->sub_surface_xoff, y + this->sub_surface_yoff, w, h, &crect)) {
			// rectangle described with x, y, w, h is outside of the surface or clipping rectangle
			outside = true;
		}
		UNCLIPSUBSURFACE
		if (outside) return false;
	}

    // set new opaque/transparent status and get drawing flags for it
    drawingflags = this->config.drawingflags;
    switch (this->config.color.a) {
    case 0x00:
    	// fill color is full transparent
    	switch (drawingflags) {
		case MMSFB_DRAW_NOFX:
		case MMSFB_DRAW_SRC_PREMULTIPLY:
			// note: we use this->config.surface_buffer->sbw and this->config.surface_buffer->sbh
			//       because this is the dimension of the real existing buffer
			if    ((crect.x <= 0) && (crect.y <= 0)
				&& (crect.x + crect.w >= this->config.surface_buffer->sbw)
				&& (crect.y + crect.h >= this->config.surface_buffer->sbh)) {
				// fill writes the whole destination surface
				MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
				MMSFBSURFACE_WRITE_BUFFER(this).transparent	= true;
			}
			else {
				// let transparent status unchanged
				MMSFBSURFACE_WRITE_BUFFER(this).opaque = false;
			}
			break;
		default:
			// after drawing surface is not opaque and not transparent
			MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
			MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			break;
		}
    	break;

	case 0xff:
		// fill color is opaque
		// so remove the DRAW_BLEND flag if set
		drawingflags = drawingflags & ~MMSFB_DRAW_BLEND;
		switch (drawingflags) {
		case MMSFB_DRAW_NOFX:
		case MMSFB_DRAW_SRC_PREMULTIPLY:
			// note: we use this->config.surface_buffer->sbw and this->config.surface_buffer->sbh
			//       because this is the dimension of the real existing buffer
			if    ((crect.x <= 0) && (crect.y <= 0)
				&& (crect.x + crect.w >= this->config.surface_buffer->sbw)
				&& (crect.y + crect.h >= this->config.surface_buffer->sbh)) {
				// fill writes the whole destination surface
				MMSFBSURFACE_WRITE_BUFFER(this).opaque		= true;
				MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			}
			else {
				// let opaque status unchanged
				MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			}
			break;
		default:
			// after drawing surface is not opaque and not transparent
			MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
			MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			break;
		}
		break;

    default:
    	// fill color is semi-transparent
    	if (!(drawingflags & MMSFB_DRAW_BLEND)) {
			// after drawing surface is not opaque
			MMSFBSURFACE_WRITE_BUFFER(this).opaque = false;
    	}

    	// after drawing surface is not transparent
		MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
    	break;
    }

    return true;
}

bool MMSFBSurface::fillRectangle(int x, int y, int w, int h) {
    bool		ret = false;

    // check if initialized
    INITCHECK;

    if ((w <= 0) || (h <= 0)) {
    	// use full surface
    	x = 0;
    	y = 0;
    	w = this->config.w;
    	h = this->config.h;
    }

	// finalize previous clear
    // note: this is very important to do it here, because doClear() calls fillRectangle() too
	finClear();

    // save opaque/transparent status
    bool opaque_saved		= MMSFBSURFACE_WRITE_BUFFER(this).opaque;
    bool transparent_saved	= MMSFBSURFACE_WRITE_BUFFER(this).transparent;

    // get final rectangle and new opaque/transparent status
    MMSFBRectangle crect;
    MMSFBDrawingFlags drawingflags;
    if (!checkDrawingStatus(x, y, w, h, crect, drawingflags)) {
    	// nothing to draw
    	return true;
    }

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    D_DEBUG_AT( MMS_Surface, "fill( %d,%d - %dx%d ) <- %dx%d, %02x %02x %02x %02x\n",
	                x, y, w, h, this->config.surface_buffer->sbw, this->config.surface_buffer->sbh,
	                this->config.color.a, this->config.color.r, this->config.color.g, this->config.color.b );
	    MMSFB_TRACE();

	    /* fill rectangle */
		if (!this->is_sub_surface) {
			if (!extendedAccelFillRectangle(crect.x, crect.y, crect.w, crect.h, this->config.drawingflags))
				if ((dfbres=this->dfb_surface->FillRectangle(this->dfb_surface, x, y, w, h)) != DFB_OK) {
					MMSFB_SetError(dfbres, "IDirectFBSurface::FillRectangle() failed");
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			if (extendedAccelFillRectangle(crect.x, crect.y, crect.w, crect.h, this->config.drawingflags))
				ret = true;
			else
				if (this->dfb_surface->FillRectangle(this->dfb_surface, x, y, w, h) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		mmsfb->bei->fillRectangle(this, crect, drawingflags);
		ret = true;
#endif
	}
	else {
		ret = extendedAccelFillRectangle(crect.x, crect.y, crect.w, crect.h, drawingflags);
	}

	if (!ret) {
		// restore opaque/transparent status
	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= opaque_saved;
	    MMSFBSURFACE_WRITE_BUFFER(this).transparent	= transparent_saved;
	}

    return ret;
}

bool MMSFBSurface::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    MMSFB_BREAK();

	bool ret = false;

    // check if initialized
    INITCHECK;

    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
	MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (!this->is_sub_surface) {

			MMSFBTriangle triangle = MMSFBTriangle(x1, y1, x2, y2, x3, y3);
			mmsfb->bei->drawTriangle(this, triangle);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			MMSFBTriangle triangle = MMSFBTriangle(x1, y1, x2, y2, x3, y3);
			mmsfb->bei->drawTriangle(this, triangle);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}
	else {
		// draw triangle
		drawLine(x1, y1, x2, y2);
		drawLine(x1, y1, x3, y3);
		drawLine(x2, y2, x3, y3);
		ret = true;
	}

    return ret;
}

bool MMSFBSurface::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
	bool ret = false;

    // check if initialized
    INITCHECK;

    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
	MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    MMSFB_BREAK();

	    // fill triangle
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->FillTriangle(this->dfb_surface, x1, y1, x2, y2, x3, y3)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::FillTriangle() failed");
				return false;
			}
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;
			x3+=this->sub_surface_xoff;
			y3+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			this->dfb_surface->FillTriangle(this->dfb_surface, x1, y1, x2, y2, x3, y3);

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif

		}

		ret = true;
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (!this->is_sub_surface) {

			MMSFBTriangle triangle = MMSFBTriangle(x1, y1, x2, y2, x3, y3);
			mmsfb->bei->fillTriangle(this, triangle);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			MMSFBTriangle triangle = MMSFBTriangle(x1, y1, x2, y2, x3, y3);
			mmsfb->bei->fillTriangle(this, triangle);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}
	else {
		//TODO
		ret = true;
	}

    return ret;
}

bool MMSFBSurface::drawCircle(int x, int y, int radius, int start_octant, int end_octant) {

    MMSFB_BREAK();

    /* check if initialized */
    INITCHECK;

    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
	MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

    /* draw circle */
    if (end_octant < start_octant) end_octant = start_octant;
    if ((start_octant<=4)&&(end_octant>=3))
        drawLine(x, y + radius, x, y + radius);
    if ((start_octant==0)||(end_octant==7))
        drawLine(x, y - radius, x, y - radius);
    if ((start_octant<=2)&&(end_octant>=1))
        drawLine(x + radius, y, x + radius, y);
    if ((start_octant<=6)&&(end_octant>=5))
        drawLine(x - radius, y, x - radius, y);
    int mr = radius * radius;
    int mx = 1;
    int my = (int) (sqrt(mr - 1) + 0.5);

    while (mx < my) {
        if ((start_octant<=0)&&(end_octant>=0))
            drawLine(x + mx, y - my, x + mx, y - my); /* octant 0 */
        if ((start_octant<=1)&&(end_octant>=1))
            drawLine(x + my, y - mx, x + my, y - mx); /* octant 1 */
        if ((start_octant<=2)&&(end_octant>=2))
            drawLine(x + my, y + mx, x + my, y + mx); /* octant 2 */
        if ((start_octant<=3)&&(end_octant>=3))
            drawLine(x + mx, y + my, x + mx, y + my); /* octant 3 */
        if ((start_octant<=4)&&(end_octant>=4))
            drawLine(x - mx, y + my, x - mx, y + my); /* octant 4 */
        if ((start_octant<=5)&&(end_octant>=5))
            drawLine(x - my, y + mx, x - my, y + mx); /* octant 5 */
        if ((start_octant<=6)&&(end_octant>=6))
            drawLine(x - my, y - mx, x - my, y - mx); /* octant 6 */
        if ((start_octant<=7)&&(end_octant>=7))
            drawLine(x - mx, y - my, x - mx, y - my); /* octant 7 */

        mx++;
        my = (int) (sqrt(mr - mx*mx) + 0.5);
    }

    if (mx == my) {
        if ((start_octant<=3)&&(end_octant>=2))
            drawLine(x + mx, y + my, x + mx, y + my);
        if ((start_octant<=1)&&(end_octant>=0))
            drawLine(x + mx, y - my, x + mx, y - my);
        if ((start_octant<=5)&&(end_octant>=4))
            drawLine(x - mx, y + my, x - mx, y + my);
        if ((start_octant<=7)&&(end_octant>=6))
            drawLine(x - mx, y - my, x - mx, y - my);
    }

    return true;
}



bool MMSFBSurface::setBlittingFlags(MMSFBBlittingFlags flags) {

    // check if initialized
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    if ((flags & MMSFB_BLIT_BLEND_ALPHACHANNEL)||(flags & MMSFB_BLIT_BLEND_COLORALPHA)) {
			// if we do alpha channel blitting, we have to change the default settings to become correct results
			if (this->config.surface_buffer->alphachannel)
				dfb_surface->SetSrcBlendFunction(dfb_surface,(DFBSurfaceBlendFunction)DSBF_ONE);
			else
				dfb_surface->SetSrcBlendFunction(dfb_surface,(DFBSurfaceBlendFunction)DSBF_SRCALPHA);
			dfb_surface->SetDstBlendFunction(dfb_surface,(DFBSurfaceBlendFunction)(DSBF_INVSRCALPHA));

			if (flags & MMSFB_BLIT_BLEND_COLORALPHA)
				 flags = (MMSFBBlittingFlags)(flags | MMSFB_BLIT_SRC_PREMULTCOLOR);
		}

		// set the blitting flags
		if ((dfbres=this->dfb_surface->SetBlittingFlags(this->dfb_surface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(flags))) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetBlittingFlags() failed");

			return false;
		}
#endif
	}

    // save the flags
    this->config.blittingflags = flags;

    return true;
}

bool MMSFBSurface::getBlittingFlags(MMSFBBlittingFlags *flags) {

	// check if initialized
    INITCHECK;

    // parameter given?
    if (!flags)
    	return false;

    // save the flags
    *flags = this->config.blittingflags;

    return true;
}

bool MMSFBSurface::extendedLock(MMSFBSurface *src, MMSFBSurfacePlanes *src_planes,
								MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes) {

	if (src) {
		memset(src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src->lock(MMSFB_LOCK_READ, src_planes, false);
		if (!src_planes->ptr) {
			return false;
		}
	}
	if (dst) {
		memset(dst_planes, 0, sizeof(MMSFBSurfacePlanes));
		dst->lock(MMSFB_LOCK_WRITE, dst_planes, false);
		if (!dst_planes->ptr) {
			if (src)
				src->unlock(false);
			return false;
		}
	}

	if (this->surface_invert_lock) {
		if (src_planes && dst_planes) {
			MMSFBSurfacePlanes t_planes;
			t_planes = *src_planes;
			*src_planes = *dst_planes;
			*dst_planes = t_planes;
		}
	}

	return true;
}

void MMSFBSurface::extendedUnlock(MMSFBSurface *src, MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes) {
	if (dst) {
		if (dst_planes) {
			// save the given dst_planes to the surface buffers array
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			sb->buffers[sb->currbuffer_write] = *dst_planes;
		}
		else {
			// dst_planes not given, reset the special flags
//			MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
			MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
		}
		dst->unlock(false);
	}
	if (src) {
		src->unlock(false);
	}
}


bool MMSFBSurface::printMissingCombination(string method, MMSFBSurface *source, MMSFBSurfacePlanes *src_planes,
										   MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
										   MMSFBBlittingFlags blittingflags) {
#ifdef  __HAVE_DIRECTFB__
	// failed, check if it must not
	if ((this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) && (!source || (source->allocated_by == MMSFBSurfaceAllocatedBy_dfb)))
		return false;
#endif

	// fatal error!!!
	// we use our own surface buffer handling, but we have not found a matching routine!!!
	// so print the missing combination and return true
	printf("DISKO: Missing following combination in method %s\n", method.c_str());
	if (source) {
		printf("  source type:               %s\n", (source->is_sub_surface)?"subsurface":"surface");
		switch (source->allocated_by) {
		case MMSFBSurfaceAllocatedBy_dfb:
			printf("  source memory:             managed by dfb\n");
			break;
		case MMSFBSurfaceAllocatedBy_malloc:
			printf("  source memory:             managed by disko\n");
			break;
		case MMSFBSurfaceAllocatedBy_xvimage:
			printf("  source memory:             managed by x11 (xvimage)\n");
			break;
		case MMSFBSurfaceAllocatedBy_ximage:
			printf("  source memory:             managed by x11 (ximage)\n");
			break;
		case MMSFBSurfaceAllocatedBy_ogl:
			printf("  source memory:             managed by opengl\n");
			break;
		}
		printf("  source pixelformat:        %s\n", getMMSFBPixelFormatString(source->config.surface_buffer->pixelformat).c_str());
		printf("  source premultiplied:      %s\n", (source->config.surface_buffer->premultiplied)?"yes":"no");
	}
	if (src_planes) {
		printf("  source type:               surface\n");
		printf("  source memory:             extern (0x%08lx, pitch=%d)\n", (unsigned long)src_planes->ptr, src_planes->pitch);
		if (src_planes->ptr2) {
			printf("                                    (0x%08lx, pitch=%d)\n",  (unsigned long)src_planes->ptr2, src_planes->pitch2);
			if (src_planes->ptr3)
				printf("                                    (0x%08lx, pitch=%d)\n",  (unsigned long)src_planes->ptr3, src_planes->pitch3);
		}
		printf("  source pixelformat:        %s\n", getMMSFBPixelFormatString(src_pixelformat).c_str());
	}
	printf("  destination type:          %s\n", (this->is_sub_surface)?"subsurface":"surface");
	switch (this->allocated_by) {
	case MMSFBSurfaceAllocatedBy_dfb:
		printf("  destination memory:        managed by dfb\n");
		break;
	case MMSFBSurfaceAllocatedBy_malloc:
		printf("  destination memory:        managed by disko\n");
		break;
	case MMSFBSurfaceAllocatedBy_xvimage:
		printf("  destination memory:        managed by x11 (xvimage)\n");
		break;
	case MMSFBSurfaceAllocatedBy_ximage:
		printf("  destination memory:        managed by x11 (ximage)\n");
		break;
	case MMSFBSurfaceAllocatedBy_ogl:
		printf("  destination memory:        managed by opengl\n");
		break;
	}
	printf("  destination pixelformat:   %s\n", getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat).c_str());
	printf("  destination premultiplied: %s\n", (this->config.surface_buffer->premultiplied)?"yes":"no");
	printf("  destination color:         r=%d, g=%d, b=%d, a=%d\n", this->config.color.r, this->config.color.g, this->config.color.b, this->config.color.a);
	if ((source)||(src_planes)) {
		printf("  blitting flags (%06x):  ", blittingflags);
		if (blittingflags == MMSFB_BLIT_NOFX)
			printf(" NOFX");
		if (blittingflags & MMSFB_BLIT_BLEND_ALPHACHANNEL)
			printf(" BLEND_ALPHACHANNEL");
		if (blittingflags & MMSFB_BLIT_BLEND_COLORALPHA)
			printf(" BLEND_COLORALPHA");
		if (blittingflags & MMSFB_BLIT_COLORIZE)
			printf(" COLORIZE");
		if (blittingflags & MMSFB_BLIT_SRC_PREMULTIPLY)
			printf(" SRC_PREMULTIPLY");
		if (blittingflags & MMSFB_BLIT_ANTIALIASING)
			printf(" ANTIALIASING");
		printf("\n");
	}
	else {
		printf("  drawing flags (%06x):   ", this->config.drawingflags);
		if (this->config.drawingflags == MMSFB_DRAW_NOFX)
			printf(" NOFX");
		if (this->config.drawingflags & MMSFB_DRAW_BLEND)
			printf(" BLEND");
		if (this->config.drawingflags & MMSFB_DRAW_SRC_PREMULTIPLY)
			printf(" SRC_PREMULTIPLY");
		printf("\n");
	}
	printf("*****\n");
	return true;
}




bool MMSFBSurface::extendedAccelBlitEx(MMSFBSurface *source,
									   MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									   MMSFBRectangle *src_rect, int x, int y, MMSFBBlittingFlags blittingflags) {

	MMSFBSurfacePlanes my_src_planes;
	if (source) {
		// premultiplied surface?
		if (!source->config.surface_buffer->premultiplied)
			return false;

		src_pixelformat = source->config.surface_buffer->pixelformat;
		src_width = (!source->root_parent)?source->config.w:source->root_parent->config.w;
		src_height = (!source->root_parent)?source->config.h:source->root_parent->config.h;

		// empty source planes
		memset(&my_src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src_planes = &my_src_planes;
	}

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = src_rect->x;
	int sy = src_rect->y;
	int sw = src_rect->w;
	int sh = src_rect->h;
	MMSFBRegion clipreg;
#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	if (x < clipreg.x1) {
		// left outside
		sx+= clipreg.x1 - x;
		sw-= clipreg.x1 - x;
		if (sw <= 0)
			return true;
		x = clipreg.x1;
	}
	else
	if (x > clipreg.x2)
		// right outside
		return true;
	if (y < clipreg.y1) {
		// top outside
		sy+= clipreg.y1 - y;
		sh-= clipreg.y1 - y;
		if (sh <= 0)
			return true;
		y = clipreg.y1;
	}
	else
	if (y > clipreg.y2)
		// bottom outside
		return true;
	if (x + sw - 1 > clipreg.x2)
		// to width
		sw = clipreg.x2 - x + 1;
	if (y + sh - 1 > clipreg.y2)
		// to height
		sh = clipreg.y2 - y + 1;

	// adjust x/y
	if (x < 0) {
		sx -= x;
		sw += x;
		x = 0;
	}
	if (y < 0) {
		sy -= y;
		sh += y;
		y = 0;
	}
	if ((sw <= 0)||(sh <= 0))
		return true;

	// extract antialiasing flag from blittingflags
//	bool antialiasing = (this->config.blittingflags & MMSFB_BLIT_ANTIALIASING);
//	MMSFBBlittingFlags blittingflags = this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING;

	// checking pixelformats...
	switch (src_pixelformat) {
#ifdef __HAVE_PF_ARGB__
	case MMSFB_PF_ARGB:
		// source is ARGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitARGBtoARGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoARGB_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGBtoARGB_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoAiRGB_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitARGBtoRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoRGB32_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGBtoRGB32_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with coloralpha
				return blitARGBtoRGB32_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitARGBtoRGB16(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoRGB16_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB3565) {
			// destination is ARGB3565
			switch (blittingflags) {
			case MMSFB_BLIT_NOFX:
				// convert without alpha channel
				return blitARGBtoARGB3565(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);

			case MMSFB_BLIT_BLEND_ALPHACHANNEL:
				// blitting with alpha channel
				return blitARGBtoARGB3565_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return blitARGBtoYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoYV12_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGBtoYV12_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB24) {
			// destination is RGB24
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return blitARGBtoRGB24(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoRGB24_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR24) {
			// destination is BGR24
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoBGR24_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGBtoBGR24_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR555) {
			// destination is BGR555
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGBtoBGR555_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB32__
	case MMSFB_PF_RGB32:
		// source is RGB32
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitRGB32toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA)) {
				// blitting with coloralpha
				return blitRGB32toRGB32_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB16__
	case MMSFB_PF_RGB16:
		// source is RGB16 (RGB565)
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitRGB16toRGB16(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitRGB16toARGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return blitRGB16toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_AiRGB__
	case MMSFB_PF_AiRGB:
		// source is AiRGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitAiRGBtoAiRGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitAiRGBtoAiRGB_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitAiRGBtoAiRGB_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitAiRGBtoRGB16(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitAiRGBtoRGB16_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_AYUV__
	case MMSFB_PF_AYUV:
		// source is AYUV
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AYUV) {
			// destination is AYUV
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitAYUVtoAYUV(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitAYUVtoAYUV_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitAYUVtoAYUV_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitAYUVtoRGB16(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitAYUVtoRGB16_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitAYUVtoYV12_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitAYUVtoYV12_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YV12__
	case MMSFB_PF_YV12:
		// source is YV12
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitYV12toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// convert without alpha channel
				return blitYV12toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_I420__
	case MMSFB_PF_I420:
		// source is I420
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_I420) {
			// destination is I420
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitI420toI420(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// convert without alpha channel
				return blitI420toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YUY2__
	case MMSFB_PF_YUY2:
		// source is YUY2
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YUY2) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitYUY2toYUY2(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// convert without alpha channel
				return blitYUY2toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB24__
	case MMSFB_PF_RGB24:
		// source is RGB24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB24) {
			// destination is RGB24
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitRGB24toRGB24(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				return blitRGB24toARGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// convert without alpha channel
				return blitRGB24toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// convert without alpha channel
				return blitRGB24toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_BGR24__
	case MMSFB_PF_BGR24:
		// source is BGR24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR24) {
			// destination is BGR24
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitBGR24toBGR24(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA)) {
				// blitting with coloralpha
				return blitBGR24toBGR24_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_ARGB3565__
	case MMSFB_PF_ARGB3565:
		// source is ARGB3565
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB3565) {
			// destination is ARGB3565
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				return blitARGB3565toARGB3565(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_ARGB4444__
	case MMSFB_PF_ARGB4444:
		// source is ARGB4444
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB4444) {
			// destination is ARGB4444
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				return blitARGB4444toARGB4444(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGB4444toARGB4444_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGB4444toARGB4444_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return blitARGB4444toRGB32_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return blitARGB4444toRGB32_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_BGR555__
	case MMSFB_PF_BGR555:
		// source is BGR555
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR555) {
			// destination is BGR555
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// blitting without alpha channel
				return blitBGR555toBGR555(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										x, y);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

	default:
		// does not match
		break;
	}


	// does not match
	return false;
}

bool MMSFBSurface::extendedAccelBlit(MMSFBSurface *source, MMSFBRectangle *src_rect,
										   int x, int y, MMSFBBlittingFlags blittingflags) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelBlitEx(source,
			                 NULL, MMSFB_PF_NONE, 0, 0,
			                 src_rect, x, y, blittingflags))
		return printMissingCombination("extendedAccelBlit()", source, NULL, MMSFB_PF_NONE, 0, 0, blittingflags);
	else
		return true;
}

bool MMSFBSurface::extendedAccelBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
										   MMSFBRectangle *src_rect, int x, int y, MMSFBBlittingFlags blittingflags) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelBlitEx(NULL,
							 src_planes, src_pixelformat, src_width, src_height,
			                 src_rect, x, y, blittingflags))
		return printMissingCombination("extendedAccelBlitBuffer()", NULL,
							src_planes, src_pixelformat, src_width, src_height, blittingflags);
	else
		return true;
}




bool MMSFBSurface::extendedAccelStretchBlitEx(MMSFBSurface *source,
											  MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
											  MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
											  MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	MMSFBSurfacePlanes my_src_planes;
	if (source) {
		// premultiplied surface?
		if (!source->config.surface_buffer->premultiplied)
			return false;

		src_pixelformat = source->config.surface_buffer->pixelformat;
		src_width = (!source->root_parent)?source->config.w:source->root_parent->config.w;
		src_height = (!source->root_parent)?source->config.h:source->root_parent->config.h;

		// empty source planes
		memset(&my_src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src_planes = &my_src_planes;
	}

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = src_rect->x;
	int sy = src_rect->y;
	int sw = src_rect->w;
	int sh = src_rect->h;
	int dx = dest_rect->x;
	int dy = dest_rect->y;
	int dw = dest_rect->w;
	int dh = dest_rect->h;
	int wf;
	int hf;
	if (!calc_dest_rect) {
		// calc factor
		wf = (dw<<16)/sw;
		hf = (dh<<16)/sh;
	}
	else {
		// have to calculate accurate factor based on surface dimensions
		wf = (this->config.w<<16)/src_width;
		hf = (this->config.h<<16)/src_height;
	}

//printf("sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);


	MMSFBRegion clipreg;
#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

//printf("cx1=%d,cy1=%d,cx2=%d,cy2=%d\n", clipreg.x1,clipreg.y1,clipreg.x2,clipreg.y2);


	if (dx < clipreg.x1) {
		// left outside
		sx+= ((clipreg.x1 - dx)<<16) / wf;
/*		sw-= (clipreg.x1 - dx) / wf;
		if (sw <= 0)
			return true;*/
		dw-= clipreg.x1 - dx;
		if (dw <= 0)
			return true;
		sw = (dw<<16) / wf;
		dx = clipreg.x1;
	}
	else
	if (dx > clipreg.x2)
		// right outside
		return true;
	if (dy < clipreg.y1) {
		// top outside
		sy+= ((clipreg.y1 - dy)<<16) / hf;
/*		sh-= (clipreg.y1 - dy) / hf;
		if (sh <= 0)
			return true;*/
		dh-= clipreg.y1 - dy;
		if (dh <= 0)
			return true;
		sh = (dh<<16) / hf;
		dy = clipreg.y1;
	}
	else
	if (dy > clipreg.y2)
		// bottom outside
		return true;
	if (dx + dw - 1 > clipreg.x2) {
		// to width
		dw = clipreg.x2 - dx + 1;
		sw = (dw<<16) / wf;
	}
	if (dy + dh - 1 > clipreg.y2) {
		// to height
		dh = clipreg.y2 - dy + 1;
		sh = (dh<<16) / hf;
	}
	if (sw<=0) sw = 1;
	if (sh<=0) sh = 1;
	if (dw<=0) dw = 1;
	if (dh<=0) dh = 1;

	if (calc_dest_rect) {
		// signal the following routines, that stretch factors have to based on surface dimensions
		dw=0;
		dh=0;
	}

//printf(">sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);

//if (source->is_sub_surface) {
//	sx+=source->sub_surface_xoff;
//	sy+=source->sub_surface_yoff;
//}

//printf("!sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);

//dy-=10;


	// extract antialiasing flag from blittingflags
	bool antialiasing = (this->config.blittingflags & MMSFB_BLIT_ANTIALIASING);
	MMSFBBlittingFlags blittingflags = this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING;


	// checking pixelformats...
	switch (src_pixelformat) {
#ifdef __HAVE_PF_ARGB__
	case MMSFB_PF_ARGB:
		// source is ARGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitARGBtoARGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return stretchBlitARGBtoARGB_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return stretchBlitARGBtoARGB_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return stretchBlitARGBtoRGB32_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB32__
	case MMSFB_PF_RGB32:
		// source is RGB32
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitRGB32toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB24__
	case MMSFB_PF_RGB24:
		// source is RGB24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitRGB24toARGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitRGB24toRGB32(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_AiRGB__
	case MMSFB_PF_AiRGB:
		// source is AiRGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitAiRGBtoAiRGB(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return stretchBlitAiRGBtoAiRGB_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return stretchBlitAiRGBtoAiRGB_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_AYUV__
	case MMSFB_PF_AYUV:
		// source is AYUV
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AYUV) {
			// destination is AYUV
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				return stretchBlitAYUVtoAYUV(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return stretchBlitAYUVtoAYUV_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return stretchBlitAYUVtoAYUV_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YV12__
	case MMSFB_PF_YV12:
		// source is YV12
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				return stretchBlitYV12toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_I420__
	case MMSFB_PF_I420:
		// source is I420
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				return stretchBlitI420toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YUY2__
	case MMSFB_PF_YUY2:
		// source is YUY2
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				return stretchBlitYUY2toYV12(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_ARGB4444__
	case MMSFB_PF_ARGB4444:
		// source is ARGB4444
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB4444) {
			// destination is ARGB4444
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				return stretchBlitARGB4444toARGB4444_BLEND(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				return stretchBlitARGB4444toARGB4444_BLEND_COLORALPHA(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB16__
	case MMSFB_PF_RGB16:
		// source is RGB16
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_NOFX))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL))) {
				// blitting without alpha channel
				return stretchBlitRGB16toRGB16(source, src_planes, src_pixelformat,
										src_width, src_height, sx, sy, sw, sh,
										dx, dy, dw, dh,
										antialiasing);
			}

			// does not match
			return false;
		}

		// does not match
		return false;
#endif

    default:
    	break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelStretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
											MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelStretchBlitEx(source,
			                        NULL, MMSFB_PF_NONE, 0, 0,
			                        src_rect, dest_rect,
			                        real_dest_rect, calc_dest_rect))
		return printMissingCombination("extendedAccelStretchBlit()", source,
				NULL, MMSFB_PF_NONE, 0, 0, this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING);
	else
		return true;
}

bool MMSFBSurface::extendedAccelStretchBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
												  MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
												  MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelStretchBlitEx(NULL,
							        src_planes, src_pixelformat, src_width, src_height,
							        src_rect, dest_rect,
							        real_dest_rect, calc_dest_rect))
		return printMissingCombination("extendedAccelStretchBlitBuffer()", NULL, src_planes,
								src_pixelformat, src_width, src_height, this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING);
	else
		return true;
}




bool MMSFBSurface::extendedAccelFillRectangleEx(int x, int y, int w, int h, MMSFBDrawingFlags drawingflags) {

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = x;
	int sy = y;
	int sw = w;
	int sh = h;
	int dst_height = (!this->root_parent)?this->config.h:this->root_parent->config.h;

	// calculate the color
	MMSFBColor color = this->config.color;
	if (drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
#ifdef __HAVE_PF_ARGB__
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleARGB(dst_height, sx, sy, sw, sh, color);
		}
		else
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			return fillRectangleARGB_BLEND(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_AYUV__
	case MMSFB_PF_AYUV:
		// destination is AYUV
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleAYUV(dst_height, sx, sy, sw, sh, color);
		}
		else
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			return fillRectangleAYUV_BLEND(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB32__
	case MMSFB_PF_RGB32:
		// destination is RGB32
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleRGB32(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB24__
	case MMSFB_PF_RGB24:
		// destination is RGB24
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleRGB24(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_RGB16__
	case MMSFB_PF_RGB16:
		// destination is RGB16 (RGB565)
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleRGB16(dst_height, sx, sy, sw, sh, color);
		}
		else
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			return fillRectangleRGB16_BLEND(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YV12__
	case MMSFB_PF_YV12:
		// destination is YV12
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleYV12(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_I420__
	case MMSFB_PF_I420:
		// destination is I420
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleI420(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_YUY2__
	case MMSFB_PF_YUY2:
		// destination is YUY2
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleYUY2(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_ARGB3565__
	case MMSFB_PF_ARGB3565:
		// destination is ARGB3565
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleARGB3565(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_ARGB4444__
	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleARGB4444(dst_height, sx, sy, sw, sh, color);
		}
		else
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			return fillRectangleARGB4444_BLEND(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_BGR24__
	case MMSFB_PF_BGR24:
		// destination is BGR24
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleBGR24(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

#ifdef __HAVE_PF_BGR555__
	case MMSFB_PF_BGR555:
		// destination is BGR555
		if   ((drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			return fillRectangleBGR555(dst_height, sx, sy, sw, sh, color);
		}

		// does not match
		return false;
#endif

	default:
		// does not match
		break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelFillRectangle(int x, int y, int w, int h, MMSFBDrawingFlags drawingflags) {

	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelFillRectangleEx(x, y, w, h, drawingflags))
		return printMissingCombination("extendedAccelFillRectangle()");
	else
		return true;
}




bool MMSFBSurface::extendedAccelDrawLineEx(int x1, int y1, int x2, int y2) {

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	MMSFBRegion clipreg;
	int dst_height = (!this->root_parent)?this->config.h:this->root_parent->config.h;

#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	// calculate the color
	MMSFBColor color = this->config.color;
	if (this->config.drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawline_argb(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				MMSFBPERF_STOP_MEASURING_DRAWLINE(this, x1, y1, x2, y2);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				// drawing with alpha channel
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawline_blend_argb(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				MMSFBPERF_STOP_MEASURING_DRAWLINE(this, x1, y1, x2, y2);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				extendedUnlock(NULL, this);
				return true;
			}
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawline_argb4444(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				MMSFBPERF_STOP_MEASURING_DRAWLINE(this, x1, y1, x2, y2);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_REGION(this, x1, y1, x2, y2);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	default:
		// does not match
		break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelDrawLine(int x1, int y1, int x2, int y2) {

	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelDrawLineEx(x1, y1, x2, y2))
		return printMissingCombination("extendedAccelDrawLine()");
	else
		return true;
}




bool MMSFBSurface::renderScene(MMS3D_VERTEX_ARRAY	**varrays,
							   MMS3D_INDEX_ARRAY	**iarrays,
							   MMS3D_MATERIAL		*materials,
							   MMSFBSurface			**textures,
							   MMS3D_OBJECT			**objects) {

    bool 		 ret = false;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

		if (!this->is_sub_surface) {

			mmsfb->bei->renderScene(this, varrays, iarrays, materials, textures, objects);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->renderScene(this, varrays, iarrays, materials, textures, objects);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}

	return ret;
}


bool MMSFBSurface::merge(MMSFBSurface *source1, MMSFBSurface *source2, MMSFBMergingMode mergingmode) {

    bool 		 ret = false;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

		if (!this->is_sub_surface) {

			mmsfb->bei->merge(this, source1, source2, mergingmode);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->merge(this, source1, source2, mergingmode);

			UNCLIPSUBSURFACE

			ret = true;
		}
#endif
	}

	return ret;
}



bool MMSFBSurface::checkBlittingStatus(bool src_opaque, bool src_transparent, int x, int y, int w, int h,
									   MMSFBRectangle &crect, MMSFBBlittingFlags &blittingflags) {

	if (src_transparent) {
    	// source is full transparent
    	if (this->config.blittingflags & MMSFB_BLIT_BLEND_ALPHACHANNEL) {
    		// nothing to draw
    		return false;
    	}
    }

    // check clipping region and calculate final rectangle
	if (!this->is_sub_surface) {
	    if (!calcClip(x, y, w, h, &crect)) {
	    	// rectangle described with x, y, w, h is outside of the surface or clipping rectangle
	    	return false;
	    }
	}
	else {
		bool outside = false;
		CLIPSUBSURFACE
		if (!calcClip(x + this->sub_surface_xoff, y + this->sub_surface_yoff, w, h, &crect)) {
			// rectangle described with x, y, w, h is outside of the surface or clipping rectangle
			outside = true;
		}
		UNCLIPSUBSURFACE
		if (outside) return false;
	}

    // set new opaque/transparent status and get blitting flags for it
    blittingflags = this->config.blittingflags;
    if (src_opaque) {
    	// source is an opaque buffer
    	// so remove the BLEND_ALPHACHANNEL flag if set
    	blittingflags = blittingflags & ~MMSFB_BLIT_BLEND_ALPHACHANNEL;
    	switch (blittingflags) {
		case MMSFB_BLIT_NOFX:
		case MMSFB_BLIT_COLORIZE:
			// note: we use this->config.surface_buffer->sbw and this->config.surface_buffer->sbh
			//       because this is the dimension of the real existing buffer
			if    ((crect.x <= 0) && (crect.y <= 0)
				&& (crect.x + crect.w >= this->config.surface_buffer->sbw)
				&& (crect.y + crect.h >= this->config.surface_buffer->sbh)) {
				// blit writes the whole destination surface
				MMSFBSURFACE_WRITE_BUFFER(this).opaque		= true;
				MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			}
			else {
				// let opaque status unchanged
				MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			}
			break;
		default:
			// after blitting surface is not opaque and not transparent
			MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
			MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
			break;
		}
    }
    else {
    	// source is semi-transparent
    	if (!(blittingflags & MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
			// after the blitting surface is not opaque
			MMSFBSURFACE_WRITE_BUFFER(this).opaque = false;
    	}

    	// after blitting surface is not transparent
		MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;
    }

    return true;
}

bool MMSFBSurface::checkBlittingStatus(MMSFBSurface *source, int x, int y, int w, int h,
									   MMSFBRectangle &crect, MMSFBBlittingFlags &blittingflags) {
	return checkBlittingStatus(MMSFBSURFACE_READ_BUFFER(source).opaque, MMSFBSURFACE_READ_BUFFER(source).transparent,
								x, y, w, h, crect, blittingflags);
}


bool MMSFBSurface::blit(MMSFBSurface *source, MMSFBRectangle *src_rect, int x, int y) {
    MMSFBRectangle src;
    bool 		 ret = false;

    // check if initialized
    INITCHECK;

    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = source->config.w;
         src.h = source->config.h;
    }

    // save opaque/transparent status
    bool opaque_saved		= MMSFBSURFACE_WRITE_BUFFER(this).opaque;
    bool transparent_saved	= MMSFBSURFACE_WRITE_BUFFER(this).transparent;

    // get final rectangle and new opaque/transparent status
    MMSFBRectangle crect;
    MMSFBBlittingFlags blittingflags;
    if (!checkBlittingStatus(source, x, y, src.w, src.h, crect, blittingflags)) {
    	// nothing to draw
    	return true;
    }

//printf(" blit to %d,%d,%d,%d    dest opaque = %d, %d\n", crect.x, crect.y, crect.w, crect.h,MMSFBSURFACE_WRITE_BUFFER(this).opaque,opaque_saved);

	// finalize previous clear
	finClear((MMSFBSURFACE_WRITE_BUFFER(this).opaque) ? &crect: NULL);

	if (this->allocated_by != MMSFBSurfaceAllocatedBy_dfb) {
		//TODO: implement changes for dfb too

		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}
		if (this->is_sub_surface) {
			src.x+= crect.x - this->sub_surface_xoff - x;
			src.y+= crect.y - this->sub_surface_yoff - y;
		}
		else {
			src.x+= crect.x - x;
			src.y+= crect.y - y;
		}
		src.w = crect.w;
		src.h = crect.h;
	}

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult    dfbres;
	    D_DEBUG_AT( MMS_Surface, "blit( %d,%d - %dx%d -> %d,%d ) <- %dx%d\n",
	                DFB_RECTANGLE_VALS(&src), x, y, this->config.w, this->config.h );
	    MMSFB_TRACE();

#ifndef USE_DFB_SUBSURFACE
		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}
#endif

	    // blit
		if (!this->is_sub_surface) {
			if (!extendedAccelBlit(source, &src, x, y, this->config.blittingflags))
				if ((dfbres=this->dfb_surface->Blit(this->dfb_surface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, x, y)) != DFB_OK) {
#ifndef USE_DFB_SUBSURFACE
					// reset source rectangle
					if (source->is_sub_surface) {
						src.x-=source->sub_surface_xoff;
						src.y-=source->sub_surface_yoff;
					}
#endif
					MMSFB_SetError(dfbres, "IDirectFBSurface::Blit() failed, src rect="
										   +iToStr(src.x)+","+iToStr(src.y)+","+iToStr(src.w)+","+iToStr(src.h)
										   +", dst="+iToStr(x)+","+iToStr(y));
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			SETSUBSURFACE_BLITTINGFLAGS;
#endif

			if (extendedAccelBlit(source, &src, x, y, this->config.blittingflags))
				ret = true;
			else
				if (this->dfb_surface->Blit(this->dfb_surface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, x, y) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_BLITTINGFLAGS;

			UNCLIPSUBSURFACE
#endif

		}
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		mmsfb->bei->blit(this, source, src, crect.x, crect.y, blittingflags);
		ret = true;
#endif
	}
	else {
		ret = extendedAccelBlit(source, &src, crect.x, crect.y, blittingflags);
	}

	if (!ret) {
		// restore opaque/transparent status
	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= opaque_saved;
	    MMSFBSURFACE_WRITE_BUFFER(this).transparent	= transparent_saved;
	}

    return ret;
}



bool MMSFBSurface::blitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							  MMSFBRectangle *src_rect, int x, int y, bool opaque) {
    MMSFBRectangle src;
    bool 		 ret = false;

    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = src_width;
         src.h = src_height;
    }

    // check if initialized
    INITCHECK;

    // save opaque/transparent status
    bool opaque_saved		= MMSFBSURFACE_WRITE_BUFFER(this).opaque;
    bool transparent_saved	= MMSFBSURFACE_WRITE_BUFFER(this).transparent;

    // get final rectangle and new opaque/transparent status
    MMSFBRectangle crect;
    MMSFBBlittingFlags blittingflags;
    if (!checkBlittingStatus(opaque, false, x, y, src.w, src.h, crect, blittingflags)) {
    	// nothing to draw
    	return true;
    }

	// finalize previous clear
    finClear((MMSFBSURFACE_WRITE_BUFFER(this).opaque) ? &crect: NULL);

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

		if (!this->is_sub_surface) {

			mmsfb->bei->blitBuffer(this, src_planes, src_pixelformat, src_width, src_height, src, x, y,
									blittingflags);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->blitBuffer(this, src_planes, src_pixelformat, src_width, src_height, src, x, y,
									blittingflags);

			UNCLIPSUBSURFACE

			ret = true;
		}

#endif
	}
	else {
		// blit buffer
		if (!this->is_sub_surface) {
			ret = extendedAccelBlitBuffer(src_planes, src_pixelformat, src_width, src_height, &src,
										  x, y, blittingflags);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			ret = extendedAccelBlitBuffer(src_planes, src_pixelformat, src_width, src_height, &src,
										  x, y, blittingflags);

			UNCLIPSUBSURFACE

		}
	}


	if (!ret) {
		// restore opaque/transparent status
	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= opaque_saved;
	    MMSFBSURFACE_WRITE_BUFFER(this).transparent	= transparent_saved;
	}


    return ret;
}

bool MMSFBSurface::blitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							  MMSFBRectangle *src_rect, int x, int y, bool opaque) {
	MMSFBSurfacePlanes src_planes = MMSFBSurfacePlanes(src_ptr, src_pitch);
	return blitBuffer(&src_planes, src_pixelformat, src_width, src_height, src_rect, x, y, opaque);
}

bool MMSFBSurface::stretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
							   MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
    MMSFBRectangle src;
    MMSFBRectangle dst;
    bool 		 ret = false;

    // use whole source surface if src_rect is not given
    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = source->config.w;
         src.h = source->config.h;
    }


    // use whole destination surface if dest_rect is not given and calc_dest_rect is not set
    if ((dest_rect)&&(!calc_dest_rect)) {
         dst = *dest_rect;
    }
    else {
    	if (!calc_dest_rect) {
			dst.x = 0;
			dst.y = 0;
			dst.w = this->config.w;
			dst.h = this->config.h;
    	}
    	else {
    		// calc dest_rect from src_rect based on src/dst surface dimension
    		dst.x = (src.x * this->config.w) / source->config.w;
    		dst.y = (src.y * this->config.h) / source->config.h;
    		dst.w = src.x + src.w - 1;
    		dst.h = src.y + src.h - 1;
    		dst.w = (dst.w * this->config.w) / source->config.w;
    		dst.h = (dst.h * this->config.h) / source->config.h;
    		dst.w = dst.w - dst.x + 1;
    		dst.h = dst.h - dst.y + 1;
    	}
    }

    // return the used destination rectangle
    if (real_dest_rect)
    	*real_dest_rect = dst;

    // check if i can blit without stretching
    if (src.w == dst.w && src.h == dst.h) {
        return blit(source, &src, dst.x, dst.y);
    }

    // check if initialized
    INITCHECK;


    // save opaque/transparent status
    bool opaque_saved		= MMSFBSURFACE_WRITE_BUFFER(this).opaque;
    bool transparent_saved	= MMSFBSURFACE_WRITE_BUFFER(this).transparent;

    // get final rectangle and new opaque/transparent status
    MMSFBRectangle crect;
    MMSFBBlittingFlags blittingflags;
    if (!checkBlittingStatus(source, dst.x, dst.y, dst.w, dst.h, crect, blittingflags)) {
    	// nothing to draw
    	return true;
    }

//printf(" stretch to %d,%d,%d,%d\n", crect.x, crect.y, crect.w, crect.h);


	// finalize previous clear
	finClear((MMSFBSURFACE_WRITE_BUFFER(this).opaque) ? &crect: NULL);

    //TODO: use crect for the following code...
	//...

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult    dfbres;
	    bool         blit_done = false;
	    D_DEBUG_AT( MMS_Surface, "stretchBlit( %d,%d - %dx%d  ->  %d,%d - %dx%d ) <- %dx%d\n",
	                DFB_RECTANGLE_VALS(&src), DFB_RECTANGLE_VALS(&dst), this->config.w, this->config.h);
	    MMSFB_BREAK();

#ifndef USE_DFB_SUBSURFACE
		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}
#endif

		if (this->config.blittingflags != MMSFB_BLIT_NOFX) {
			/* stretch blit with blitting flags */

			if (!this->is_sub_surface) {
				if (extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect)) {
					blit_done = true;
					ret = true;
				}
			}
			else {

#ifndef USE_DFB_SUBSURFACE
				CLIPSUBSURFACE

				dst.x+=this->sub_surface_xoff;
				dst.y+=this->sub_surface_yoff;

				SETSUBSURFACE_BLITTINGFLAGS;
#endif

				if (extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect)) {
					blit_done = true;
					ret = true;
				}


#ifndef USE_DFB_SUBSURFACE
				RESETSUBSURFACE_BLITTINGFLAGS;

				dst.x-=this->sub_surface_xoff;
				dst.y-=this->sub_surface_yoff;

				UNCLIPSUBSURFACE
#endif
			}

			if (!blit_done) {
				/* we use a temporary surface to separate the stretchblit from the extra blitting functions */
				MMSFBSurface *tempsuf = mmsfbsurfacemanager->getTemporarySurface(dst.w, dst.h);

				if (tempsuf) {
					MMSFBRectangle temp;
					temp.x=0;
					temp.y=0;
					temp.w=dst.w;
					temp.h=dst.h;

					dfbres = DFB_OK;
					dfbres=((IDirectFBSurface *)tempsuf->getDFBSurface())->StretchBlit((IDirectFBSurface *)tempsuf->getDFBSurface(), (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&temp);
					if (dfbres == DFB_OK) {
						if (!this->is_sub_surface) {
							if (extendedAccelBlit(tempsuf, &temp, dst.x, dst.y, this->config.blittingflags)) {
								blit_done = true;
								ret = true;
							}
							else
							if ((dfbres=this->dfb_surface->Blit(this->dfb_surface, (IDirectFBSurface *)tempsuf->getDFBSurface(), (DFBRectangle*)&temp, dst.x, dst.y)) == DFB_OK) {
								blit_done = true;
								ret = true;
							}
						}
						else {

#ifndef USE_DFB_SUBSURFACE
							CLIPSUBSURFACE

							dst.x+=this->sub_surface_xoff;
							dst.y+=this->sub_surface_yoff;

							SETSUBSURFACE_BLITTINGFLAGS;
#endif

							if (!extendedAccelBlit(tempsuf, &temp, dst.x, dst.y, this->config.blittingflags))
								this->dfb_surface->Blit(this->dfb_surface, (IDirectFBSurface *)tempsuf->getDFBSurface(), (DFBRectangle*)&temp, dst.x, dst.y);

#ifndef USE_DFB_SUBSURFACE
							RESETSUBSURFACE_BLITTINGFLAGS;

							dst.x-=this->sub_surface_xoff;
							dst.y-=this->sub_surface_yoff;

							UNCLIPSUBSURFACE
#endif

							blit_done = true;
							ret = true;

						}
					}

					mmsfbsurfacemanager->releaseTemporarySurface(tempsuf);
				}
			}
		}

		if (!blit_done) {
			// normal stretch blit
			if (!this->is_sub_surface) {
				dfbres = DFB_OK;
				if (!extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect))
					dfbres=this->dfb_surface->StretchBlit(this->dfb_surface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&dst);
				if (dfbres != DFB_OK) {
#ifndef USE_DFB_SUBSURFACE
					// reset source rectangle
					if (source->is_sub_surface) {
						src.x-=source->sub_surface_xoff;
						src.y-=source->sub_surface_yoff;
					}
#endif
					MMSFB_SetError(dfbres, "IDirectFBSurface::StretchBlit() failed");
					return false;
				}
				ret = true;
			}
			else {

#ifndef USE_DFB_SUBSURFACE
				CLIPSUBSURFACE

				dst.x+=this->sub_surface_xoff;
				dst.y+=this->sub_surface_yoff;

				SETSUBSURFACE_BLITTINGFLAGS;
#endif

				if (!extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect))
					this->dfb_surface->StretchBlit(this->dfb_surface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&dst);
				ret = true;

#ifndef USE_DFB_SUBSURFACE
				RESETSUBSURFACE_BLITTINGFLAGS;

				dst.x-=this->sub_surface_xoff;
				dst.y-=this->sub_surface_yoff;

				UNCLIPSUBSURFACE
#endif
			}
		}

#ifndef USE_DFB_SUBSURFACE
		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
#endif

#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

		if (!this->is_sub_surface) {

			mmsfb->bei->stretchBlit(this, source, src, dst, this->config.blittingflags);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->stretchBlit(this, source, src, dst, this->config.blittingflags);

			UNCLIPSUBSURFACE

			ret = true;
		}

#endif
	}
	else {

		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}

		// normal stretch blit
		if (!this->is_sub_surface) {
			ret = extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect);
		}
		else {
			CLIPSUBSURFACE

			dst.x+=this->sub_surface_xoff;
			dst.y+=this->sub_surface_yoff;

			ret = extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect);

			dst.x-=this->sub_surface_xoff;
			dst.y-=this->sub_surface_yoff;

			UNCLIPSUBSURFACE
		}

		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
	}

	if (!ret) {
		// restore opaque/transparent status
	    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= opaque_saved;
	    MMSFBSURFACE_WRITE_BUFFER(this).transparent	= transparent_saved;
	}

    return ret;
}

bool MMSFBSurface::stretchBlitBuffer(MMSFBExternalSurfaceBuffer *extbuf, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									 MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
									 MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
    MMSFBRectangle src;
    MMSFBRectangle dst;
    bool ret = false;

    // use whole source surface if src_rect is not given
    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = src_width;
         src.h = src_height;
    }



    // use whole destination surface if dest_rect is not given and calc_dest_rect is not set
    if ((dest_rect)&&(!calc_dest_rect)) {
         dst = *dest_rect;
    }
    else {
    	if (!calc_dest_rect) {
			dst.x = 0;
			dst.y = 0;
			dst.w = this->config.w;
			dst.h = this->config.h;
    	}
    	else {
    		// calc dest_rect from src_rect based on src/dst surface dimension
    		dst.x = (src.x * this->config.w) / src_width;
    		dst.y = (src.y * this->config.h) / src_height;
    		dst.w = src.x + src.w - 1;
    		dst.h = src.y + src.h - 1;
    		dst.w = (dst.w * this->config.w) / src_width;
    		dst.h = (dst.h * this->config.h) / src_height;
    		dst.w = dst.w - dst.x + 1;
    		dst.h = dst.h - dst.y + 1;
    	}
    }

    // return the used dest_rect
    if (real_dest_rect)
    	*real_dest_rect = dst;

    // check if i can blit without stretching
    if (src.w == dst.w && src.h == dst.h) {
        return blitBuffer(extbuf->ptr, extbuf->pitch, src_pixelformat, src_width, src_height, &src, dst.x, dst.y);
    }

    // check if initialized
    INITCHECK;


    //TODO: this reset is to be improved...
    MMSFBSURFACE_WRITE_BUFFER(this).opaque		= false;
    MMSFBSURFACE_WRITE_BUFFER(this).transparent	= false;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__

		if (!this->is_sub_surface) {

			mmsfb->bei->stretchBlitBuffer(this, extbuf, src_pixelformat, src_width, src_height,
											src, dst, this->config.blittingflags);

			ret = true;
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->stretchBlitBuffer(this, extbuf, src_pixelformat, src_width, src_height,
											src, dst, this->config.blittingflags);

			UNCLIPSUBSURFACE

			ret = true;
		}

#endif
	}
	else {
		/* normal stretch blit */
		if (!this->is_sub_surface) {
			ret = extendedAccelStretchBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, &dst,
												 real_dest_rect, calc_dest_rect);
		}
		else {
			CLIPSUBSURFACE

			dst.x+=this->sub_surface_xoff;
			dst.y+=this->sub_surface_yoff;

			ret = extendedAccelStretchBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, &dst,
												 real_dest_rect, calc_dest_rect);

			dst.x-=this->sub_surface_xoff;
			dst.y-=this->sub_surface_yoff;

			UNCLIPSUBSURFACE
		}

	}

    return ret;
}

bool MMSFBSurface::stretchBlitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									 MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
									 MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	MMSFBExternalSurfaceBuffer extbuf;
	memset(&extbuf, 0, sizeof(extbuf));
	extbuf.ptr = src_ptr;
	extbuf.pitch = src_pitch;
	return stretchBlitBuffer(&extbuf, src_pixelformat, src_width, src_height, src_rect, dest_rect,
						     real_dest_rect, calc_dest_rect);
}


void MMSFBSurface::processSwapDisplay(void *in_data, int in_data_len, void **out_data, int *out_data_len) {
#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;

	if (in_data_len >> 8) {
		MMSFBPERF_START_MEASURING;

		// vsync
		mmsfb->mmsfbdev->waitForVSync();

		MMSFBPERF_STOP_MEASURING_VSYNC(sb->mmsfbdev_surface);
	}

	MMSFBPERF_START_MEASURING;

	// swap display
	mmsfb->mmsfbdev->panDisplay(in_data_len & 0xff, sb->buffers[0].ptr);

	MMSFBPERF_STOP_MEASURING_SWAPDISPLAY(sb->mmsfbdev_surface);
#endif
}

void MMSFBSurface::swapDisplay(bool vsync) {
#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;

	if (sb->mmsfbdev_surface != this)
		return;

	// surface is the fb layer surface
	if (sb->numbuffers > 2) {
		// more than two buffers (e.g. TRIPLE buffering), so we should use non-blocked panning
		if (!this->fbdev_ts) {
			// init thread server for display panning
			this->fbdev_ts = new MMSThreadServer(100, "MMSThreadServer4MMSFBSurface", false);
			this->fbdev_ts->onProcessData.connect(sigc::mem_fun(this,&MMSFBSurface::processSwapDisplay));
			this->fbdev_ts->start();
		}

		// hard disable vsync
		// reason: difference between current and next buffer is minimal and both buffers are not the next write buffer
		// TODO: this can be hw dependent, then we have to change the code
		vsync = false;

		// trigger panning and return immediately
		this->fbdev_ts->trigger(NULL, sb->currbuffer_read | ((vsync)?0x100:0));
	}
	else
	if (sb->numbuffers == 2) {
		// two buffers (BACKVIDEO)
		if (vsync) {
			MMSFBPERF_START_MEASURING;

			// vsync
			mmsfb->mmsfbdev->waitForVSync();

			MMSFBPERF_STOP_MEASURING_VSYNC(sb->mmsfbdev_surface);
		}

		MMSFBPERF_START_MEASURING;

		// swap display
		mmsfb->mmsfbdev->panDisplay(sb->currbuffer_read, sb->buffers[0].ptr);

		MMSFBPERF_STOP_MEASURING_SWAPDISPLAY(sb->mmsfbdev_surface);
	}
#endif
}


bool MMSFBSurface::flip(MMSFBRegion *region) {

    // check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    if (region)
	         D_DEBUG_AT( MMS_Surface, "flip( %d,%d - %dx%d ) <- %dx%d\n",
	                     DFB_RECTANGLE_VALS_FROM_REGION(region), this->config.w, this->config.h );
	    else
	         D_DEBUG_AT( MMS_Surface, "flip( %d,%d - %dx%d ) <- %dx%d\n",
	                     0, 0, this->config.w, this->config.h, this->config.w, this->config.h );

	    MMSFB_TRACE();

	    DFBResult   dfbres;

#ifdef USE_DFB_WINMAN

		// flip
		if ((dfbres=this->dfb_surface->Flip(this->dfb_surface, region, this->flipflags)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::Flip() failed");

			return false;
		}

#endif

#ifdef USE_MMSFB_WINMAN

		// flip
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->Flip(this->dfb_surface, (DFBRegion*)region, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::Flip() failed");

				return false;
			}
		}
		else {
#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			MMSFBRegion myregion;
			if (!region) {
				myregion.x1 = 0;
				myregion.y1 = 0;
				myregion.x2 = this->config.w - 1;
				myregion.y2 = this->config.h - 1;
			}
			else
				myregion = *region;

			myregion.x1+=this->sub_surface_xoff;
			myregion.y1+=this->sub_surface_yoff;
			myregion.x2+=this->sub_surface_xoff;
			myregion.y2+=this->sub_surface_yoff;

			this->dfb_surface->Flip(this->dfb_surface, (DFBRegion*)&myregion, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags));

#else
			this->dfb_surface->Flip(this->dfb_surface, region, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags));
#endif

#ifndef USE_DFB_SUBSURFACE
			UNCLIPSUBSURFACE
#endif
		}

		if (this->config.iswinsurface) {
			// inform the window manager
			mmsfbwindowmanager->flipSurface(this, region);
		}
		else {
	    	if (this->is_sub_surface) {
				// sub surface, use the root parent surface
	    		if (this->root_parent->config.iswinsurface) {
	    			// inform the window manager, use the correct region
	    			MMSFBRegion reg;
	    			if (region)
	    				reg = *region;
	    			else {
	    				reg.x1=0;
	    				reg.y1=0;
	    				reg.x2=sub_surface_rect.w-1;
	    				reg.y2=sub_surface_rect.h-1;
	    			}
	    			reg.x1+=this->sub_surface_xoff;
	    			reg.y1+=this->sub_surface_yoff;
	    			reg.x2+=this->sub_surface_xoff;
	    			reg.y2+=this->sub_surface_yoff;
	    			mmsfbwindowmanager->flipSurface(this->root_parent, &reg);
	    		}
			}
		}

#endif

		return true;
#else
		return false;
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (this->config.surface_buffer->numbuffers > 1) {
			// currently we work with fbo frontbuffers only, ogl flips will only supported via glXSwapBuffers()

			//TODO...
		}

		if (!this->config.surface_buffer->ogl_fbo) {
			// this is the primary fbo, flip it to the xwindow
			// note: there is no chance to flip a region with glXSwapBuffers!!!
			mmsfb->bei->swap();
		}

		if (this->config.iswinsurface) {
			// inform the window manager
#ifdef  __HAVE_GLX__
			mmsfbwindowmanager->flipSurface(NULL);
#else
			mmsfbwindowmanager->flipSurface(this, region);
#endif
		}
		else {
	    	if (this->is_sub_surface) {
				// sub surface, use the root parent surface
	    		if (this->root_parent->config.iswinsurface) {

	    			// inform the window manager, use the correct region
#ifdef  __HAVE_GLX__
	    			mmsfbwindowmanager->flipSurface(NULL);
#else
	    			MMSFBRegion reg;
	    			if (region)
	    				reg = *region;
	    			else {
	    				reg.x1=0;
	    				reg.y1=0;
	    				reg.x2=sub_surface_rect.w-1;
	    				reg.y2=sub_surface_rect.h-1;
	    			}
	    			reg.x1+=this->sub_surface_xoff;
	    			reg.y1+=this->sub_surface_yoff;
	    			reg.x2+=this->sub_surface_xoff;
	    			reg.y2+=this->sub_surface_yoff;
	    			mmsfbwindowmanager->flipSurface(this->root_parent, &reg);
#endif
	    		}
			}
		}




		return true;
#else
		return false;
#endif
	}
	else {
		// flip my own surfaces
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (sb->numbuffers > 1) {
			// flip is only needed, if we have at least one backbuffer
			if (!this->is_sub_surface) {
				// not a subsurface
				if (!region) {
					// flip my buffers without blitting
					sb->currbuffer_read++;
					if (sb->currbuffer_read >= sb->numbuffers)
						sb->currbuffer_read = 0;
					sb->currbuffer_write++;
					if (sb->currbuffer_write >= sb->numbuffers)
						sb->currbuffer_write = 0;

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
					if (sb->mmsfbdev_surface == this) {
						// surface is the fb layer surface, so we have to swap the display
						swapDisplay(true);
					}
#endif
				}
				else {
					MMSFBRectangle src_rect;
					src_rect.x = region->x1;
					src_rect.y = region->y1;
					src_rect.w = region->x2 - region->x1 + 1;
					src_rect.h = region->y2 - region->y1 + 1;

					// check if region is equal to the whole surface
					if   ((src_rect.x == 0) && (src_rect.y == 0)
						&&(src_rect.w == this->config.w) && (src_rect.h == this->config.h)) {
						// yes, flip my buffers without blitting
						sb->currbuffer_read++;
						if (sb->currbuffer_read >= sb->numbuffers)
							sb->currbuffer_read = 0;
						sb->currbuffer_write++;
						if (sb->currbuffer_write >= sb->numbuffers)
							sb->currbuffer_write = 0;

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
						if (sb->mmsfbdev_surface == this) {
							// surface is the fb layer surface, so we have to swap the display
							swapDisplay(true);
						}
#endif
					}
					else {
						// blit region from write to read buffer of the same MMSFBSurface
						MMSFBBlittingFlags savedbf = this->config.blittingflags;
						this->config.blittingflags = (MMSFBBlittingFlags)MMSFB_BLIT_NOFX;

						this->surface_invert_lock = true;

						if (MMSFBSURFACE_READ_BUFFER(this).opaque)
							MMSFBSURFACE_READ_BUFFER(this).opaque = MMSFBSURFACE_WRITE_BUFFER(this).opaque;
						if (MMSFBSURFACE_READ_BUFFER(this).transparent)
							MMSFBSURFACE_READ_BUFFER(this).transparent = MMSFBSURFACE_WRITE_BUFFER(this).transparent;

						this->extendedAccelBlit(this, &src_rect, src_rect.x, src_rect.y, MMSFB_BLIT_NOFX);

						this->surface_invert_lock = false;

						this->config.blittingflags = savedbf;

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
						if (sb->mmsfbdev_surface == this) {
							// surface is the fb layer surface
							// there is no need to swapDisplay() because current read buffer (currbuffer_read) has NOT changed
							if (this->flipflags & MMSFB_FLIP_FLUSH) {
								// BUT: MMSFB_FLIP_FLUSH tells us, that we have to trigger an pan event to frame buffer driver
								swapDisplay(true);
							}
						}
#endif
					}
				}
			}
			else {
				CLIPSUBSURFACE

				MMSFBRectangle src_rect;
				if (!region) {
					src_rect.x = 0;
					src_rect.y = 0;
					src_rect.w = this->config.w;
					src_rect.h = this->config.h;
				}
				else {
					src_rect.x = region->x1;
					src_rect.y = region->y1;
					src_rect.w = region->x2 - region->x1 + 1;
					src_rect.h = region->y2 - region->y1 + 1;
				}

				src_rect.x+=this->sub_surface_xoff;
				src_rect.y+=this->sub_surface_yoff;

				// blit region from write to read buffer of the same MMSFBSurface
				MMSFBBlittingFlags savedbf = this->config.blittingflags;
				this->config.blittingflags = (MMSFBBlittingFlags)MMSFB_BLIT_NOFX;

				this->surface_invert_lock = true;

				if (MMSFBSURFACE_READ_BUFFER(this).opaque)
					MMSFBSURFACE_READ_BUFFER(this).opaque = MMSFBSURFACE_WRITE_BUFFER(this).opaque;
				if (MMSFBSURFACE_READ_BUFFER(this).transparent)
					MMSFBSURFACE_READ_BUFFER(this).transparent = MMSFBSURFACE_WRITE_BUFFER(this).transparent;

				this->extendedAccelBlit(this, &src_rect, src_rect.x, src_rect.y, MMSFB_BLIT_NOFX);

				this->surface_invert_lock = false;

				this->config.blittingflags = savedbf;

				UNCLIPSUBSURFACE
			}
		}

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
		if (sb->mmsfbdev_surface) {
			if (sb->mmsfbdev_surface != this) {
				// this surface is the backbuffer in system memory of the layer (BACKSYSTEM buffer mode)

				MMSFBPERF_START_MEASURING;

				// sync
				mmsfb->mmsfbdev->waitForVSync();

				MMSFBPERF_STOP_MEASURING_VSYNC(sb->mmsfbdev_surface);

				// put the image to the framebuffer
				if (!region) {
					// full
					sb->mmsfbdev_surface->blit(this, NULL, 0, 0);
				}
				else {
					// a few lines
					MMSFBRectangle rect;
					rect.x = 0;
					rect.y = region->y1;
					rect.w = this->config.w;
					rect.h = region->y2 - region->y1 + 1;
					sb->mmsfbdev_surface->blit(this, &rect, rect.x, rect.y);
				}
			}
		}
		else {
#endif

#ifdef __HAVE_XLIB__
		if (sb->x_image[0]) {
			// XSHM, put the image to the x-server
			if (!this->scaler) {
				// no scaler defined
				mmsfb->xlock.lock();
				XLockDisplay(mmsfb->x_display);
				if (!region) {
					// put whole image
					int dx = 0;
					int dy = 0;
					if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
						dx = (mmsfb->display_w - this->config.w) >> 1;
						dy = (mmsfb->display_h - this->config.h) >> 1;
					}
					if(this->layer) {
						//printf("before putimage window %d\n layerid %d\n this->config.w: %d\n this->config.h: %d\n", layer->x_window, layer->config.id, layer->config.w, layer->config.h);

						if ((mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO)) {
//TODO: change the ifdef, what to do if XRenderComposite not available?
#ifdef __HAVE_XV__

							MMSFBPERF_START_MEASURING;

							//put image to layer pixmap
							XShmPutImage(mmsfb->x_display, layer->pixmap, layer->x_gc, sb->x_image[sb->currbuffer_read],
										  0, 0, dx, dy,
										  layer->config.w, layer->config.h, False);

							double scale = (double)layer->x_window_w / layer->config.w;

							// Scaling matrix
							XTransform xform = {{
								{ XDoubleToFixed( 1 ), XDoubleToFixed( 0 ), XDoubleToFixed(     0 ) },
								{ XDoubleToFixed( 0 ), XDoubleToFixed( 1 ), XDoubleToFixed(     0 ) },
								{ XDoubleToFixed( 0 ), XDoubleToFixed( 0 ), XDoubleToFixed( scale ) }
							}};

							XRenderSetPictureTransform( mmsfb->x_display, layer->x_pixmap_pict, &xform );
							XRenderSetPictureFilter( mmsfb->x_display, layer->x_pixmap_pict, FilterBilinear, 0, 0 );


							//put render image
							  /* copy the pixmap content using XRender */
							XRenderComposite(mmsfb->x_display,
								   PictOpSrc,
								   layer->x_pixmap_pict,
								   None,
								   layer->x_window_pict,
								   0, 0,
								   0, 0,
								   0, 0,
								   layer->x_window_w, layer->x_window_h);

							//XFlush(mmsfb->x_display);
							XSync(mmsfb->x_display, False);

							MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(this, layer->config.w, layer->config.h);

#endif
						} else {

							MMSFBPERF_START_MEASURING;

							XShmPutImage(mmsfb->x_display, layer->x_window, layer->x_gc, sb->x_image[sb->currbuffer_read],
										  0, 0, dx, dy,
										  this->config.w, this->config.h, False);

							//XFlush(mmsfb->x_display);
							XSync(mmsfb->x_display, False);

							MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(this, this->config.w, this->config.h);
						}
					}
				}
				else {
					// put only a region
					MMSFBRegion myreg = *region;
					if (myreg.x1 < 0) myreg.x1 = 0;
					if (myreg.y1 < 0) myreg.y1 = 0;
					if (myreg.x2 >= this->config.w) myreg.x2 = this->config.w - 1;
					if (myreg.y2 >= this->config.h) myreg.y2 = this->config.h - 1;
					if ((myreg.x2 >= myreg.x1)&&(myreg.y2 >= myreg.y1)) {
						int dx = 0;
						int dy = 0;
						if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
							dx = (mmsfb->display_w - this->config.w) >> 1;
							dy = (mmsfb->display_h - this->config.h) >> 1;
						}
						if(this->layer) {
							//printf("before putimage region %d\n layerid %d\n this->config.w: %d\n this->config.h: %d\n", layer->x_window, layer->config.id, layer->config.w, layer->config.h);

							if ((mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO)) {
//TODO: change the ifdef, what to do if XRenderComposite not available?
#ifdef __HAVE_XV__

								MMSFBPERF_START_MEASURING;

								int ww = myreg.x2 - myreg.x1 + 1;
								int hh = myreg.y2 - myreg.y1 + 1;

								//put image to layer pixmap
								XShmPutImage(mmsfb->x_display, layer->pixmap, layer->x_gc, sb->x_image[sb->currbuffer_read],
											 myreg.x1, myreg.y1, myreg.x1 + dx, myreg.y1 + dy,
											 ww, hh, False);

								double scale = (double)layer->x_window_w / layer->config.w; // We'll scale the window to 50% of its original size

								// Scaling matrix
								XTransform xform = {{
									{ XDoubleToFixed( 1 ), XDoubleToFixed( 0 ), XDoubleToFixed(     0 ) },
									{ XDoubleToFixed( 0 ), XDoubleToFixed( 1 ), XDoubleToFixed(     0 ) },
									{ XDoubleToFixed( 0 ), XDoubleToFixed( 0 ), XDoubleToFixed( scale ) }
								}};

								XRenderSetPictureTransform( mmsfb->x_display, layer->x_pixmap_pict, &xform );
								XRenderSetPictureFilter( mmsfb->x_display, layer->x_pixmap_pict, FilterBilinear, 0, 0 );

								//put render image
								/* copy the pixmap content using XRender */
								XRenderComposite(mmsfb->x_display,
									   PictOpSrc,
									   layer->x_pixmap_pict,
									   None,
									   layer->x_window_pict,
									   0, 0,
									   0, 0,
									   0, 0,
									   layer->x_window_w, layer->x_window_h);


								//XFlush(mmsfb->x_display);
								XSync(mmsfb->x_display, False);

								MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(this, ww, hh);

#endif
							} else {

								MMSFBPERF_START_MEASURING;

								XShmPutImage(mmsfb->x_display, layer->x_window, layer->x_gc, sb->x_image[sb->currbuffer_read],
										  0, 0, dx, dy,
										  this->config.w, this->config.h, False);

								//XFlush(mmsfb->x_display);
								XSync(mmsfb->x_display, False);

								MMSFBPERF_STOP_MEASURING_XSHMPUTIMAGE(this, this->config.w, this->config.h);
							}
						}
					}
				}
				XUnlockDisplay(mmsfb->x_display);
				mmsfb->xlock.unlock();
			} else {
				//printf("do scaler....\n");

				// scale to scaler
				if (!region) {
					// scale whole image
					this->scaler->stretchBlit(this, NULL, NULL);
					this->scaler->flip();
				}
				else {
					// scale only a region
					MMSFBRegion myreg = *region;

					// enlarge the region because of little calulation errors while stretching
					myreg.x1--;
					myreg.y1--;
					myreg.x2++;
					myreg.y2++;

					// check region
					if (myreg.x1 < 0) myreg.x1 = 0;
					if (myreg.y1 < 0) myreg.y1 = 0;
					if (myreg.x2 >= this->config.w) myreg.x2 = this->config.w - 1;
					if (myreg.y2 >= this->config.h) myreg.y2 = this->config.h - 1;
					if ((myreg.x2 >= myreg.x1)&&(myreg.y2 >= myreg.y1)) {
						// stretch & flip to make it visible on the screen
						MMSFBRectangle src_rect;
						src_rect.x = myreg.x1;
						src_rect.y = myreg.y1;
						src_rect.w = myreg.x2 - myreg.x1 + 1;
						src_rect.h = myreg.y2 - myreg.y1 + 1;
						MMSFBRectangle dst_rect;
						this->scaler->stretchBlit(this, &src_rect, NULL, &dst_rect, true);
						myreg.x1 = dst_rect.x;
						myreg.y1 = dst_rect.y;
						myreg.x2 = dst_rect.x + dst_rect.w - 1;
						myreg.y2 = dst_rect.y + dst_rect.h - 1;
						this->scaler->flip(&myreg);
					}
				}
			}
		}
#ifdef __HAVE_XV__
		else
		if (sb->xv_image[0]) {
			// XVSHM, put the image to the x-server
			mmsfb->xlock.lock();
			XLockDisplay(mmsfb->x_display);
			if (mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
				// calc ratio
				MMSFBRectangle dest;
				calcAspectRatio(mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->display_w, mmsfb->display_h, dest,
								(mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO), true);

				MMSFBPERF_START_MEASURING;

				//printf("do vputimage\n");
				// put image
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  dest.x, dest.y, dest.w, dest.h, False);

				//XFlush(mmsfb->x_display);
				XSync(mmsfb->x_display, False);

				MMSFBPERF_STOP_MEASURING_XVSHMPUTIMAGE(this, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, dest.w, dest.h);
			}
			else if (mmsfb->resized) {

				MMSFBPERF_START_MEASURING;

				//printf("stretch to %d:%d\n",mmsfb->target_window_w, mmsfb->target_window_h);
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->target_window_w, mmsfb->target_window_h, False);

				//XFlush(mmsfb->x_display);
				XSync(mmsfb->x_display, False);

				MMSFBPERF_STOP_MEASURING_XVSHMPUTIMAGE(this, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->target_window_w, mmsfb->target_window_h);
			}
			else {
				/*printf("do vputimage2\n");
				printf("layer: %x, this: %x\n", layer, this);
				printf("sb->xv_image: %x\n", sb->xv_image[sb->currbuffer_read]);
*/

				MMSFBPERF_START_MEASURING;

				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, False);

				//XFlush(mmsfb->x_display);
				XSync(mmsfb->x_display, False);

				MMSFBPERF_STOP_MEASURING_XVSHMPUTIMAGE(this, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h);
			}

			XUnlockDisplay(mmsfb->x_display);
			mmsfb->xlock.unlock();
		}
#endif
#endif

#if defined(__HAVE_FBDEV__) || defined(__HAVE_L4_FB__)
		}
#endif

		if (this->config.iswinsurface) {
			// inform the window manager
			mmsfbwindowmanager->flipSurface(this, region);
		}
		else {
	    	if (this->is_sub_surface) {
				// sub surface, use the root parent surface
	    		if (this->root_parent->config.iswinsurface) {
	    			// inform the window manager, use the correct region
	    			MMSFBRegion reg;
	    			if (region)
	    				reg = *region;
	    			else {
	    				reg.x1=0;
	    				reg.y1=0;
	    				reg.x2=sub_surface_rect.w-1;
	    				reg.y2=sub_surface_rect.h-1;
	    			}
	    			reg.x1+=this->sub_surface_xoff;
	    			reg.y1+=this->sub_surface_yoff;
	    			reg.x2+=this->sub_surface_xoff;
	    			reg.y2+=this->sub_surface_yoff;
	    			mmsfbwindowmanager->flipSurface(this->root_parent, &reg);
	    		}
			}
		}

		return true;
	}
}


bool MMSFBSurface::flip(int x1, int y1, int x2, int y2) {
	MMSFBRegion region(x1, y1, x2, y2);
	return flip(&region);
}


bool MMSFBSurface::refresh() {
    // check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else {
#ifdef __HAVE_XLIB__
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (sb->x_image[0]) {
			// XSHM, put the image to the x-server
			if (!this->scaler) {
				// no scaler defined
				mmsfb->xlock.lock();
				XLockDisplay(mmsfb->x_display);
				int dx = 0;
				int dy = 0;
/*				if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
					dx = (mmsfb->display_w - this->config.w) >> 1;
					dy = (mmsfb->display_h - this->config.h) >> 1;
				}

				XShmPutImage(mmsfb->x_display, layer->x_window, layer->x_gc, sb->x_image[sb->currbuffer_read],
							  0, 0, dx, dy,
							  this->config.w, this->config.h, False);
*/

                if (mmsfb->fullscreen == MMSFB_FSM_TRUE) {
                        // put image to layer pixmap
                        XShmPutImage(mmsfb->x_display, layer->pixmap, layer->x_gc, sb->x_image[sb->currbuffer_read],
                                                  0, 0, dx, dy,
                                                  layer->config.w, layer->config.h, False);

                        double scale = (double)layer->x_window_w / layer->config.w;

                        // Scaling matrix
                        XTransform xform = {{
                                { XDoubleToFixed( 1 ), XDoubleToFixed( 0 ), XDoubleToFixed(     0 ) },
                                { XDoubleToFixed( 0 ), XDoubleToFixed( 1 ), XDoubleToFixed(     0 ) },
                                { XDoubleToFixed( 0 ), XDoubleToFixed( 0 ), XDoubleToFixed( scale ) }
                        }};

                        XRenderSetPictureTransform( mmsfb->x_display, layer->x_pixmap_pict, &xform );
                        XRenderSetPictureFilter( mmsfb->x_display, layer->x_pixmap_pict, FilterBilinear, 0, 0 );

                        //put render image, copy the pixmap content using XRender
                        XRenderComposite(mmsfb->x_display,
                                   PictOpSrc,
                                   layer->x_pixmap_pict,
                                   None,
                                   layer->x_window_pict,
                                   0, 0,
                                   0, 0,
                                   0, 0,
                                   layer->x_window_w, layer->x_window_h);

                } else {
                        if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
                                dx = (mmsfb->display_w - this->config.w) >> 1;
                                dy = (mmsfb->display_h - this->config.h) >> 1;
                        }

                        XShmPutImage(mmsfb->x_display, layer->x_window, layer->x_gc, sb->x_image[sb->currbuffer_read],
                                                  0, 0, dx, dy,
                                                  this->config.w, this->config.h, False);
                }

				//XFlush(mmsfb->x_display);
				XSync(mmsfb->x_display, False);
				XUnlockDisplay(mmsfb->x_display);
				mmsfb->xlock.unlock();
			}
			else {
				// scale to scaler
				this->scaler->stretchBlit(this, NULL, NULL);
				this->scaler->flip();
			}
		}
#ifdef __HAVE_XV__
		else
		if (sb->xv_image[0]) {
			// XVSHM, put the image to the x-server
			this->lock();
			mmsfb->xlock.lock();
			XLockDisplay(mmsfb->x_display);
			if (mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
				// calc ratio
				MMSFBRectangle dest;
				calcAspectRatio(mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->display_w, mmsfb->display_h, dest,
								(mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO), true);

				// put image
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  dest.x, dest.y, dest.w, dest.h, False);
			} else if(mmsfb->resized) {
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->target_window_w, mmsfb->target_window_h, False);
			}else{
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, layer->x_window, layer->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, False);
			}
			//XFlush(mmsfb->x_display);
			XSync(mmsfb->x_display, False);
			XUnlockDisplay(mmsfb->x_display);
			mmsfb->xlock.unlock();
			this->unlock();
		}
#endif
#endif
	}

	return true;
}

bool MMSFBSurface::createCopy(MMSFBSurface **dstsurface, int w, int h,
                              bool copycontent, bool withbackbuffer, MMSFBSurfacePixelFormat pixelformat) {

    // check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

    if (this->is_sub_surface)
    	return false;

    *dstsurface = NULL;

    if (!w) w = config.w;
    if (!h) h = config.h;

    // create new surface
    if (!mmsfb->createSurface(dstsurface, w, h, (pixelformat==MMSFB_PF_NONE)?this->config.surface_buffer->pixelformat:pixelformat,
                             (withbackbuffer)?this->config.surface_buffer->backbuffer:0,this->config.surface_buffer->systemonly)) {
        if (*dstsurface)
            delete *dstsurface;
        *dstsurface = NULL;
        return false;
    }

    if (copycontent) {
        // copy the content
        MMSFBRectangle dstrect;
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = w;
        dstrect.h = h;
        (*dstsurface)->setDrawingFlags((MMSFBDrawingFlags) MMSFB_DRAW_NOFX);
        (*dstsurface)->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_NOFX);
        (*dstsurface)->stretchBlit(this, NULL, &dstrect);
        if (withbackbuffer) {
            (*dstsurface)->flip();
        }
    }

    return true;
}

bool MMSFBSurface::resize(int w, int h) {

	// check old size, resize only if size changed
    int old_w, old_h;
    if (!getSize(&old_w, &old_h)) return false;
    if ((old_w == w) && (old_h == h)) return true;

	// finalize previous clear
	finClear();

    if (!this->is_sub_surface) {
        // normal surface
	    lock();

	    // create a copy
	    MMSFBSurface *dstsurface;
	    createCopy(&dstsurface, w, h, true, true);

		if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
		    D_DEBUG_AT( MMS_Surface, "resize( %dx%d -> %dx%d )\n",
		                this->config.w, this->config.h, w, h );

		    MMSFB_TRACE();

		    // move the dfb pointers
			IDirectFBSurface *s = this->dfb_surface;
			this->dfb_surface = dstsurface->dfb_surface;
			dstsurface->dfb_surface = s;

			// load the new configuration
			this->getConfiguration();
			dstsurface->getConfiguration();
#endif
		}
		else {
			// move the surface buffer data
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			this->config.surface_buffer = dstsurface->config.surface_buffer;
			dstsurface->config.surface_buffer = sb;

			// load the new configuration
			this->getConfiguration();
		}

	    // free dstsurface
	    delete dstsurface;

	    unlock();
	    return true;
    }
    else  {
    	// sub surface
	    MMSFBRectangle rect = this->sub_surface_rect;
	    rect.w = w;
	    rect.h = h;
	    return setSubSurface(&rect);

    }
}



void MMSFBSurface::modulateBrightness(MMSFBColor *color, unsigned char brightness) {

    /* full brightness? */
    if (brightness == 255) return;

    /* full darkness? */
    if (brightness == 0) {
        color->r = 0;
        color->g = 0;
        color->b = 0;
        return;
    }

    /* modulate the color */
    unsigned int bn = 100000 * (255-brightness);
    if (color->r > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->r;
        color->r = (5+((10 * (unsigned int)color->r) - (bn / i))) / 10;
    }
    if (color->g > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->g;
        color->g = (5+((10 * (unsigned int)color->g) - (bn / i))) / 10;
    }
    if (color->b > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->b;
        color->b = (5+((10 * (unsigned int)color->b) - (bn / i))) / 10;
    }
}

void MMSFBSurface::modulateOpacity(MMSFBColor *color, unsigned char opacity) {

    /* full opacity? */
    if (opacity == 255) return;

    /* complete transparent? */
    if (opacity == 0) {
        color->a = 0;
        return;
    }

    /* modulate the alpha value */
    unsigned int bn = 100000 * (255-opacity);
    if (color->a > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->a;
        color->a = (5+((10 * (unsigned int)color->a) - (bn / i))) / 10;
    }
}


bool MMSFBSurface::setBlittingFlagsByBrightnessAlphaAndOpacity(
                    unsigned char brightness, unsigned char alpha, unsigned char opacity) {
    MMSFBColor color;

    /* check if initialized */
    INITCHECK;

    /* modulate the opacity into the color */
    color.a = alpha;
    modulateOpacity(&color, opacity);

    /* set color for blitting */
    setColor(brightness, brightness, brightness, color.a);

    /* set blitting flags */
    if (brightness != 255) {
        if (color.a == 255)
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL));
        else
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
    }
    else {
        if (color.a == 255)
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL));
        else
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
    }

    return true;
}


bool MMSFBSurface::setBlittingFlagsByBrightnessAlphaAndOpacityAndSource(
                    unsigned char brightness, unsigned char alpha, unsigned char opacity,
                    MMSFBSurface *source) {
    MMSFBColor color;

    // check if initialized
    INITCHECK;

    // modulate the opacity into the color
    color.a = alpha;
    modulateOpacity(&color, opacity);

    // set color for blitting
    setColor(brightness, brightness, brightness, color.a);

    // set blitting flags
    if (!MMSFBSURFACE_READ_BUFFER(source).opaque) {
    	// source is not opaque
		if (brightness != 255) {
			if (color.a == 255)
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL));
			else
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
		}
		else {
			if (color.a == 255)
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL));
			else
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
		}
    }
    else {
		// source is opaque, so we do not need MMSFB_BLIT_BLEND_ALPHACHANNEL
		if (brightness != 255) {
			if (color.a == 255)
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE));
			else
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_COLORALPHA));
		}
		else {
			if (color.a == 255)
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_NOFX));
			else
				setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA));
		}
    }

    return true;
}



bool MMSFBSurface::setDrawingFlagsByAlpha(unsigned char alpha) {

    // check if initialized
    INITCHECK;

    // set the drawing flags
    if (this->config.surface_buffer->premultiplied) {
    	// premultiplied surface, have to premultiply the color
	    if (alpha == 255)
	        setDrawingFlags((MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY));
	    else
	        setDrawingFlags((MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY));
    }
    else {
	    if (alpha == 255)
	        setDrawingFlags((MMSFBDrawingFlags)MMSFB_DRAW_NOFX);
	    else
	        setDrawingFlags((MMSFBDrawingFlags)MMSFB_DRAW_BLEND);
    }

    return true;
}


bool MMSFBSurface::setDrawingColorAndFlagsByBrightnessAndOpacity(
                        MMSFBColor color, unsigned char brightness, unsigned char opacity) {

    // check if initialized
    INITCHECK;

    // modulate the brightness into the color
    modulateBrightness(&color, brightness);

    // modulate the opacity into the color
    modulateOpacity(&color, opacity);

    // set the color for drawing
    setColor(color.r, color.g, color.b, color.a);

    // set the drawing flags
    setDrawingFlagsByAlpha(color.a);

    return true;
}


bool MMSFBSurface::setDrawingColorAndFlagsByBrightnessAndOpacity(
                        MMSFBColor color,
                        MMSFBColor shadow_top_color, MMSFBColor shadow_bottom_color,
    					MMSFBColor shadow_left_color, MMSFBColor shadow_right_color,
                        MMSFBColor shadow_top_left_color, MMSFBColor shadow_top_right_color,
    					MMSFBColor shadow_bottom_left_color, MMSFBColor shadow_bottom_right_color,
                        unsigned char brightness, unsigned char opacity) {

	if (!setDrawingColorAndFlagsByBrightnessAndOpacity(color, brightness, opacity))
		return false;

    // modulate the brightness/opacity into the shadow colors
    modulateBrightness(&shadow_top_color, brightness);
    modulateOpacity(&shadow_top_color, opacity);
    modulateBrightness(&shadow_bottom_color, brightness);
    modulateOpacity(&shadow_bottom_color, opacity);
    modulateBrightness(&shadow_left_color, brightness);
    modulateOpacity(&shadow_left_color, opacity);
    modulateBrightness(&shadow_right_color, brightness);
    modulateOpacity(&shadow_right_color, opacity);
    modulateBrightness(&shadow_top_left_color, brightness);
    modulateOpacity(&shadow_top_left_color, opacity);
    modulateBrightness(&shadow_top_right_color, brightness);
    modulateOpacity(&shadow_top_right_color, opacity);
    modulateBrightness(&shadow_bottom_left_color, brightness);
    modulateOpacity(&shadow_bottom_left_color, opacity);
    modulateBrightness(&shadow_bottom_right_color, brightness);
    modulateOpacity(&shadow_bottom_right_color, opacity);

    // set the shadow colors
    return setShadowColor(shadow_top_color, shadow_bottom_color, shadow_left_color, shadow_right_color,
						  shadow_top_left_color, shadow_top_right_color, shadow_bottom_left_color, shadow_bottom_right_color);
}


bool MMSFBSurface::setFont(MMSFBFont *font) {

    /* check if initialized */
    INITCHECK;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set font */
		if ((dfbres=this->dfb_surface->SetFont(this->dfb_surface, (IDirectFBFont*)font->dfbfont)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetFont() failed");
			return false;
		}
#endif
	}
	else {
		//TODO
	}

    /* save the font */
    this->config.font = font;

    return true;
}



bool MMSFBSurface::blit_text(string &text, int len, int x, int y) {
	MMSFBRegion clipreg;
	MMSFBSurfacePlanes dst_planes;

#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	// calculate the color
	MMSFBColor color = this->config.color;
	if (this->config.drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawstring_blend_argb(
							&dst_planes, this->config.font, clipreg,
							text, len, x, y, color);
				MMSFBPERF_STOP_MEASURING_DRAWSTRING(this, clipreg, text, len, x, y);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawstring_blend_coloralpha_argb(
							&dst_planes, this->config.font, clipreg,
							text, len, x, y, color);
				MMSFBPERF_STOP_MEASURING_DRAWSTRING(this, clipreg, text, len, x, y);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawstring_blend_argb4444(
							&dst_planes, this->config.font, clipreg,
							text, len, x, y, color);
				MMSFBPERF_STOP_MEASURING_DRAWSTRING(this, clipreg, text, len, x, y);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	case MMSFB_PF_RGB16:
		// destination is RGB16
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawstring_blend_rgb16(
							&dst_planes, this->config.font, clipreg,
							text, len, x, y, color);
				MMSFBPERF_STOP_MEASURING_DRAWSTRING(this, clipreg, text, len, x, y);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				MMSFBPERF_START_MEASURING;
					mmsfb_drawstring_blend_coloralpha_rgb16(
							&dst_planes, this->config.font, clipreg,
							text, len, x, y, color);
				MMSFBPERF_STOP_MEASURING_DRAWSTRING(this, clipreg, text, len, x, y);
				MMSFB_ROTATE_180_REGION(this, clipreg.x1, clipreg.y1, clipreg.x2, clipreg.y2);
				MMSFB_ROTATE_180_RECT(this, x, y, 1, 1);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	default:
		// does not match
		break;
	}

	return printMissingCombination("blit_text()", NULL, NULL, MMSFB_PF_NONE, 0, 0);
}


bool MMSFBSurface::blit_text_with_shadow(string &text, int len, int x, int y) {

	bool top			= (this->config.shadow_top_color.a);
	bool bottom			= (this->config.shadow_bottom_color.a);
	bool left			= (this->config.shadow_left_color.a);
	bool right			= (this->config.shadow_right_color.a);
	bool top_left		= (this->config.shadow_top_left_color.a);
	bool top_right		= (this->config.shadow_top_right_color.a);
	bool bottom_left	= (this->config.shadow_bottom_left_color.a);
	bool bottom_right	= (this->config.shadow_bottom_right_color.a);
	bool shadow = (top || bottom || left || right || top_left || top_right || bottom_left || bottom_right);

	if (shadow) {
		// drawing color and flags will be temporary changed during the shadow blits
		MMSFBColor savedcol = this->config.color;
		MMSFBDrawingFlags saveddf = this->config.drawingflags;


		//TODO: for now we assume that this->config.color.a is 0xff!!!
		//      else we have to blit text and shadow in an temporary buffer with (text with a=0xff)
		//      and finally blend the result with coloralpha to this destination surface


		if (top) {
			// draw shadow on the top
			this->config.color = this->config.shadow_top_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x, y-1);
		}
		if (bottom) {
			// draw shadow on the bottom
			this->config.color = this->config.shadow_bottom_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x, y+1);
		}
		if (left) {
			// draw shadow on the left
			this->config.color = this->config.shadow_left_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x-1, y);
		}
		if (right) {
			// draw shadow on the right
			this->config.color = this->config.shadow_right_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x+1, y);
		}
		if (top_left) {
			// draw shadow on the top-left
			this->config.color = this->config.shadow_top_left_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x-1, y-1);
		}
		if (top_right) {
			// draw shadow on the top-right
			this->config.color = this->config.shadow_top_right_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x+1, y-1);
		}
		if (bottom_left) {
			// draw shadow on the bottom-left
			this->config.color = this->config.shadow_bottom_left_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x-1, y+1);
		}
		if (bottom_right) {
			// draw shadow on the bottom-right
			this->config.color = this->config.shadow_bottom_right_color;
			this->setDrawingFlagsByAlpha(this->config.color.a);
			blit_text(text, len, x+1, y+1);
		}

		// restore drawing color and flags
		this->config.color = savedcol;
		this->config.drawingflags = saveddf;

		// final blit
		return blit_text(text, len, x, y);
	}
	else {
		// blitting text without shadow
		return blit_text(text, len, x, y);
	}
}


bool MMSFBSurface::drawString(string text, int len, int x, int y) {

    // check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

    if (!this->config.font)
    	return false;

	// get the length of the string
	if (len < 0) len = text.size();
	if (!len) return true;

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
	    D_DEBUG_AT( MMS_Surface, "drawString( '%s', %d, %d,%d ) <- %dx%d\n",
	                text.c_str(), len, x, y, this->config.w, this->config.h );
	    MMSFB_TRACE();

	    // draw a string
	    DFBResult dfbres;
		if (!this->is_sub_surface) {
			if ((dfbres=this->dfb_surface->DrawString(this->dfb_surface, text.c_str(), len, x, y, DSTF_TOPLEFT)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::DrawString() failed");

				return false;
			}
		}
		else {
#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			this->dfb_surface->DrawString(this->dfb_surface, text.c_str(), len, x, y, DSTF_TOPLEFT);

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	else
	if (this->allocated_by == MMSFBSurfaceAllocatedBy_ogl) {
#ifdef  __HAVE_OPENGL__
		if (!this->is_sub_surface) {
			mmsfb->bei->drawString(this, text, len, x, y);
		}
		else {
			CLIPSUBSURFACE

			mmsfb->bei->drawString(this, text, len, x, y);

			UNCLIPSUBSURFACE
		}
#endif
	}
	else {
		// draw a string
		if (!this->is_sub_surface) {
			blit_text_with_shadow(text, len, x, y);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			blit_text_with_shadow(text, len, x, y);

			UNCLIPSUBSURFACE
		}
	}

    return true;
}

void MMSFBSurface::lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes, bool pthread_lock) {
	if (!pthread_lock) {
		// no pthread lock needed
		if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
			if (flags && planes) {
				// get the access to the surface buffer
				memset(planes, 0, sizeof(MMSFBSurfacePlanes));
				if (flags == MMSFB_LOCK_READ) {
					if (this->dfb_surface->Lock(this->dfb_surface, DSLF_READ, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
				}
				else
				if (flags == MMSFB_LOCK_WRITE) {
					if (this->dfb_surface->Lock(this->dfb_surface, DSLF_WRITE, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
				}
			}
#endif
		}
		else {
			if (flags && planes) {
				// get the access to the surface buffer
				memset(planes, 0, sizeof(MMSFBSurfacePlanes));
				MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
				if (flags == MMSFB_LOCK_READ) {
					*planes = sb->buffers[sb->currbuffer_read];
				}
				else
				if (flags == MMSFB_LOCK_WRITE) {
					*planes = sb->buffers[sb->currbuffer_write];
				}
			}
		}
		return;
	}

	// which surface is to lock?
	MMSFBSurface *tolock = this;
	if (this->root_parent)
		tolock = this->root_parent;
	else
	if (this->parent)
		tolock = this->parent;

    if (tolock->Lock.trylock() == 0) {
        // I have got the lock the first time
        tolock->TID = (long unsigned int)pthread_self();
    	tolock->Lock_cnt = 1;
    }
    else {
	if ((tolock->TID == (long unsigned int)pthread_self())&&(tolock->Lock_cnt > 0)) {
            // I am the thread which has already locked this surface
        	tolock->Lock_cnt++;
        }
        else {
            // another thread has already locked this surface, waiting for...
        	tolock->Lock.lock();
		tolock->TID = (long unsigned int)pthread_self();
        	tolock->Lock_cnt = 1;
        }
    }

	if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
		if (flags && planes) {
			// get the access to the surface buffer
			memset(planes, 0, sizeof(MMSFBSurfacePlanes));
			if (flags == MMSFB_LOCK_READ) {
				if (!tolock->surface_read_locked) {
					if (this->dfb_surface->Lock(this->dfb_surface, DSLF_READ, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
					else {
						tolock->surface_read_locked = true;
						tolock->surface_read_lock_cnt = tolock->Lock_cnt;
					}
				}
			}
			else
			if (flags == MMSFB_LOCK_WRITE) {
				if (!tolock->surface_write_locked) {
					if (this->dfb_surface->Lock(this->dfb_surface, DSLF_WRITE, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
					else {
						tolock->surface_write_locked = true;
						tolock->surface_write_lock_cnt = tolock->Lock_cnt;
					}
				}
			}
		}
#endif
	}
	else {
		if (flags && planes) {
			// get the access to the surface buffer
			memset(planes, 0, sizeof(MMSFBSurfacePlanes));
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			if (flags == MMSFB_LOCK_READ) {
				if (!tolock->surface_read_locked) {
					*planes = sb->buffers[sb->currbuffer_read];
					tolock->surface_read_locked = true;
					tolock->surface_read_lock_cnt = tolock->Lock_cnt;
				}
			}
			else
			if (flags == MMSFB_LOCK_WRITE) {
				if (!tolock->surface_write_locked) {
					*planes = sb->buffers[sb->currbuffer_write];
					tolock->surface_write_locked = true;
					tolock->surface_write_lock_cnt = tolock->Lock_cnt;
				}
			}
		}
	}
}

void MMSFBSurface::lock(MMSFBLockFlags flags, void **ptr, int *pitch) {
	if (!ptr || !pitch) {
		// nothing to return
		lock(flags, NULL, true);
	}
	else {
		// get the planes an return the first one
		MMSFBSurfacePlanes planes;
		lock(flags, &planes, true);
		*ptr = planes.ptr;
		*pitch = planes.pitch;
	}
}

void MMSFBSurface::lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes) {
	lock(flags, planes, true);
}

void MMSFBSurface::unlock(bool pthread_unlock) {
	if (!pthread_unlock) {
		// no pthread unlock needed
		if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
			this->dfb_surface->Unlock(this->dfb_surface);
#endif
		}
		return;
	}

	// which surface is to lock?
	MMSFBSurface *tolock = this;
	if (this->root_parent)
		tolock = this->root_parent;
	else
	if (this->parent)
		tolock = this->parent;

	if (tolock->TID != (long unsigned int)pthread_self())
        return;

    if (tolock->Lock_cnt==0)
    	return;

	// unlock dfb surface?
	if ((tolock->surface_read_locked)&&(tolock->surface_read_lock_cnt == tolock->Lock_cnt)) {
		if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
			this->dfb_surface->Unlock(this->dfb_surface);
#endif
		}
		tolock->surface_read_locked = false;
		tolock->surface_read_lock_cnt = 0;
	}
	else
	if ((tolock->surface_write_locked)&&(tolock->surface_write_lock_cnt == tolock->Lock_cnt)) {
		if (this->allocated_by == MMSFBSurfaceAllocatedBy_dfb) {
#ifdef  __HAVE_DIRECTFB__
			this->dfb_surface->Unlock(this->dfb_surface);
#endif
		}
		tolock->surface_write_locked = false;
		tolock->surface_write_lock_cnt = 0;
	}

    tolock->Lock_cnt--;

    if (tolock->Lock_cnt == 0)
    	tolock->Lock.unlock();
}

void MMSFBSurface::unlock() {
	unlock(true);
}

unsigned int MMSFBSurface::getNumberOfSubSurfaces() {
	return this->children.size();
}

MMSFBSurface *MMSFBSurface::getSubSurface(MMSFBRectangle *rect) {
#ifdef  __HAVE_DIRECTFB__
	IDirectFBSurface *subsuf = NULL;
#endif
    MMSFBSurface 	*surface;

    // check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

#ifdef USE_DFB_SUBSURFACE
    // get a sub surface
    DFBResult dfbres;
    if ((dfbres=this->dfb_surface->GetSubSurface(this->dfb_surface, rect, &subsuf)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBSurface::GetSubSurface() failed");
        return false;
    }

    // create a new surface instance
    surface = new MMSFBSurface(subsuf, this, rect);
#else
    // create a new surface instance
    surface = new MMSFBSurface(this, rect);
#endif

    if (!surface) {
#ifdef USE_DFB_SUBSURFACE
    	if (subsuf)
    		subsuf->Release(subsuf);
#endif
        MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
        return NULL;
    }

    // add to my list
    this->children.push_back(surface);

    return surface;
}

bool MMSFBSurface::setSubSurface(MMSFBRectangle *rect) {

	// check if initialized
    INITCHECK;

	// finalize previous clear
	finClear();

    // only sub surfaces can be moved
    if (!this->is_sub_surface)
		return false;

    lock();

    if (memcmp(rect, &(this->sub_surface_rect), sizeof(this->sub_surface_rect)) == 0) {
    	/* nothing changed */
    	unlock();
    	return false;
    }

#ifdef USE_DFB_SUBSURFACE
    /* because dfb has no IDirectFBSurface::setSubSurface(), allocate a new and release the old one */
    DFBResult dfbres;
    IDirectFBSurface *subsuf = NULL;
    if ((dfbres=this->parent->dfb_surface->GetSubSurface(this->parent->dfb_surface, rect, &subsuf)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBSurface::GetSubSurface() failed");
        unlock();
        return false;
    }

    if (this->dfb_surface)
    	this->dfb_surface->Release(this->dfb_surface);

    this->dfb_surface = subsuf;

#endif

    this->sub_surface_rect = *rect;

#ifndef USE_DFB_SUBSURFACE

    getRealSubSurfacePos(NULL, true);

#endif

    unlock();

    return true;
}

bool MMSFBSurface::setSubSurface(MMSFBRegion *region) {
	MMSFBRectangle rect;

	if (!region)
		return false;

	rect.x = region->x1;
	rect.y = region->y1;
	rect.w = region->x2 - region->x1 + 1;
	rect.h = region->y2 - region->y1 + 1;

	return setSubSurface(&rect);
}

bool MMSFBSurface::moveTo(int x, int y) {
	MMSFBRectangle rect;

	rect = this->sub_surface_rect;
	rect.x = x;
	rect.y = y;

	return setSubSurface(&rect);
}

bool MMSFBSurface::move(int x, int y) {
	MMSFBRectangle rect;

	rect = this->sub_surface_rect;
	rect.x += x;
	rect.y += y;

	return setSubSurface(&rect);
}


bool MMSFBSurface::dump2fcb(bool (*fcb)(char *, int, void *, int *), void *argp, int *argi,
						   int x, int y, int w, int h, MMSFBSurfaceDumpMode dumpmode) {
#define D2FCB_ADDSTR1(f) {int l=sprintf(ob,f);if(!fcb(ob,l,argp,argi)){this->unlock();return false;}}
#define D2FCB_ADDSTR2(f,v) {int l=sprintf(ob,f,v);if(!fcb(ob,l,argp,argi)){this->unlock();return false;}}
	// check inputs
	if (!fcb)
		return false;
	if ((x < 0)||(y < 0)||(w < 0)||(h < 0))
		return false;
	if (w == 0)
		w = this->config.w - x;
	if (h == 0)
		h = this->config.h - y;
	if ((x + w > this->config.w)||(y + h > this->config.h))
		return false;

	// finalize previous clear
	finClear();

	// set buffer
	char ob[65536];

	// get access to the surface memory
	unsigned char 	*sbuf;
	int				pitch;
	this->lock(MMSFB_LOCK_READ, (void**)&sbuf, &pitch);
	if (!sbuf)
		return false;

	// print format
	sprintf(ob, "* %s: x=%d, y=%d, w=%d, h=%d",
				getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat).c_str(),
				x, y, w, h);
	fcb(ob, strlen(ob), argp, argi);

	bool dumpok = false;
	if (dumpmode == MMSFBSURFACE_DUMPMODE_BYTE) {
		// dump byte-by-byte
		switch (this->config.surface_buffer->pixelformat) {
		case MMSFB_PF_I420:
		case MMSFB_PF_YV12:
		case MMSFB_PF_NV12:
		case MMSFB_PF_NV16:
		case MMSFB_PF_NV21:
		case MMSFB_PF_ARGB3565:
			// do not dump plane formats here
			break;
		default: {
				// all other formats
				MMSFBPixelDef pixeldef;
				getBitsPerPixel(this->config.surface_buffer->pixelformat, &pixeldef);
				int bits_pp = pixeldef.bits;
				int bytes_pp = bits_pp / 8;
				unsigned char *buf = sbuf + x * bytes_pp + y * pitch;
				D2FCB_ADDSTR1("\n* byte-by-byte ****************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch;
					D2FCB_ADDSTR2("\n%02x", buf[i++]);
					while (i < (w-x) * bytes_pp + j * pitch) {
						D2FCB_ADDSTR2(",%02x", buf[i]);
						i++;
					}
				}
				dumpok = true;
			}
			break;
		}
	}

	if (!dumpok) {
		// dump pixels
		switch (this->config.surface_buffer->pixelformat) {
		case MMSFB_PF_ARGB:
		case MMSFB_PF_RGB32: {
				int pitch_pix = pitch >> 2;
				unsigned int *buf = (unsigned int*)sbuf + x + y * pitch_pix;
				switch (this->config.surface_buffer->pixelformat) {
				case MMSFB_PF_ARGB:
					D2FCB_ADDSTR1("\n* aarrggbb hex (4-byte integer) ***********************************************");
					break;
				case MMSFB_PF_RGB32:
					D2FCB_ADDSTR1("\n* --rrggbb hex (4-byte integer) ***********************************************");
					break;
				default:
					break;
				}
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%08x", (int)buf[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%08x", (int)buf[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_BGR24: {
				D2FCB_ADDSTR1("\n* bbggrr hex (3-byte) *********************************************************");
				D2FCB_ADDSTR1("\nn/a");
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_RGB16:
		case MMSFB_PF_BGR555: {
				int pitch_pix = pitch >> 1;
				unsigned short int *buf = (unsigned short int*)sbuf + x + y * pitch_pix;
				switch (this->config.surface_buffer->pixelformat) {
				case MMSFB_PF_RGB16:
					D2FCB_ADDSTR1("\n* rrrrrggggggbbbbb bin (2-byte integer) ***************************************");
					break;
				case MMSFB_PF_BGR555:
					D2FCB_ADDSTR1("\n* 0bbbbbgggggrrrrr bin (2-byte integer) ***************************************");
					break;
				default:
					break;
				}
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%04x", buf[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%04x", buf[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_I420:
		case MMSFB_PF_YV12: {
				int pitch_pix = pitch;
				unsigned char *buf_y;
				unsigned char *buf_u = sbuf + pitch_pix * this->config.h + (x >> 1) + (y >> 1) * (pitch_pix >> 1);
				unsigned char *buf_v = sbuf + pitch_pix * (this->config.h + (this->config.h >> 2)) + (x >> 1) + (y >> 1) * (pitch_pix >> 1);
				if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
					buf_y = buf_u;
					buf_u = buf_v;
					buf_v = buf_y;
				}
				buf_y = sbuf + x + y * pitch_pix;
				D2FCB_ADDSTR1("\n* Y plane *********************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%02x", buf_y[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%02x", buf_y[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n* U plane *********************************************************************");
				x = x >> 1;
				y = y >> 1;
				w = w >> 1;
				h = h >> 1;
				for (int j = 0; j < h-y; j++) {
					int i = j * (pitch_pix >> 1);
					D2FCB_ADDSTR2("\n%02x", buf_u[i++]);
					while (i < (w-x) + j * (pitch_pix >> 1)) {
						D2FCB_ADDSTR2(",%02x", buf_u[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n* V plane *********************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * (pitch_pix >> 1);
					D2FCB_ADDSTR2("\n%02x", buf_v[i++]);
					while (i < (w-x) + j * (pitch_pix >> 1)) {
						D2FCB_ADDSTR2(",%02x", buf_v[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		default:
			// no dump routine for this pixelformat
			this->unlock();
			return false;
		}
	}

	// finalize
	this->unlock();
	D2FCB_ADDSTR1("\n");
	return true;
}

bool dump2buffer_fcb(char *buf, int len, void *argp, int *argi) {
	if (len >= *argi) return false;
	char *ap = *((char**)argp);
	memcpy(ap, buf, len);
	ap+= len;
	*((void**)argp) = ap;
	*argi = *argi - len;
	return true;
}

int MMSFBSurface::dump2buffer(char *out_buffer, int out_buffer_len, int x, int y, int w, int h,
							  MMSFBSurfaceDumpMode dumpmode) {
	int obl = out_buffer_len;
	if (dump2fcb(dump2buffer_fcb, (void*)(&out_buffer), &obl, x, y, w, h, dumpmode)) {
		out_buffer[out_buffer_len - obl] = 0;
		return out_buffer_len - obl;
	}
	return 0;
}

bool dump2file_fcb(char *buf, int len, void *argp, int *argi) {
	size_t ritems;
	((MMSFile *)argp)->writeBuffer(buf, &ritems, 1, len);
	return true;
}

bool MMSFBSurface::dump2file(string filename, int x, int y, int w, int h,
							 MMSFBSurfaceDumpMode dumpmode) {
	MMSFile *mmsfile = new MMSFile(filename, MMSFM_WRITE);
	if (mmsfile) {
		if (dump2fcb(dump2file_fcb, mmsfile, NULL, x, y, w, h, dumpmode)) {
			delete mmsfile;
			return true;
		}
		delete mmsfile;
	}
	return false;
}

bool MMSFBSurface::dump2file(string filename, MMSFBSurfaceDumpMode dumpmode) {
	return dump2file(filename, 0, 0, 0, 0, dumpmode);
}

bool dump_fcb(char *buf, int len, void *argp, int *argi) {
	buf[len] = 0;
	printf("%s", buf);
	return true;
}

bool MMSFBSurface::dump(int x, int y, int w, int h,
						MMSFBSurfaceDumpMode dumpmode) {
	if (dump2fcb(dump_fcb, NULL, NULL, x, y, w, h, dumpmode)) {
		printf("\n");
		return true;
	}
	return false;
}

bool MMSFBSurface::dump(MMSFBSurfaceDumpMode dumpmode) {
	return dump(0, 0, 0, 0, dumpmode);
}

bool mmsfb_create_cached_surface(MMSFBSurface **cs, int width, int height,
								 MMSFBSurfacePixelFormat pixelformat) {
	if (!cs) return false;

	// check the properties of the existing surface
	if (*cs) {
		// check if old surface has the same dimension
		int w, h;
        (*cs)->getSize(&w, &h);
        if ((w != width) || (h != height)) {
        	delete *cs;
        	*cs = NULL;
        }
	}

	if (*cs) {
		// check if old surface has the same pixelformat
		MMSFBSurfacePixelFormat pf;
        (*cs)->getPixelFormat(&pf);
        if (pf != pixelformat) {
        	delete *cs;
        	*cs = NULL;
        }
	}

	if (!*cs) {
		// create new surface
		*cs = new MMSFBSurface(width, height, pixelformat);
	}

	return (*cs);
}





bool MMSFBSurface::blitARGBtoARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoARGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoARGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}


bool MMSFBSurface::blitARGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_airgb(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}


bool MMSFBSurface::blitARGBtoRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB32_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB32_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_coloralpha_argb_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_rgb16(src_planes, src_height,
									 sx, sy, sw, sh,
									 (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									 x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_rgb16(src_planes, src_height,
										   sx, sy, sw, sh,
										   &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										   x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoARGB3565(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_ARGB3565__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_argb3565(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoARGB3565_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_ARGB3565__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_argb3565(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}


bool MMSFBSurface::blitARGBtoYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoYV12_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoYV12_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb_to_rgb24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoRGB24_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_rgb24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoBGR24_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_BGR24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_bgr24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoBGR24_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_BGR24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb_to_bgr24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGBtoBGR555_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_BGR555__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb_to_bgr555(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitRGB32toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb32_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitRGB32toRGB32_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_coloralpha_rgb32_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitRGB16toRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb16_to_rgb16(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitRGB16toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB16__
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb16_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitRGB16toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB16__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb16_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAiRGBtoAiRGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_airgb_to_airgb(src_planes, src_height,
									  sx, sy, sw, sh,
									  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAiRGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_airgb_to_airgb(src_planes, src_height,
											sx, sy, sw, sh,
											(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAiRGBtoAiRGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_airgb_to_airgb(src_planes, src_height,
													   sx, sy, sw, sh,
													   (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
													   x, y,
													   this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAiRGBtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AiRGB__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_airgb_to_rgb16(src_planes, src_height,
									  sx, sy, sw, sh,
									  (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAiRGBtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AiRGB__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_airgb_to_rgb16(src_planes, src_height,
											sx, sy, sw, sh,
											(unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoAYUV(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_ayuv_to_ayuv(src_planes, src_height,
									sx, sy, sw, sh,
									&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoAYUV_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_ayuv_to_ayuv(src_planes, src_height,
										  sx, sy, sw, sh,
										  (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoAYUV_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_ayuv_to_ayuv(src_planes, src_height,
													 sx, sy, sw, sh,
													 (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
													 x, y,
													 this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_ayuv_to_rgb16(src_planes, src_height,
									 sx, sy, sw, sh,
									 (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									 x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoRGB16_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_ayuv_to_rgb16(src_planes, src_height,
										   sx, sy, sw, sh,
										   (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										   x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoYV12_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_ayuv_to_yv12(src_planes, src_height,
										  sx, sy, sw, sh,
										  (unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitAYUVtoYV12_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_AYUV__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_ayuv_to_yv12(src_planes, src_height,
													 sx, sy, sw, sh,
													 (unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
													 x, y,
													 this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitYV12toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_yv12_to_yv12(src_planes, src_height,
									sx, sy, sw, sh,
									&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitYV12toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_YV12__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_yv12_to_rgb32(src_planes, src_height,
									 sx, sy, sw, sh,
									 (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									 x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitI420toI420(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_I420__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_i420_to_i420(src_planes, src_height,
									sx, sy, sw, sh,
									&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitI420toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_I420__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_i420_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitYUY2toYUY2(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_YUY2__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_yuy2_to_yuy2(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitYUY2toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_YUY2__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_yuy2_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitRGB24toRGB24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb24_to_rgb24(src_planes, src_height,
									  sx, sy, sw, sh,
									  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitRGB24toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB24__
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb24_to_argb(src_planes, src_height,
									sx, sy, sw, sh,
									&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitRGB24toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB24__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb24_to_rgb32(src_planes, src_height,
									  sx, sy, sw, sh,
									  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
									  x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitRGB24toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_RGB24__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_rgb24_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitBGR24toBGR24(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_BGR24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_bgr24_to_bgr24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitBGR24toBGR24_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_BGR24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_coloralpha_bgr24_to_bgr24(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGB3565toARGB3565(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB3565__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb3565_to_argb3565(src_planes, src_height,
											sx, sy, sw, sh,
											&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGB4444toARGB4444(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_argb4444_to_argb4444(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGB4444toARGB4444_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb4444_to_argb4444(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGB4444toARGB4444_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb4444_to_argb4444(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::blitARGB4444toRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB4444__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_argb4444_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitARGB4444toRGB32_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_ARGB4444__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_blend_coloralpha_argb4444_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::blitBGR555toBGR555(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int x, int y) {
#ifdef __HAVE_PF_BGR555__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		MMSFBPERF_START_MEASURING;
			mmsfb_blit_bgr555_to_bgr555(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					x, y);
		MMSFBPERF_STOP_MEASURING_BLIT(this, src_pixelformat, sw, sh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, x, y, sw, sh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}



bool MMSFBSurface::stretchBlitARGBtoARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}


bool MMSFBSurface::stretchBlitARGBtoARGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitARGBtoARGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_coloralpha_argb_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitARGBtoRGB32_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_argb_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::stretchBlitRGB32toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_rgb32_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitRGB24toARGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_RGB24__
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_rgb24_to_argb(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::stretchBlitRGB24toRGB32(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_RGB24__
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_rgb24_to_rgb32(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif
#endif

	return false;
}


bool MMSFBSurface::stretchBlitAiRGBtoAiRGB(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_airgb_to_airgb(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitAiRGBtoAiRGB_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_airgb_to_airgb(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitAiRGBtoAiRGB_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AiRGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_coloralpha_airgb_to_airgb(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitAYUVtoAYUV(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_ayuv_to_ayuv(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitAYUVtoAYUV_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_ayuv_to_ayuv(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitAYUVtoAYUV_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_coloralpha_ayuv_to_ayuv(
					src_planes, src_height,
					sx, sy, sw, sh,
					(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitYV12toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_yv12_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitI420toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_I420__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_i420_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}

bool MMSFBSurface::stretchBlitYUY2toYV12(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_YUY2__
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_yuy2_to_yv12(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);
		return true;
	}
#endif
#endif

	return false;
}


bool MMSFBSurface::stretchBlitARGB4444toARGB4444_BLEND(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_argb4444_to_argb4444(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}


bool MMSFBSurface::stretchBlitARGB4444toARGB4444_BLEND_COLORALPHA(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_blend_coloralpha_argb4444_to_argb4444(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					this->config.color.a);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::stretchBlitRGB16toRGB16(MMSFBSurface *source, MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat,
									int src_width, int src_height, int sx, int sy, int sw, int sh,
									int dx, int dy, int dw, int dh,
									bool antialiasing) {
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(source, src_planes, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_stretchblit_rgb16_to_rgb16(
					src_planes, src_height,
					sx, sy, sw, sh,
					&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
					dx, dy, dw, dh,
					antialiasing);
		MMSFBPERF_STOP_MEASURING_STRETCHBLIT(this, src_pixelformat, sw, sh, dw, dh);
		MMSFB_ROTATE_180_RECT_WH(src_width, src_height, sx, sy, sw, sh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(source, this);

		return true;
	}
#endif

	return false;
}


bool MMSFBSurface::fillRectangleARGB(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_argb(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this, &dst_planes);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleARGB_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_ARGB__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_blend_argb(&dst_planes, dst_height,
										   dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this, &dst_planes);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleAYUV(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_ayuv(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleAYUV_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_AYUV__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_blend_ayuv(&dst_planes, dst_height,
										   dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleRGB32(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_RGB32__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_rgb32(&dst_planes, dst_height,
									  dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleRGB24(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_RGB24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_rgb24(&dst_planes, dst_height,
									  dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleRGB16(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_rgb16(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this, &dst_planes);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleRGB16_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_RGB16__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_blend_rgb16(&dst_planes, dst_height,
										   dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this, &dst_planes);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleYV12(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_YV12__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_yv12(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleI420(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_I420__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_i420(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleYUY2(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_YUY2__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_yuy2(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleARGB3565(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_ARGB3565__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_argb3565(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleARGB4444(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_argb4444(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleARGB4444_BLEND(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_ARGB4444__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_blend_argb4444(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleBGR24(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_BGR24__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_bgr24(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

bool MMSFBSurface::fillRectangleBGR555(int dst_height, int dx, int dy, int dw, int dh, MMSFBColor &color) {
#ifdef __HAVE_PF_BGR555__
	MMSFBSurfacePlanes dst_planes;

	if (extendedLock(NULL, NULL, this, &dst_planes)) {
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		MMSFBPERF_START_MEASURING;
			mmsfb_fillrectangle_bgr555(&dst_planes, dst_height,
									 dx, dy, dw, dh, color);
		MMSFBPERF_STOP_MEASURING_FILLRECT(this, dw, dh);
		MMSFB_ROTATE_180_RECT(this, dx, dy, dw, dh);
		extendedUnlock(NULL, this);
		return true;
	}
#endif

	return false;
}

