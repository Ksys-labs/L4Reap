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

#ifndef MMSCDA_H_
#define MMSCDA_H_

#include "mmsmedia/mmsav.h"

/**
 * @brief   Handles CDA playback.
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
 * handling CDA playback.
 */
class MMSCDA : public MMSAV {
    private:
        string  		device;             /**< dvd device to use                      */
        unsigned int 	windowWidth,		/**< width of given video window			*/
						windowHeight;		/**< height of given video window			*/
        int 			titlecount;
        int 			currtitle;

        void checkDevice(const string device);
        void initialize(MMSWindow *window, const string device, const bool verbose);

#ifdef __HAVE_XINE__
        void xineOpen();
#endif

    protected:

    public:
        /* status constants */
        static const unsigned short STATUS_PREVIOUS                 =  100;     /**< playing previous chapter           */
        static const unsigned short STATUS_NEXT                     =  101;     /**< playing next chapter               */
        static const unsigned short STATUS_EJECT                    =  102;     /**< ejecting DVD                       */
        static const unsigned short STATUS_FINISHED                 =  103;     /**< ejecting DVD                       */


        MMSCDA(MMSWindow *window, const string device = "/dev/cdrom", const bool verbose = false);
        ~MMSCDA();

        void startPlaying(int tracknum=1);
        void rewind();
        void previous();
        void next();
        void checktoc();

        void eject();

		/* get DVD information */
        int    getTitleNumber();
        int    getTitleCount();

};

#endif /*MMSDVD_H_*/
