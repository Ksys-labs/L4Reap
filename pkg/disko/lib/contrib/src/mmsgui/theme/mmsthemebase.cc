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

#include "mmsgui/theme/mmsthemebase.h"

MMSALIGNMENT getAlignmentFromString(string inputstr) {
    MMSALIGNMENT alignment;

    alignment = MMSALIGNMENT_NOTSET;

    if (inputstr == "center")
        alignment = MMSALIGNMENT_CENTER;
    else if (inputstr == "left")
        alignment = MMSALIGNMENT_LEFT;
    else if (inputstr == "right")
        alignment = MMSALIGNMENT_RIGHT;
    else if (inputstr == "justify")
        alignment = MMSALIGNMENT_JUSTIFY;
    else if (inputstr == "top-center")
        alignment = MMSALIGNMENT_TOP_CENTER;
    else if (inputstr == "top-left")
        alignment = MMSALIGNMENT_TOP_LEFT;
    else if (inputstr == "top-right")
        alignment = MMSALIGNMENT_TOP_RIGHT;
    else if (inputstr == "top-justify")
        alignment = MMSALIGNMENT_TOP_JUSTIFY;
    else if (inputstr == "bottom-center")
        alignment = MMSALIGNMENT_BOTTOM_CENTER;
    else if (inputstr == "bottom-left")
        alignment = MMSALIGNMENT_BOTTOM_LEFT;
    else if (inputstr == "bottom-right")
        alignment = MMSALIGNMENT_BOTTOM_RIGHT;
    else if (inputstr == "bottom-justify")
        alignment = MMSALIGNMENT_BOTTOM_JUSTIFY;

    return alignment;
}

MMSALIGNMENT swapAlignmentHorizontal(MMSALIGNMENT alignment) {
	switch (alignment) {
	case MMSALIGNMENT_LEFT:
		return MMSALIGNMENT_RIGHT;
	case MMSALIGNMENT_RIGHT:
		return MMSALIGNMENT_LEFT;
	case MMSALIGNMENT_TOP_LEFT:
		return MMSALIGNMENT_TOP_RIGHT;
	case MMSALIGNMENT_TOP_RIGHT:
		return MMSALIGNMENT_TOP_LEFT;
	case MMSALIGNMENT_BOTTOM_LEFT:
		return MMSALIGNMENT_BOTTOM_RIGHT;
	case MMSALIGNMENT_BOTTOM_RIGHT:
		return MMSALIGNMENT_BOTTOM_LEFT;
	default:
		return alignment;
	}
}

MMSDIRECTION getDirectionFromString(string inputstr) {
    MMSDIRECTION direction;

    direction = MMSDIRECTION_NOTSET;

    if (inputstr == "left")
        direction = MMSDIRECTION_LEFT;
    else if (inputstr == "right")
        direction = MMSDIRECTION_RIGHT;
    else if (inputstr == "up")
        direction = MMSDIRECTION_UP;
    else if (inputstr == "down")
        direction = MMSDIRECTION_DOWN;
    else if (inputstr == "up-left")
        direction = MMSDIRECTION_UP_LEFT;
    else if (inputstr == "up-right")
        direction = MMSDIRECTION_UP_RIGHT;
    else if (inputstr == "down-left")
        direction = MMSDIRECTION_DOWN_LEFT;
    else if (inputstr == "down-right")
        direction = MMSDIRECTION_DOWN_RIGHT;

    return direction;
}



MMSPOSITION getPositionFromString(string inputstr) {
    MMSPOSITION position;

    position = MMSPOSITION_NOTSET;

    if (inputstr == "left")
        position = MMSPOSITION_LEFT;
    else if (inputstr == "right")
        position = MMSPOSITION_RIGHT;
    else if (inputstr == "top")
        position = MMSPOSITION_TOP;
    else if (inputstr == "bottom")
        position = MMSPOSITION_BOTTOM;
    else if (inputstr == "top-left")
        position = MMSPOSITION_TOP_LEFT;
    else if (inputstr == "top-right")
        position = MMSPOSITION_TOP_RIGHT;
    else if (inputstr == "bottom-left")
        position = MMSPOSITION_BOTTOM_LEFT;
    else if (inputstr == "bottom-right")
        position = MMSPOSITION_BOTTOM_RIGHT;

    return position;
}


