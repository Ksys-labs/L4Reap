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

#ifndef MMSSWITCHERTHREAD_H_
#define MMSSWITCHERTHREAD_H_

#include "mmstools/mmsmutex.h"
#include "mmsgui/mmswidgets.h"

class MMSSwitcher;

class MMSSwitcherThread : public MMSThread {
    private:
        int     mode;   /**< mode of the thread, 0: main thread, 1: show preview thread */

        MMSMutex  lock;

        string curr_date;
        string curr_time;

        MMSSwitcher *sw;
        MMSLabel    *date_s;
        MMSLabel    *time_s;
        MMSLabel    *date_p;
        MMSLabel    *time_p;

        unsigned int    sleepcnt;

        int                 invoke_plugin;
        unsigned int        plugin_invoked;
        MMSSwitcherThread   *my_spt;
        bool                preview_shown;

        void threadMain();

    public:
        MMSSwitcherThread(MMSSwitcher *sw, MMSLabelWidget *date_s, MMSLabelWidget *time_s, MMSLabelWidget *date_p, MMSLabelWidget *time_p);
        MMSSwitcherThread(MMSSwitcher *sw);

        void invokeShowPreview();
        void previewShown();
};


#endif /*MMSSWITCHERTHREAD_H_*/
