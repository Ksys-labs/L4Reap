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

#ifndef MMSFBSURFACEMANAGER_H_
#define MMSFBSURFACEMANAGER_H_

#include "mmstools/mmslogger.h"
#include "mmsgui/fb/mmsfbsurface.h"

typedef struct {
    MMSFBSurface    *surface;
    time_t          insert_time;
} MMSFBSURMANLIST;

class MMSFBSurfaceManager {
    private:
        vector<MMSFBSURMANLIST> used_surfaces;
        vector<MMSFBSURMANLIST> free_surfaces;

        MMSFBSurface *tempsuf;

        int 			surface_mem_cnt;
        pthread_mutex_t	surface_mem_cnt_lock;

    public:
        MMSFBSurfaceManager();
        ~MMSFBSurfaceManager();
        MMSFBSurface *createSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, bool systemonly);
        void releaseSurface(MMSFBSurface *surface);

        bool createTemporarySurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, bool systemonly);
        MMSFBSurface *getTemporarySurface(int w, int h);
        void releaseTemporarySurface(MMSFBSurface *tempsuf);
};

/* access to global mmsfbsurfacemanager */
extern MMSFBSurfaceManager *mmsfbsurfacemanager;

#endif /*MMSFBSURFACEMANAGER_H_*/
