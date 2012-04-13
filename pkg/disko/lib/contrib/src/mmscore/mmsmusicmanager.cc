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

#ifdef __HAVE_MMSMEDIA__
#include "mmscore/mmsmusicmanager.h"
#include <stdlib.h>

MMSMusicManager::MMSMusicManager() :
    onNextSong(NULL),
    onPrevSong(NULL),
    cont(false),
    repeat(false),
    shuffle(false) {
	/* register myself to the music interface */
	MMSMusicInterface interface;
	interface.setManager(this);
    this->player.onPlaybackFinished.connect(sigc::mem_fun(this, &MMSMusicManager::next));
}

MMSMusicManager::~MMSMusicManager() {
	if(this->player.isPlaying())
		this->player.stop();
}

void MMSMusicManager::init(PLAYLIST list, int offset) {
	this->mutex.lock();
	this->playlist = list;
	this->offset = offset;
    this->alreadyPlayed.clear();
    for(unsigned int i = 0; i < playlist.size(); i++)
        this->alreadyPlayed.push_back(false);
	this->mutex.unlock();

	DEBUGMSG("MMSMusicManager", "got playlist size: %d offset: %d", list.size(), offset);
}

void MMSMusicManager::stopAll() {
	this->mutex.lock();
	if(player.isPlaying())
		player.stop();
	this->cont=false;
	this->mutex.unlock();
}

void MMSMusicManager::next() {
	this->mutex.lock();
    if(this->shuffle && (this->playlist.size() > 2)) {
        int newOffset;
        do {
            newOffset = int((double(rand())/RAND_MAX) * (this->playlist.size() - 1));
        }
        while(this->alreadyPlayed.at(newOffset));
        this->offset = newOffset;
    }
    else {
    	this->offset++;
    	if(this->offset>=(int)this->playlist.size()) {
            if(!this->repeat) return;
    		this->offset=0;
    	}
    }
	string file = this->playlist.at(this->offset);

	if(player.isPlaying())
		player.stop();
	player.startPlaying(file, false);
    this->alreadyPlayed.at(this->offset) = true;
	if(this->onNextSong)
		this->onNextSong->emit(this->offset);

	this->mutex.unlock();

}

void MMSMusicManager::prev() {
	this->mutex.lock();
	if(player.isPlaying())
		player.stop();
    if(this->shuffle && (this->playlist.size() > 2)) {
        int newOffset;
        do {
            newOffset = int((double(rand())/RAND_MAX) * (this->playlist.size() - 1));
        }
        while(this->alreadyPlayed.at(newOffset));
        this->offset = newOffset;
    }
    else {
	    this->offset--;
        if(this->offset < 0)
		  this->offset = this->playlist.size() - 1;
	}
	string file = this->playlist.at(this->offset);
	player.startPlaying(file,false);
    this->alreadyPlayed.at(this->offset) = true;
	if(this->onPrevSong)
		this->onPrevSong->emit(this->offset);
	this->mutex.unlock();
}

void MMSMusicManager::play() {
	if(this->cont) {
		player.play();
	}
	else {
		if(this->playlist.size() > (size_t)this->offset) {
			string file = this->playlist.at(this->offset);
			if(player.isPlaying()) player.stop();
			player.startPlaying(file, cont);
		    this->alreadyPlayed.at(this->offset) = true;
		}
	}
}

void MMSMusicManager::pause() {
	this->mutex.lock();
    player.pause();
	cont=true;
	this->mutex.unlock();
}

bool MMSMusicManager::hasPlaylist() {
	this->mutex.lock();
	if(this->playlist.size()>0) {
		this->mutex.unlock();
		return true;
	}
	this->mutex.unlock();
	return false;
}

PLAYLIST MMSMusicManager::getPlaylist() {
	return this->playlist;
}

int MMSMusicManager::getPlaylistOffset() {
	return this->offset;

}

bool MMSMusicManager::isPlaying() {
	return player.isPlaying();
}

bool MMSMusicManager::isPaused() {
	return player.isPaused();
}

bool MMSMusicManager::getTimes(int *pos, int *length) {
    return player.getTimes(pos, length);
}

void MMSMusicManager::setOnNextSong(sigc::signal<void, int> *onNextSong) {
	this->onNextSong = onNextSong;
}

void MMSMusicManager::setOnPrevSong(sigc::signal<void, int> *onPrevSong) {
	this->onPrevSong = onPrevSong;
}

void MMSMusicManager::setRepeat(bool repeat) {
    this->repeat = repeat;
}

void MMSMusicManager::setShuffle(bool shuffle) {
    this->shuffle = shuffle;
}
#endif /* __HAVE_MMSMEDIA__ */
