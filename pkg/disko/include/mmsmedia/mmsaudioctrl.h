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

#ifdef __HAVE_MIXER__
#ifndef MMSAUDIOCTRL_H_
#define MMSAUDIOCTRL_H_

#include <alsa/asoundlib.h>

#include "mmstools/mmstools.h"

MMS_CREATEERROR(MMSAudioCtrlError);

/**
 * @brief   Controls the audio device.
 *
 * @ingroup     mmsmedia
 *
 * @author      Stefan Schwarzer (stefan.schwarzer@diskohq.org)
 * @author      Matthias Hardt (matthias.hardt@diskohq.org)
 * @author      Jens Schneider (pupeider@gmx.de)
 * @author      Guido Madaus (guido.madaus@diskohq.org)
 * @author      Patrick Helterhoff (patrick.helterhoff@diskohq.org)
 * @author		René Bählkow (rene.baehlkow@diskohq.org)
 */
class MMSAudioCtrl {
    private:
        static snd_mixer_t      *handle;    /**< alsa handle to mixer           */
        static snd_mixer_elem_t *elem;      /**< alsa mixer element handle      */
        static string           card;       /**< card id                        */
        static int              volume;     /**< volume (in percent)            */
        static long             xval;       /**< re-calculated volume for card  */
        static bool             muteFlag;   /**< if true sound is muted         */
        static long             pmin, pmax; /**< range (not in percent)         */
        static string           channel;    /**< current audio channel          */
        static bool             isSwitchable;  /**< has a switch (to mute)?        */

    public:
        /* constructor */
        MMSAudioCtrl(string channel="");

        /* destructor */
        ~MMSAudioCtrl();

        /* set volume (count in percent (0-100)) */
        void setVolume(int count);

        /* get volume */
        int getVolume(bool dfCard = false);

        /* change volume (count in percent (0-100)) */
        void volumeUp(int count = 10);

        /* change volume (count in percent (0-100)) */
        void volumeDown(int count = 10);

        /* check if is already mute */
        bool isMute();

        /* toggle mute on/off */
        void mute();
};

#endif /*MMSAUDIOCTRL_H_*/
#endif /* __HAVE_MIXER__ */
