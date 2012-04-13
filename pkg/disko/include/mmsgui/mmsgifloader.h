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

#ifndef MMSGIFLOADER_H_
#define MMSGIFLOADER_H_

#include "mmsgui/mmsimagemanager.h"
#include <sigc++/sigc++.h>

class MMSGIFLoader : public MMSThread {
    private:
        //GIF: header
        typedef struct {
            char signature[3+1];
            char version[3+1];
        } MMS_GIF_HEADER;

        //GIF: logical screen descriptor
        typedef struct {
            unsigned short  width;
            unsigned short  height;
            unsigned char   flags;
            unsigned char   bgcolor;
            unsigned char   ratio;
            bool            global_color_table;
        } MMS_GIF_LSD;

        //GIF: color table
        typedef struct {
            unsigned short  size;
            unsigned char   table[3*256];
        } MMS_GIF_CT;

        //GIF: graphic control extension
        typedef struct {
            unsigned char   flags;
            unsigned short  delaytime;
            unsigned char   transcolor;
            bool            transparent_color;
            unsigned char   disposal;
        } MMS_GIF_GCE;

        //GIF: image descriptor
        typedef struct {
            unsigned short  x;
            unsigned short  y;
            unsigned short  w;
            unsigned short  h;
            unsigned char   flags;
            bool            local_color_table;
            bool            interlaced;
        } MMS_GIF_ID;

        //name of gif file
        MMSIM_DESC *desc;

        //display layer
        MMSFBLayer *layer;

        //file pointer
        MMSFile     *myfile;

        //gif data
        MMS_GIF_HEADER  gif_header;
        MMS_GIF_LSD     gif_lsd;
        MMS_GIF_CT      gif_gct;

        pthread_cond_t	cond;
        pthread_mutex_t	mutex;

        bool loadHeader();
        bool loadBlocks();

        void threadMain();

    public:
        MMSGIFLoader(MMSIM_DESC      *desc,
        		     MMSFBLayer      *layer);

        void block();
};

bool isGIF(string file);

#endif /*MMSGIFLOADER_H_*/
