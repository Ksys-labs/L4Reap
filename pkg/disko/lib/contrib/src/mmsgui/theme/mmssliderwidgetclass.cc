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

#include "mmsgui/theme/mmssliderwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_SLIDERWIDGET_ATTR_I[] = MMSGUI_SLIDERWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_SLIDERWIDGET_ATTR_I[MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_SLIDERWIDGET_ATTR_I[MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_##aname].type


MMSSliderWidgetClass::MMSSliderWidgetClass() {
    unsetAll();
}

void MMSSliderWidgetClass::unsetAll() {
    this->className = "";
    unsetImagePath();
    unsetImageName();
    unsetSelImagePath();
    unsetSelImageName();
    unsetImagePath_p();
    unsetImageName_p();
    unsetSelImagePath_p();
    unsetSelImageName_p();
    unsetImagePath_i();
    unsetImageName_i();
    unsetSelImagePath_i();
    unsetSelImageName_i();
    unsetPosition();
    unsetBarImagePath();
    unsetBarImageName();
    unsetSelBarImagePath();
    unsetSelBarImageName();
}

void MMSSliderWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
        unsetImagePath();
        unsetSelImagePath();
        unsetImagePath_p();
        unsetSelImagePath_p();
        unsetImagePath_i();
        unsetSelImagePath_i();
        unsetBarImagePath();
        unsetSelBarImagePath();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image:
	            if (*attrval_str)
	                setImagePath("");
	            else
	                setImagePath((path)?*path:"");
	            setImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_path:
	            if (*attrval_str)
	                setImagePath(attrval_str);
	            else
	                setImagePath((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_name:
	            setImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage:
	            if (*attrval_str)
	                setSelImagePath("");
	            else
	                setSelImagePath((path)?*path:"");
	            setSelImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_path:
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_name:
	            setSelImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_p:
	            if (*attrval_str)
	                setImagePath_p("");
	            else
	                setImagePath_p((path)?*path:"");
	            setImageName_p(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_p_path:
	            if (*attrval_str)
	                setImagePath_p(attrval_str);
	            else
	                setImagePath_p((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_p_name:
	            setImageName_p(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_p:
	            if (*attrval_str)
	                setSelImagePath_p("");
	            else
	                setSelImagePath_p((path)?*path:"");
	            setSelImageName_p(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_p_path:
	            if (*attrval_str)
	                setSelImagePath_p(attrval_str);
	            else
	                setSelImagePath_p((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_p_name:
	            setSelImageName_p(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_i:
	            if (*attrval_str)
	                setImagePath_i("");
	            else
	                setImagePath_i((path)?*path:"");
	            setImageName_i(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_i_path:
	            if (*attrval_str)
	                setImagePath_i(attrval_str);
	            else
	                setImagePath_i((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_image_i_name:
	            setImageName_i(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_i:
	            if (*attrval_str)
	                setSelImagePath_i("");
	            else
	                setSelImagePath_i((path)?*path:"");
	            setSelImageName_i(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_i_path:
	            if (*attrval_str)
	                setSelImagePath_i(attrval_str);
	            else
	                setSelImagePath_i((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selimage_i_name:
	            setSelImageName_i(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_position:
	            setPosition(attrval_int);
				break;


			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_barimage:
	            if (*attrval_str)
	                setBarImagePath("");
	            else
	                setBarImagePath((path)?*path:"");
	            setBarImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_barimage_path:
	            if (*attrval_str)
	                setBarImagePath(attrval_str);
	            else
	                setBarImagePath((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_barimage_name:
	            setBarImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selbarimage:
	            if (*attrval_str)
	                setSelBarImagePath("");
	            else
	                setSelBarImagePath((path)?*path:"");
	            setSelBarImageName(attrval_str);
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selbarimage_path:
	            if (*attrval_str)
	                setSelBarImagePath(attrval_str);
	            else
	                setSelBarImagePath((path)?*path:"");
				break;
			case MMSGUI_SLIDERWIDGET_ATTR::MMSGUI_SLIDERWIDGET_ATTR_IDS_selbarimage_name:
	            setSelBarImageName(attrval_str);
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
            if (ISATTRNAME(image)) {
	            if (*attrval_str)
	                setImagePath("");
	            else
	                setImagePath((path)?*path:"");
	            setImageName(attrval_str);
            }
            else
            if (ISATTRNAME(image_path)) {
	            if (*attrval_str)
	                setImagePath(attrval_str);
	            else
	                setImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(image_name)) {
	            setImageName(attrval_str);
            }
            else
            if (ISATTRNAME(selimage)) {
	            if (*attrval_str)
	                setSelImagePath("");
	            else
	                setSelImagePath((path)?*path:"");
	            setSelImageName(attrval_str);
            }
            else
            if (ISATTRNAME(selimage_path)) {
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(selimage_name)) {
	            setSelImageName(attrval_str);
            }
            else
            if (ISATTRNAME(image_p)) {
	            if (*attrval_str)
	                setImagePath_p("");
	            else
	                setImagePath_p((path)?*path:"");
	            setImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(image_p_path)) {
	            if (*attrval_str)
	                setImagePath_p(attrval_str);
	            else
	                setImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(image_p_name)) {
	            setImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(selimage_p)) {
	            if (*attrval_str)
	                setSelImagePath_p("");
	            else
	                setSelImagePath_p((path)?*path:"");
	            setSelImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(selimage_p_path)) {
	            if (*attrval_str)
	                setSelImagePath_p(attrval_str);
	            else
	                setSelImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(selimage_p_name)) {
	            setSelImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(image_i)) {
	            if (*attrval_str)
	                setImagePath_i("");
	            else
	                setImagePath_i((path)?*path:"");
	            setImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(image_i_path)) {
	            if (*attrval_str)
	                setImagePath_i(attrval_str);
	            else
	                setImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(image_i_name)) {
	            setImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(selimage_i)) {
	            if (*attrval_str)
	                setSelImagePath_i("");
	            else
	                setSelImagePath_i((path)?*path:"");
	            setSelImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(selimage_i_path)) {
	            if (*attrval_str)
	                setSelImagePath_i(attrval_str);
	            else
	                setSelImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(selimage_i_name)) {
	            setSelImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(position)) {
	            setPosition(attrval_int);
			}
            else
			if (ISATTRNAME(barimage)) {
				if (*attrval_str)
					setBarImagePath("");
				else
					setBarImagePath((path)?*path:"");
				setBarImageName(attrval_str);
			}
			else
			if (ISATTRNAME(barimage_path)) {
				if (*attrval_str)
					setBarImagePath(attrval_str);
				else
					setBarImagePath((path)?*path:"");
			}
			else
			if (ISATTRNAME(barimage_name)) {
				setBarImageName(attrval_str);
			}
			else
			if (ISATTRNAME(selbarimage)) {
				if (*attrval_str)
					setSelBarImagePath("");
				else
					setSelBarImagePath((path)?*path:"");
				setSelBarImageName(attrval_str);
			}
			else
			if (ISATTRNAME(selbarimage_path)) {
				if (*attrval_str)
					setSelBarImagePath(attrval_str);
				else
					setSelBarImagePath((path)?*path:"");
			}
			else
			if (ISATTRNAME(selbarimage_name)) {
				setSelBarImageName(attrval_str);
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
	    if (!isImagePath())
	        setImagePath(*path);
	    if (!isSelImagePath())
	        setSelImagePath(*path);
	    if (!isImagePath_p())
	        setImagePath_p(*path);
	    if (!isSelImagePath_p())
	        setSelImagePath_p(*path);
	    if (!isImagePath_i())
	        setImagePath_i(*path);
	    if (!isSelImagePath_i())
	        setSelImagePath_i(*path);
	    if (!isBarImagePath())
	        setBarImagePath(*path);
	    if (!isSelBarImagePath())
	        setSelBarImagePath(*path);
    }
}

void MMSSliderWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSSliderWidgetClass::getClassName() {
    return this->className;
}

bool MMSSliderWidgetClass::isImagePath() {
    return this->isimagepath;
}

void MMSSliderWidgetClass::setImagePath(string imagepath) {
    this->imagepath = imagepath;
    this->isimagepath = true;
}

void MMSSliderWidgetClass::unsetImagePath() {
    this->isimagepath = false;
}

string MMSSliderWidgetClass::getImagePath() {
    return this->imagepath;
}

bool MMSSliderWidgetClass::isImageName() {
    return this->isimagename;
}

void MMSSliderWidgetClass::setImageName(string imagename) {
    this->imagename = imagename;
    this->isimagename = true;
}

void MMSSliderWidgetClass::unsetImageName() {
    this->isimagename = false;
}

string MMSSliderWidgetClass::getImageName() {
    return this->imagename;
}

bool MMSSliderWidgetClass::isSelImagePath() {
    return this->isselimagepath;
}

void MMSSliderWidgetClass::setSelImagePath(string selimagepath) {
    this->selimagepath = selimagepath;
    this->isselimagepath = true;
}

void MMSSliderWidgetClass::unsetSelImagePath() {
    this->isselimagepath = false;
}

string MMSSliderWidgetClass::getSelImagePath() {
    return this->selimagepath;
}

bool MMSSliderWidgetClass::isSelImageName() {
    return this->isselimagename;
}

void MMSSliderWidgetClass::setSelImageName(string selimagename) {
    this->selimagename = selimagename;
    this->isselimagename = true;
}

void MMSSliderWidgetClass::unsetSelImageName() {
    this->isselimagename = false;
}

string MMSSliderWidgetClass::getSelImageName() {
    return this->selimagename;
}

bool MMSSliderWidgetClass::isImagePath_p() {
    return this->isimagepath_p;
}

void MMSSliderWidgetClass::setImagePath_p(string imagepath_p) {
    this->imagepath_p = imagepath_p;
    this->isimagepath_p = true;
}

void MMSSliderWidgetClass::unsetImagePath_p() {
    this->isimagepath_p = false;
}

string MMSSliderWidgetClass::getImagePath_p() {
    return this->imagepath_p;
}

bool MMSSliderWidgetClass::isImageName_p() {
    return this->isimagename_p;
}

void MMSSliderWidgetClass::setImageName_p(string imagename_p) {
    this->imagename_p = imagename_p;
    this->isimagename_p = true;
}

void MMSSliderWidgetClass::unsetImageName_p() {
    this->isimagename_p = false;
}

string MMSSliderWidgetClass::getImageName_p() {
    return this->imagename_p;
}

bool MMSSliderWidgetClass::isSelImagePath_p() {
    return this->isselimagepath_p;
}

void MMSSliderWidgetClass::setSelImagePath_p(string selimagepath_p) {
    this->selimagepath_p = selimagepath_p;
    this->isselimagepath_p = true;
}

void MMSSliderWidgetClass::unsetSelImagePath_p() {
    this->isselimagepath_p = false;
}

string MMSSliderWidgetClass::getSelImagePath_p() {
    return this->selimagepath_p;
}

bool MMSSliderWidgetClass::isSelImageName_p() {
    return this->isselimagename_p;
}

void MMSSliderWidgetClass::setSelImageName_p(string selimagename_p) {
    this->selimagename_p = selimagename_p;
    this->isselimagename_p = true;
}

void MMSSliderWidgetClass::unsetSelImageName_p() {
    this->isselimagename_p = false;
}

string MMSSliderWidgetClass::getSelImageName_p() {
    return this->selimagename_p;
}

bool MMSSliderWidgetClass::isImagePath_i() {
    return this->isimagepath_i;
}

void MMSSliderWidgetClass::setImagePath_i(string imagepath_i) {
    this->imagepath_i = imagepath_i;
    this->isimagepath_i = true;
}

void MMSSliderWidgetClass::unsetImagePath_i() {
    this->isimagepath_i = false;
}

string MMSSliderWidgetClass::getImagePath_i() {
    return this->imagepath_i;
}

bool MMSSliderWidgetClass::isImageName_i() {
    return this->isimagename_i;
}

void MMSSliderWidgetClass::setImageName_i(string imagename_i) {
    this->imagename_i = imagename_i;
    this->isimagename_i = true;
}

void MMSSliderWidgetClass::unsetImageName_i() {
    this->isimagename_i = false;
}

string MMSSliderWidgetClass::getImageName_i() {
    return this->imagename_i;
}

bool MMSSliderWidgetClass::isSelImagePath_i() {
    return this->isselimagepath_i;
}

void MMSSliderWidgetClass::setSelImagePath_i(string selimagepath_i) {
    this->selimagepath_i = selimagepath_i;
    this->isselimagepath_i = true;
}

void MMSSliderWidgetClass::unsetSelImagePath_i() {
    this->isselimagepath_i = false;
}

string MMSSliderWidgetClass::getSelImagePath_i() {
    return this->selimagepath_i;
}

bool MMSSliderWidgetClass::isSelImageName_i() {
    return this->isselimagename_i;
}

void MMSSliderWidgetClass::setSelImageName_i(string selimagename_i) {
    this->selimagename_i = selimagename_i;
    this->isselimagename_i = true;
}

void MMSSliderWidgetClass::unsetSelImageName_i() {
    this->isselimagename_i = false;
}

string MMSSliderWidgetClass::getSelImageName_i() {
    return this->selimagename_i;
}

bool MMSSliderWidgetClass::isPosition() {
    return this->isposition;
}

void MMSSliderWidgetClass::setPosition(unsigned int position) {
    this->position = position;
    this->isposition = true;
}

void MMSSliderWidgetClass::unsetPosition() {
    this->isposition = false;
}

unsigned int MMSSliderWidgetClass::getPosition() {
    return this->position;
}





bool MMSSliderWidgetClass::isBarImagePath() {
    return this->isbarimagepath;
}

void MMSSliderWidgetClass::setBarImagePath(string barimagepath) {
    this->barimagepath = barimagepath;
    this->isbarimagepath = true;
}

void MMSSliderWidgetClass::unsetBarImagePath() {
    this->isbarimagepath = false;
}

string MMSSliderWidgetClass::getBarImagePath() {
    return this->barimagepath;
}

bool MMSSliderWidgetClass::isBarImageName() {
    return this->isbarimagename;
}

void MMSSliderWidgetClass::setBarImageName(string barimagename) {
    this->barimagename = barimagename;
    this->isbarimagename = true;
}

void MMSSliderWidgetClass::unsetBarImageName() {
    this->isbarimagename = false;
}

string MMSSliderWidgetClass::getBarImageName() {
    return this->barimagename;
}

bool MMSSliderWidgetClass::isSelBarImagePath() {
    return this->isselbarimagepath;
}

void MMSSliderWidgetClass::setSelBarImagePath(string selbarimagepath) {
    this->selbarimagepath = selbarimagepath;
    this->isselbarimagepath = true;
}

void MMSSliderWidgetClass::unsetSelBarImagePath() {
    this->isselbarimagepath = false;
}

string MMSSliderWidgetClass::getSelBarImagePath() {
    return this->selbarimagepath;
}

bool MMSSliderWidgetClass::isSelBarImageName() {
    return this->isselbarimagename;
}

void MMSSliderWidgetClass::setSelBarImageName(string selbarimagename) {
    this->selbarimagename = selbarimagename;
    this->isselbarimagename = true;
}

void MMSSliderWidgetClass::unsetSelBarImageName() {
    this->isselbarimagename = false;
}

string MMSSliderWidgetClass::getSelBarImageName() {
    return this->selbarimagename;
}


