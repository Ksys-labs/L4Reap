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

#ifndef _NO_MMSFLASH

#include "mmsflash/mmsflashthread.h"

MMSFlashThread::MMSFlashThread(MMSFlash *flash, MMSFLASHTHREAD_MODE mode, string identity) : MMSThread(identity) {
	this->flash = flash;
	this->mode = mode;
	this->started = false;
	this->stop = false;
}

void MMSFlashThread::threadMain() {
	this->started = true;
	this->stop = false;
	if (mode == MMSFLASHTHREAD_MODE_LOADER)
		this->flash->loader(this->stop);
	else
	if (mode == MMSFLASHTHREAD_MODE_PLAYER)
		this->flash->player(this->stop);
}

void MMSFlashThread::start(void) {
	this->started = false;
	MMSThread::start();
}

bool MMSFlashThread::isStarted(void) {
	return this->started;
}

void MMSFlashThread::invokeStop(void) {
	this->stop = true;
}

void MMSFlashThread::waitUntilStopped(void) {
	while (isRunning()) msleep(50);
}

#endif /* _NO_MMSFLASH */

