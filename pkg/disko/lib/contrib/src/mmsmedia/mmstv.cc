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
#include "disko.h"
#include "mmsmedia/mmstv.h"
#include <strings.h>

#ifdef __HAVE_XINE__
#include <xine/xineutils.h>

#define MMSTV_RECORD_START  XINE_EVENT_INPUT_MENU2
#define MMSTV_RECORD_STOP   XINE_EVENT_INPUT_MENU2
#define MMSTV_RECORD_PAUSE  XINE_EVENT_INPUT_MENU4
#endif

MMS_CREATEERROR(MMSTVError);

#ifdef __HAVE_XINE__
/**
 * Callback, that will be called if xine sends event messages.
 *
 * It also emits signals that can be handled by sigc++ connectors.
 *
 * @param   userData    [in/out]    pointer to the MMSTV object
 * @param   event       [in]        pointer to event structure
 */
static void queue_cb(void *userData, const xine_event_t *event) {
    MMSTV *mmstv  = static_cast<MMSTV*>(userData);

    if(event->type == XINE_EVENT_UI_MESSAGE) {
        xine_ui_message_data_t *msg = (xine_ui_message_data_t*)event->data;
        DEBUGMSG("MMSTV", "event: %s", (char*)msg + msg->parameters);
    }
    else if(event->type == XINE_EVENT_PROGRESS) {
        xine_progress_data_t *prog = (xine_progress_data_t*)event->data;
        DEBUGMSG("MMSTV", "event: %s (%d%%)", prog->description, prog->percent);
        if(mmstv) mmstv->onProgressChange.emit(prog->percent);
    }
    else if(event->type == XINE_EVENT_UI_PLAYBACK_FINISHED) {
    	DEBUGMSG("MMSTV", "event: signal lost");
    	mmstv->startPlaying(mmstv->getCurrentChannelName());
    } else
    	DEBUGMSG("MMSTV", "event: %u", event->type);
}
#endif

/**
 * Initializes everything that is needed by MMSTV.
 *
 * The timeout attribute is set to 10 seconds.
 *
 * @param   window      [in]    main window for dvd playing
 * @param   _channel    [in]    channel to open
 * @param   verbose     [in]    if true the xine engine writes debug messages to stdout
 *
 * @see MMSAV::MMSAV()
 * @see MMSAV::initialize()
 */
#ifdef __HAVE_XINE_BLDVB__
MMSTV::MMSTV(MMSWindow *window, const string _channel, const bool verbose, const bool _useBlDvb) :
#else
MMSTV::MMSTV(MMSWindow *window, const string _channel, const bool verbose) :
#endif
            channel(_channel),
            captureFilename(""),
            recording(false)
#ifdef __HAVE_XINE_BLDVB__
            , useBlDvb(_useBlDvb)
#endif
{
    MMSAV::initialize(verbose, window);
    setTuningTimeout(10);
}

/**
 * Destructor of MMSTV class.
 */
MMSTV::~MMSTV() {
}

#ifdef __HAVE_XINE__
/**
 * Calls MMSAV::open() with the queue_cb callback.
 */
void MMSTV::xineOpen() {
    MMSAV::xineOpen(queue_cb, this);
}
#endif

/**
 * Starts playing.
 *
 * If usingInputDVBMorphine is set, it tries to use our own
 * input plugin for playback.
 *
 * @param   channel [in]    channel name to be played
 */
void MMSTV::startPlaying(const string channel) {
#ifdef __HAVE_XINE__
  this->xineOpen();
#endif

    if(strncasecmp(channel.c_str(), "OTH:",4)==0) {
    	FILE *fp;
    	fp=fopen(channel.c_str(),"r");
    	if(fp!=NULL){
        	char line[1024];
        	if(!fgets(line,1024,fp))
                throw MMSTVError(0, "Error reading from file " + channel);
        	this->channel = line;
        	fclose(fp);
        	DEBUGMSG("MMSTV", "trying to play " + this->channel);
            MMSAV::startPlaying(this->channel, false);
    	}

    } else {
    	DEBUGMSG("MMSTV", "trying to play " + this->channel);
#ifdef __HAVE_XINE_BLDVB__
    	if(this->useBlDvb)
        	MMSAV::startPlaying("bldvb://" + channel, false);
    	else
#endif
    	MMSAV::startPlaying("dvb://" + channel, false);
		this->channel = channel;
	}
}

/**
 * Continues playing the stream.
 *
 * If recording is on, it will be continued, too.
 */
void MMSTV::play() {
    //TODO: recording
	//this->recordPause();
    MMSAV::play();
}

