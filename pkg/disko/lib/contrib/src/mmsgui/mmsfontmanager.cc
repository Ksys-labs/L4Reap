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

#include "mmsgui/mmsfontmanager.h"

MMSFontManager::MMSFontManager() {
}

MMSFontManager::~MMSFontManager() {
    /* free all fonts */
    for (unsigned int i = 0; i < this->fonts.size(); i++) {
    	if (this->fonts.at(i).font)
    		delete this->fonts.at(i).font;
    }
}

MMSFBFont *MMSFontManager::getFont(string path, string filename, unsigned int size) {
    string          fontfile;
    MMSFM_DESC      fm_desc;

    /* build filename */
#ifdef __L4_RE__
    fontfile = "rom";
#else
    fontfile = path;
#endif
    if (fontfile != "") fontfile+= "/";
    fontfile += filename;
    if (fontfile == "")
        return NULL;

    // lock threads
    this->lock.lock();

    /* search within fonts list */
    for (unsigned int i = 0; i < this->fonts.size(); i++) {
        if (this->fonts.at(i).fontfile == fontfile)
            if (this->fonts.at(i).size == size) {
                this->lock.unlock();
                return this->fonts.at(i).font;
            }
    }

    /* load font */
    fm_desc.font = NULL;
    if (!loadFont(&(fm_desc.font), "", fontfile, 0, size)) {
        DEBUGMSG("MMSGUI", "cannot load font file '%s'", fontfile.c_str());
        this->lock.unlock();
        return NULL;
    }
    fm_desc.fontfile = fontfile;
    fm_desc.size = size;

    /* add to fonts list and return the font */
    this->fonts.push_back(fm_desc);
    this->lock.unlock();
    return fm_desc.font;
}

void MMSFontManager::releaseFont(string path, string filename, unsigned int size) {
    /*TODO*/
}

void MMSFontManager::releaseFont(MMSFBFont *font) {
    /*TODO*/
}

