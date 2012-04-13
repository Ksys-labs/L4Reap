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

#include "mmsbase/mmsmusicinterface.h"

MMSMusicInterface::MMSMusicInterface() {
	this->onNextSong = new sigc::signal<void, int>;
    this->onPrevSong = new sigc::signal<void, int>;
}

MMSMusicInterface::~MMSMusicInterface() {
	if(this->onNextSong) {
		delete this->onNextSong;
	}

	if(this->onPrevSong)
		delete this->onPrevSong;

	if(this->manager) {
		this->manager->setOnNextSong(NULL);
		this->manager->setOnPrevSong(NULL);
	}
}

void MMSMusicInterface::setManager(IMMSMusicManager *manager) {
	this->manager = manager;
}

void MMSMusicInterface::init(string file) {
	if(this->manager!=NULL) {
		PLAYLIST playlist;
		playlist.push_back(file);
		this->manager->init(playlist);
	}
}

void MMSMusicInterface::init(PLAYLIST list, int offset) {
	if(this->manager!=NULL) {
		this->manager->init(list,offset);
		if(this->onNextSong)
			this->manager->setOnNextSong(this->onNextSong);
		if(this->onPrevSong)
			this->manager->setOnPrevSong(this->onPrevSong);
	}
}

void MMSMusicInterface::play() {
	if(this->manager!=NULL) {
		this->manager->play();
	}
}

void MMSMusicInterface::stop() {
	if(this->manager!=NULL) {
		this->manager->stopAll();
	}
}

void MMSMusicInterface::next() {
	if(this->manager)
		this->manager->next();
}

void MMSMusicInterface::prev() {
	if(this->manager)
		this->manager->prev();

}

void MMSMusicInterface::pause() {
	if(this->manager)
		this->manager->pause();

}

bool MMSMusicInterface::hasPlaylist()  {
	if(this->manager)
		return this->manager->hasPlaylist();

	return false;
}

PLAYLIST MMSMusicInterface::getPlaylist() {
	PLAYLIST list;
	if(this->manager)
		return this->manager->getPlaylist();

	return list;
}

int MMSMusicInterface::getPlaylistOffset() {
	if(this->manager)
		return this->manager->getPlaylistOffset();

	return -1;
}

bool MMSMusicInterface::isPlaying() {
	if(this->manager)
		return manager->isPlaying();

	return false;
}

bool MMSMusicInterface::isPaused() {
	if(this->manager)
		return manager->isPaused();

	return true;
}

bool MMSMusicInterface::getTimes(int *pos, int *length) {
    if(this->manager)
        return manager->getTimes(pos, length);

    return false;
}

void MMSMusicInterface::setRepeat(bool repeat) {
    this->manager->setRepeat(repeat);
}

void MMSMusicInterface::setShuffle(bool shuffle) {
    this->manager->setShuffle(shuffle);
}

IMMSMusicManager *MMSMusicInterface::manager=NULL;
