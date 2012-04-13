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

#include <fstream>
#include <cstdlib>
#include <cerrno>
#include "mmsmedia/mmscda.h"
extern "C" {
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

MMS_CREATEERROR(MMSCDAError);

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
/**
 * Callback, that will be called if xine sends event messages.
 *
 * At the moment it just handles the error message if an encrypted
 * dvd cannot be played and it updates the channel information.
 *
 * It also emits signals that can be handled by sigc++ connectors.
 *
 * @param   userData    [in/out]    pointer to the MMSCDA object
 * @param   event       [in]        pointer to event structure
 */
static void queue_cb(void *userData, const xine_event_t *event) {
	MMSCDA                 *mmscda = (MMSCDA*)userData;
    //xine_ui_message_data_t *msg    = (xine_ui_message_data_t*)event->data;
	switch(event->type) {
        case XINE_EVENT_UI_PLAYBACK_FINISHED:
			mmscda->onStatusChange->emit(103,0);
			break;
    }
}
#endif

/**
 * Check for CD device.
 *
 * It uses xine health check to implement this.
 * First it will check for a given device. If it fails
 * it also checks for '/dev/dvd'.
 *
 * @param   device  [in]    device to check as dvd device
 *
 * @exception   MMSCDAError no usable dvd device found
 */
void MMSCDA::checkDevice(const string device) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
		string                  d = device;
		xine_health_check_t hc, *result;

		if(d.length() == 0)
			d = "/dev/cdrom";

		hc.cdrom_dev = xine_config_register_string(xine, "input.cdrom_dev", d.c_str(), "device used as cdrom drive", NULL, 0, NULL, NULL);
		result = xine_health_check(&hc, CHECK_CDROM);
		if(result->status != XINE_HEALTH_CHECK_OK) {
				throw MMSCDAError(0, "No DVD Device found at " + d);
		}

		this->device = d;
		DEBUGMSG("MMSMedia", "Using " + this->device + " as CDROM device");
#endif
    }
}

/**
 * Constructor of MMSCDA class.
 *
 * It does initializing by calling MMSAV::initialize()
 * and checks the given device. If the device is not correct
 * "/dev/dvd" will be used.
 *
 * @param   window  [in]    main window for dvd playing
 * @param   device  [in]    device to check as dvd device
 * @param   verbose [in]    if true the xine engine writes debug messages to stdout
 */
MMSCDA::MMSCDA(MMSWindow *window, const string device, const bool verbose)  {
    MMSAV::initialize(verbose, window);

    /* at first check for DVD device */
    checkDevice(device);

    /* save window width/height */
    if(window) {
		MMSFBRectangle rect = window->getGeometry();
		this->windowWidth  = rect.w;
		this->windowHeight = rect.h;
    }
}

/**
 * Destructor of MMSCDA class.
 */
MMSCDA::~MMSCDA() {
}

#if defined __HAVE_GSTREAMER__
#elif defined __HAVE_XINE__
/**
 * Calls MMSAV::open() with the queue_cb callback.
 */
void MMSCDA::xineOpen() {
    MMSAV::xineOpen(queue_cb, (void*)this);
}
#endif

/**
 * Starts playing.
 *
 * If the continue flag is set it tries to continue
 * at the position where it was stopped before.
 *
 * @param   cont    if true it tries to continue at a position stopped before
 */
void MMSCDA::startPlaying(int tracknum) {
	string mrl = "cdda://" + this->device;
	if(tracknum <= titlecount && tracknum >= 1)
		mrl += "/" + iToStr(tracknum);

	this->currtitle = (tracknum >= 1 ? tracknum : 1);

#ifdef __HAVE_XINE__
	if(!this->stream) MMSAV::xineOpen(queue_cb, (void*)this);
#endif

	MMSAV::startPlaying(mrl, false);
}

/**
 * Playback will be switched to rewind.
 *
 * @see MMSCDA::slow()
 * @see MMSCDA::ffwd()
 */
void MMSCDA::rewind() {
    DEBUGMSG("MMSMedia", "MMSCDA::rewind() not yet implemented");
#if 0
    if(this->status != this->STATUS_NONE) {
        this->setStatus(this->STATUS_REWIND);
        xine_trick_mode (this->stream, XINE_TRICK_MODE_FAST_REWIND, 1);
    }
#endif
}

/**
 * Jump to previous chapter.
 *
 * @see MMSCDA::next()
 */
void MMSCDA::previous() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	if(currtitle>1)
    		currtitle--;
    	else
    		currtitle=titlecount;
    	this->stop(false);
    	this->startPlaying(currtitle);
#endif
    }
    //this->setStatus(this->STATUS_PREVIOUS);
}

/**
 * Jump to next chapter.
 *
 * @see MMSCDA::previous()
 */
void MMSCDA::next() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	if(currtitle<titlecount)
    		currtitle++;
    	else
    		currtitle=1;
    	this->stop(false);
    	this->startPlaying(currtitle);
#endif
    }
    //this->setStatus(this->STATUS_NEXT);
}

/**
 * Eject the cd.
 *
 * It disposes the xine stream and tries to eject dvd with ioctl
 *
 */
void MMSCDA::eject() {

	this->setStatus(this->STATUS_NONE);

    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
		if(this->stream)
			xine_dispose(this->stream);
		this->stream = 0;

#endif
    }
    /* eject with ioctl */
	int status = -1;
	int fd = open(device.c_str(), O_RDONLY|O_NONBLOCK);

	if(fd < 0) {
		DEBUGMSG("MMSCDA", "Eject failed (can't open device: %s.)", strerror(errno));
		return;
	}

#if defined(CDROMEJECT)
	status = ioctl(fd, CDROMEJECT);
#elif defined(CDIOCEJECT)
	status = ioctl(fd, CDIOCEJECT);
#endif

	close(fd);
	if(status != 0) {
		DEBUGMSG("MMSCDA", "Eject failed: %s.", strerror(errno));
	}
}


/**
 * Gets the title number that is currently being played.
 *
 * @return  title number
 */
int MMSCDA::getTitleNumber() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	// TODO: implement getTitleNumber() for gstreamer
    	return 0;
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	return this->currtitle;
#endif
    }

    throw MMSCDAError(0, "MMSCDA::getTitleNumber() called but media backend does not match supported backends");
}

/**
 * Gets amount of titles available on the current dvd.
 *
 * @return  title count
 */
int MMSCDA::getTitleCount() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	// TODO: implement getTitleCount() for gstreamer
    	return 0;
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	return this->titlecount;
#endif
    }

    throw MMSCDAError(0, "MMSCDA::getTitleCount() called but media backend does not match supported backends");
}


void MMSCDA::checktoc() {
	int fd_cd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
	if(fd_cd<0) {
		this->titlecount=-1;
		return;
	}
	struct cdrom_tochdr hdr;
	if(ioctl(fd_cd, CDROMREADTOCHDR, &hdr) == -1) {
		this->titlecount=-1;
		return;
	} else {
		DEBUGMSG("MMSMedia", "tochdr cdth_trk0: " + iToStr(hdr.cdth_trk0) + " cdth_trk1: " + iToStr(hdr.cdth_trk1));
		this->titlecount = hdr.cdth_trk1;
	}
}
