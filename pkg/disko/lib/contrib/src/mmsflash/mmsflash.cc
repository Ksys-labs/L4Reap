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

#ifdef __HAVE_MMSFLASH__

#include "mmsflash/mmsflash.h"
#include "mmsflash/mmsflashthread.h"

extern "C" {
#include <swfdec/swfdec.h>
#include <swfdec/swfdec_buffer.h>
}

#include <cairo.h>
#include <cairo-features.h>


// static variables
bool MMSFlash::swfdec_initialized = false;


MMSFlash::MMSFlash(MMSWindow *window) {
	// init the window
	this->window = window;
	this->window->onHandleInput->connect(sigc::mem_fun(this,&MMSFlash::onHandleInput));

	// init all others
	this->ready 				= false;
	this->playing				= false;
	this->swfdec_player 		= NULL;
	this->swfdec_rate   		= 0;
	this->width					= 0;
	this->height				= 0;
	this->flash_temp_surface	= NULL;
	this->loaderthread			= new MMSFlashThread(this, MMSFLASHTHREAD_MODE_LOADER, "MMSFlashLoaderThread");
	this->playerthread			= new MMSFlashThread(this, MMSFLASHTHREAD_MODE_PLAYER, "MMSFlashPlayerThread");

	if (!this->swfdec_initialized) {
		DEBUGMSG("MMSFLASH", "initializing swfdec");
		swfdec_init();
		DEBUGMSG("MMSFLASH", "swfdec initialized");
		this->swfdec_initialized = true;
	}
}

MMSFlash::~MMSFlash() {
	stopThreads();
	if (this->flash_temp_surface)
		delete this->flash_temp_surface;
	if (this->swfdec_player)
		g_object_unref(this->swfdec_player);
}


void MMSFlash::loader(bool &stop) {

	// lock me
	lock.lock();
	this->ready = false;
	this->playing = false;

    // new player object
	if (this->swfdec_player)
		g_object_unref(this->swfdec_player);
	DEBUGMSG("MMSFLASH", "creating swfdec player");
    this->swfdec_player = swfdec_player_new(NULL);
    if (!this->swfdec_player) {
    	lock.unlock();
    	DEBUGMSG("MMSFLASH", "Cannot get a new SwfdecPlayer object");
    	return;
    }

    // set url
	DEBUGMSG("MMSFLASH", "setting swfdec player url");
	SwfdecURL *url = swfdec_url_new(this->filename.c_str());
    swfdec_player_set_url((SwfdecPlayer*)this->swfdec_player, url);
    swfdec_url_free(url);

    // check if player is initialized
	DEBUGMSG("MMSFLASH", "checking swfdec player");
    swfdec_player_advance((SwfdecPlayer*)this->swfdec_player, 0);
    if (!swfdec_player_is_initialized((SwfdecPlayer*)this->swfdec_player)) {
    	g_object_unref(this->swfdec_player);
    	this->swfdec_player = NULL;
    	lock.unlock();
    	fprintf(stderr, "Cannot initialize SwfdecPlayer object\n");
    	return;
    }

    // get frame rate
    this->swfdec_rate = swfdec_player_get_rate((SwfdecPlayer*)this->swfdec_player);
	DEBUGMSG("MMSFLASH", "frame rate = %d", this->swfdec_rate);

    // get size of the flash image
    guint ww,hh;
    swfdec_player_get_default_size((SwfdecPlayer*)this->swfdec_player, &ww, &hh);
    this->width=ww;
    this->height=hh;
	DEBUGMSG("MMSFLASH", "size = %d x %d", this->width, this->height);

    // (re-)create surface for cairo/swfdec
    if (this->flash_temp_surface)
    	delete flash_temp_surface;
    DEBUGMSG("MMSFLASH", "creating surface for cairo/swfdec");
	this->window->getLayer()->createSurface(&(this->flash_temp_surface), this->width, this->height, MMSFB_PF_ARGB, 0);
	if (!this->flash_temp_surface) {
    	g_object_unref(this->swfdec_player);
    	this->swfdec_player = NULL;
    	lock.unlock();
    	DEBUGMSG("MMSFLASH", "Cannot create temporary surface");
    	return;
	}
	void *ptr;
	int pitch;
	this->flash_temp_surface->lock(MMSFB_LOCK_WRITE, &ptr, &pitch);
	this->flash_temp_surface->unlock();
    DEBUGMSG("MMSFLASH", "creating cairo surface");
    this->cairosurface = cairo_image_surface_create_for_data((unsigned char*)ptr, CAIRO_FORMAT_ARGB32,
    															this->width, this->height, pitch);
	if (!this->cairosurface) {
		delete this->flash_temp_surface;
		this->flash_temp_surface = NULL;
    	g_object_unref(this->swfdec_player);
    	this->swfdec_player = NULL;
    	lock.unlock();
    	DEBUGMSG("MMSFLASH", "Cannot create cairo surface");
    	return;
	}
    DEBUGMSG("MMSFLASH", "creating cairo object");
    this->cairo = cairo_create((cairo_surface_t *)cairosurface);
    cairo_surface_destroy((cairo_surface_t *)cairosurface);
    if (!this->cairo) {
		delete this->flash_temp_surface;
		this->flash_temp_surface = NULL;
    	g_object_unref(this->swfdec_player);
    	this->swfdec_player = NULL;
    	lock.unlock();
    	DEBUGMSG("MMSFLASH", "Cannot create cairo object");
    	return;
	}

    // ready for playing
    this->ready = true;

    // unlock me
	lock.unlock();
	DEBUGMSG("MMSFLASH", "loading finished");
}

