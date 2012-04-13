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

#include "mmsgui/theme/mmsarrowwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_ARROWWIDGET_ATTR_I[] = MMSGUI_ARROWWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_ARROWWIDGET_ATTR_I[MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_ARROWWIDGET_ATTR_I[MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_##aname].type


MMSArrowWidgetClass::MMSArrowWidgetClass() {
    unsetAll();
}

void MMSArrowWidgetClass::unsetAll() {
    this->className = "";
    unsetColor();
    unsetSelColor();
    unsetDirection();
    unsetCheckSelected();
}

void MMSArrowWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_color:
	            setColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_color_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.a = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_color_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.r = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_color_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.g = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_color_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.b = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_selcolor:
	            setSelColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_selcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.a = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_selcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.r = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_selcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.g = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_selcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.b = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_direction:
	            setDirection(getDirectionFromString(attrval_str));
	            break;
			case MMSGUI_ARROWWIDGET_ATTR::MMSGUI_ARROWWIDGET_ATTR_IDS_check_selected:
	            setCheckSelected((attrval_int) ? true : false);
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
            if (ISATTRNAME(color)) {
	            setColor(MMSFBColor((unsigned int)attrval_int));
			}
            else
            if (ISATTRNAME(color_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.a = attrval_int;
	            setColor(color);
			}
            else
            if (ISATTRNAME(color_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.r = attrval_int;
	            setColor(color);
			}
            else
            if (ISATTRNAME(color_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.g = attrval_int;
	            setColor(color);
			}
            else
            if (ISATTRNAME(color_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.b = attrval_int;
	            setColor(color);
			}
            else
            if (ISATTRNAME(selcolor)) {
	            setSelColor(MMSFBColor((unsigned int)attrval_int));
			}
            else
            if (ISATTRNAME(selcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.a = attrval_int;
	            setSelColor(color);
			}
            else
            if (ISATTRNAME(selcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.r = attrval_int;
	            setSelColor(color);
			}
            else
            if (ISATTRNAME(selcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.g = attrval_int;
	            setSelColor(color);
			}
            else
            if (ISATTRNAME(selcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.b = attrval_int;
	            setSelColor(color);
			}
            else
            if (ISATTRNAME(direction)) {
	            setDirection(getDirectionFromString(attrval_str));
			}
            else
            if (ISATTRNAME(check_selected)) {
	            setCheckSelected((attrval_int) ? true : false);
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }
}

void MMSArrowWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSArrowWidgetClass::getClassName() {
    return this->className;
}

bool MMSArrowWidgetClass::isColor() {
    return this->iscolor;
}

void MMSArrowWidgetClass::setColor(MMSFBColor color) {
    this->color = color;
    this->iscolor = true;
}

void MMSArrowWidgetClass::unsetColor() {
    this->iscolor = false;
}

MMSFBColor MMSArrowWidgetClass::getColor() {
    return this->color;
}

bool MMSArrowWidgetClass::isSelColor() {
    return this->isselcolor;
}

void MMSArrowWidgetClass::setSelColor(MMSFBColor selcolor) {
    this->selcolor = selcolor;
    this->isselcolor = true;
}

void MMSArrowWidgetClass::unsetSelColor() {
    this->isselcolor = false;
}

MMSFBColor MMSArrowWidgetClass::getSelColor() {
    return this->selcolor;
}

bool MMSArrowWidgetClass::isDirection() {
    return this->isdirection;
}

void MMSArrowWidgetClass::setDirection(MMSDIRECTION direction) {
    this->direction = direction;
    this->isdirection = true;
}

void MMSArrowWidgetClass::unsetDirection() {
    this->isdirection = false;
}

MMSDIRECTION MMSArrowWidgetClass::getDirection() {
    return this->direction;
}

bool MMSArrowWidgetClass::isCheckSelected() {
    return this->ischeckselected;
}

void MMSArrowWidgetClass::setCheckSelected(bool checkselected) {
    this->checkselected = checkselected;
    this->ischeckselected = true;
}

void MMSArrowWidgetClass::unsetCheckSelected() {
    this->ischeckselected = false;
}

bool MMSArrowWidgetClass::getCheckSelected() {
    return this->checkselected;
}

