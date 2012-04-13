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

#ifndef MMSSOUND_H_
#define MMSSOUND_H_

#include "mmsav.h"

/**
 * @brief   Handles any kind of music playback.
 *
 * @ingroup     mmsmedia
 *
 * @author      Stefan Schwarzer (stefan.schwarzer@diskohq.org)
 * @author      Matthias Hardt (matthias.hardt@diskohq.org)
 * @author      Jens Schneider (pupeider@gmx.de)
 * @author      Guido Madaus (guido.madaus@diskohq.org)
 * @author      Patrick Helterhoff (patrick.helterhoff@diskohq.org)
 * @author		René Bählkow (rene.baehlkow@diskohq.org)
 * 
 * This class is derived from MMSAV and specialized in
 * handling music playback.
 */
class MMSSound : public MMSAV {
    private:
        void initialize(const bool verbose);
#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
        void xineOpen();
#endif

    protected:

    public:

        /* if you specify the source, the constructor will try to open it */
        MMSSound(const bool verbose = false);

        /* destructor */
        ~MMSSound();

        void startPlaying(string mrl, bool cont);
        void ffwd();

        /**
         * callback that will be called if playback finished
         */
        sigc::signal<void> onPlaybackFinished;
};

#endif /*MMSSOUND_H_*/
