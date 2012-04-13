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

#ifndef IMMSMUSICMANAGER_H_
#define IMMSMUSICMANAGER_H_

using namespace std;

#include <string>
#include <vector>
#include <sigc++/sigc++.h>

typedef vector<string> PLAYLIST;

class IMMSMusicManager {
	public:
	    virtual ~IMMSMusicManager() {};
		virtual void init(PLAYLIST, int offset=0) = 0;
		virtual void stopAll() = 0;
		virtual void next() = 0;
		virtual void prev() = 0;
		virtual void play() = 0;
		virtual void pause() = 0;
		virtual bool hasPlaylist() = 0;
		virtual PLAYLIST getPlaylist() = 0;
		virtual int getPlaylistOffset() = 0;
        virtual void setOnNextSong(sigc::signal<void, int> *onNextSong) = 0;
        virtual void setOnPrevSong(sigc::signal<void, int> *onPrevSong) = 0;
        virtual bool isPlaying() = 0;
        virtual bool isPaused() = 0;
        virtual bool getTimes(int *pos, int *length) = 0;
        virtual void setRepeat(bool repeat) = 0;
        virtual void setShuffle(bool shuffle) = 0;
};

#endif /*IMMSMUSICMANAGER_H_*/
