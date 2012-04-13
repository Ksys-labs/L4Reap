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

#include "mmsgui/theme/mmscheckboxwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_CHECKBOXWIDGET_ATTR_I[] = MMSGUI_CHECKBOXWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_CHECKBOXWIDGET_ATTR_I[MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_CHECKBOXWIDGET_ATTR_I[MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_##aname].type


MMSCheckBoxWidgetClass::MMSCheckBoxWidgetClass() {
	initCheckedBgColor();
    initCheckedSelBgColor();
    initCheckedBgColor_p();
    initCheckedSelBgColor_p();
    initCheckedBgColor_i();
    initCheckedSelBgColor_i();

    initCheckedBgImagePath();
    initCheckedBgImageName();
    initCheckedSelBgImagePath();
    initCheckedSelBgImageName();
    initCheckedBgImagePath_p();
    initCheckedBgImageName_p();
    initCheckedSelBgImagePath_p();
    initCheckedSelBgImageName_p();
    initCheckedBgImagePath_i();
    initCheckedBgImageName_i();
    initCheckedSelBgImagePath_i();
    initCheckedSelBgImageName_i();

    initChecked();
}

MMSCheckBoxWidgetClass::~MMSCheckBoxWidgetClass() {
	freeCheckedBgColor();
    freeCheckedSelBgColor();
    freeCheckedBgColor_p();
    freeCheckedSelBgColor_p();
    freeCheckedBgColor_i();
    freeCheckedSelBgColor_i();

    freeCheckedBgImagePath();
    freeCheckedBgImageName();
    freeCheckedSelBgImagePath();
    freeCheckedSelBgImageName();
    freeCheckedBgImagePath_p();
    freeCheckedBgImageName_p();
    freeCheckedSelBgImagePath_p();
    freeCheckedSelBgImageName_p();
    freeCheckedBgImagePath_i();
    freeCheckedBgImageName_i();
    freeCheckedSelBgImagePath_i();
    freeCheckedSelBgImageName_i();

    freeChecked();
}

MMSCheckBoxWidgetClass &MMSCheckBoxWidgetClass::operator=(const MMSCheckBoxWidgetClass &c) {
	if (this != &c) {
		/* copy internal fix data area */
		this->widgetClass = c.widgetClass;
		this->id = c.id;

		/* copy external data */
		memset(&(this->ed), 0, sizeof(this->ed));
		if (c.id.ischecked_bgimagepath)
			this->ed.checked_bgimagepath = new string(*c.ed.checked_bgimagepath);
		if (c.id.ischecked_bgimagename)
			this->ed.checked_bgimagename = new string(*c.ed.checked_bgimagename);
		if (c.id.ischecked_selbgimagepath)
			this->ed.checked_selbgimagepath = new string(*c.ed.checked_selbgimagepath);
		if (c.id.ischecked_selbgimagename)
			this->ed.checked_selbgimagename = new string(*c.ed.checked_selbgimagename);
		if (c.id.ischecked_bgimagepath_p)
			this->ed.checked_bgimagepath_p = new string(*c.ed.checked_bgimagepath_p);
		if (c.id.ischecked_bgimagename_p)
			this->ed.checked_bgimagename_p = new string(*c.ed.checked_bgimagename_p);
		if (c.id.ischecked_selbgimagepath_p)
			this->ed.checked_selbgimagepath_p = new string(*c.ed.checked_selbgimagepath_p);
		if (c.id.ischecked_selbgimagename_p)
			this->ed.checked_selbgimagename_p = new string(*c.ed.checked_selbgimagename_p);
		if (c.id.ischecked_bgimagepath_i)
			this->ed.checked_bgimagepath_i = new string(*c.ed.checked_bgimagepath_i);
		if (c.id.ischecked_bgimagename_i)
			this->ed.checked_bgimagename_i = new string(*c.ed.checked_bgimagename_i);
		if (c.id.ischecked_selbgimagepath_i)
			this->ed.checked_selbgimagepath_i = new string(*c.ed.checked_selbgimagepath_i);
		if (c.id.ischecked_selbgimagename_i)
			this->ed.checked_selbgimagename_i = new string(*c.ed.checked_selbgimagename_i);
	}
	return *this;
}

