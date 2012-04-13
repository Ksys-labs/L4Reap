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

#include "mmsgui/theme/mmsinputwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_INPUTWIDGET_ATTR_I[] = MMSGUI_INPUTWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_INPUTWIDGET_ATTR_I[MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_INPUTWIDGET_ATTR_I[MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_##aname].type


MMSInputWidgetClass::MMSInputWidgetClass() {
    unsetAll();
}

void MMSInputWidgetClass::unsetAll() {
    this->className = "";
    unsetCursorState();
    MMSTextBaseClass::unsetAll();
}

void MMSInputWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
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
			SET_FONT_FROM_TAFF(MMSGUI_INPUTWIDGET_ATTR)

			// special macro for shadow parameters
			SET_SHADOW_FROM_TAFF(MMSGUI_INPUTWIDGET_ATTR)

			// special macro for textinfo parameters
			SET_TEXTINFO_FROM_TAFF(MMSGUI_INPUTWIDGET_ATTR)

			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_cursor_state:
				if ((attrval_int & 0xff) == 0x01)
					setCursorState(MMSSTATE_AUTO);
				else
				if (attrval_int)
					setCursorState(MMSSTATE_TRUE);
				else
					setCursorState(MMSSTATE_FALSE);
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
            SET_FONT_FROM_TAFF_WITH_PREFIX(MMSGUI_INPUTWIDGET_ATTR)
            else
			// special macro for shadow parameters
			SET_SHADOW_FROM_TAFF_WITH_PREFIX(MMSGUI_INPUTWIDGET_ATTR)
            else
			// special macro for textinfo parameters
			SET_TEXTINFO_FROM_TAFF_WITH_PREFIX(MMSGUI_INPUTWIDGET_ATTR)
            else
            if (ISATTRNAME(cursor_state)) {
				if ((attrval_int & 0xff) == 0x01)
					setCursorState(MMSSTATE_AUTO);
				else
				if (attrval_int)
					setCursorState(MMSSTATE_TRUE);
				else
					setCursorState(MMSSTATE_FALSE);
	            break;
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

void MMSInputWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSInputWidgetClass::getClassName() {
    return this->className;
}

bool MMSInputWidgetClass::isCursorState() {
    return this->iscursor_state;
}

void MMSInputWidgetClass::setCursorState(MMSSTATE cursor_state) {
    this->cursor_state = cursor_state;
    this->iscursor_state = true;
}

void MMSInputWidgetClass::unsetCursorState() {
    this->iscursor_state = false;
}

MMSSTATE MMSInputWidgetClass::getCursorState() {
    return this->cursor_state;
}

