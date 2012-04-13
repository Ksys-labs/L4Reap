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

#include "mmsgui/fb/mmsfbsurfacemanager.h"
#include "mmsgui/fb/mmsfb.h"

/* initialize the mmsfbsurfacemanager object */
MMSFBSurfaceManager *mmsfbsurfacemanager = new MMSFBSurfaceManager();

MMSFBSurfaceManager::MMSFBSurfaceManager() {
    this->tempsuf = NULL;
    this->surface_mem_cnt = 0;
    pthread_mutex_init(&this->surface_mem_cnt_lock, NULL);
}

MMSFBSurfaceManager::~MMSFBSurfaceManager() {
	pthread_mutex_destroy(&this->surface_mem_cnt_lock);
}

MMSFBSurface *MMSFBSurfaceManager::createSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, bool systemonly) {
//    DFBResult               dfbres;
//    IDirectFBSurface        *dfbsurface;
//    DFBSurfaceDescription   surface_desc;
    MMSFBSurface            *surface;

#if 0
    /* searching for free surface */
    for (unsigned int i = 0; i < this->free_surfaces.size(); i++) {
        surface = free_surfaces.at(i).surface;
        MMSFBSurfaceBuffer *sb = surface->config.surface_buffer;
        if   ((surface->config.w == w) && (surface->config.h == h)
            &&(sb->pixelformat == pixelformat) && (sb->backbuffer == backbuffer)) {
            /* found, return it */
            this->free_surfaces.erase(this->free_surfaces.begin()+i);
/*TRACE
            this->used_surfaces.push_back(surface);
*/

//DEBUGOUT("reuse surface=%d,%d\n", w,h);

            return surface;
        }
        else {
            // this surface is not the right one, check the timestamp
            if (free_surfaces.at(i).insert_time < time(NULL) - 3) {
                // the surface is longer than 3 seconds in the free_surfaces list, remove it
            	// TODO: Rewrite memory handling while porting to PXA...
				surface->freeSurfaceBuffer();
                delete surface;
                this->free_surfaces.erase(this->free_surfaces.begin()+i);
            }
        }
    }
#endif

    /* create a new surface instance */
    surface = new MMSFBSurface(w, h, pixelformat, backbuffer, systemonly);
    if (!surface) {
        MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
        return NULL;
    }
    if (!surface->isInitialized()) {
    	delete surface;
    	surface = NULL;
		MMSFB_SetError(0, "cannot initialize MMSFBSurface");
		return false;
	}

    // get size of surface memory
    int size, bnum;
    surface->getMemSize(&size);
    surface->getNumberOfBuffers(&bnum);
    DEBUGMSG("MMSGUI", "New surface memory allocated: "
							+ iToStr(size) + " byte, "
							+ iToStr(bnum) + " buffer(s), "
							+ iToStr(size/(bnum)) + " byte for each buffer");

    // add size of the surface to my global counter
	pthread_mutex_lock(&this->surface_mem_cnt_lock);
    this->surface_mem_cnt+=size;
	pthread_mutex_unlock(&this->surface_mem_cnt_lock);
    DEBUGMSG("MMSGUI", "Sum of allocated surface memory: " + iToStr(this->surface_mem_cnt) + " byte");

    /* add to used surfaces */
/* TRACE
    this->used_surfaces.push_back(surface);
*/

    return surface;
}

void MMSFBSurfaceManager::releaseSurface(MMSFBSurface *surface) {

       /* surface->dfbsurface->Release(surface->dfbsurface);
return;*/



//    MMSFBSurface        *new_surface;
//    MMSFBSURMANLIST     sml;

    if (!surface)
        return;

    if (surface->config.islayersurface)
        return;

	if (surface->is_sub_surface)
        return;

	surface->freeSurfaceBuffer();

#if 0
    /* create a new surface instance */
    new_surface = new MMSFBSurface(NULL);
    if (!new_surface) {
		surface->freeSurfaceBuffer();
        return;
    }

    /* set values to new surface */
    new_surface->llsurface = surface->llsurface;
    new_surface->config = surface->config;

    /* add to free surfaces */
    sml.surface = new_surface;
    sml.insert_time = time(NULL);
    this->free_surfaces.push_back(sml);
#endif
    /* remove from used surfaces */
/*TRACE

    for (unsigned int i = 0; i < this->used_surfaces.size(); i++) {
        if (used_surfaces.at(i) == surface) {
            this->used_surfaces.erase(this->used_surfaces.begin()+i);
            break;
        }
    }
*/
}



bool MMSFBSurfaceManager::createTemporarySurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, bool systemonly) {
	if (!this->tempsuf)
		mmsfb->createSurface(&this->tempsuf, w, h, pixelformat, 0, systemonly);
	if (!this->tempsuf)
		return false;
	return true;
}

MMSFBSurface *MMSFBSurfaceManager::getTemporarySurface(int w, int h) {
	if (!this->tempsuf)
		return NULL;
	this->tempsuf->lock();
	int ww, hh;
	this->tempsuf->getSize(&ww, &hh);
	if ((ww>=w)&&(hh>=h))
		return this->tempsuf;

	DEBUGMSG("MMSGUI", "the temporary surface " + iToStr(ww) + "x" + iToStr(hh) + " is to small - requested size is "
											 + iToStr(w) + "x" + iToStr(h));
	this->tempsuf->unlock();
	return NULL;
}

void MMSFBSurfaceManager::releaseTemporarySurface(MMSFBSurface *tempsuf) {
	if (tempsuf!=this->tempsuf)
		return;
	this->tempsuf->unlock();
}

