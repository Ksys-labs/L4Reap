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

#include "mmsinput/mmsinputgesture.h"
#include <stdlib.h>

MMSInputGesture::MMSInputGesture(int sensitivity)  {
	dy=0;
	dx=0;
	this->threshold = sensitivity;
}

MMSInputGesture::~MMSInputGesture() {

}

void MMSInputGesture::addInfo(MMSInputEvent *event) {
	dx+=event->dx;
	dy+=event->dy;
}

void MMSInputGesture::reset() {
	dx=0;
	dy=0;
}

MMSINPUTGESTURE MMSInputGesture::guess() {

	if((dx < (-1 *this->threshold))&&(abs(dy)< abs(dx)/2))
		return MMSINPUTGESTURE_LEFT;
	if((dx > this->threshold)&&(abs(dy)<abs(dx)/2))
		return MMSINPUTGESTURE_RIGHT;
	if((dy < (-1 * this->threshold))&&(abs(dx)<abs(dy)/2))
		return MMSINPUTGESTURE_UP;
	if((dy > this->threshold)&&(abs(dx)<abs(dy)/2))
		return MMSINPUTGESTURE_DOWN;


	return MMSINPUTGESTURE_UKN;
}

