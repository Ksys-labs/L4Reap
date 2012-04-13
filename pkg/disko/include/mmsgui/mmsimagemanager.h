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

#ifndef MMSIMAGEMANAGER_H_
#define MMSIMAGEMANAGER_H_

#include "mmsgui/mmsguitools.h"
#include "mmstools/mmstafffile.h"
#include "mmstools/mmslogger.h"
#include "mmsconfig/mmsconfigdata.h"

#define MMSIM_MAX_DESC_SUF      64
#define MMSIM_DESC_SUF_LOADING  0xfffffffe
#define MMSIM_DESC_SUF_END      0xffffffff

typedef struct {
    MMSFBSurface    *surface;   //pointer to an surface
    unsigned int    delaytime;  //milliseconds
} MMSIM_DESC_SUF;

typedef struct {
    string          name;                       //extra name
    string          imagefile;                  //filename or url
    time_t          mtime;                      //modification time of the file
    int             usecount;                   //number of times this image is used
    MMSIM_DESC_SUF  suf[MMSIM_MAX_DESC_SUF+1];  //buffer for 'sub'-surfaces of the image (e.g. neede for GIF)
    int             sufcount;                   //number of surfaces available
    bool            loading;                    //true if loader is already running
} MMSIM_DESC;

class MMSImageManager {
    private:
        MMSFBLayer          *layer;   		// this is the layer on which the image is to display
        MMSFBSurfacePixelFormat	pixelformat;// pixelformat for all my images
        bool				usetaff;		// use the taff (image) format?
        MMSTAFF_PF			taffpf;			// pixelformat for the taff converter

        vector<MMSIM_DESC*> images;

        MMSMutex  			lock;

        MMSConfigData       config;

        bool surface2TAFF(string &imagefile, MMSFBSurface *surface);

        bool loadGIF(string file, MMSIM_DESC *desc);

    public:
        MMSImageManager(MMSFBLayer *layer = NULL);
        ~MMSImageManager();
        MMSFBSurface *getImage(const string &path, const string &filename, MMSIM_DESC_SUF **surfdesc = NULL,
        					   int mirror_size = 0, bool gen_taff = true);
        MMSFBSurface *newImage(const string &name, unsigned int width, unsigned int height, MMSFBSurfacePixelFormat pixelformat = MMSFB_PF_NONE);
        void releaseImage(const string &path, const string &filename);
        void releaseImage(MMSFBSurface *surface);
};

#endif /*MMSIMAGEMANAGER_H_*/