void MMSFlash::player(bool &stop) {
	// waiting for ready state
	this->playing = false;
	while ((!stop)&&(!this->ready)) {
    	msleep(50);
    	if ((!this->ready)&&(!this->loaderthread->isRunning()))
			return;
    }
	this->playing = true;

    // until stopped
    unsigned int sleep_time = (unsigned int)(1000 / this->swfdec_rate);
	while (!stop) {
		if (!this->window->isShown(true)) {
			// window is not shown, so playback is not needed
			msleep(500);
			continue;
		}

		// lock me
		lock.lock();

		// get start time
	    unsigned int start_ts = getMTimeStamp();

	    // get background color of the current frame and clear the surface
	    unsigned int bg = swfdec_player_get_background_color((SwfdecPlayer*)this->swfdec_player);
	    this->flash_temp_surface->clear( (bg >> 16) & 0xff, (bg >> 8) & 0xff, bg & 0xff, bg >> 24 );

	    // let swfdec render to the temporary surface
	    DEBUGMSG("MMSFLASH", "rendering");
	    swfdec_player_render((SwfdecPlayer*)this->swfdec_player, (cairo_t *)this->cairo);
	    DEBUGMSG("MMSFLASH", "finished rendering");

		// unlock me
		lock.unlock();

		// do an stretchblit to the window and flip it
	    this->window->getSurface()->stretchBlit(this->flash_temp_surface, NULL, NULL);
	    this->window->flip();

	    // calc sleep time and sleep
	    unsigned int end_ts = getMTimeStamp();
	    end_ts = getMDiff(start_ts, end_ts);
	    if (end_ts < sleep_time)
	    	msleep(sleep_time - end_ts);
	}

	// reset state
	this->playing = false;
}

void MMSFlash::stopThreads(void) {
	// stop it all
	this->loaderthread->invokeStop();
	this->playerthread->invokeStop();
	this->loaderthread->waitUntilStopped();
	this->playerthread->waitUntilStopped();

	// reset state
	this->ready = false;
	this->playing = false;
}

