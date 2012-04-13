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

#include "mmsgui/theme/mmsborderclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_BORDER_ATTR_I[] = MMSGUI_BORDER_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_BORDER_ATTR_I[MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_BORDER_ATTR_I[MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_##aname].type


MMSBorderClass::MMSBorderClass() {
    initColor();
    initSelColor();

    initImagePath();
    initImageNames();
    initSelImagePath();
    initSelImageNames();

    initThickness();
    initMargin();
    initRCorners();
}

MMSBorderClass::~MMSBorderClass() {
    freeColor();
    freeSelColor();

    freeImagePath();
    freeImageNames();
    freeSelImagePath();
    freeSelImageNames();

    freeThickness();
    freeMargin();
    freeRCorners();
}

MMSBorderClass &MMSBorderClass::operator=(const MMSBorderClass &c) {
	if (this != &c) {
		/* copy internal fix data area */
		this->id = c.id;

		/* copy external data */
		memset(&(this->ed), 0, sizeof(this->ed));
		if (c.id.isimagepath)
			this->ed.imagepath = new string(*c.ed.imagepath);
		if (c.id.isselimagepath)
			this->ed.selimagepath = new string(*c.ed.selimagepath);
		if (c.id.isimagenames)
			for (int i = 0; i < 8; i++)
				this->ed.imagenames[i] = new string(*c.ed.imagenames[i]);
		if (c.id.isselimagenames)
			for (int i = 0; i < 8; i++)
				this->ed.selimagenames[i] = new string(*c.ed.selimagenames[i]);
	}
	return *this;
}

void MMSBorderClass::unsetAll() {
    unsetColor();
    unsetSelColor();

    unsetImagePath();
    unsetImageNames();
    unsetSelImagePath();
    unsetSelImageNames();

    unsetThickness();
    unsetMargin();
    unsetRCorners();
}

void MMSBorderClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
	    unsetImagePath();
	    unsetSelImagePath();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_color:
	            setColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_color_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.a = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_color_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.r = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_color_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.g = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_color_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.b = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selcolor:
	            setSelColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.a = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.r = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.g = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.b = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_path:
	            if (*attrval_str)
	                setImagePath(attrval_str);
	            else
	                setImagePath((path)?*path:"");
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_top_left:
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_top:
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_top_right:
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_right:
	            setImageNames(MMSBORDER_IMAGE_NUM_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_bottom_right:
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_bottom:
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_bottom_left:
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_image_left:
	            setImageNames(MMSBORDER_IMAGE_NUM_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_path:
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_top_left:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_top:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_top_right:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_right:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom_right:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom_left:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_selimage_left:
	            setSelImageNames(MMSBORDER_IMAGE_NUM_LEFT, attrval_str);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_thickness:
	            setThickness(attrval_int);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_margin:
	            setMargin(attrval_int);
	            break;
			case MMSGUI_BORDER_ATTR::MMSGUI_BORDER_ATTR_IDS_border_rcorners:
	            setRCorners((attrval_int) ? true : false);
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
            if (ISATTRNAME(border_color)) {
	            setColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(border_color_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.a = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(border_color_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.r = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(border_color_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.g = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(border_color_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) getColor(color);
	            color.b = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(border_selcolor)) {
	            setSelColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(border_selcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.a = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(border_selcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.r = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(border_selcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.g = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(border_selcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) getSelColor(color);
	            color.b = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(border_image_path)) {
	            if (*attrval_str)
	                setImagePath(attrval_str);
	            else
	                setImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(border_image_top_left)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_top)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_top_right)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_right)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_bottom_right)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_bottom)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_bottom_left)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_image_left)) {
	            setImageNames(MMSBORDER_IMAGE_NUM_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_path)) {
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(border_selimage_top_left)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_top)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_top_right)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_right)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_bottom_right)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_bottom)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_bottom_left)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_selimage_left)) {
	            setSelImageNames(MMSBORDER_IMAGE_NUM_LEFT, attrval_str);
            }
            else
            if (ISATTRNAME(border_thickness)) {
	            setThickness(attrval_int);
            }
            else
            if (ISATTRNAME(border_margin)) {
	            setMargin(attrval_int);
            }
            else
            if (ISATTRNAME(border_rcorners)) {
	            setRCorners((attrval_int) ? true : false);
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
    }
}


