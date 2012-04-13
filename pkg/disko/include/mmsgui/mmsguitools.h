/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#ifndef MMSGUITOOLS_H_
#define MMSGUITOOLS_H_

#include "mmstools/mmslogger.h"
#include "mmsgui/fb/mmsfb.h"
#include "mmsinput/mmskeymap.h"

typedef enum {
	MMSINPUTEVENTTYPE_NONE = 0,
	MMSINPUTEVENTTYPE_KEYPRESS,
	MMSINPUTEVENTTYPE_KEYRELEASE,
	MMSINPUTEVENTTYPE_BUTTONPRESS,
	MMSINPUTEVENTTYPE_BUTTONRELEASE,
	MMSINPUTEVENTTYPE_AXISMOTION,
    MMSINPUTEVENTTYPE_TSCALIBRATION
} MMSINPUTEVENTTYPE;

typedef struct {
	MMSINPUTEVENTTYPE		type;
	MMSKeySymbol			key;
	short	    			posx;
	short					posy;
	short					dx;
	short					dy;
	short					absx;
	short					absy;
} MMSInputEvent;



//! Translate the size given in hint into pixel value.
/*!
\param retpix      return value
\param hint        size with following formats
                     \<size\>px            - size in pixel, example "100px"
                     \<size\>%             - size in percent, example "100%"
                     \<size\>px-\<size2\>  - size in pixel minus pixel, example "100px-50"
                     \<size\>%-\<size2\>   - size in percent minus pixel, example "100%-50"
                     \<factor\>$           - factor which will used in conjunction with secondaxis
                                             result=\<factor\>*secondaxis
                                           example: "1.333$"
\param maxpixel    maximum pixel for calculation and check
\param secondaxis  size of the second axis, will only used with hint \<factor\>$
\return true if input is correct and retpix is filled
*/
bool getPixelFromSizeHint(int *retpix, string hint, int maxpixel, int secondaxis);

#ifdef  __HAVE_DIRECTFB__
bool loadImage(IDirectFBImageProvider **image, string path, string filename);
#endif

bool loadFont(MMSFBFont **font, string path, string filename, int width, int height);

unsigned int getFrameNum(unsigned int delay_time);

unsigned int getFrameDelay(unsigned int start_ts, unsigned int end_ts);

#endif /*MMSGUITOOLS_H_*/