#ifdef __HAVE_XINE_BLDVB__
void MMSTV::play(const string &channel) {
	if(this->useBlDvb) {
		if(this->isPlaying()) {
			char *ch = strdup(channel.c_str());
			this->sendEvent(XINE_EVENT_INPUT_SELECT, ch, sizeof(char) * (strlen(ch) + 1));
			free(ch);
		} else
			this->startPlaying(channel);
	} else
		this->next();
}
#endif

/**
 * Pauses the stream.
 *
 * If recording is on, it will be paused, too.
 */
void MMSTV::pause() {
    this->recordPause();
    MMSAV::pause();
}

/**
 * Switch to previous channel.
 *
 * @see MMSTV::next()
 */
void MMSTV::previous() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		    this->sendEvent(XINE_EVENT_INPUT_PREVIOUS);
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Switch to next channel.
 *
 * @see MMSTV::previous()
 */
void MMSTV::next() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		    this->sendEvent(XINE_EVENT_INPUT_NEXT);
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Start/Stop recording.
 */
void MMSTV::record() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->recording)
		        this->sendEvent(MMSTV_RECORD_STOP);
		    else
		        this->sendEvent(MMSTV_RECORD_START);
		    this->recording = !this->recording;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Start recording.
 */
void MMSTV::recordStart() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		    if(!this->recording) {
		        this->sendEvent(MMSTV_RECORD_START);
		        this->recording = true;
		    }
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Stop recording.
 */
void MMSTV::recordStop() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		    if(this->recording) {
		        this->sendEvent(MMSTV_RECORD_STOP);
		        this->recording = false;
		    }
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Pause recording.
 */
void MMSTV::recordPause() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		    if(this->recording)
		        this->sendEvent(MMSTV_RECORD_PAUSE);
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}
}

/**
 * Retrieve the current channel name.
 *
 * @return  current channel name
 */
string MMSTV::getCurrentChannelName(void) {
    return this->channel;
}

/**
 * Sets the maximum time for tuning to a channel.
 *
 * @param	timeout	[in]	timeout in seconds
 *
 * @note A value of 0 means infinite. Otherwise a minimum of 5
 * seconds is required.
 *
 * @see MMSTV::getTuningTimeout()
 */
void MMSTV::setTuningTimeout(const unsigned int timeout) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
		xine_cfg_entry_t  conf;

		if(!this->xine) return;
		this->timeout = timeout;

		if(xine_config_lookup_entry(this->xine, "media.dvb.tuning_timeout", &conf)) {
			conf.num_value = timeout;
			xine_config_update_entry(this->xine, &conf);
		}
		else
			xine_config_register_num(this->xine, "media.dvb.tuning_timeout", timeout,
									 "Number of seconds until tuning times out.",
									 "Leave at 0 means try forever. "
									 "Greater than 0 means wait that many seconds to get a lock. Minimum is 5 seconds.",
									 XINE_CONFIG_SECURITY, NULL, NULL);
#endif
    }
}

/**
 * Returns the setting for the tuning timeout.
 *
 * @return timeout in seconds
 *
 * @see MMSTV::setTuningTimeout()
 */
const unsigned int MMSTV::getTuningTimeout() {
	return this->timeout;
}

/**
 * Sets the directory where tv recordings will be stored.
 *
 * It actually changes the value of the xine config option
 * "media.capture.save_dir".
 *
 * @param   dir [in]    directory to save recordings into
 */
void MMSTV::setRecordDir(const string dir) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
    xine_cfg_entry_t  conf;

    if(!this->xine) return;

    if(xine_config_lookup_entry(this->xine, "media.capture.save_dir", &conf)) {
        conf.str_value = strdup(dir.c_str());
        xine_config_update_entry(this->xine, &conf);
    }
    else
#ifdef XINE_CONFIG_STRING_IS_DIRECTORY_NAME
        xine_config_register_filename(this->xine,
                "media.capture.save_dir",
                dir.c_str(),
                XINE_CONFIG_STRING_IS_DIRECTORY_NAME,
#else
        xine_config_register_string(this->xine,
                "media.capture.save_dir",
                dir.c_str(),
#endif
                "directory for saving streams",
                NULL,
                XINE_CONFIG_SECURITY,
                NULL ,
                NULL);
#endif
    }
}

/**
 * Retrieves the directory where tv recordings will be stored.
 *
 * @return  current recordings dir
 */
const string MMSTV::getRecordDir() {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
#endif
    }
    else {
#ifdef __HAVE_XINE__
		xine_cfg_entry_t  conf;

		if(this->xine && xine_config_lookup_entry(this->xine, "media.capture.save_dir", &conf))
			return string(conf.str_value);

		return xine_get_homedir();
#endif
    }

    return "";
}

/**
 * Retrieves the filename of the current recording if
 * recording is on.
 *
 * @return  current recording filename or "" if nothing is recorded
 */
const string MMSTV::getRecordFilename() {
    if(this->recording)
        return this->captureFilename;

    return "";
}