unsigned int MMSFlash::mapKey(MMSKeySymbol key) {
	switch (key) {
	case MMSKEY_CURSOR_UP:
		return SWFDEC_KEY_UP;
	case MMSKEY_CURSOR_DOWN:
		return SWFDEC_KEY_DOWN;
	case MMSKEY_CURSOR_LEFT:
		return SWFDEC_KEY_LEFT;
	case MMSKEY_CURSOR_RIGHT:
		return SWFDEC_KEY_RIGHT;
	case MMSKEY_SPACE:
		return SWFDEC_KEY_SPACE;
	case MMSKEY_BACKSPACE:
		return SWFDEC_KEY_BACKSPACE;
	case MMSKEY_SLASH:
		return SWFDEC_KEY_SLASH;
	case MMSKEY_BACKSLASH:
		return SWFDEC_KEY_BACKSLASH;
	case MMSKEY_TAB:
		return SWFDEC_KEY_TAB;
	case MMSKEY_CLEAR:
		return SWFDEC_KEY_CLEAR;
	case MMSKEY_RETURN:
		return SWFDEC_KEY_ENTER;
	case MMSKEY_SHIFT:
		return SWFDEC_KEY_SHIFT;
	case MMSKEY_CONTROL:
		return SWFDEC_KEY_CONTROL;
	case MMSKEY_ALT:
		return SWFDEC_KEY_ALT;
	case MMSKEY_CAPS_LOCK:
		return SWFDEC_KEY_CAPS_LOCK;
	case MMSKEY_ESCAPE:
		return SWFDEC_KEY_ESCAPE;
	case MMSKEY_PAGE_UP:
		return SWFDEC_KEY_PAGE_UP;
	case MMSKEY_PAGE_DOWN:
		return SWFDEC_KEY_PAGE_DOWN;
	case MMSKEY_END:
		return SWFDEC_KEY_END;
	case MMSKEY_HOME:
		return SWFDEC_KEY_HOME;
	case MMSKEY_INSERT:
		return SWFDEC_KEY_INSERT;
	case MMSKEY_DELETE:
		return SWFDEC_KEY_DELETE;
	case MMSKEY_HELP:
		return SWFDEC_KEY_HELP;
	case MMSKEY_0:
		return SWFDEC_KEY_0;
	case MMSKEY_1:
		return SWFDEC_KEY_1;
	case MMSKEY_2:
		return SWFDEC_KEY_2;
	case MMSKEY_3:
		return SWFDEC_KEY_3;
	case MMSKEY_4:
		return SWFDEC_KEY_4;
	case MMSKEY_5:
		return SWFDEC_KEY_5;
	case MMSKEY_6:
		return SWFDEC_KEY_6;
	case MMSKEY_7:
		return SWFDEC_KEY_7;
	case MMSKEY_8:
		return SWFDEC_KEY_8;
	case MMSKEY_9:
		return SWFDEC_KEY_9;
	case MMSKEY_CAPITAL_A:
	case MMSKEY_SMALL_A:
		return SWFDEC_KEY_A;
	case MMSKEY_CAPITAL_B:
	case MMSKEY_SMALL_B:
		return SWFDEC_KEY_B;
	case MMSKEY_CAPITAL_C:
	case MMSKEY_SMALL_C:
		return SWFDEC_KEY_C;
	case MMSKEY_CAPITAL_D:
	case MMSKEY_SMALL_D:
		return SWFDEC_KEY_D;
	case MMSKEY_CAPITAL_E:
	case MMSKEY_SMALL_E:
		return SWFDEC_KEY_E;
	case MMSKEY_CAPITAL_F:
	case MMSKEY_SMALL_F:
		return SWFDEC_KEY_F;
	case MMSKEY_CAPITAL_G:
	case MMSKEY_SMALL_G:
		return SWFDEC_KEY_G;
	case MMSKEY_CAPITAL_H:
	case MMSKEY_SMALL_H:
		return SWFDEC_KEY_H;
	case MMSKEY_CAPITAL_I:
	case MMSKEY_SMALL_I:
		return SWFDEC_KEY_I;
	case MMSKEY_CAPITAL_J:
	case MMSKEY_SMALL_J:
		return SWFDEC_KEY_J;
	case MMSKEY_CAPITAL_K:
	case MMSKEY_SMALL_K:
		return SWFDEC_KEY_K;
	case MMSKEY_CAPITAL_L:
	case MMSKEY_SMALL_L:
		return SWFDEC_KEY_L;
	case MMSKEY_CAPITAL_M:
	case MMSKEY_SMALL_M:
		return SWFDEC_KEY_M;
	case MMSKEY_CAPITAL_N:
	case MMSKEY_SMALL_N:
		return SWFDEC_KEY_N;
	case MMSKEY_CAPITAL_O:
	case MMSKEY_SMALL_O:
		return SWFDEC_KEY_O;
	case MMSKEY_CAPITAL_P:
	case MMSKEY_SMALL_P:
		return SWFDEC_KEY_P;
	case MMSKEY_CAPITAL_Q:
	case MMSKEY_SMALL_Q:
		return SWFDEC_KEY_Q;
	case MMSKEY_CAPITAL_R:
	case MMSKEY_SMALL_R:
		return SWFDEC_KEY_R;
	case MMSKEY_CAPITAL_S:
	case MMSKEY_SMALL_S:
		return SWFDEC_KEY_S;
	case MMSKEY_CAPITAL_T:
	case MMSKEY_SMALL_T:
		return SWFDEC_KEY_T;
	case MMSKEY_CAPITAL_U:
	case MMSKEY_SMALL_U:
		return SWFDEC_KEY_U;
	case MMSKEY_CAPITAL_V:
	case MMSKEY_SMALL_V:
		return SWFDEC_KEY_V;
	case MMSKEY_CAPITAL_W:
	case MMSKEY_SMALL_W:
		return SWFDEC_KEY_W;
	case MMSKEY_CAPITAL_X:
	case MMSKEY_SMALL_X:
		return SWFDEC_KEY_X;
	case MMSKEY_CAPITAL_Y:
	case MMSKEY_SMALL_Y:
		return SWFDEC_KEY_Y;
	case MMSKEY_CAPITAL_Z:
	case MMSKEY_SMALL_Z:
		return SWFDEC_KEY_Z;
	case MMSKEY_F1:
		return SWFDEC_KEY_F1;
	case MMSKEY_F2:
		return SWFDEC_KEY_F2;
	case MMSKEY_F3:
		return SWFDEC_KEY_F3;
	case MMSKEY_F4:
		return SWFDEC_KEY_F4;
	case MMSKEY_F5:
		return SWFDEC_KEY_F5;
	case MMSKEY_F6:
		return SWFDEC_KEY_F6;
	case MMSKEY_F7:
		return SWFDEC_KEY_F7;
	case MMSKEY_F8:
		return SWFDEC_KEY_F8;
	case MMSKEY_F9:
		return SWFDEC_KEY_F9;
	case MMSKEY_F10:
		return SWFDEC_KEY_F10;
	case MMSKEY_F11:
		return SWFDEC_KEY_F11;
	case MMSKEY_F12:
		return SWFDEC_KEY_F12;
	case MMSKEY_NUM_LOCK:
		return SWFDEC_KEY_NUM_LOCK;
	case MMSKEY_SCROLL_LOCK:
		return SWFDEC_KEY_SCROLL_LOCK;
	case MMSKEY_SEMICOLON:
		return SWFDEC_KEY_SEMICOLON;
	case MMSKEY_EQUALS_SIGN:
		return SWFDEC_KEY_EQUAL;
	case MMSKEY_COMMA:
		return SWFDEC_KEY_COMMA;
	case MMSKEY_MINUS_SIGN:
		return SWFDEC_KEY_MINUS;
	case MMSKEY_PERIOD:
		return SWFDEC_KEY_DOT;
	case MMSKEY_GRAVE_ACCENT:
		return SWFDEC_KEY_GRAVE;
	case MMSKEY_APOSTROPHE:
		return SWFDEC_KEY_APOSTROPHE;
	default:
		return 0;
	}
}

