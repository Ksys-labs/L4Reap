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

#include "mmsmedia/mmsvideo.h"

MMS_CREATEERROR(MMSVideoError);

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
/**
 * Callback, that will be called if xine sends event messages.
 *
 * @param   userData    [in/out]    pointer to the MMSVideo object
 * @param   event       [in]        pointer to event structure
 */
static void queue_cb(void *userData, const xine_event_t *event) {
	MMSVideo               *mmsvideo = (MMSVideo*)userData;

	switch(event->type) {
        case XINE_EVENT_UI_MESSAGE: {
            xine_ui_message_data_t *msg      = (xine_ui_message_data_t*)event->data;
			if(msg->explanation)
			    mmsvideo->onError->emit(string((char*)msg + msg->parameters));
    	    break;
        }
        case XINE_EVENT_MRL_REFERENCE_EXT: {
            xine_mrl_reference_data_ext_t *mrl_ref = (xine_mrl_reference_data_ext_t*)event->data;
        	DEBUGMSG("MMSVideo", "new mrl: %s\n", mrl_ref->mrl);
        	mmsvideo->add2Playlist(mrl_ref->mrl);
            break;
        }
        case XINE_EVENT_PROGRESS: {
            xine_progress_data_t *prog = (xine_progress_data_t*)event->data;
            DEBUGMSG("MMSVideo", "event: %s (%d%%)", prog->description, prog->percent);
            if(mmsvideo) mmsvideo->onProgressChange.emit(prog->percent);
            break;
        }
        case XINE_EVENT_UI_PLAYBACK_FINISHED:
        	try {
        		mmsvideo->playNext();
        	} catch(MMSError &e) {
        		DEBUGMSG("MMSVideo", "Error playing stream: " + e.getMessage());
			    mmsvideo->onError->emit(e.getMessage());
        	}
        	break;
        default:
    		DEBUGMSG("MMSVideo", "Unhandled event: %d", event->type);
            break;
    }
}
#endif

/**
 * Initializes everything that is needed by MMSVideo.
 *
 * @param   window  [in]    main window for video playback
 * @param   verbose [in]    if true the xine engine writes debug messages to stdout
 *
 * @see MMSAV::MMSAV()
 * @see MMSAV::initialize()
 */
MMSVideo::MMSVideo(MMSWindow *window, const bool verbose) {
    MMSAV::initialize(verbose, window);
}

/**
 * Destructor of MMSVideo class.
 */
MMSVideo::~MMSVideo() {
}

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
/**
 * Calls MMSAV::open() with the queue_cb callback.
 */
void MMSVideo::xineOpen() {
    MMSAV::xineOpen(queue_cb, (void*)this);
}
#endif

/**
 * Starts playing.
 *
 * If the continue flag is set it tries to continue
 * at the position where it was stopped before.
 *
 * @param   file    file to play
 * @param   cont    if true it tries to continue at a position stopped before
 *
 * @exception   MMSAVError stream could not be opened
 */
void MMSVideo::startPlaying(const string file, const bool cont) {
//   this->open();
#ifdef __HAVE_XINE__
  this->xineOpen();
#endif


   string::size_type loc = file.find( "://", 0 );
   if( loc != string::npos ) {
	   /* we found a mrl, so we play file directly */
	   MMSAV::startPlaying(file, cont);
   }
   else {
	   /* we found no mrl type, it seems to be a file */
	   MMSAV::startPlaying("file://" + file, cont);
   }
}

/**
 * Adds a file to the internal playlist.
 *
 * @param   file    file to add
 */
void MMSVideo::add2Playlist(const string file) {
	   playlist.push(file);
}

/**
 * Starts playing the next file from the playlist.
 *
 * @exception   MMSAVError stream could not be opened
 */
void MMSVideo::playNext() {
	if(!playlist.empty()) {
		string file = playlist.front();
		playlist.pop();
		startPlaying(file, false);
	}
}
