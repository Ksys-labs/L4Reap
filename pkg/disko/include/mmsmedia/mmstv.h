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

#ifndef MMSTV_H_
#define MMSTV_H_

#include "mmsmedia/mmsav.h"

/**
 * @brief   Handles TV playback.
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
 * handling TV playback. It only works for DVB-T/C/S streams.
 */
class MMSTV : public MMSAV {
    private:
        string	     channel;               /**< currently played channel name              */
        string       captureFilename;       /**< filename for saved stream                  */
        unsigned int timeout;               /**< tuning timeout in seconds                  */
        bool         recording;             /**< if true recording is on                    */

#ifdef __HAVE_XINE_BLDVB__
        bool		 useBlDvb;				/**< if true use our own dvb input plugin		*/
#endif

#ifdef __HAVE_XINE__
        void xineOpen();
#endif

    public:
#ifdef __HAVE_XINE_BLDVB__
    	MMSTV(MMSWindow *window, const string channel = "", const bool verbose = false, const bool useBlDvb = true);
#else
    	MMSTV(MMSWindow *window, const string channel = "", const bool verbose = false);
#endif
        ~MMSTV();

        void startPlaying(const string channel = "");
        void play();
#ifdef __HAVE_XINE_BLDVB__
        void play(const string &channel);
#endif
        void pause();
        void previous();
        void next();
        void record();
        void recordStart();
        void recordStop();
        void recordPause();

        string getCurrentChannelName(void);

        void setTuningTimeout(const unsigned int timeout);
        const unsigned int getTuningTimeout();
        void setRecordDir(const string dir);
        const string getRecordDir();
        const string getRecordFilename();

        /**
         * Callback that is used to receive buffering progress changes.
         */
        sigc::signal<void, const unsigned short> onProgressChange;
};

#endif /*MMSTV_H_*/