void MMSBorderClass::initColor() {
	MMSTHEMECLASS_INIT_BASIC(color);
}

void MMSBorderClass::freeColor() {
	MMSTHEMECLASS_FREE_BASIC(color);
}

bool MMSBorderClass::isColor() {
	MMSTHEMECLASS_ISSET(color);
}

void MMSBorderClass::unsetColor() {
	MMSTHEMECLASS_UNSET(color);
}

void MMSBorderClass::setColor(const MMSFBColor &color) {
	MMSTHEMECLASS_SET_BASIC(color);
}

bool MMSBorderClass::getColor(MMSFBColor &color) {
	MMSTHEMECLASS_GET_BASIC(color);
}


void MMSBorderClass::initSelColor() {
	MMSTHEMECLASS_INIT_BASIC(selcolor);
}

void MMSBorderClass::freeSelColor() {
	MMSTHEMECLASS_FREE_BASIC(selcolor);
}

bool MMSBorderClass::isSelColor() {
	MMSTHEMECLASS_ISSET(selcolor);
}

void MMSBorderClass::unsetSelColor() {
	MMSTHEMECLASS_UNSET(selcolor);
}

void MMSBorderClass::setSelColor(const MMSFBColor &selcolor) {
	MMSTHEMECLASS_SET_BASIC(selcolor);
}

bool MMSBorderClass::getSelColor(MMSFBColor &selcolor) {
	MMSTHEMECLASS_GET_BASIC(selcolor);
}



void MMSBorderClass::initImagePath() {
	MMSTHEMECLASS_INIT_STRING(imagepath);
}

void MMSBorderClass::freeImagePath() {
	MMSTHEMECLASS_FREE_STRING(imagepath);
}

bool MMSBorderClass::isImagePath() {
	MMSTHEMECLASS_ISSET(imagepath);
}

void MMSBorderClass::unsetImagePath() {
	MMSTHEMECLASS_UNSET(imagepath);
}

void MMSBorderClass::setImagePath(const string &imagepath) {
	MMSTHEMECLASS_SET_STRING(imagepath);
}

bool MMSBorderClass::getImagePath(string &imagepath) {
	MMSTHEMECLASS_GET_STRING(imagepath);
}




void MMSBorderClass::initImageNames() {
	MMSTHEMECLASS_INIT_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

void MMSBorderClass::freeImageNames() {
	MMSTHEMECLASS_FREE_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

bool MMSBorderClass::isImageNames() {
	MMSTHEMECLASS_ISSET(imagenames);
}

void MMSBorderClass::unsetImageNames() {
	MMSTHEMECLASS_UNSET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

void MMSBorderClass::setImageNames(const string &imagename_1, const string &imagename_2, const string &imagename_3, const string &imagename_4,
								   const string &imagename_5, const string &imagename_6, const string &imagename_7, const string &imagename_8) {
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_TOP_LEFT,imagename_1);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_TOP,imagename_2);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_TOP_RIGHT,imagename_3);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_RIGHT,imagename_4);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT,imagename_5);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_BOTTOM,imagename_6);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_BOTTOM_LEFT,imagename_7);
	MMSTHEMECLASS_SET_STRINGS(imagenames,MMSBORDER_IMAGE_NUM_LEFT,imagename_8);
}

void MMSBorderClass::setImageNames(MMSBORDER_IMAGE_NUM num, const string &imagename) {
	MMSTHEMECLASS_SET_STRINGS(imagenames,num,imagename);
}

bool MMSBorderClass::getImageNames(MMSBORDER_IMAGE_NUM num, string &imagename) {
	MMSTHEMECLASS_GET_STRINGS(imagenames,num,imagename);
}


void MMSBorderClass::initSelImagePath() {
	MMSTHEMECLASS_INIT_STRING(selimagepath);
}

void MMSBorderClass::freeSelImagePath() {
	MMSTHEMECLASS_FREE_STRING(selimagepath);
}

bool MMSBorderClass::isSelImagePath() {
	MMSTHEMECLASS_ISSET(selimagepath);
}

