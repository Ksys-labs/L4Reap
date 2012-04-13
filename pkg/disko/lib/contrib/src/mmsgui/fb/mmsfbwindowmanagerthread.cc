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

#include "mmsgui/fb/mmsfbwindowmanagerthread.h"
#include "mmsgui/fb/mmsfbwindowmanager.h"

MMSFBWindowManagerThread::MMSFBWindowManagerThread(MMSFBSurface **high_freq_surface,
                                                   MMSFBSurface **high_freq_saved_surface,
                                                   int *high_freq_lastflip,
                                                   MMSMutex *lock) {
    this->high_freq_surface = high_freq_surface;
    this->high_freq_saved_surface = high_freq_saved_surface;
    this->high_freq_lastflip = high_freq_lastflip;
    this->lock = lock;
}

void MMSFBWindowManagerThread::threadMain() {
	//int	pointer_opacity = 0;
	//int	hidecnt = 1;

	while (1) {

		// fade out the pointer
		mmsfbwindowmanager->fadePointer();


        if (!*(this->high_freq_surface)) {
            /* have no region */
            sleep(1);
            continue;
        }

        /* get the flip time */
        struct  timeval tv;
        gettimeofday(&tv, NULL);
        int newfliptime = (((int)tv.tv_sec)%1000000)*1000+((int)tv.tv_usec)/1000;
        int diff = newfliptime - *(this->high_freq_lastflip);
        if ((diff > 0)&&(diff < 1000)) {
            /* already running */
            msleep(200);
            continue;
        }

        /* have a surface but it was not flipped fast enough */
        lock->lock();
        if (!*(this->high_freq_surface)) {
            lock->unlock();
            continue;
        }
        if (*(this->high_freq_saved_surface)) {
            /* copy saved surface because window works direct with layer */
            (*(this->high_freq_surface))->setBlittingFlags(MMSFB_BLIT_NOFX);
            (*(this->high_freq_surface))->blit(*(this->high_freq_saved_surface), NULL, 0, 0);
        }
        DEBUGOUT("flipped not fast enough");
        mmsfbwindowmanager->flipSurface(*(this->high_freq_surface), NULL,
                                       MMSFB_FLIP_NONE, true);
        *(this->high_freq_surface) = NULL;
        *(this->high_freq_saved_surface) = NULL;
        *(this->high_freq_lastflip) = 0;
        lock->unlock();
    }
}

