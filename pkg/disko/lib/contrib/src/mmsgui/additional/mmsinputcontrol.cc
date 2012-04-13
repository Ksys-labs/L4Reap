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

#include "mmsgui/additional/mmsinputcontrol.h"
#include "mmsinfo/mmsinfo.h"

#define INPUTCONTROL_TEXTWIN	"inputcontrol_textwin"
#define INPUTCONTROL_TEXT		"inputcontrol_text"
#define INPUTCONTROL_SPRITE		"inputcontrol_sprite"
#define INPUTCONTROL_STEXT		"inputcontrol_stext"

MMSInputControl::MMSInputControl(MMSWindow *window) {
}

MMSInputControl::~MMSInputControl() {
}

bool MMSInputControl::load(MMSWindow *parent, string dialogfile, MMSTheme *theme) {
	if (!MMSGUIControl::load(parent, dialogfile, theme)) {
		// base class has failed to load...
		if (parent) {
			// load the default dialog file which includes a child window
			// do this only if a parent window is given!!!
			this->window = this->dm->loadChildDialog((string)getPrefix() + "/share/disko/mmsgui/mmsinputcontrol.xml", theme);
		}
	}

	if (!this->window)
		return false;

	// get access to the widgets
	this->inputcontrol_textwin	= dynamic_cast<MMSWindow*>(this->window->findWindow(INPUTCONTROL_TEXTWIN));
	this->inputcontrol_text		= dynamic_cast<MMSInputWidget*>(this->window->findWidget(INPUTCONTROL_TEXT));
	this->inputcontrol_sprite	= dynamic_cast<MMSWindow*>(this->window->findWindow(INPUTCONTROL_SPRITE));
	this->inputcontrol_stext	= dynamic_cast<MMSLabelWidget*>(this->window->findWidget(INPUTCONTROL_STEXT));

	// check something and/or connect callbacks if widgets does exist
	if (this->inputcontrol_text) {
		this->inputcontrol_text->onBeforeChange->connect(sigc::mem_fun(this,&MMSInputControl::onBeforeChange));
	}

	return true;
}

bool MMSInputControl::onBeforeChange(MMSWidget *widget, string text, bool add, MMSFBRectangle rect) {
	if (add) {
		MMSFBRectangle r = this->inputcontrol_textwin->getGeometry();
		MMSFBRectangle rs = this->inputcontrol_sprite->getGeometry();
		this->inputcontrol_stext->setText(text);
		this->inputcontrol_sprite->moveTo(r.x + rect.x - rs.w / 2 + rect.w / 2, r.y + rect.y - rs.h / 2 + rect.h / 2);
		this->inputcontrol_sprite->show();
	}
//	printf("'%s':%d,%d,%d,%d\n", text.c_str(),rect.x, rect.y, rect.w, rect.h);
	return true;
}