void MMSBorderClass::unsetSelImagePath() {
	MMSTHEMECLASS_UNSET(selimagepath);
}

void MMSBorderClass::setSelImagePath(const string &selimagepath) {
	MMSTHEMECLASS_SET_STRING(selimagepath);
}

bool MMSBorderClass::getSelImagePath(string &selimagepath) {
	MMSTHEMECLASS_GET_STRING(selimagepath);
}



void MMSBorderClass::initSelImageNames() {
	MMSTHEMECLASS_INIT_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

void MMSBorderClass::freeSelImageNames() {
	MMSTHEMECLASS_FREE_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

bool MMSBorderClass::isSelImageNames() {
	MMSTHEMECLASS_ISSET(selimagenames);
}

void MMSBorderClass::unsetSelImageNames() {
	MMSTHEMECLASS_UNSET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_SIZE);
}

void MMSBorderClass::setSelImageNames(const string &selimagename_1, const string &selimagename_2, const string &selimagename_3, const string &selimagename_4,
								      const string &selimagename_5, const string &selimagename_6, const string &selimagename_7, const string &selimagename_8) {
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_TOP_LEFT,selimagename_1);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_TOP,selimagename_2);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_TOP_RIGHT,selimagename_3);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_RIGHT,selimagename_4);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT,selimagename_5);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_BOTTOM,selimagename_6);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_BOTTOM_LEFT,selimagename_7);
	MMSTHEMECLASS_SET_STRINGS(selimagenames,MMSBORDER_IMAGE_NUM_LEFT,selimagename_8);
}

void MMSBorderClass::setSelImageNames(MMSBORDER_IMAGE_NUM num, const string &selimagename) {
	MMSTHEMECLASS_SET_STRINGS(selimagenames,num,selimagename);
}

bool MMSBorderClass::getSelImageNames(MMSBORDER_IMAGE_NUM num, string &selimagename) {
	MMSTHEMECLASS_GET_STRINGS(selimagenames,num,selimagename);
}


void MMSBorderClass::initThickness() {
	MMSTHEMECLASS_INIT_BASIC(thickness);
}

void MMSBorderClass::freeThickness() {
	MMSTHEMECLASS_FREE_BASIC(thickness);
}

bool MMSBorderClass::isThickness() {
	MMSTHEMECLASS_ISSET(thickness);
}

void MMSBorderClass::unsetThickness() {
	MMSTHEMECLASS_UNSET(thickness);
}

void MMSBorderClass::setThickness(unsigned int thickness) {
	MMSTHEMECLASS_SET_BASIC(thickness);
}

bool MMSBorderClass::getThickness(unsigned int &thickness) {
	MMSTHEMECLASS_GET_BASIC(thickness);
}

void MMSBorderClass::initMargin() {
	MMSTHEMECLASS_INIT_BASIC(margin);
}

void MMSBorderClass::freeMargin() {
	MMSTHEMECLASS_FREE_BASIC(margin);
}

bool MMSBorderClass::isMargin() {
	MMSTHEMECLASS_ISSET(margin);
}

void MMSBorderClass::unsetMargin() {
	MMSTHEMECLASS_UNSET(margin);
}

void MMSBorderClass::setMargin(unsigned int margin) {
	MMSTHEMECLASS_SET_BASIC(margin);
}

bool MMSBorderClass::getMargin(unsigned int &margin) {
	MMSTHEMECLASS_GET_BASIC(margin);
}

void MMSBorderClass::initRCorners() {
	MMSTHEMECLASS_INIT_BASIC(rcorners);
}

void MMSBorderClass::freeRCorners() {
	MMSTHEMECLASS_FREE_BASIC(rcorners);
}

bool MMSBorderClass::isRCorners() {
	MMSTHEMECLASS_ISSET(rcorners);
}

void MMSBorderClass::unsetRCorners() {
	MMSTHEMECLASS_UNSET(rcorners);
}

void MMSBorderClass::setRCorners(bool rcorners) {
	MMSTHEMECLASS_SET_BASIC(rcorners);
}

bool MMSBorderClass::getRCorners(bool &rcorners) {
	MMSTHEMECLASS_GET_BASIC(rcorners);
}

