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

#include "mmsgui/theme/mmsmenuwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_MENUWIDGET_ATTR_I[] = MMSGUI_MENUWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_MENUWIDGET_ATTR_I[MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_MENUWIDGET_ATTR_I[MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_##aname].type


MMSMenuWidgetClass::MMSMenuWidgetClass() {
    this->tafff = NULL;
    unsetAll();
}

MMSMenuWidgetClass::~MMSMenuWidgetClass() {
    if (this->tafff)
        delete this->tafff;
    this->tafff = NULL;
}

void MMSMenuWidgetClass::unsetAll() {
    this->className = "";
    if (this->tafff)
        delete this->tafff;
    this->tafff = NULL;
    unsetItemWidth();
    unsetItemHeight();
    unsetItemHMargin();
    unsetItemVMargin();
    unsetCols();
    unsetDimItems();
    unsetFixedPos();
    unsetHLoop();
    unsetVLoop();
    unsetTransItems();
    unsetDimTop();
    unsetDimBottom();
    unsetDimLeft();
    unsetDimRight();
    unsetTransTop();
    unsetTransBottom();
    unsetTransLeft();
    unsetTransRight();
    unsetZoomSelWidth();
    unsetZoomSelHeight();
    unsetZoomSelShiftX();
    unsetZoomSelShiftY();
    unsetSmoothScrolling();
    unsetParentWindow();
    unsetSelImagePath();
    unsetSelImageName();
    unsetSmoothSelection();
    unsetSmoothDelay();
}

void MMSMenuWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
	    unsetSelImagePath();
    }

    if (!prefix) {
		startTAFFScan
		{
			switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_item_width:
	            setItemWidth(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_item_height:
	            setItemHeight(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_item_hmargin:
	            setItemHMargin(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_item_vmargin:
	            setItemVMargin(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_cols:
	            setCols(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_dim_items:
	            setDimItems(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_fixed_pos:
	            setFixedPos(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_hloop:
	            setHLoop((attrval_int) ? true : false);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_vloop:
	            setVLoop((attrval_int) ? true : false);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_trans_items:
	            setTransItems(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_dim_top:
	            setDimTop(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_dim_bottom:
	            setDimBottom(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_dim_left:
	            setDimLeft(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_dim_right:
	            setDimRight(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_trans_top:
	            setTransTop(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_trans_bottom:
	            setTransBottom(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_trans_left:
	            setTransLeft(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_trans_right:
	            setTransRight(attrval_int);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_width:
	            setZoomSelWidth(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_height:
	            setZoomSelHeight(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_shiftx:
	            setZoomSelShiftX(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_shifty:
	            setZoomSelShiftY(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_smooth_scrolling:
				if ((attrval_int & 0xff) == 0x01)
					setSmoothScrolling(MMSSEQUENCEMODE_LINEAR);
				else
				if ((attrval_int & 0xff) == 0x02)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG);
				else
				if ((attrval_int & 0xff) == 0x03)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG_SOFT_START);
				else
				if ((attrval_int & 0xff) == 0x04)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG_SOFT_END);
				else
				if (attrval_int)
					setSmoothScrolling(MMSSEQUENCEMODE_LINEAR);
				else
					setSmoothScrolling(MMSSEQUENCEMODE_NONE);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_parent_window:
	            setParentWindow(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_selimage:
	            if (*attrval_str)
	                setSelImagePath("");
	            else
	                setSelImagePath((path)?*path:"");
	            setSelImageName(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_selimage_path:
	            if (*attrval_str)
	                setSelImagePath(attrval_str);
	            else
	                setSelImagePath((path)?*path:"");
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_selimage_name:
	            setSelImageName(attrval_str);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_smooth_selection:
				if ((attrval_int & 0xff) == 0x01)
					setSmoothSelection(MMSSEQUENCEMODE_LINEAR);
				else
				if ((attrval_int & 0xff) == 0x02)
					setSmoothSelection(MMSSEQUENCEMODE_LOG);
				else
				if ((attrval_int & 0xff) == 0x03)
					setSmoothSelection(MMSSEQUENCEMODE_LOG_SOFT_START);
				else
				if ((attrval_int & 0xff) == 0x04)
					setSmoothSelection(MMSSEQUENCEMODE_LOG_SOFT_END);
				else
				if (attrval_int)
					setSmoothSelection(MMSSEQUENCEMODE_LINEAR);
				else
					setSmoothSelection(MMSSEQUENCEMODE_NONE);
	            break;
			case MMSGUI_MENUWIDGET_ATTR::MMSGUI_MENUWIDGET_ATTR_IDS_smooth_delay:
	            setSmoothDelay(attrval_int);
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
            if (ISATTRNAME(item_width)) {
	            setItemWidth(attrval_str);
            }
            else
            if (ISATTRNAME(item_height)) {
	            setItemHeight(attrval_str);
            }
            else
            if (ISATTRNAME(item_hmargin)) {
	            setItemHMargin(attrval_int);
            }
            else
            if (ISATTRNAME(item_vmargin)) {
	            setItemVMargin(attrval_int);
            }
            else
            if (ISATTRNAME(cols)) {
	            setCols(attrval_int);
            }
            else
            if (ISATTRNAME(dim_items)) {
	            setDimItems(attrval_int);
            }
            else
            if (ISATTRNAME(fixed_pos)) {
	            setFixedPos(attrval_int);
            }
            else
            if (ISATTRNAME(hloop)) {
	            setHLoop((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(vloop)) {
	            setVLoop((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(trans_items)) {
	            setTransItems(attrval_int);
            }
            else
            if (ISATTRNAME(dim_top)) {
	            setDimTop(attrval_int);
            }
            else
            if (ISATTRNAME(dim_bottom)) {
	            setDimBottom(attrval_int);
            }
            else
            if (ISATTRNAME(dim_left)) {
	            setDimLeft(attrval_int);
            }
            else
            if (ISATTRNAME(dim_right)) {
	            setDimRight(attrval_int);
            }
            else
            if (ISATTRNAME(trans_top)) {
	            setTransTop(attrval_int);
            }
            else
            if (ISATTRNAME(trans_bottom)) {
	            setTransBottom(attrval_int);
            }
            else
            if (ISATTRNAME(trans_left)) {
	            setTransLeft(attrval_int);
            }
            else
            if (ISATTRNAME(trans_right)) {
	            setTransRight(attrval_int);
            }
            else
            if (ISATTRNAME(zoomsel_width)) {
	            setZoomSelWidth(attrval_str);
            }
            else
            if (ISATTRNAME(zoomsel_height)) {
	            setZoomSelHeight(attrval_str);
            }
            else
            if (ISATTRNAME(zoomsel_shiftx)) {
	            setZoomSelShiftX(attrval_str);
            }
            else
            if (ISATTRNAME(zoomsel_shifty)) {
	            setZoomSelShiftY(attrval_str);
            }
            else
            if (ISATTRNAME(smooth_scrolling)) {
				if ((attrval_int & 0xff) == 0x01)
					setSmoothScrolling(MMSSEQUENCEMODE_LINEAR);
				else
				if ((attrval_int & 0xff) == 0x02)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG);
				else
				if ((attrval_int & 0xff) == 0x03)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG_SOFT_START);
				else
				if ((attrval_int & 0xff) == 0x04)
					setSmoothScrolling(MMSSEQUENCEMODE_LOG_SOFT_END);
				else
				if (attrval_int)
					setSmoothScrolling(MMSSEQUENCEMODE_LINEAR);
				else
					setSmoothScrolling(MMSSEQUENCEMODE_NONE);
			}
            else
            if (ISATTRNAME(parent_window)) {
	            setParentWindow(attrval_str);
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
            if (ISATTRNAME(smooth_selection)) {
				if ((attrval_int & 0xff) == 0x01)
					setSmoothSelection(MMSSEQUENCEMODE_LINEAR);
				else
				if ((attrval_int & 0xff) == 0x02)
					setSmoothSelection(MMSSEQUENCEMODE_LOG);
				else
				if ((attrval_int & 0xff) == 0x03)
					setSmoothSelection(MMSSEQUENCEMODE_LOG_SOFT_START);
				else
				if ((attrval_int & 0xff) == 0x04)
					setSmoothSelection(MMSSEQUENCEMODE_LOG_SOFT_END);
				else
				if (attrval_int)
					setSmoothSelection(MMSSEQUENCEMODE_LINEAR);
				else
					setSmoothSelection(MMSSEQUENCEMODE_NONE);
			}
            else
            if (ISATTRNAME(smooth_delay)) {
	            setSmoothDelay(attrval_int);
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
		if (!isSelImagePath())
		    setSelImagePath(*path);
    }
}

void MMSMenuWidgetClass::duplicateTAFF(MMSTaffFile *tafff) {
    if (this->tafff)
        delete this->tafff;
    this->tafff = tafff->copyCurrentTag();
}

MMSTaffFile *MMSMenuWidgetClass::getTAFF() {
	if (this->tafff)
		this->tafff->getFirstTag();
    return this->tafff;
}

void MMSMenuWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSMenuWidgetClass::getClassName() {
    return this->className;
}

bool MMSMenuWidgetClass::isItemWidth() {
    return this->isitemwidth;
}

void MMSMenuWidgetClass::setItemWidth(string itemwidth) {
    this->itemwidth = itemwidth;
    this->isitemwidth = true;
}

void MMSMenuWidgetClass::unsetItemWidth() {
    this->isitemwidth = false;
}

string MMSMenuWidgetClass::getItemWidth() {
    return this->itemwidth;
}

bool MMSMenuWidgetClass::isItemHeight() {
    return this->isitemheight;
}

void MMSMenuWidgetClass::setItemHeight(string itemheight) {
    this->itemheight = itemheight;
    this->isitemheight = true;
}

void MMSMenuWidgetClass::unsetItemHeight() {
    this->isitemheight = false;
}

string MMSMenuWidgetClass::getItemHeight() {
    return this->itemheight;
}

bool MMSMenuWidgetClass::isItemHMargin() {
    return this->isitemhmargin;
}

void MMSMenuWidgetClass::setItemHMargin(unsigned int itemhmargin) {
    this->itemhmargin = itemhmargin;
    this->isitemhmargin = true;
}

void MMSMenuWidgetClass::unsetItemHMargin() {
    this->isitemhmargin = false;
}

unsigned int MMSMenuWidgetClass::getItemHMargin() {
    return this->itemhmargin;
}

bool MMSMenuWidgetClass::isItemVMargin() {
    return this->isitemvmargin;
}

void MMSMenuWidgetClass::setItemVMargin(unsigned int itemvmargin) {
    this->itemvmargin = itemvmargin;
    this->isitemvmargin = true;
}

void MMSMenuWidgetClass::unsetItemVMargin() {
    this->isitemvmargin = false;
}

unsigned int MMSMenuWidgetClass::getItemVMargin() {
    return this->itemvmargin;
}

bool MMSMenuWidgetClass::isCols() {
    return this->iscols;
}

void MMSMenuWidgetClass::setCols(unsigned int cols) {
    this->cols = cols;
    this->iscols = true;
}

void MMSMenuWidgetClass::unsetCols() {
    this->iscols = false;
}

unsigned int MMSMenuWidgetClass::getCols() {
    return this->cols;
}

bool MMSMenuWidgetClass::isDimItems() {
    return this->isdimitems;
}

void MMSMenuWidgetClass::setDimItems(unsigned int dimitems) {
    this->dimitems = dimitems;
    this->isdimitems = true;
}

void MMSMenuWidgetClass::unsetDimItems() {
    this->isdimitems = false;
}

unsigned int MMSMenuWidgetClass::getDimItems() {
    return this->dimitems;
}

bool MMSMenuWidgetClass::isFixedPos() {
    return this->isfixedpos;
}

void MMSMenuWidgetClass::setFixedPos(int fixedpos) {
    this->fixedpos = fixedpos;
    this->isfixedpos = true;
}

void MMSMenuWidgetClass::unsetFixedPos() {
    this->isfixedpos = false;
}

int MMSMenuWidgetClass::getFixedPos() {
    return this->fixedpos;
}

bool MMSMenuWidgetClass::isHLoop() {
    return this->ishloop;
}

void MMSMenuWidgetClass::setHLoop(bool hloop) {
    this->hloop = hloop;
    this->ishloop = true;
}

void MMSMenuWidgetClass::unsetHLoop() {
    this->ishloop = false;
}

bool MMSMenuWidgetClass::getHLoop() {
    return this->hloop;
}

bool MMSMenuWidgetClass::isVLoop() {
    return this->isvloop;
}

void MMSMenuWidgetClass::setVLoop(bool vloop) {
    this->vloop = vloop;
    this->isvloop = true;
}

void MMSMenuWidgetClass::unsetVLoop() {
    this->isvloop = false;
}

bool MMSMenuWidgetClass::getVLoop() {
    return this->vloop;
}

bool MMSMenuWidgetClass::isTransItems() {
    return this->istransitems;
}

void MMSMenuWidgetClass::setTransItems(unsigned int transitems) {
    this->transitems = transitems;
    this->istransitems = true;
}

void MMSMenuWidgetClass::unsetTransItems() {
    this->istransitems = false;
}

unsigned int MMSMenuWidgetClass::getTransItems() {
    return this->transitems;
}

bool MMSMenuWidgetClass::isDimTop() {
    return this->isdimtop;
}

void MMSMenuWidgetClass::setDimTop(unsigned int dimtop) {
    this->dimtop = dimtop;
    this->isdimtop = true;
}

void MMSMenuWidgetClass::unsetDimTop() {
    this->isdimtop = false;
}

unsigned int MMSMenuWidgetClass::getDimTop() {
    return this->dimtop;
}

bool MMSMenuWidgetClass::isDimBottom() {
    return this->isdimbottom;
}

void MMSMenuWidgetClass::setDimBottom(unsigned int dimbottom) {
    this->dimbottom = dimbottom;
    this->isdimbottom = true;
}

void MMSMenuWidgetClass::unsetDimBottom() {
    this->isdimbottom = false;
}

unsigned int MMSMenuWidgetClass::getDimBottom() {
    return this->dimbottom;
}

bool MMSMenuWidgetClass::isDimLeft() {
    return this->isdimleft;
}

void MMSMenuWidgetClass::setDimLeft(unsigned int dimleft) {
    this->dimleft = dimleft;
    this->isdimleft = true;
}

void MMSMenuWidgetClass::unsetDimLeft() {
    this->isdimleft = false;
}

unsigned int MMSMenuWidgetClass::getDimLeft() {
    return this->dimleft;
}

bool MMSMenuWidgetClass::isDimRight() {
    return this->isdimright;
}

void MMSMenuWidgetClass::setDimRight(unsigned int dimright) {
    this->dimright = dimright;
    this->isdimright = true;
}

void MMSMenuWidgetClass::unsetDimRight() {
    this->isdimright = false;
}

unsigned int MMSMenuWidgetClass::getDimRight() {
    return this->dimright;
}

bool MMSMenuWidgetClass::isTransTop() {
    return this->istranstop;
}

void MMSMenuWidgetClass::setTransTop(unsigned int transtop) {
    this->transtop = transtop;
    this->istranstop = true;
}

void MMSMenuWidgetClass::unsetTransTop() {
    this->istranstop = false;
}

unsigned int MMSMenuWidgetClass::getTransTop() {
    return this->transtop;
}

bool MMSMenuWidgetClass::isTransBottom() {
    return this->istransbottom;
}

void MMSMenuWidgetClass::setTransBottom(unsigned int transbottom) {
    this->transbottom = transbottom;
    this->istransbottom = true;
}

void MMSMenuWidgetClass::unsetTransBottom() {
    this->istransbottom = false;
}

unsigned int MMSMenuWidgetClass::getTransBottom() {
    return this->transbottom;
}

bool MMSMenuWidgetClass::isTransLeft() {
    return this->istransleft;
}

void MMSMenuWidgetClass::setTransLeft(unsigned int transleft) {
    this->transleft = transleft;
    this->istransleft = true;
}

void MMSMenuWidgetClass::unsetTransLeft() {
    this->istransleft = false;
}

unsigned int MMSMenuWidgetClass::getTransLeft() {
    return this->transleft;
}

bool MMSMenuWidgetClass::isTransRight() {
    return this->istransright;
}

void MMSMenuWidgetClass::setTransRight(unsigned int transright) {
    this->transright = transright;
    this->istransright = true;
}

void MMSMenuWidgetClass::unsetTransRight() {
    this->istransright = false;
}

unsigned int MMSMenuWidgetClass::getTransRight() {
    return this->transright;
}



bool MMSMenuWidgetClass::isZoomSelWidth() {
    return this->iszoomselwidth;
}

void MMSMenuWidgetClass::setZoomSelWidth(string zoomselwidth) {
    this->zoomselwidth = zoomselwidth;
    this->iszoomselwidth = true;
}

void MMSMenuWidgetClass::unsetZoomSelWidth() {
    this->iszoomselwidth = false;
}

string MMSMenuWidgetClass::getZoomSelWidth() {
    return this->zoomselwidth;
}

bool MMSMenuWidgetClass::isZoomSelHeight() {
    return this->iszoomselheight;
}

void MMSMenuWidgetClass::setZoomSelHeight(string zoomselheight) {
    this->zoomselheight = zoomselheight;
    this->iszoomselheight = true;
}

void MMSMenuWidgetClass::unsetZoomSelHeight() {
    this->iszoomselheight = false;
}

string MMSMenuWidgetClass::getZoomSelHeight() {
    return this->zoomselheight;
}

bool MMSMenuWidgetClass::isZoomSelShiftX() {
    return this->iszoomselshiftx;
}

void MMSMenuWidgetClass::setZoomSelShiftX(string zoomselshiftx) {
    this->zoomselshiftx = zoomselshiftx;
    this->iszoomselshiftx = true;
}

void MMSMenuWidgetClass::unsetZoomSelShiftX() {
    this->iszoomselshiftx = false;
}

string MMSMenuWidgetClass::getZoomSelShiftX() {
    return this->zoomselshiftx;
}

bool MMSMenuWidgetClass::isZoomSelShiftY() {
    return this->iszoomselshifty;
}

void MMSMenuWidgetClass::setZoomSelShiftY(string zoomselshifty) {
    this->zoomselshifty = zoomselshifty;
    this->iszoomselshifty = true;
}

void MMSMenuWidgetClass::unsetZoomSelShiftY() {
    this->iszoomselshifty = false;
}

string MMSMenuWidgetClass::getZoomSelShiftY() {
    return this->zoomselshifty;
}

bool MMSMenuWidgetClass::isSmoothScrolling() {
    return this->issmoothscrolling;
}

void MMSMenuWidgetClass::setSmoothScrolling(MMSSEQUENCEMODE smoothscrolling) {
    this->smoothscrolling = smoothscrolling;
    this->issmoothscrolling = true;
}

void MMSMenuWidgetClass::unsetSmoothScrolling() {
    this->issmoothscrolling = false;
}

MMSSEQUENCEMODE MMSMenuWidgetClass::getSmoothScrolling() {
    return this->smoothscrolling;
}

bool MMSMenuWidgetClass::isParentWindow() {
    return this->isparentwindow;
}

void MMSMenuWidgetClass::setParentWindow(string parentwindow) {
    this->parentwindow = parentwindow;
    this->isparentwindow = true;
}

void MMSMenuWidgetClass::unsetParentWindow() {
    this->isparentwindow = false;
}

string MMSMenuWidgetClass::getParentWindow() {
    return this->parentwindow;
}



bool MMSMenuWidgetClass::isSelImagePath() {
    return this->isselimagepath;
}

void MMSMenuWidgetClass::setSelImagePath(string selimagepath) {
    this->selimagepath = selimagepath;
    this->isselimagepath = true;
}

void MMSMenuWidgetClass::unsetSelImagePath() {
    this->isselimagepath = false;
}

string MMSMenuWidgetClass::getSelImagePath() {
    return this->selimagepath;
}

bool MMSMenuWidgetClass::isSelImageName() {
    return this->isselimagename;
}

void MMSMenuWidgetClass::setSelImageName(string selimagename) {
    this->selimagename = selimagename;
    this->isselimagename = true;
}

void MMSMenuWidgetClass::unsetSelImageName() {
    this->isselimagename = false;
}

string MMSMenuWidgetClass::getSelImageName() {
    return this->selimagename;
}

bool MMSMenuWidgetClass::isSmoothSelection() {
    return this->issmoothselection;
}

void MMSMenuWidgetClass::setSmoothSelection(MMSSEQUENCEMODE smoothselection) {
    this->smoothselection = smoothselection;
    this->issmoothselection = true;
}

void MMSMenuWidgetClass::unsetSmoothSelection() {
    this->issmoothselection = false;
}

MMSSEQUENCEMODE MMSMenuWidgetClass::getSmoothSelection() {
    return this->smoothselection;
}


bool MMSMenuWidgetClass::isSmoothDelay() {
    return this->issmoothdelay;
}

void MMSMenuWidgetClass::setSmoothDelay(unsigned int smoothdelay) {
    this->smoothdelay = smoothdelay;
    this->issmoothdelay = true;
}

void MMSMenuWidgetClass::unsetSmoothDelay() {
    this->issmoothdelay = false;
}

unsigned int MMSMenuWidgetClass::getSmoothDelay() {
    return this->smoothdelay;
}


