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
#include "mmsmedia/mmsaudioctrl.h"

snd_mixer_t         *MMSAudioCtrl::handle = NULL;
string              MMSAudioCtrl::card = "default";
int                 MMSAudioCtrl::volume = -1;      /* in percent */
long                MMSAudioCtrl::xval = -1;         /* re-calculated volume for card */
bool                MMSAudioCtrl::muteFlag = false; /* mute on/off */
long                MMSAudioCtrl::pmin;             /* range (not in percent) */
long                MMSAudioCtrl::pmax;             /* range (not in percent) */
string              MMSAudioCtrl::channel = "";
snd_mixer_elem_t    *MMSAudioCtrl::elem = NULL;
bool                MMSAudioCtrl::isSwitchable = false;

/**
 * Constructor of MMSAudioCtrl class.
 *
 * The first instance of this class assigns
 * an audio channel, attaches to the mixer
 * of the sound card and gets the current
 * settings.
 *
 * @param   channel [in]    audio channel
 */
MMSAudioCtrl::MMSAudioCtrl(string channel) {
    int     err;

    if(this->channel=="") {
        this->channel=channel;
    }

    if (!this->handle) {
        /* open the mixer */
        if ((err = snd_mixer_open(&(this->handle), 0)) < 0)
            throw MMSAudioCtrlError(err,"snd_mixer_open() failed");

        /* attach the card */
        if ((err = snd_mixer_attach(this->handle, this->card.c_str())) < 0) {
            snd_mixer_close(this->handle);
            throw MMSAudioCtrlError(err,"snd_mixer_attach() with card = '"
                                            + this->card + "' failed");
        }

        /* register */
        if ((err = snd_mixer_selem_register(this->handle, NULL, NULL)) < 0) {
            snd_mixer_close(this->handle);
            string s = snd_strerror(err);
            throw MMSAudioCtrlError(err,"snd_mixer_selem_register() failed with '" + s + "'");
        }

        /* load */
        if ((err = snd_mixer_load(this->handle)) < 0) {
            snd_mixer_close(this->handle);
            string s = snd_strerror(err);
            throw MMSAudioCtrlError(err,"snd_mixer_load() failed with '" + s + "'");
        }
    }

    if (!this->elem) {
        /* searching for the first active element */
        for (this->elem = snd_mixer_first_elem(this->handle);
             this->elem;
             this->elem = snd_mixer_elem_next(this->elem)) {

            string mix = snd_mixer_selem_get_name(this->elem);
            DEBUGMSG("MMSMedia", "got mixer channel: %s", mix.c_str());
            /* is active? */
            if (!snd_mixer_selem_is_active(this->elem))
                continue;

            /* has playback volume? */
            if (!snd_mixer_selem_has_playback_volume(this->elem))
                continue;

            if (this->channel!="") {
                if(strcmp(this->channel.c_str(),snd_mixer_selem_get_name(this->elem))!=0)
                    continue;
            }

            /* we have found our channel*/
            /* get volume range */
            snd_mixer_selem_get_playback_volume_range(this->elem, &(this->pmin), &(this->pmax));

            /* check if this elem is mutable */
            isSwitchable = (snd_mixer_selem_has_playback_switch(elem) > 0);

            /* get the current volume settings */
            getVolume();

            return;
        }

        throw MMSAudioCtrlError(0,"no element found");
    }
}

/**
 * Destructor of MMSAudioCtrl class.
 */
MMSAudioCtrl::~MMSAudioCtrl() {
    /* release all */
}

/**
 * Sets the volume of the audio device.
 *
 * @param   count   [in]    volume in percent
 *
 * @note If count is less than 0 or more than 100
 * it will be set correctly to 0 or 100.
 */
void MMSAudioCtrl::setVolume(int count) {
    /* calculate the new value */
    this->xval = -1;
    this->volume = count;
    if (this->volume < 0) this->volume = 0;
    if (this->volume > 100) this->volume = 100;
    if (this->volume == 0)
        this->xval = this->pmin;
    else
    if (this->volume == 100)
        this->xval = this->pmax;
    else
        this->xval = this->pmin + ((this->pmax - this->pmin) * (long)this->volume) / 100;

    /* set the new value */
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, this->xval);
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, this->xval);

    /* set mute to off */
    this->muteFlag = false;
}

/**
 * Returns the currently set volume.
 *
 * @param   dfCard  [in]    get the volume directly from card?
 *
 * @return Volume in percent
 */
int MMSAudioCtrl::getVolume(bool dfCard) {
    long    lval, rval;
    int     retval = this->volume;

    if ((!this->muteFlag)||(dfCard)) {
        /* get the current volume settings */
        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &lval);
        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &rval);
        if ((lval != this->xval)||(rval != this->xval))
            retval = (int)(((lval + (rval - lval) / 2) * 100) / (this->pmax - this->pmin));
        if (!this->muteFlag) this->volume = retval;
        return retval;
    }
    return this->volume;
}

/**
 * Turns up the volume.
 *
 * @param   count   [in]    value in percent to increase
 */
void MMSAudioCtrl::volumeUp(int count) {
    setVolume(getVolume() + count);
}

/**
 * Turns down the volume.
 *
 * @param   count   [in]    value in percent to decrease
 */
void MMSAudioCtrl::volumeDown(int count) {
    setVolume(getVolume() - count);
}

/**
 * Checks if sound is muted.
 *
 * @return  true if sound is muted
 */
bool MMSAudioCtrl::isMute() {
    if(isSwitchable) {
        int valL = 0;
        int valR = 0;
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &valL);
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, &valR);
        if(valL > 0 || valR > 0) {
            return false;
        }
        return true;
    }

    if (this->muteFlag) {
        if (getVolume(true) > 0) {
            /* switch to off */
            this->muteFlag = false;
        }
    }
    return this->muteFlag;
}

/**
 * Mutes/unmutes the sound.
 */
void MMSAudioCtrl::mute() {
    if (isMute()) {
        /* switch to on */
        if(isSwitchable) {
            snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, 1);
            snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, 1);
        } else {
            setVolume(this->volume);
        }
    } else {
        /* set mute */
        if(isSwitchable) {
            snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, 0);
            snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, 0);
        } else {
            snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, 0);
            snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, 0);

            /* set mute to on */
            this->muteFlag = true;
        }
    }
}

#endif /* __HAVE_MIXER__ */
