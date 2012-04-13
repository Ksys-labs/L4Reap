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

#include "mmsmedia/mmssound.h"

MMS_CREATEERROR(MMSSoundError);

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
/**
 * Callback, that will be called if xine sends event messages.
 *
 * @param   userData    [in/out]    pointer to the MMSSound object
 * @param   event       [in]        pointer to event structure
 */
static void queue_cb(void *userData, const xine_event_t *event) {

	if(!userData)
		return;

    MMSSound *mmssound = (MMSSound*)userData;

    if(event->type == XINE_EVENT_UI_PLAYBACK_FINISHED)
        mmssound->onPlaybackFinished.emit();
}
#endif

/**
 * Constructor of MMSSound class.
 *
 * @param   verbose [in]    if true the xine engine writes debug messages to stdout
 *
 * @see MMSAV::MMSAV()
 * @see MMSAV::initialize()
 */
MMSSound::MMSSound(const bool verbose) {
    MMSAV::initialize(verbose);
}

/**
 * Destructor of MMSSound class.
 */
MMSSound::~MMSSound() {
    this->onPlaybackFinished.clear();
}

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
/**
 * Calls MMSAV::open() with the queue_cb callback.
 */
void MMSSound::xineOpen() {

    MMSAV::xineOpen(queue_cb);

    // ignore video
    xine_set_param(this->stream, XINE_PARAM_IGNORE_VIDEO, true);
}
#endif

/**
 * Starts playing.
 *
 * If the continue flag is set it tries to continue
 * at the position where it was stopped before.
 *
 * @param   mrl     MRL to play
 * @param   cont    if true it tries to continue at a position stopped before
 */
void MMSSound::startPlaying(string mrl, bool cont) {
/*    if(!this->stream)
        this->open();*/
#ifdef __HAVE_XINE__
	if(!this->stream) MMSAV::xineOpen(queue_cb, (void*)this);
#endif

    MMSAV::startPlaying(mrl, cont);
}

/**
 * Playback will be switched to fast forward.
 *
 * There are two different speed settings for fast forward.
 * Twice as fast and four times as fast.
 */
void MMSSound::ffwd() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
		if(this->status != this->STATUS_NONE) {
			this->setStatus(this->STATUS_FFWD);
			xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_FAST_4);
		}
#endif
    }
}
