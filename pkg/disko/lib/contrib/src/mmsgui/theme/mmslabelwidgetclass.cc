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

#include "mmsgui/theme/mmslabelwidgetclass.h"
#include <string.h>

// store attribute descriptions here
TAFF_ATTRDESC MMSGUI_LABELWIDGET_ATTR_I[] = MMSGUI_LABELWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_LABELWIDGET_ATTR_I[MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_LABELWIDGET_ATTR_I[MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_##aname].type


MMSLabelWidgetClass::MMSLabelWidgetClass() {
    unsetAll();
}

void MMSLabelWidgetClass::unsetAll() {
    this->className = "";
    unsetSlidable();
    unsetSlideSpeed();
    unsetTranslate();
    MMSTextBaseClass::unsetAll();
}

void MMSLabelWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
        unsetFontPath();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;

			// special macro for font parameters
			SET_FONT_FROM_TAFF(MMSGUI_LABELWIDGET_ATTR)

			// special macro for shadow parameters
			SET_SHADOW_FROM_TAFF(MMSGUI_LABELWIDGET_ATTR)

			// special macro for textinfo parameters
			SET_TEXTINFO_FROM_TAFF(MMSGUI_LABELWIDGET_ATTR)

			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_slidable:
	            setSlidable((attrval_int)?true:false);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_slide_speed:
	            setSlideSpeed(attrval_int);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_translate:
	            setTranslate((attrval_int)?true:false);
	            break;
	        }
		}
		endTAFFScan
    }
    else {
    	unsigned int pl = strlen(prefix->c_str());

    	startTAFFScan_WITHOUT_ID
    	{
    		// check if attrname has correct prefix
    		if (pl >= strlen(attrname))
        		continue;
            if (memcmp(attrname, prefix->c_str(), pl)!=0)
            	continue;
            attrname = &attrname[pl];

            // special storage for macros
			bool attrval_str_valid;
			bool int_val_set;
			bool byte_val_set;
			int  *p_int_val = &attrval_int;

    		// okay, correct prefix, check attributes now

            // special macro for font parameters
            SET_FONT_FROM_TAFF_WITH_PREFIX(MMSGUI_LABELWIDGET_ATTR)
            else
			// special macro for shadow parameters
			SET_SHADOW_FROM_TAFF_WITH_PREFIX(MMSGUI_LABELWIDGET_ATTR)
            else
			// special macro for textinfo parameters
			SET_TEXTINFO_FROM_TAFF_WITH_PREFIX(MMSGUI_LABELWIDGET_ATTR)
			else
            if (ISATTRNAME(slidable)) {
	            setSlidable((attrval_int)?true:false);
			}
            else
            if (ISATTRNAME(slide_speed)) {
	            setSlideSpeed(attrval_int);
			}
            else
            if (ISATTRNAME(translate)) {
	            setTranslate((attrval_int)?true:false);
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
	    if (!isFontPath())
	        setFontPath(*path);
    }
}

void MMSLabelWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSLabelWidgetClass::getClassName() {
    return this->className;
}

bool MMSLabelWidgetClass::isSlidable() {
    return this->isslidable;
}

void MMSLabelWidgetClass::setSlidable(bool slidable) {
    this->slidable = slidable;
    this->isslidable = true;
}

void MMSLabelWidgetClass::unsetSlidable() {
    this->isslidable = false;
}

bool MMSLabelWidgetClass::getSlidable() {
    return this->slidable;
}

bool MMSLabelWidgetClass::isSlideSpeed() {
    return this->isslidespeed;
}

void MMSLabelWidgetClass::setSlideSpeed(unsigned char slidespeed) {
    this->slidespeed = slidespeed;
    this->isslidespeed = true;
}

void MMSLabelWidgetClass::unsetSlideSpeed() {
    this->isslidespeed = false;
}

unsigned char MMSLabelWidgetClass::getSlideSpeed() {
    return this->slidespeed;
}


bool MMSLabelWidgetClass::isTranslate() {
    return this->istranslate;
}

void MMSLabelWidgetClass::setTranslate(bool translate) {
    this->translate = translate;
    this->istranslate = true;
}

void MMSLabelWidgetClass::unsetTranslate() {
    this->istranslate = false;
}

bool MMSLabelWidgetClass::getTranslate() {
    return this->translate;
}

