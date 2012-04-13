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

#ifndef MMSINPUTCONTROL_H_
#define MMSINPUTCONTROL_H_

#include "mmsgui/mmswindows.h"
#include "mmsgui/mmswidgets.h"
#include "mmsgui/mmsdialogmanager.h"
#include "mmsgui/additional/mmsguicontrol.h"

//! Input Control class.
/*!
This class uses e.g. the MMSInputWidget to get input text from the user.
User inputs are additionally used to trigger animation effects.
\author Jens Schneider
*/
class MMSInputControl : public MMSGUIControl {
    private:
    	MMSWindow			*inputcontrol_textwin;
    	MMSInputWidget		*inputcontrol_text;
    	MMSWindow			*inputcontrol_sprite;
    	MMSLabelWidget		*inputcontrol_stext;

    	bool onBeforeChange(MMSWidget *widget, string text, bool add, MMSFBRectangle rect);

    public:
        MMSInputControl(MMSWindow *window = NULL);
        ~MMSInputControl();
        bool load(MMSWindow *parent, string dialogfile = "", MMSTheme *theme = NULL);
};

#endif /*MMSINPUTCONTROL_H_*/