void MMSCheckBoxWidgetClass::unsetAll() {
    unsetCheckedBgColor();
    unsetCheckedSelBgColor();
    unsetCheckedBgColor_p();
    unsetCheckedSelBgColor_p();
    unsetCheckedBgColor_i();
    unsetCheckedSelBgColor_i();

    unsetCheckedBgImagePath();
    unsetCheckedBgImageName();
    unsetCheckedSelBgImagePath();
    unsetCheckedSelBgImageName();
    unsetCheckedBgImagePath_p();
    unsetCheckedBgImageName_p();
    unsetCheckedSelBgImagePath_p();
    unsetCheckedSelBgImageName_p();
    unsetCheckedBgImagePath_i();
    unsetCheckedBgImageName_i();
    unsetCheckedSelBgImagePath_i();
    unsetCheckedSelBgImageName_i();

    unsetChecked();
}

void MMSCheckBoxWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
	    unsetCheckedBgImagePath();
	    unsetCheckedSelBgImagePath();
	    unsetCheckedBgImagePath_p();
	    unsetCheckedSelBgImagePath_p();
	    unsetCheckedBgImagePath_i();
	    unsetCheckedSelBgImagePath_i();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor:
	            setCheckedBgColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.a = attrval_int;
	            setCheckedBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.r = attrval_int;
	            setCheckedBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.g = attrval_int;
	            setCheckedBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.b = attrval_int;
	            setCheckedBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor:
	            setCheckedSelBgColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p:
	            setCheckedBgColor_p(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.a = attrval_int;
	            setCheckedBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.r = attrval_int;
	            setCheckedBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.g = attrval_int;
	            setCheckedBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.b = attrval_int;
	            setCheckedBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p:
	            setCheckedSelBgColor_p(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor_p(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i:
	            setCheckedBgColor_i(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.a = attrval_int;
	            setCheckedBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.r = attrval_int;
	            setCheckedBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.g = attrval_int;
	            setCheckedBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.b = attrval_int;
	            setCheckedBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i:
	            setCheckedSelBgColor_i(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor_i(color);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage:
	            if (*attrval_str)
	                setCheckedBgImagePath("");
	            else
	                setCheckedBgImagePath((path)?*path:"");
	            setCheckedBgImageName(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_path:
	            if (*attrval_str)
	                setCheckedBgImagePath(attrval_str);
	            else
	                setCheckedBgImagePath((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_name:
	            setCheckedBgImageName(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage:
	            if (*attrval_str)
	                setCheckedSelBgImagePath("");
	            else
	                setCheckedSelBgImagePath((path)?*path:"");
	            setCheckedSelBgImageName(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_path:
	            if (*attrval_str)
	                setCheckedSelBgImagePath(attrval_str);
	            else
	                setCheckedSelBgImagePath((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_name:
	            setCheckedSelBgImageName(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p:
	            if (*attrval_str)
	                setCheckedBgImagePath_p("");
	            else
	                setCheckedBgImagePath_p((path)?*path:"");
	            setCheckedBgImageName_p(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p_path:
	            if (*attrval_str)
	                setCheckedBgImagePath_p(attrval_str);
	            else
	                setCheckedBgImagePath_p((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p_name:
	            setCheckedBgImageName_p(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p:
	            if (*attrval_str)
	                setCheckedSelBgImagePath_p("");
	            else
	                setCheckedSelBgImagePath_p((path)?*path:"");
	            setCheckedSelBgImageName_p(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p_path:
	            if (*attrval_str)
	                setCheckedSelBgImagePath_p(attrval_str);
	            else
	                setCheckedSelBgImagePath_p((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p_name:
	            setCheckedSelBgImageName_p(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i:
	            if (*attrval_str)
	                setCheckedBgImagePath_i("");
	            else
	                setCheckedBgImagePath_i((path)?*path:"");
	            setCheckedBgImageName_i(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i_path:
	            if (*attrval_str)
	                setCheckedBgImagePath_i(attrval_str);
	            else
	                setCheckedBgImagePath_i((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i_name:
	            setCheckedBgImageName_i(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i:
	            if (*attrval_str)
	                setCheckedSelBgImagePath_i("");
	            else
	                setCheckedSelBgImagePath_i((path)?*path:"");
	            setCheckedSelBgImageName_i(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i_path:
	            if (*attrval_str)
	                setCheckedSelBgImagePath_i(attrval_str);
	            else
	                setCheckedSelBgImagePath_i((path)?*path:"");
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i_name:
	            setCheckedSelBgImageName_i(attrval_str);
	            break;
			case MMSGUI_CHECKBOXWIDGET_ATTR::MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked:
	            setChecked((attrval_int) ? true : false);
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
            if (ISATTRNAME(checked_bgcolor)) {
	            setCheckedBgColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_bgcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.a = attrval_int;
	            setCheckedBgColor(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.r = attrval_int;
	            setCheckedBgColor(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.g = attrval_int;
	            setCheckedBgColor(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor()) getCheckedBgColor(color);
	            color.b = attrval_int;
	            setCheckedBgColor(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor)) {
	            setCheckedSelBgColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_selbgcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor()) getCheckedSelBgColor(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_p)) {
	            setCheckedBgColor_p(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_bgcolor_p_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.a = attrval_int;
	            setCheckedBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_p_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.r = attrval_int;
	            setCheckedBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_p_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.g = attrval_int;
	            setCheckedBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_p()) getCheckedBgColor_p(color);
	            color.b = attrval_int;
	            setCheckedBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_p)) {
	            setCheckedSelBgColor_p(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_selbgcolor_p_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_p_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_p_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_p_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_p()) getCheckedSelBgColor_p(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_i)) {
	            setCheckedBgColor_i(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_bgcolor_i_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.a = attrval_int;
	            setCheckedBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_i_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.r = attrval_int;
	            setCheckedBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_i_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.g = attrval_int;
	            setCheckedBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_bgcolor_i_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedBgColor_i()) getCheckedBgColor_i(color);
	            color.b = attrval_int;
	            setCheckedBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_i)) {
	            setCheckedSelBgColor_i(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(checked_selbgcolor_i_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.a = attrval_int;
	            setCheckedSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_i_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.r = attrval_int;
	            setCheckedSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_i_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.g = attrval_int;
	            setCheckedSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_selbgcolor_i_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isCheckedSelBgColor_i()) getCheckedSelBgColor_i(color);
	            color.b = attrval_int;
	            setCheckedSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(checked_bgimage)) {
	            if (*attrval_str)
	                setCheckedBgImagePath("");
	            else
	                setCheckedBgImagePath((path)?*path:"");
	            setCheckedBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(checked_bgimage_path)) {
	            if (*attrval_str)
	                setCheckedBgImagePath(attrval_str);
	            else
	                setCheckedBgImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_bgimage_name)) {
	            setCheckedBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath("");
	            else
	                setCheckedSelBgImagePath((path)?*path:"");
	            setCheckedSelBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage_path)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath(attrval_str);
	            else
	                setCheckedSelBgImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_selbgimage_name)) {
	            setCheckedSelBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(checked_bgimage_p)) {
	            if (*attrval_str)
	                setCheckedBgImagePath_p("");
	            else
	                setCheckedBgImagePath_p((path)?*path:"");
	            setCheckedBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(checked_bgimage_p_path)) {
	            if (*attrval_str)
	                setCheckedBgImagePath_p(attrval_str);
	            else
	                setCheckedBgImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_bgimage_p_name)) {
	            setCheckedBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage_p)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath_p("");
	            else
	                setCheckedSelBgImagePath_p((path)?*path:"");
	            setCheckedSelBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage_p_path)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath_p(attrval_str);
	            else
	                setCheckedSelBgImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_selbgimage_p_name)) {
	            setCheckedSelBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(checked_bgimage_i)) {
	            if (*attrval_str)
	                setCheckedBgImagePath_i("");
	            else
	                setCheckedBgImagePath_i((path)?*path:"");
	            setCheckedBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(checked_bgimage_i_path)) {
	            if (*attrval_str)
	                setCheckedBgImagePath_i(attrval_str);
	            else
	                setCheckedBgImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_bgimage_i_name)) {
	            setCheckedBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage_i)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath_i("");
	            else
	                setCheckedSelBgImagePath_i((path)?*path:"");
	            setCheckedSelBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(checked_selbgimage_i_path)) {
	            if (*attrval_str)
	                setCheckedSelBgImagePath_i(attrval_str);
	            else
	                setCheckedSelBgImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(checked_selbgimage_i_name)) {
	            setCheckedSelBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(checked)) {
	            setChecked((attrval_int) ? true : false);
            }
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
		if (!isCheckedBgImagePath())
		    setCheckedBgImagePath(*path);
		if (!isCheckedSelBgImagePath())
		    setCheckedSelBgImagePath(*path);
		if (!isCheckedBgImagePath_p())
		    setCheckedBgImagePath_p(*path);
		if (!isCheckedSelBgImagePath_p())
		    setCheckedSelBgImagePath_p(*path);
		if (!isCheckedBgImagePath_i())
		    setCheckedBgImagePath_i(*path);
		if (!isCheckedSelBgImagePath_i())
		    setCheckedSelBgImagePath_i(*path);
    }
}

void MMSCheckBoxWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSCheckBoxWidgetClass::getClassName() {
    return this->className;
}

void MMSCheckBoxWidgetClass::initCheckedBgColor() {
	MMSTHEMECLASS_INIT_BASIC(checked_bgcolor);
}

void MMSCheckBoxWidgetClass::freeCheckedBgColor() {
	MMSTHEMECLASS_FREE_BASIC(checked_bgcolor);
}

bool MMSCheckBoxWidgetClass::isCheckedBgColor() {
	MMSTHEMECLASS_ISSET(checked_bgcolor);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgColor() {
	MMSTHEMECLASS_UNSET(checked_bgcolor);
}

void MMSCheckBoxWidgetClass::setCheckedBgColor(const MMSFBColor &checked_bgcolor) {
	MMSTHEMECLASS_SET_BASIC(checked_bgcolor);
}

bool MMSCheckBoxWidgetClass::getCheckedBgColor(MMSFBColor &checked_bgcolor) {
	MMSTHEMECLASS_GET_BASIC(checked_bgcolor);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgColor() {
	MMSTHEMECLASS_INIT_BASIC(checked_selbgcolor);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgColor() {
	MMSTHEMECLASS_FREE_BASIC(checked_selbgcolor);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgColor() {
	MMSTHEMECLASS_ISSET(checked_selbgcolor);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgColor() {
	MMSTHEMECLASS_UNSET(checked_selbgcolor);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgColor(const MMSFBColor &checked_selbgcolor) {
	MMSTHEMECLASS_SET_BASIC(checked_selbgcolor);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgColor(MMSFBColor &checked_selbgcolor) {
	MMSTHEMECLASS_GET_BASIC(checked_selbgcolor);
}


void MMSCheckBoxWidgetClass::initCheckedBgColor_p() {
	MMSTHEMECLASS_INIT_BASIC(checked_bgcolor_p);
}

void MMSCheckBoxWidgetClass::freeCheckedBgColor_p() {
	MMSTHEMECLASS_FREE_BASIC(checked_bgcolor_p);
}

bool MMSCheckBoxWidgetClass::isCheckedBgColor_p() {
	MMSTHEMECLASS_ISSET(checked_bgcolor_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgColor_p() {
	MMSTHEMECLASS_UNSET(checked_bgcolor_p);
}

void MMSCheckBoxWidgetClass::setCheckedBgColor_p(const MMSFBColor &checked_bgcolor_p) {
	MMSTHEMECLASS_SET_BASIC(checked_bgcolor_p);
}

bool MMSCheckBoxWidgetClass::getCheckedBgColor_p(MMSFBColor &checked_bgcolor_p) {
	MMSTHEMECLASS_GET_BASIC(checked_bgcolor_p);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgColor_p() {
	MMSTHEMECLASS_INIT_BASIC(checked_selbgcolor_p);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgColor_p() {
	MMSTHEMECLASS_FREE_BASIC(checked_selbgcolor_p);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgColor_p() {
	MMSTHEMECLASS_ISSET(checked_selbgcolor_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgColor_p() {
	MMSTHEMECLASS_UNSET(checked_selbgcolor_p);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgColor_p(const MMSFBColor &checked_selbgcolor_p) {
	MMSTHEMECLASS_SET_BASIC(checked_selbgcolor_p);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgColor_p(MMSFBColor &checked_selbgcolor_p) {
	MMSTHEMECLASS_GET_BASIC(checked_selbgcolor_p);
}

void MMSCheckBoxWidgetClass::initCheckedBgColor_i() {
	MMSTHEMECLASS_INIT_BASIC(checked_bgcolor_i);
}

void MMSCheckBoxWidgetClass::freeCheckedBgColor_i() {
	MMSTHEMECLASS_FREE_BASIC(checked_bgcolor_i);
}

bool MMSCheckBoxWidgetClass::isCheckedBgColor_i() {
	MMSTHEMECLASS_ISSET(checked_bgcolor_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgColor_i() {
	MMSTHEMECLASS_UNSET(checked_bgcolor_i);
}

void MMSCheckBoxWidgetClass::setCheckedBgColor_i(const MMSFBColor &checked_bgcolor_i) {
	MMSTHEMECLASS_SET_BASIC(checked_bgcolor_i);
}

bool MMSCheckBoxWidgetClass::getCheckedBgColor_i(MMSFBColor &checked_bgcolor_i) {
	MMSTHEMECLASS_GET_BASIC(checked_bgcolor_i);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgColor_i() {
	MMSTHEMECLASS_INIT_BASIC(checked_selbgcolor_i);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgColor_i() {
	MMSTHEMECLASS_FREE_BASIC(checked_selbgcolor_i);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgColor_i() {
	MMSTHEMECLASS_ISSET(checked_selbgcolor_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgColor_i() {
	MMSTHEMECLASS_UNSET(checked_selbgcolor_i);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgColor_i(const MMSFBColor &checked_selbgcolor_i) {
	MMSTHEMECLASS_SET_BASIC(checked_selbgcolor_i);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgColor_i(MMSFBColor &checked_selbgcolor_i) {
	MMSTHEMECLASS_GET_BASIC(checked_selbgcolor_i);
}


void MMSCheckBoxWidgetClass::initCheckedBgImagePath() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagepath);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImagePath() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagepath);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImagePath() {
	MMSTHEMECLASS_ISSET(checked_bgimagepath);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImagePath() {
	MMSTHEMECLASS_UNSET(checked_bgimagepath);
}

void MMSCheckBoxWidgetClass::setCheckedBgImagePath(const string &checked_bgimagepath) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagepath);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImagePath(string &checked_bgimagepath) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagepath);
}



void MMSCheckBoxWidgetClass::initCheckedBgImageName() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagename);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImageName() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagename);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImageName() {
	MMSTHEMECLASS_ISSET(checked_bgimagename);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImageName() {
	MMSTHEMECLASS_UNSET(checked_bgimagename);
}

void MMSCheckBoxWidgetClass::setCheckedBgImageName(const string &checked_bgimagename) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagename);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImageName(string &checked_bgimagename) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagename);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImagePath() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagepath);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImagePath() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagepath);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImagePath() {
	MMSTHEMECLASS_ISSET(checked_selbgimagepath);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImagePath() {
	MMSTHEMECLASS_UNSET(checked_selbgimagepath);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImagePath(const string &checked_selbgimagepath) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagepath);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImagePath(string &checked_selbgimagepath) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagepath);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImageName() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagename);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImageName() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagename);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImageName() {
	MMSTHEMECLASS_ISSET(checked_selbgimagename);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImageName() {
	MMSTHEMECLASS_UNSET(checked_selbgimagename);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImageName(const string &checked_selbgimagename) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagename);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImageName(string &checked_selbgimagename) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagename);
}

void MMSCheckBoxWidgetClass::initCheckedBgImagePath_p() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagepath_p);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImagePath_p() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagepath_p);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImagePath_p() {
	MMSTHEMECLASS_ISSET(checked_bgimagepath_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImagePath_p() {
	MMSTHEMECLASS_UNSET(checked_bgimagepath_p);
}

void MMSCheckBoxWidgetClass::setCheckedBgImagePath_p(const string &checked_bgimagepath_p) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagepath_p);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImagePath_p(string &checked_bgimagepath_p) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagepath_p);
}

void MMSCheckBoxWidgetClass::initCheckedBgImageName_p() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagename_p);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImageName_p() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagename_p);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImageName_p() {
	MMSTHEMECLASS_ISSET(checked_bgimagename_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImageName_p() {
	MMSTHEMECLASS_UNSET(checked_bgimagename_p);
}

void MMSCheckBoxWidgetClass::setCheckedBgImageName_p(const string &checked_bgimagename_p) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagename_p);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImageName_p(string &checked_bgimagename_p) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagename_p);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImagePath_p() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagepath_p);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImagePath_p() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagepath_p);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImagePath_p() {
	MMSTHEMECLASS_ISSET(checked_selbgimagepath_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImagePath_p() {
	MMSTHEMECLASS_UNSET(checked_selbgimagepath_p);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImagePath_p(const string &checked_selbgimagepath_p) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagepath_p);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImagePath_p(string &checked_selbgimagepath_p) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagepath_p);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImageName_p() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagename_p);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImageName_p() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagename_p);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImageName_p() {
	MMSTHEMECLASS_ISSET(checked_selbgimagename_p);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImageName_p() {
	MMSTHEMECLASS_UNSET(checked_selbgimagename_p);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImageName_p(const string &checked_selbgimagename_p) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagename_p);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImageName_p(string &checked_selbgimagename_p) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagename_p);
}


void MMSCheckBoxWidgetClass::initCheckedBgImagePath_i() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagepath_i);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImagePath_i() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagepath_i);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImagePath_i() {
	MMSTHEMECLASS_ISSET(checked_bgimagepath_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImagePath_i() {
	MMSTHEMECLASS_UNSET(checked_bgimagepath_i);
}

void MMSCheckBoxWidgetClass::setCheckedBgImagePath_i(const string &checked_bgimagepath_i) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagepath_i);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImagePath_i(string &checked_bgimagepath_i) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagepath_i);
}

void MMSCheckBoxWidgetClass::initCheckedBgImageName_i() {
	MMSTHEMECLASS_INIT_STRING(checked_bgimagename_i);
}

void MMSCheckBoxWidgetClass::freeCheckedBgImageName_i() {
	MMSTHEMECLASS_FREE_STRING(checked_bgimagename_i);
}

bool MMSCheckBoxWidgetClass::isCheckedBgImageName_i() {
	MMSTHEMECLASS_ISSET(checked_bgimagename_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedBgImageName_i() {
	MMSTHEMECLASS_UNSET(checked_bgimagename_i);
}

void MMSCheckBoxWidgetClass::setCheckedBgImageName_i(const string &checked_bgimagename_i) {
	MMSTHEMECLASS_SET_STRING(checked_bgimagename_i);
}

bool MMSCheckBoxWidgetClass::getCheckedBgImageName_i(string &checked_bgimagename_i) {
	MMSTHEMECLASS_GET_STRING(checked_bgimagename_i);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImagePath_i() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagepath_i);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImagePath_i() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagepath_i);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImagePath_i() {
	MMSTHEMECLASS_ISSET(checked_selbgimagepath_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImagePath_i() {
	MMSTHEMECLASS_UNSET(checked_selbgimagepath_i);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImagePath_i(const string &checked_selbgimagepath_i) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagepath_i);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImagePath_i(string &checked_selbgimagepath_i) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagepath_i);
}

void MMSCheckBoxWidgetClass::initCheckedSelBgImageName_i() {
	MMSTHEMECLASS_INIT_STRING(checked_selbgimagename_i);
}

void MMSCheckBoxWidgetClass::freeCheckedSelBgImageName_i() {
	MMSTHEMECLASS_FREE_STRING(checked_selbgimagename_i);
}

bool MMSCheckBoxWidgetClass::isCheckedSelBgImageName_i() {
	MMSTHEMECLASS_ISSET(checked_selbgimagename_i);
}

void MMSCheckBoxWidgetClass::unsetCheckedSelBgImageName_i() {
	MMSTHEMECLASS_UNSET(checked_selbgimagename_i);
}

void MMSCheckBoxWidgetClass::setCheckedSelBgImageName_i(const string &checked_selbgimagename_i) {
	MMSTHEMECLASS_SET_STRING(checked_selbgimagename_i);
}

bool MMSCheckBoxWidgetClass::getCheckedSelBgImageName_i(string &checked_selbgimagename_i) {
	MMSTHEMECLASS_GET_STRING(checked_selbgimagename_i);
}


void MMSCheckBoxWidgetClass::initChecked() {
    this->id.checked = false;
    MMSTHEMECLASS_INIT_BASIC(checked);
}

void MMSCheckBoxWidgetClass::freeChecked() {
    this->id.checked = false;
    MMSTHEMECLASS_FREE_BASIC(checked);
}

bool MMSCheckBoxWidgetClass::isChecked() {
	MMSTHEMECLASS_ISSET(checked);
}

void MMSCheckBoxWidgetClass::unsetChecked() {
    this->id.checked = false;
    MMSTHEMECLASS_UNSET(checked);
}

void MMSCheckBoxWidgetClass::setChecked(bool checked) {
	MMSTHEMECLASS_SET_BASIC(checked);
}

bool MMSCheckBoxWidgetClass::getChecked(bool &checked) {
	MMSTHEMECLASS_GET_BASIC(checked);
}


