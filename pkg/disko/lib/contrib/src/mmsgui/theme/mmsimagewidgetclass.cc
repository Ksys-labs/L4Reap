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

#include "mmsgui/theme/mmsimagewidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_IMAGEWIDGET_ATTR_I[] = MMSGUI_IMAGEWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_IMAGEWIDGET_ATTR_I[MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_IMAGEWIDGET_ATTR_I[MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_##aname].type


MMSImageWidgetClass::MMSImageWidgetClass() {
    unsetAll();
}

void MMSImageWidgetClass::unsetAll() {
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
    unsetUseRatio();
    unsetFitWidth();
    unsetFitHeight();
    unsetAlignment();
    unsetMirrorSize();
    unsetGenTaff();
}

void MMSImageWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
        unsetImagePath();
        unsetSelImagePath();
        unsetImagePath_p();
        unsetSelImagePath_p();
        unsetImagePath_i();
        unsetSelImagePath_i();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image:
	            if (*attrval_str)
	                setImagePath("");
	            else
	                setImagePath((path)?*path:"");
	            setImageName(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_path:
	            if (*attrval_str)
	                setImagePath(attrval_str);
	            else
	                setImagePath((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_name:
	            setImageName(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage:
	            if (*attrval_str)
	                setSelImagePath("");
	            else
	                setSelImagePath((path)?*path:"");
	            setSelImageName(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_path:
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_name:
	            setSelImageName(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p:
	            if (*attrval_str)
	                setImagePath_p("");
	            else
	                setImagePath_p((path)?*path:"");
	            setImageName_p(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p_path:
	            if (*attrval_str)
	                setImagePath_p(attrval_str);
	            else
	                setImagePath_p((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p_name:
	            setImageName_p(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p:
	            if (*attrval_str)
	                setSelImagePath_p("");
	            else
	                setSelImagePath_p((path)?*path:"");
	            setSelImageName_p(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p_path:
	            if (*attrval_str)
	                setSelImagePath_p(attrval_str);
	            else
	                setSelImagePath_p((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p_name:
	            setSelImageName_p(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i:
	            if (*attrval_str)
	                setImagePath_i("");
	            else
	                setImagePath_i((path)?*path:"");
	            setImageName_i(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i_path:
	            if (*attrval_str)
	                setImagePath_i(attrval_str);
	            else
	                setImagePath_i((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i_name:
	            setImageName_i(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i:
	            if (*attrval_str)
	                setSelImagePath_i("");
	            else
	                setSelImagePath_i((path)?*path:"");
	            setSelImageName_i(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i_path:
	            if (*attrval_str)
	                setSelImagePath_i(attrval_str);
	            else
	                setSelImagePath_i((path)?*path:"");
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i_name:
	            setSelImageName_i(attrval_str);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_useratio:
	            setUseRatio((attrval_int) ? true : false);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_fit_width:
	            setFitWidth((attrval_int) ? true : false);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_fit_height:
	            setFitHeight((attrval_int) ? true : false);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_alignment:
	            setAlignment(getAlignmentFromString(attrval_str));
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_mirror_size:
	            setMirrorSize(attrval_int);
				break;
			case MMSGUI_IMAGEWIDGET_ATTR::MMSGUI_IMAGEWIDGET_ATTR_IDS_gen_taff:
	            setGenTaff((attrval_int) ? true : false);
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
            if (ISATTRNAME(useratio)) {
	            setUseRatio((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(fit_width)) {
	            setFitWidth((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(fit_height)) {
	            setFitHeight((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(alignment)) {
	            setAlignment(getAlignmentFromString(attrval_str));
			}
            else
            if (ISATTRNAME(mirror_size)) {
	            setMirrorSize(attrval_int);
			}
            else
            if (ISATTRNAME(gen_taff)) {
	            setGenTaff((attrval_int) ? true : false);
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
    }
}

void MMSImageWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSImageWidgetClass::getClassName() {
    return this->className;
}

bool MMSImageWidgetClass::isImagePath() {
    return this->isimagepath;
}

void MMSImageWidgetClass::setImagePath(string imagepath) {
    this->imagepath = imagepath;
    this->isimagepath = true;
}

void MMSImageWidgetClass::unsetImagePath() {
    this->isimagepath = false;
}

string MMSImageWidgetClass::getImagePath() {
    return this->imagepath;
}

bool MMSImageWidgetClass::isImageName() {
    return this->isimagename;
}

void MMSImageWidgetClass::setImageName(string imagename) {
    this->imagename = imagename;
    this->isimagename = true;
}

void MMSImageWidgetClass::unsetImageName() {
    this->isimagename = false;
}

string MMSImageWidgetClass::getImageName() {
    return this->imagename;
}

bool MMSImageWidgetClass::isSelImagePath() {
    return this->isselimagepath;
}

void MMSImageWidgetClass::setSelImagePath(string selimagepath) {
    this->selimagepath = selimagepath;
    this->isselimagepath = true;
}

void MMSImageWidgetClass::unsetSelImagePath() {
    this->isselimagepath = false;
}

string MMSImageWidgetClass::getSelImagePath() {
    return this->selimagepath;
}

bool MMSImageWidgetClass::isSelImageName() {
    return this->isselimagename;
}

void MMSImageWidgetClass::setSelImageName(string selimagename) {
    this->selimagename = selimagename;
    this->isselimagename = true;
}

void MMSImageWidgetClass::unsetSelImageName() {
    this->isselimagename = false;
}

string MMSImageWidgetClass::getSelImageName() {
    return this->selimagename;
}


bool MMSImageWidgetClass::isImagePath_p() {
    return this->isimagepath_p;
}

void MMSImageWidgetClass::setImagePath_p(string imagepath_p) {
    this->imagepath_p = imagepath_p;
    this->isimagepath_p = true;
}

void MMSImageWidgetClass::unsetImagePath_p() {
    this->isimagepath_p = false;
}

string MMSImageWidgetClass::getImagePath_p() {
    return this->imagepath_p;
}

bool MMSImageWidgetClass::isImageName_p() {
    return this->isimagename_p;
}

void MMSImageWidgetClass::setImageName_p(string imagename_p) {
    this->imagename_p = imagename_p;
    this->isimagename_p = true;
}

void MMSImageWidgetClass::unsetImageName_p() {
    this->isimagename_p = false;
}

string MMSImageWidgetClass::getImageName_p() {
    return this->imagename_p;
}

bool MMSImageWidgetClass::isSelImagePath_p() {
    return this->isselimagepath_p;
}

void MMSImageWidgetClass::setSelImagePath_p(string selimagepath_p) {
    this->selimagepath_p = selimagepath_p;
    this->isselimagepath_p = true;
}

void MMSImageWidgetClass::unsetSelImagePath_p() {
    this->isselimagepath_p = false;
}

string MMSImageWidgetClass::getSelImagePath_p() {
    return this->selimagepath_p;
}

bool MMSImageWidgetClass::isSelImageName_p() {
    return this->isselimagename_p;
}

void MMSImageWidgetClass::setSelImageName_p(string selimagename_p) {
    this->selimagename_p = selimagename_p;
    this->isselimagename_p = true;
}

void MMSImageWidgetClass::unsetSelImageName_p() {
    this->isselimagename_p = false;
}

string MMSImageWidgetClass::getSelImageName_p() {
    return this->selimagename_p;
}


bool MMSImageWidgetClass::isImagePath_i() {
    return this->isimagepath_i;
}

void MMSImageWidgetClass::setImagePath_i(string imagepath_i) {
    this->imagepath_i = imagepath_i;
    this->isimagepath_i = true;
}

void MMSImageWidgetClass::unsetImagePath_i() {
    this->isimagepath_i = false;
}

string MMSImageWidgetClass::getImagePath_i() {
    return this->imagepath_i;
}

bool MMSImageWidgetClass::isImageName_i() {
    return this->isimagename_i;
}

void MMSImageWidgetClass::setImageName_i(string imagename_i) {
    this->imagename_i = imagename_i;
    this->isimagename_i = true;
}

void MMSImageWidgetClass::unsetImageName_i() {
    this->isimagename_i = false;
}

string MMSImageWidgetClass::getImageName_i() {
    return this->imagename_i;
}

bool MMSImageWidgetClass::isSelImagePath_i() {
    return this->isselimagepath_i;
}

void MMSImageWidgetClass::setSelImagePath_i(string selimagepath_i) {
    this->selimagepath_i = selimagepath_i;
    this->isselimagepath_i = true;
}

void MMSImageWidgetClass::unsetSelImagePath_i() {
    this->isselimagepath_i = false;
}

string MMSImageWidgetClass::getSelImagePath_i() {
    return this->selimagepath_i;
}

bool MMSImageWidgetClass::isSelImageName_i() {
    return this->isselimagename_i;
}

void MMSImageWidgetClass::setSelImageName_i(string selimagename_i) {
    this->selimagename_i = selimagename_i;
    this->isselimagename_i = true;
}

void MMSImageWidgetClass::unsetSelImageName_i() {
    this->isselimagename_i = false;
}

string MMSImageWidgetClass::getSelImageName_i() {
    return this->selimagename_i;
}

bool MMSImageWidgetClass::isUseRatio() {
    return this->isuseratio;
}

void MMSImageWidgetClass::setUseRatio(bool useratio) {
    this->useratio = useratio;
    this->isuseratio = true;
}

void MMSImageWidgetClass::unsetUseRatio() {
    this->isuseratio = false;
}

bool MMSImageWidgetClass::getUseRatio() {
    return this->useratio;
}

bool MMSImageWidgetClass::isFitWidth() {
    return this->isfitwidth;
}

void MMSImageWidgetClass::setFitWidth(bool fitwidth) {
    this->fitwidth = fitwidth;
    this->isfitwidth = true;
}

void MMSImageWidgetClass::unsetFitWidth() {
    this->isfitwidth = false;
}

bool MMSImageWidgetClass::getFitWidth() {
    return this->fitwidth;
}

bool MMSImageWidgetClass::isFitHeight() {
    return this->isfitheight;
}

void MMSImageWidgetClass::setFitHeight(bool fitheight) {
    this->fitheight = fitheight;
    this->isfitheight = true;
}

void MMSImageWidgetClass::unsetFitHeight() {
    this->isfitheight = false;
}

bool MMSImageWidgetClass::getFitHeight() {
    return this->fitheight;
}


bool MMSImageWidgetClass::isAlignment() {
    return this->isalignment;
}

void MMSImageWidgetClass::setAlignment(MMSALIGNMENT alignment) {
    this->alignment = alignment;
    this->isalignment = true;
}

void MMSImageWidgetClass::unsetAlignment() {
    this->isalignment = false;
}

MMSALIGNMENT MMSImageWidgetClass::getAlignment() {
    return this->alignment;
}

bool MMSImageWidgetClass::isMirrorSize() {
    return this->ismirrorsize;
}

void MMSImageWidgetClass::setMirrorSize(unsigned int mirrorsize) {
    this->mirrorsize = mirrorsize;
    this->ismirrorsize = true;
}

void MMSImageWidgetClass::unsetMirrorSize() {
    this->ismirrorsize = false;
}

unsigned int MMSImageWidgetClass::getMirrorSize() {
    return this->mirrorsize;
}

bool MMSImageWidgetClass::isGenTaff() {
    return this->isgentaff;
}

void MMSImageWidgetClass::setGenTaff(bool gentaff) {
    this->gentaff = gentaff;
    this->isgentaff = true;
}

void MMSImageWidgetClass::unsetGenTaff() {
    this->isgentaff = false;
}

bool MMSImageWidgetClass::getGenTaff() {
    return this->gentaff;
}