bool MMSFlash::onHandleInput(MMSWindow *window, MMSInputEvent *input) {
	unsigned int 	key;

	// check state
	if (!this->swfdec_player)
		return false;
	if (!input)
		return false;

	// lock me
	lock.lock();

	// calculate the pointer position within flash image
	int posx = input->posx;
	int posy = input->posy;
	if (input->type == MMSINPUTEVENTTYPE_BUTTONPRESS || input->type == MMSINPUTEVENTTYPE_BUTTONRELEASE || input->type == MMSINPUTEVENTTYPE_AXISMOTION) {
		MMSFBRectangle ig = window->getGeometry();
		if (this->width != ig.w)
			posx = ((posx * (this->width << 8)) / ig.w) >> 8;
		if (this->height != ig.h)
			posy = ((posy * (this->height << 8)) / ig.h) >> 8;
	}

	// send event to the player
	switch (input->type) {
	case MMSINPUTEVENTTYPE_KEYPRESS:
		if ((key = mapKey(input->key)))
			swfdec_player_key_press((SwfdecPlayer*)this->swfdec_player, key, 0);
		break;
	case MMSINPUTEVENTTYPE_KEYRELEASE:
		if ((key = mapKey(input->key)))
			swfdec_player_key_release((SwfdecPlayer*)this->swfdec_player, key, 0);
		break;
	case MMSINPUTEVENTTYPE_BUTTONPRESS:
		swfdec_player_mouse_press((SwfdecPlayer*)this->swfdec_player, posx, posy, 1);
		break;
	case MMSINPUTEVENTTYPE_BUTTONRELEASE:
		swfdec_player_mouse_release((SwfdecPlayer*)this->swfdec_player, posx, posy, 1);
		break;
	case MMSINPUTEVENTTYPE_AXISMOTION:
		swfdec_player_mouse_move((SwfdecPlayer*)this->swfdec_player, posx, posy);
		break;
	default:
		break;
	}

    // unlock me
	lock.unlock();

	return true;
}

void MMSFlash::startPlaying(string filename) {
	// check if i have a window
	if (!this->window)
        return;

	// first, stop the threads
	stopThreads();

	// check filename
    string prefix = filename.substr(0, 7);
    strToUpr(&prefix);
    if ((prefix != "FILE://")&&(prefix != "HTTP://"))
    	if (filename.substr(0,1) != "/") {
    		char path[1024];
    		memset(path, 0, sizeof(path));
			this->filename = "file://" + (string)getcwd(path, 1024) + "/" + filename;
    	}
    	else
    		this->filename = "file://" + filename;
    else
    	this->filename = filename;

    // start the loader
    this->loaderthread->start();

    // start the player
    this->playerthread->start();
}

bool MMSFlash::isReady() {
    // waiting until loader thread is started
    while (!this->loaderthread->isStarted())
    	msleep(50);

    // waiting for ready flag
    while ((!this->ready)&&(this->loaderthread->isRunning()))
   		msleep(50);

    return this->ready;
}

bool MMSFlash::isPlaying(bool wait) {
	if (wait)
    	while ((!this->playing)&&(this->playerthread->isRunning()))
        	msleep(50);
	return this->playing;
}


#endif /* __HAVE_MMSFLASH__ */
