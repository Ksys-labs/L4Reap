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

#include "mmsgui/theme/mmswidgetclass.h"
#include <string.h>
#include <stdlib.h>

// store attribute descriptions here
TAFF_ATTRDESC MMSGUI_WIDGET_ATTR_I[] = MMSGUI_WIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_WIDGET_ATTR_I[MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_WIDGET_ATTR_I[MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_##aname].type


MMSWidgetClass::MMSWidgetClass() {
	initBgColor();
    initSelBgColor();
    initBgColor_p();
    initSelBgColor_p();
    initBgColor_i();
    initSelBgColor_i();

    initBgImagePath();
    initBgImageName();
    initSelBgImagePath();
    initSelBgImageName();
    initBgImagePath_p();
    initBgImageName_p();
    initSelBgImagePath_p();
    initSelBgImageName_p();
    initBgImagePath_i();
    initBgImageName_i();
    initSelBgImagePath_i();
    initSelBgImageName_i();

    initMargin();
    initFocusable();
    initSelectable();

    initUpArrow();
    initDownArrow();
    initLeftArrow();
    initRightArrow();

    initData();

    initNavigateUp();
    initNavigateDown();
    initNavigateLeft();
    initNavigateRight();

    initVSlider();
    initHSlider();

    initImagesOnDemand();

    initBlend();
    initBlendFactor();

    initScrollOnFocus();
    initClickable();
    initReturnOnScroll();

    initInputMode();
    initJoinedWidget();

    initActivated();
}

MMSWidgetClass::~MMSWidgetClass() {
	freeBgColor();
    freeSelBgColor();
    freeBgColor_p();
    freeSelBgColor_p();
    freeBgColor_i();
    freeSelBgColor_i();

    freeBgImagePath();
    freeBgImageName();
    freeSelBgImagePath();
    freeSelBgImageName();
    freeBgImagePath_p();
    freeBgImageName_p();
    freeSelBgImagePath_p();
    freeSelBgImageName_p();
    freeBgImagePath_i();
    freeBgImageName_i();
    freeSelBgImagePath_i();
    freeSelBgImageName_i();

    freeMargin();
    freeFocusable();
    freeSelectable();

    freeUpArrow();
    freeDownArrow();
    freeLeftArrow();
    freeRightArrow();

    freeData();

    freeNavigateUp();
    freeNavigateDown();
    freeNavigateLeft();
    freeNavigateRight();

    freeVSlider();
    freeHSlider();

    freeImagesOnDemand();

    freeBlend();
    freeBlendFactor();

    freeScrollOnFocus();
    freeClickable();
    freeReturnOnScroll();

    freeInputMode();
    freeJoinedWidget();

    freeActivated();
}

MMSWidgetClass &MMSWidgetClass::operator=(const MMSWidgetClass &c) {
	if (this != &c) {
		/* copy internal fix data area */
		this->border = c.border;
		this->id = c.id;

		/* copy external data */
		memset(&(this->ed), 0, sizeof(this->ed));
		if (c.id.isbgimagepath)
			this->ed.bgimagepath = new string(*c.ed.bgimagepath);
		if (c.id.isbgimagename)
			this->ed.bgimagename = new string(*c.ed.bgimagename);
		if (c.id.isselbgimagepath)
			this->ed.selbgimagepath = new string(*c.ed.selbgimagepath);
		if (c.id.isselbgimagename)
			this->ed.selbgimagename = new string(*c.ed.selbgimagename);
		if (c.id.isbgimagepath_p)
			this->ed.bgimagepath_p = new string(*c.ed.bgimagepath_p);
		if (c.id.isbgimagename_p)
			this->ed.bgimagename_p = new string(*c.ed.bgimagename_p);
		if (c.id.isselbgimagepath_p)
			this->ed.selbgimagepath_p = new string(*c.ed.selbgimagepath_p);
		if (c.id.isselbgimagename_p)
			this->ed.selbgimagename_p = new string(*c.ed.selbgimagename_p);
		if (c.id.isbgimagepath_i)
			this->ed.bgimagepath_i = new string(*c.ed.bgimagepath_i);
		if (c.id.isbgimagename_i)
			this->ed.bgimagename_i = new string(*c.ed.bgimagename_i);
		if (c.id.isselbgimagepath_i)
			this->ed.selbgimagepath_i = new string(*c.ed.selbgimagepath_i);
		if (c.id.isselbgimagename_i)
			this->ed.selbgimagename_i = new string(*c.ed.selbgimagename_i);
		if (c.id.isuparrow)
			this->ed.uparrow = new string(*c.ed.uparrow);
		if (c.id.isdownarrow)
			this->ed.downarrow = new string(*c.ed.downarrow);
		if (c.id.isleftarrow)
			this->ed.leftarrow = new string(*c.ed.leftarrow);
		if (c.id.isleftarrow)
			this->ed.leftarrow = new string(*c.ed.leftarrow);
		if (c.id.isdata)
			this->ed.data = new string(*c.ed.data);
		if (c.id.isnavigateup)
			this->ed.navigateup = new string(*c.ed.navigateup);
		if (c.id.isnavigatedown)
			this->ed.navigatedown = new string(*c.ed.navigatedown);
		if (c.id.isnavigateleft)
			this->ed.navigateleft = new string(*c.ed.navigateleft);
		if (c.id.isnavigateright)
			this->ed.navigateright = new string(*c.ed.navigateright);
		if (c.id.isvslider)
			this->ed.vslider = new string(*c.ed.vslider);
		if (c.id.ishslider)
			this->ed.hslider = new string(*c.ed.hslider);
		if (c.id.isinputmode)
			this->ed.inputmode = new string(*c.ed.inputmode);
		if (c.id.isjoinedwidget)
			this->ed.joinedwidget = new string(*c.ed.joinedwidget);
	}
	return *this;
}

void MMSWidgetClass::unsetAll() {
    unsetBgColor();
    unsetSelBgColor();
    unsetBgColor_p();
    unsetSelBgColor_p();
    unsetBgColor_i();
    unsetSelBgColor_i();

    unsetBgImagePath();
    unsetBgImageName();
    unsetSelBgImagePath();
    unsetSelBgImageName();
    unsetBgImagePath_p();
    unsetBgImageName_p();
    unsetSelBgImagePath_p();
    unsetSelBgImageName_p();
    unsetBgImagePath_i();
    unsetBgImageName_i();
    unsetSelBgImagePath_i();
    unsetSelBgImageName_i();

    unsetMargin();
    unsetFocusable();
    unsetSelectable();

    unsetUpArrow();
    unsetDownArrow();
    unsetLeftArrow();
    unsetRightArrow();

    unsetData();

    unsetNavigateUp();
    unsetNavigateDown();
    unsetNavigateLeft();
    unsetNavigateRight();

    unsetVSlider();
    unsetHSlider();

    unsetImagesOnDemand();

    unsetBlend();
    unsetBlendFactor();

    unsetScrollOnFocus();
    unsetClickable();
    unsetReturnOnScroll();

    unsetInputMode();
    unsetJoinedWidget();

    unsetActivated();
}

void MMSWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
	    unsetBgImagePath();
	    unsetSelBgImagePath();
	    unsetBgImagePath_p();
	    unsetSelBgImagePath_p();
	    unsetBgImagePath_i();
	    unsetSelBgImagePath_i();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor:
	            setBgColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.a = attrval_int;
	            setBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.r = attrval_int;
	            setBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.g = attrval_int;
	            setBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.b = attrval_int;
	            setBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor:
	            setSelBgColor(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.a = attrval_int;
	            setSelBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.r = attrval_int;
	            setSelBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.g = attrval_int;
	            setSelBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.b = attrval_int;
	            setSelBgColor(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_p:
	            setBgColor_p(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.a = attrval_int;
	            setBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.r = attrval_int;
	            setBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.g = attrval_int;
	            setBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.b = attrval_int;
	            setBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p:
	            setSelBgColor_p(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.a = attrval_int;
	            setSelBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.r = attrval_int;
	            setSelBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.g = attrval_int;
	            setSelBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.b = attrval_int;
	            setSelBgColor_p(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_i:
	            setBgColor_i(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.a = attrval_int;
	            setBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.r = attrval_int;
	            setBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.g = attrval_int;
	            setBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.b = attrval_int;
	            setBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i:
	            setSelBgColor_i(MMSFBColor((unsigned int)attrval_int));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.a = attrval_int;
	            setSelBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.r = attrval_int;
	            setSelBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.g = attrval_int;
	            setSelBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.b = attrval_int;
	            setSelBgColor_i(color);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage:
	            if (*attrval_str)
	                setBgImagePath("");
	            else
	                setBgImagePath((path)?*path:"");
	            setBgImageName(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_path:
	            if (*attrval_str)
	                setBgImagePath(attrval_str);
	            else
	                setBgImagePath((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_name:
	            setBgImageName(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage:
	            if (*attrval_str)
	                setSelBgImagePath("");
	            else
	                setSelBgImagePath((path)?*path:"");
	            setSelBgImageName(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_path:
	            if (*attrval_str)
	                setSelBgImagePath(attrval_str);
	            else
	                setSelBgImagePath((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_name:
	            setSelBgImageName(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_p:
	            if (*attrval_str)
	                setBgImagePath_p("");
	            else
	                setBgImagePath_p((path)?*path:"");
	            setBgImageName_p(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_p_path:
	            if (*attrval_str)
	                setBgImagePath_p(attrval_str);
	            else
	                setBgImagePath_p((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_p_name:
	            setBgImageName_p(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_p:
	            if (*attrval_str)
	                setSelBgImagePath_p("");
	            else
	                setSelBgImagePath_p((path)?*path:"");
	            setSelBgImageName_p(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_p_path:
	            if (*attrval_str)
	                setSelBgImagePath_p(attrval_str);
	            else
	                setSelBgImagePath_p((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_p_name:
	            setSelBgImageName_p(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_i:
	            if (*attrval_str)
	                setBgImagePath_i("");
	            else
	                setBgImagePath_i((path)?*path:"");
	            setBgImageName_i(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_i_path:
	            if (*attrval_str)
	                setBgImagePath_i(attrval_str);
	            else
	                setBgImagePath_i((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_bgimage_i_name:
	            setBgImageName_i(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_i:
	            if (*attrval_str)
	                setSelBgImagePath_i("");
	            else
	                setSelBgImagePath_i((path)?*path:"");
	            setSelBgImageName_i(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_i_path:
	            if (*attrval_str)
	                setSelBgImagePath_i(attrval_str);
	            else
	                setSelBgImagePath_i((path)?*path:"");
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selbgimage_i_name:
	            setSelBgImageName_i(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_margin:
	            setMargin(attrval_int);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_focusable:
	            setFocusable((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_selectable:
	            setSelectable((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_up_arrow:
	            setUpArrow(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_down_arrow:
	            setDownArrow(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_left_arrow:
	            setLeftArrow(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_right_arrow:
	            setRightArrow(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_data:
	            setData(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_navigate_up:
	            setNavigateUp(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_navigate_down:
	            setNavigateDown(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_navigate_left:
	            setNavigateLeft(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_navigate_right:
	            setNavigateRight(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_vslider:
	            setVSlider(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_hslider:
	            setHSlider(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_imagesondemand:
	            setImagesOnDemand((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_blend:
	            setBlend(attrval_int);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_blend_factor:
	            setBlendFactor(atof(attrval_str));
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_scroll_onfocus:
	            setScrollOnFocus((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_clickable:
	            setClickable((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_return_onscroll:
	            setReturnOnScroll((attrval_int) ? true : false);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_inputmode:
	            setInputMode(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_joined_widget:
	            setJoinedWidget(attrval_str);
	            break;
			case MMSGUI_WIDGET_ATTR::MMSGUI_WIDGET_ATTR_IDS_activated:
	            setActivated((attrval_int) ? true : false);
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
            if (ISATTRNAME(bgcolor)) {
	            setBgColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(bgcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.a = attrval_int;
	            setBgColor(color);
            }
            else
            if (ISATTRNAME(bgcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.r = attrval_int;
	            setBgColor(color);
            }
            else
            if (ISATTRNAME(bgcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.g = attrval_int;
	            setBgColor(color);
            }
            else
            if (ISATTRNAME(bgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor()) getBgColor(color);
	            color.b = attrval_int;
	            setBgColor(color);
            }
            else
            if (ISATTRNAME(selbgcolor)) {
	            setSelBgColor(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(selbgcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.a = attrval_int;
	            setSelBgColor(color);
            }
            else
            if (ISATTRNAME(selbgcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.r = attrval_int;
	            setSelBgColor(color);
            }
            else
            if (ISATTRNAME(selbgcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.g = attrval_int;
	            setSelBgColor(color);
            }
            else
            if (ISATTRNAME(selbgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor()) getSelBgColor(color);
	            color.b = attrval_int;
	            setSelBgColor(color);
            }
            else
            if (ISATTRNAME(bgcolor_p)) {
	            setBgColor_p(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(bgcolor_p_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.a = attrval_int;
	            setBgColor_p(color);
            }
            else
            if (ISATTRNAME(bgcolor_p_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.r = attrval_int;
	            setBgColor_p(color);
            }
            else
            if (ISATTRNAME(bgcolor_p_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.g = attrval_int;
	            setBgColor_p(color);
            }
            else
            if (ISATTRNAME(bgcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_p()) getBgColor_p(color);
	            color.b = attrval_int;
	            setBgColor_p(color);
            }
            else
            if (ISATTRNAME(selbgcolor_p)) {
	            setSelBgColor_p(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(selbgcolor_p_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.a = attrval_int;
	            setSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(selbgcolor_p_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.r = attrval_int;
	            setSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(selbgcolor_p_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.g = attrval_int;
	            setSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(selbgcolor_p_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_p()) getSelBgColor_p(color);
	            color.b = attrval_int;
	            setSelBgColor_p(color);
            }
            else
            if (ISATTRNAME(bgcolor_i)) {
	            setBgColor_i(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(bgcolor_i_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.a = attrval_int;
	            setBgColor_i(color);
            }
            else
            if (ISATTRNAME(bgcolor_i_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.r = attrval_int;
	            setBgColor_i(color);
            }
            else
            if (ISATTRNAME(bgcolor_i_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.g = attrval_int;
	            setBgColor_i(color);
            }
            else
            if (ISATTRNAME(bgcolor_i_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isBgColor_i()) getBgColor_i(color);
	            color.b = attrval_int;
	            setBgColor_i(color);
            }
            else
            if (ISATTRNAME(selbgcolor_i)) {
	            setSelBgColor_i(MMSFBColor((unsigned int)attrval_int));
            }
            else
            if (ISATTRNAME(selbgcolor_i_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.a = attrval_int;
	            setSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(selbgcolor_i_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.r = attrval_int;
	            setSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(selbgcolor_i_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.g = attrval_int;
	            setSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(selbgcolor_i_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelBgColor_i()) getSelBgColor_i(color);
	            color.b = attrval_int;
	            setSelBgColor_i(color);
            }
            else
            if (ISATTRNAME(bgimage)) {
	            if (*attrval_str)
	                setBgImagePath("");
	            else
	                setBgImagePath((path)?*path:"");
	            setBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(bgimage_path)) {
	            if (*attrval_str)
	                setBgImagePath(attrval_str);
	            else
	                setBgImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(bgimage_name)) {
	            setBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage)) {
	            if (*attrval_str)
	                setSelBgImagePath("");
	            else
	                setSelBgImagePath((path)?*path:"");
	            setSelBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage_path)) {
	            if (*attrval_str)
	                setSelBgImagePath(attrval_str);
	            else
	                setSelBgImagePath((path)?*path:"");
            }
            else
            if (ISATTRNAME(selbgimage_name)) {
	            setSelBgImageName(attrval_str);
            }
            else
            if (ISATTRNAME(bgimage_p)) {
	            if (*attrval_str)
	                setBgImagePath_p("");
	            else
	                setBgImagePath_p((path)?*path:"");
	            setBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(bgimage_p_path)) {
	            if (*attrval_str)
	                setBgImagePath_p(attrval_str);
	            else
	                setBgImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(bgimage_p_name)) {
	            setBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage_p)) {
	            if (*attrval_str)
	                setSelBgImagePath_p("");
	            else
	                setSelBgImagePath_p((path)?*path:"");
	            setSelBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage_p_path)) {
	            if (*attrval_str)
	                setSelBgImagePath_p(attrval_str);
	            else
	                setSelBgImagePath_p((path)?*path:"");
            }
            else
            if (ISATTRNAME(selbgimage_p_name)) {
	            setSelBgImageName_p(attrval_str);
            }
            else
            if (ISATTRNAME(bgimage_i)) {
	            if (*attrval_str)
	                setBgImagePath_i("");
	            else
	                setBgImagePath_i((path)?*path:"");
	            setBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(bgimage_i_path)) {
	            if (*attrval_str)
	                setBgImagePath_i(attrval_str);
	            else
	                setBgImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(bgimage_i_name)) {
	            setBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage_i)) {
	            if (*attrval_str)
	                setSelBgImagePath_i("");
	            else
	                setSelBgImagePath_i((path)?*path:"");
	            setSelBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(selbgimage_i_path)) {
	            if (*attrval_str)
	                setSelBgImagePath_i(attrval_str);
	            else
	                setSelBgImagePath_i((path)?*path:"");
            }
            else
            if (ISATTRNAME(selbgimage_i_name)) {
	            setSelBgImageName_i(attrval_str);
            }
            else
            if (ISATTRNAME(margin)) {
	            setMargin(attrval_int);
            }
            else
            if (ISATTRNAME(focusable)) {
	            setFocusable((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(selectable)) {
	            setSelectable((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(up_arrow)) {
	            setUpArrow(attrval_str);
            }
            else
            if (ISATTRNAME(down_arrow)) {
	            setDownArrow(attrval_str);
            }
            else
            if (ISATTRNAME(left_arrow)) {
	            setLeftArrow(attrval_str);
            }
            else
            if (ISATTRNAME(right_arrow)) {
	            setRightArrow(attrval_str);
            }
            else
            if (ISATTRNAME(data)) {
	            setData(attrval_str);
            }
            else
            if (ISATTRNAME(navigate_up)) {
	            setNavigateUp(attrval_str);
            }
            else
            if (ISATTRNAME(navigate_down)) {
	            setNavigateDown(attrval_str);
            }
            else
            if (ISATTRNAME(navigate_left)) {
	            setNavigateLeft(attrval_str);
            }
            else
            if (ISATTRNAME(navigate_right)) {
	            setNavigateRight(attrval_str);
            }
            else
            if (ISATTRNAME(vslider)) {
	            setVSlider(attrval_str);
            }
            else
            if (ISATTRNAME(hslider)) {
	            setHSlider(attrval_str);
            }
            else
            if (ISATTRNAME(imagesondemand)) {
	            setImagesOnDemand((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(blend)) {
	            setBlend(attrval_int);
            }
            else
            if (ISATTRNAME(blend_factor)) {
	            setBlendFactor(atof(attrval_str));
			}
            else
            if (ISATTRNAME(scroll_onfocus)) {
	            setScrollOnFocus((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(clickable)) {
	            setClickable((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(return_onscroll)) {
	            setReturnOnScroll((attrval_int) ? true : false);
            }
            else
            if (ISATTRNAME(inputmode)) {
	            setInputMode(attrval_str);
            }
            else
            if (ISATTRNAME(joined_widget)) {
	            setJoinedWidget(attrval_str);
            }
            else
            if (ISATTRNAME(activated)) {
				setActivated((attrval_int) ? true : false);
            }
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
		if (!isBgImagePath())
		    setBgImagePath(*path);
		if (!isSelBgImagePath())
		    setSelBgImagePath(*path);
		if (!isBgImagePath_p())
		    setBgImagePath_p(*path);
		if (!isSelBgImagePath_p())
		    setSelBgImagePath_p(*path);
		if (!isBgImagePath_i())
		    setBgImagePath_i(*path);
		if (!isSelBgImagePath_i())
		    setSelBgImagePath_i(*path);
    }
}

void MMSWidgetClass::initBgColor() {
	MMSTHEMECLASS_INIT_BASIC(bgcolor);
}

void MMSWidgetClass::freeBgColor() {
	MMSTHEMECLASS_FREE_BASIC(bgcolor);
}

bool MMSWidgetClass::isBgColor() {
	MMSTHEMECLASS_ISSET(bgcolor);
}

void MMSWidgetClass::unsetBgColor() {
	MMSTHEMECLASS_UNSET(bgcolor);
}

void MMSWidgetClass::setBgColor(const MMSFBColor &bgcolor) {
	MMSTHEMECLASS_SET_BASIC(bgcolor);
}

bool MMSWidgetClass::getBgColor(MMSFBColor &bgcolor) {
	MMSTHEMECLASS_GET_BASIC(bgcolor);
}

void MMSWidgetClass::initSelBgColor() {
	MMSTHEMECLASS_INIT_BASIC(selbgcolor);
}

void MMSWidgetClass::freeSelBgColor() {
	MMSTHEMECLASS_FREE_BASIC(selbgcolor);
}

bool MMSWidgetClass::isSelBgColor() {
	MMSTHEMECLASS_ISSET(selbgcolor);
}

void MMSWidgetClass::unsetSelBgColor() {
	MMSTHEMECLASS_UNSET(selbgcolor);
}

void MMSWidgetClass::setSelBgColor(const MMSFBColor &selbgcolor) {
	MMSTHEMECLASS_SET_BASIC(selbgcolor);
}

bool MMSWidgetClass::getSelBgColor(MMSFBColor &selbgcolor) {
	MMSTHEMECLASS_GET_BASIC(selbgcolor);
}


void MMSWidgetClass::initBgColor_p() {
	MMSTHEMECLASS_INIT_BASIC(bgcolor_p);
}

void MMSWidgetClass::freeBgColor_p() {
	MMSTHEMECLASS_FREE_BASIC(bgcolor_p);
}

bool MMSWidgetClass::isBgColor_p() {
	MMSTHEMECLASS_ISSET(bgcolor_p);
}

void MMSWidgetClass::unsetBgColor_p() {
	MMSTHEMECLASS_UNSET(bgcolor_p);
}

void MMSWidgetClass::setBgColor_p(const MMSFBColor &bgcolor_p) {
	MMSTHEMECLASS_SET_BASIC(bgcolor_p);
}

bool MMSWidgetClass::getBgColor_p(MMSFBColor &bgcolor_p) {
	MMSTHEMECLASS_GET_BASIC(bgcolor_p);
}

void MMSWidgetClass::initSelBgColor_p() {
	MMSTHEMECLASS_INIT_BASIC(selbgcolor_p);
}

void MMSWidgetClass::freeSelBgColor_p() {
	MMSTHEMECLASS_FREE_BASIC(selbgcolor_p);
}

bool MMSWidgetClass::isSelBgColor_p() {
	MMSTHEMECLASS_ISSET(selbgcolor_p);
}

void MMSWidgetClass::unsetSelBgColor_p() {
	MMSTHEMECLASS_UNSET(selbgcolor_p);
}

void MMSWidgetClass::setSelBgColor_p(const MMSFBColor &selbgcolor_p) {
	MMSTHEMECLASS_SET_BASIC(selbgcolor_p);
}

bool MMSWidgetClass::getSelBgColor_p(MMSFBColor &selbgcolor_p) {
	MMSTHEMECLASS_GET_BASIC(selbgcolor_p);
}

void MMSWidgetClass::initBgColor_i() {
	MMSTHEMECLASS_INIT_BASIC(bgcolor_i);
}

void MMSWidgetClass::freeBgColor_i() {
	MMSTHEMECLASS_FREE_BASIC(bgcolor_i);
}

bool MMSWidgetClass::isBgColor_i() {
	MMSTHEMECLASS_ISSET(bgcolor_i);
}

void MMSWidgetClass::unsetBgColor_i() {
	MMSTHEMECLASS_UNSET(bgcolor_i);
}

void MMSWidgetClass::setBgColor_i(const MMSFBColor &bgcolor_i) {
	MMSTHEMECLASS_SET_BASIC(bgcolor_i);
}

bool MMSWidgetClass::getBgColor_i(MMSFBColor &bgcolor_i) {
	MMSTHEMECLASS_GET_BASIC(bgcolor_i);
}

void MMSWidgetClass::initSelBgColor_i() {
	MMSTHEMECLASS_INIT_BASIC(selbgcolor_i);
}

void MMSWidgetClass::freeSelBgColor_i() {
	MMSTHEMECLASS_FREE_BASIC(selbgcolor_i);
}

bool MMSWidgetClass::isSelBgColor_i() {
	MMSTHEMECLASS_ISSET(selbgcolor_i);
}

void MMSWidgetClass::unsetSelBgColor_i() {
	MMSTHEMECLASS_UNSET(selbgcolor_i);
}

void MMSWidgetClass::setSelBgColor_i(const MMSFBColor &selbgcolor_i) {
	MMSTHEMECLASS_SET_BASIC(selbgcolor_i);
}

bool MMSWidgetClass::getSelBgColor_i(MMSFBColor &selbgcolor_i) {
	MMSTHEMECLASS_GET_BASIC(selbgcolor_i);
}


void MMSWidgetClass::initBgImagePath() {
	MMSTHEMECLASS_INIT_STRING(bgimagepath);
}

void MMSWidgetClass::freeBgImagePath() {
	MMSTHEMECLASS_FREE_STRING(bgimagepath);
}

bool MMSWidgetClass::isBgImagePath() {
	MMSTHEMECLASS_ISSET(bgimagepath);
}

void MMSWidgetClass::unsetBgImagePath() {
	MMSTHEMECLASS_UNSET(bgimagepath);
}

void MMSWidgetClass::setBgImagePath(const string &bgimagepath) {
	MMSTHEMECLASS_SET_STRING(bgimagepath);
}

bool MMSWidgetClass::getBgImagePath(string &bgimagepath) {
	MMSTHEMECLASS_GET_STRING(bgimagepath);
}



void MMSWidgetClass::initBgImageName() {
	MMSTHEMECLASS_INIT_STRING(bgimagename);
}

void MMSWidgetClass::freeBgImageName() {
	MMSTHEMECLASS_FREE_STRING(bgimagename);
}

bool MMSWidgetClass::isBgImageName() {
	MMSTHEMECLASS_ISSET(bgimagename);
}

void MMSWidgetClass::unsetBgImageName() {
	MMSTHEMECLASS_UNSET(bgimagename);
}

void MMSWidgetClass::setBgImageName(const string &bgimagename) {
	MMSTHEMECLASS_SET_STRING(bgimagename);
}

bool MMSWidgetClass::getBgImageName(string &bgimagename) {
	MMSTHEMECLASS_GET_STRING(bgimagename);
}

void MMSWidgetClass::initSelBgImagePath() {
	MMSTHEMECLASS_INIT_STRING(selbgimagepath);
}

void MMSWidgetClass::freeSelBgImagePath() {
	MMSTHEMECLASS_FREE_STRING(selbgimagepath);
}

bool MMSWidgetClass::isSelBgImagePath() {
	MMSTHEMECLASS_ISSET(selbgimagepath);
}

void MMSWidgetClass::unsetSelBgImagePath() {
	MMSTHEMECLASS_UNSET(selbgimagepath);
}

void MMSWidgetClass::setSelBgImagePath(const string &selbgimagepath) {
	MMSTHEMECLASS_SET_STRING(selbgimagepath);
}

bool MMSWidgetClass::getSelBgImagePath(string &selbgimagepath) {
	MMSTHEMECLASS_GET_STRING(selbgimagepath);
}

void MMSWidgetClass::initSelBgImageName() {
	MMSTHEMECLASS_INIT_STRING(selbgimagename);
}

void MMSWidgetClass::freeSelBgImageName() {
	MMSTHEMECLASS_FREE_STRING(selbgimagename);
}

bool MMSWidgetClass::isSelBgImageName() {
	MMSTHEMECLASS_ISSET(selbgimagename);
}

void MMSWidgetClass::unsetSelBgImageName() {
	MMSTHEMECLASS_UNSET(selbgimagename);
}

void MMSWidgetClass::setSelBgImageName(const string &selbgimagename) {
	MMSTHEMECLASS_SET_STRING(selbgimagename);
}

bool MMSWidgetClass::getSelBgImageName(string &selbgimagename) {
	MMSTHEMECLASS_GET_STRING(selbgimagename);
}

void MMSWidgetClass::initBgImagePath_p() {
	MMSTHEMECLASS_INIT_STRING(bgimagepath_p);
}

void MMSWidgetClass::freeBgImagePath_p() {
	MMSTHEMECLASS_FREE_STRING(bgimagepath_p);
}

bool MMSWidgetClass::isBgImagePath_p() {
	MMSTHEMECLASS_ISSET(bgimagepath_p);
}

void MMSWidgetClass::unsetBgImagePath_p() {
	MMSTHEMECLASS_UNSET(bgimagepath_p);
}

void MMSWidgetClass::setBgImagePath_p(const string &bgimagepath_p) {
	MMSTHEMECLASS_SET_STRING(bgimagepath_p);
}

bool MMSWidgetClass::getBgImagePath_p(string &bgimagepath_p) {
	MMSTHEMECLASS_GET_STRING(bgimagepath_p);
}

void MMSWidgetClass::initBgImageName_p() {
	MMSTHEMECLASS_INIT_STRING(bgimagename_p);
}

void MMSWidgetClass::freeBgImageName_p() {
	MMSTHEMECLASS_FREE_STRING(bgimagename_p);
}

bool MMSWidgetClass::isBgImageName_p() {
	MMSTHEMECLASS_ISSET(bgimagename_p);
}

void MMSWidgetClass::unsetBgImageName_p() {
	MMSTHEMECLASS_UNSET(bgimagename_p);
}

void MMSWidgetClass::setBgImageName_p(const string &bgimagename_p) {
	MMSTHEMECLASS_SET_STRING(bgimagename_p);
}

bool MMSWidgetClass::getBgImageName_p(string &bgimagename_p) {
	MMSTHEMECLASS_GET_STRING(bgimagename_p);
}

void MMSWidgetClass::initSelBgImagePath_p() {
	MMSTHEMECLASS_INIT_STRING(selbgimagepath_p);
}

void MMSWidgetClass::freeSelBgImagePath_p() {
	MMSTHEMECLASS_FREE_STRING(selbgimagepath_p);
}

bool MMSWidgetClass::isSelBgImagePath_p() {
	MMSTHEMECLASS_ISSET(selbgimagepath_p);
}

void MMSWidgetClass::unsetSelBgImagePath_p() {
	MMSTHEMECLASS_UNSET(selbgimagepath_p);
}

void MMSWidgetClass::setSelBgImagePath_p(const string &selbgimagepath_p) {
	MMSTHEMECLASS_SET_STRING(selbgimagepath_p);
}

bool MMSWidgetClass::getSelBgImagePath_p(string &selbgimagepath_p) {
	MMSTHEMECLASS_GET_STRING(selbgimagepath_p);
}

void MMSWidgetClass::initSelBgImageName_p() {
	MMSTHEMECLASS_INIT_STRING(selbgimagename_p);
}

void MMSWidgetClass::freeSelBgImageName_p() {
	MMSTHEMECLASS_FREE_STRING(selbgimagename_p);
}

bool MMSWidgetClass::isSelBgImageName_p() {
	MMSTHEMECLASS_ISSET(selbgimagename_p);
}

void MMSWidgetClass::unsetSelBgImageName_p() {
	MMSTHEMECLASS_UNSET(selbgimagename_p);
}

void MMSWidgetClass::setSelBgImageName_p(const string &selbgimagename_p) {
	MMSTHEMECLASS_SET_STRING(selbgimagename_p);
}

bool MMSWidgetClass::getSelBgImageName_p(string &selbgimagename_p) {
	MMSTHEMECLASS_GET_STRING(selbgimagename_p);
}


void MMSWidgetClass::initBgImagePath_i() {
	MMSTHEMECLASS_INIT_STRING(bgimagepath_i);
}

void MMSWidgetClass::freeBgImagePath_i() {
	MMSTHEMECLASS_FREE_STRING(bgimagepath_i);
}

bool MMSWidgetClass::isBgImagePath_i() {
	MMSTHEMECLASS_ISSET(bgimagepath_i);
}

void MMSWidgetClass::unsetBgImagePath_i() {
	MMSTHEMECLASS_UNSET(bgimagepath_i);
}

void MMSWidgetClass::setBgImagePath_i(const string &bgimagepath_i) {
	MMSTHEMECLASS_SET_STRING(bgimagepath_i);
}

bool MMSWidgetClass::getBgImagePath_i(string &bgimagepath_i) {
	MMSTHEMECLASS_GET_STRING(bgimagepath_i);
}

void MMSWidgetClass::initBgImageName_i() {
	MMSTHEMECLASS_INIT_STRING(bgimagename_i);
}

void MMSWidgetClass::freeBgImageName_i() {
	MMSTHEMECLASS_FREE_STRING(bgimagename_i);
}

bool MMSWidgetClass::isBgImageName_i() {
	MMSTHEMECLASS_ISSET(bgimagename_i);
}

void MMSWidgetClass::unsetBgImageName_i() {
	MMSTHEMECLASS_UNSET(bgimagename_i);
}

void MMSWidgetClass::setBgImageName_i(const string &bgimagename_i) {
	MMSTHEMECLASS_SET_STRING(bgimagename_i);
}

bool MMSWidgetClass::getBgImageName_i(string &bgimagename_i) {
	MMSTHEMECLASS_GET_STRING(bgimagename_i);
}

void MMSWidgetClass::initSelBgImagePath_i() {
	MMSTHEMECLASS_INIT_STRING(selbgimagepath_i);
}

void MMSWidgetClass::freeSelBgImagePath_i() {
	MMSTHEMECLASS_FREE_STRING(selbgimagepath_i);
}

bool MMSWidgetClass::isSelBgImagePath_i() {
	MMSTHEMECLASS_ISSET(selbgimagepath_i);
}

void MMSWidgetClass::unsetSelBgImagePath_i() {
	MMSTHEMECLASS_UNSET(selbgimagepath_i);
}

void MMSWidgetClass::setSelBgImagePath_i(const string &selbgimagepath_i) {
	MMSTHEMECLASS_SET_STRING(selbgimagepath_i);
}

bool MMSWidgetClass::getSelBgImagePath_i(string &selbgimagepath_i) {
	MMSTHEMECLASS_GET_STRING(selbgimagepath_i);
}

void MMSWidgetClass::initSelBgImageName_i() {
	MMSTHEMECLASS_INIT_STRING(selbgimagename_i);
}

void MMSWidgetClass::freeSelBgImageName_i() {
	MMSTHEMECLASS_FREE_STRING(selbgimagename_i);
}

bool MMSWidgetClass::isSelBgImageName_i() {
	MMSTHEMECLASS_ISSET(selbgimagename_i);
}

void MMSWidgetClass::unsetSelBgImageName_i() {
	MMSTHEMECLASS_UNSET(selbgimagename_i);
}

void MMSWidgetClass::setSelBgImageName_i(const string &selbgimagename_i) {
	MMSTHEMECLASS_SET_STRING(selbgimagename_i);
}

bool MMSWidgetClass::getSelBgImageName_i(string &selbgimagename_i) {
	MMSTHEMECLASS_GET_STRING(selbgimagename_i);
}


void MMSWidgetClass::initMargin() {
	MMSTHEMECLASS_INIT_BASIC(margin);
}

void MMSWidgetClass::freeMargin() {
	MMSTHEMECLASS_FREE_BASIC(margin);
}

bool MMSWidgetClass::isMargin() {
	MMSTHEMECLASS_ISSET(margin);
}

void MMSWidgetClass::unsetMargin() {
	MMSTHEMECLASS_UNSET(margin);
}

void MMSWidgetClass::setMargin(unsigned int margin) {
	MMSTHEMECLASS_SET_BASIC(margin);
}

bool MMSWidgetClass::getMargin(unsigned int &margin) {
	MMSTHEMECLASS_GET_BASIC(margin);
}

void MMSWidgetClass::initFocusable() {
    this->id.focusable = false;
    MMSTHEMECLASS_INIT_BASIC(focusable);
}

void MMSWidgetClass::freeFocusable() {
    this->id.focusable = false;
    MMSTHEMECLASS_FREE_BASIC(focusable);
}

bool MMSWidgetClass::isFocusable() {
	MMSTHEMECLASS_ISSET(focusable);
}

void MMSWidgetClass::unsetFocusable() {
    this->id.focusable = false;
    MMSTHEMECLASS_UNSET(focusable);
}

void MMSWidgetClass::setFocusable(bool focusable) {
	MMSTHEMECLASS_SET_BASIC(focusable);
}

bool MMSWidgetClass::getFocusable(bool &focusable) {
	MMSTHEMECLASS_GET_BASIC(focusable);
}

void MMSWidgetClass::initSelectable() {
    this->id.selectable = false;
    MMSTHEMECLASS_INIT_BASIC(selectable);
}

void MMSWidgetClass::freeSelectable() {
    this->id.selectable = false;
    MMSTHEMECLASS_FREE_BASIC(selectable);
}

bool MMSWidgetClass::isSelectable() {
	MMSTHEMECLASS_ISSET(selectable);
}

void MMSWidgetClass::unsetSelectable() {
    this->id.selectable = false;
    MMSTHEMECLASS_UNSET(selectable);
}

void MMSWidgetClass::setSelectable(bool selectable) {
	MMSTHEMECLASS_SET_BASIC(selectable);
}

bool MMSWidgetClass::getSelectable(bool &selectable) {
	MMSTHEMECLASS_GET_BASIC(selectable);
}




void MMSWidgetClass::initUpArrow() {
	MMSTHEMECLASS_INIT_STRING(uparrow);
}

void MMSWidgetClass::freeUpArrow() {
	MMSTHEMECLASS_FREE_STRING(uparrow);
}

bool MMSWidgetClass::isUpArrow() {
	MMSTHEMECLASS_ISSET(uparrow);
}

void MMSWidgetClass::unsetUpArrow() {
	MMSTHEMECLASS_UNSET(uparrow);
}

void MMSWidgetClass::setUpArrow(const string &uparrow) {
	MMSTHEMECLASS_SET_STRING(uparrow);
}

bool MMSWidgetClass::getUpArrow(string &uparrow) {
	MMSTHEMECLASS_GET_STRING(uparrow);
}

void MMSWidgetClass::initDownArrow() {
	MMSTHEMECLASS_INIT_STRING(downarrow);
}

void MMSWidgetClass::freeDownArrow() {
	MMSTHEMECLASS_FREE_STRING(downarrow);
}

bool MMSWidgetClass::isDownArrow() {
	MMSTHEMECLASS_ISSET(downarrow);
}

void MMSWidgetClass::unsetDownArrow() {
	MMSTHEMECLASS_UNSET(downarrow);
}

void MMSWidgetClass::setDownArrow(const string &downarrow) {
	MMSTHEMECLASS_SET_STRING(downarrow);
}

bool MMSWidgetClass::getDownArrow(string &downarrow) {
	MMSTHEMECLASS_GET_STRING(downarrow);
}

void MMSWidgetClass::initLeftArrow() {
	MMSTHEMECLASS_INIT_STRING(leftarrow);
}

void MMSWidgetClass::freeLeftArrow() {
	MMSTHEMECLASS_FREE_STRING(leftarrow);
}

bool MMSWidgetClass::isLeftArrow() {
	MMSTHEMECLASS_ISSET(leftarrow);
}

void MMSWidgetClass::unsetLeftArrow() {
	MMSTHEMECLASS_UNSET(leftarrow);
}

void MMSWidgetClass::setLeftArrow(const string &leftarrow) {
	MMSTHEMECLASS_SET_STRING(leftarrow);
}

bool MMSWidgetClass::getLeftArrow(string &leftarrow) {
	MMSTHEMECLASS_GET_STRING(leftarrow);
}

void MMSWidgetClass::initRightArrow() {
	MMSTHEMECLASS_INIT_STRING(rightarrow);
}

void MMSWidgetClass::freeRightArrow() {
	MMSTHEMECLASS_FREE_STRING(rightarrow);
}

bool MMSWidgetClass::isRightArrow() {
	MMSTHEMECLASS_ISSET(rightarrow);
}

void MMSWidgetClass::unsetRightArrow() {
	MMSTHEMECLASS_UNSET(rightarrow);
}

void MMSWidgetClass::setRightArrow(const string &rightarrow) {
	MMSTHEMECLASS_SET_STRING(rightarrow);
}

bool MMSWidgetClass::getRightArrow(string &rightarrow) {
	MMSTHEMECLASS_GET_STRING(rightarrow);
}



void MMSWidgetClass::initData() {
	MMSTHEMECLASS_INIT_STRING(data);
}

void MMSWidgetClass::freeData() {
	MMSTHEMECLASS_FREE_STRING(data);
}

bool MMSWidgetClass::isData() {
	MMSTHEMECLASS_ISSET(data);
}

void MMSWidgetClass::unsetData() {
	MMSTHEMECLASS_UNSET(data);
}

void MMSWidgetClass::setData(const string &data) {
	MMSTHEMECLASS_SET_STRING(data);
}

bool MMSWidgetClass::getData(string &data) {
	MMSTHEMECLASS_GET_STRING(data);
}

void MMSWidgetClass::initNavigateUp() {
	MMSTHEMECLASS_INIT_STRING(navigateup);
}

void MMSWidgetClass::freeNavigateUp() {
	MMSTHEMECLASS_FREE_STRING(navigateup);
}

bool MMSWidgetClass::isNavigateUp() {
	MMSTHEMECLASS_ISSET(navigateup);
}

void MMSWidgetClass::unsetNavigateUp() {
	MMSTHEMECLASS_UNSET(navigateup);
}

void MMSWidgetClass::setNavigateUp(const string &navigateup) {
	MMSTHEMECLASS_SET_STRING(navigateup);
}

bool MMSWidgetClass::getNavigateUp(string &navigateup) {
	MMSTHEMECLASS_GET_STRING(navigateup);
}

void MMSWidgetClass::initNavigateDown() {
	MMSTHEMECLASS_INIT_STRING(navigatedown);
}

void MMSWidgetClass::freeNavigateDown() {
	MMSTHEMECLASS_FREE_STRING(navigatedown);
}

bool MMSWidgetClass::isNavigateDown() {
	MMSTHEMECLASS_ISSET(navigatedown);
}

void MMSWidgetClass::unsetNavigateDown() {
	MMSTHEMECLASS_UNSET(navigatedown);
}

void MMSWidgetClass::setNavigateDown(const string &navigatedown) {
	MMSTHEMECLASS_SET_STRING(navigatedown);
}

bool MMSWidgetClass::getNavigateDown(string &navigatedown) {
	MMSTHEMECLASS_GET_STRING(navigatedown);
}

void MMSWidgetClass::initNavigateLeft() {
	MMSTHEMECLASS_INIT_STRING(navigateleft);
}

void MMSWidgetClass::freeNavigateLeft() {
	MMSTHEMECLASS_FREE_STRING(navigateleft);
}

bool MMSWidgetClass::isNavigateLeft() {
	MMSTHEMECLASS_ISSET(navigateleft);
}

void MMSWidgetClass::unsetNavigateLeft() {
	MMSTHEMECLASS_UNSET(navigateleft);
}

void MMSWidgetClass::setNavigateLeft(const string &navigateleft) {
	MMSTHEMECLASS_SET_STRING(navigateleft);
}

bool MMSWidgetClass::getNavigateLeft(string &navigateleft) {
	MMSTHEMECLASS_GET_STRING(navigateleft);
}

void MMSWidgetClass::initNavigateRight() {
	MMSTHEMECLASS_INIT_STRING(navigateright);
}

void MMSWidgetClass::freeNavigateRight() {
	MMSTHEMECLASS_FREE_STRING(navigateright);
}

bool MMSWidgetClass::isNavigateRight() {
	MMSTHEMECLASS_ISSET(navigateright);
}

void MMSWidgetClass::unsetNavigateRight() {
	MMSTHEMECLASS_UNSET(navigateright);
}

void MMSWidgetClass::setNavigateRight(const string &navigateright) {
	MMSTHEMECLASS_SET_STRING(navigateright);
}

bool MMSWidgetClass::getNavigateRight(string &navigateright) {
	MMSTHEMECLASS_GET_STRING(navigateright);
}


void MMSWidgetClass::initVSlider() {
	MMSTHEMECLASS_INIT_STRING(vslider);
}

void MMSWidgetClass::freeVSlider() {
	MMSTHEMECLASS_FREE_STRING(vslider);
}

bool MMSWidgetClass::isVSlider() {
	MMSTHEMECLASS_ISSET(vslider);
}

void MMSWidgetClass::unsetVSlider() {
	MMSTHEMECLASS_UNSET(vslider);
}

void MMSWidgetClass::setVSlider(const string &vslider) {
	MMSTHEMECLASS_SET_STRING(vslider);
}

bool MMSWidgetClass::getVSlider(string &vslider) {
	MMSTHEMECLASS_GET_STRING(vslider);
}

void MMSWidgetClass::initHSlider() {
	MMSTHEMECLASS_INIT_STRING(hslider);
}

void MMSWidgetClass::freeHSlider() {
	MMSTHEMECLASS_FREE_STRING(hslider);
}

bool MMSWidgetClass::isHSlider() {
	MMSTHEMECLASS_ISSET(hslider);
}

void MMSWidgetClass::unsetHSlider() {
	MMSTHEMECLASS_UNSET(hslider);
}

void MMSWidgetClass::setHSlider(const string &hslider) {
	MMSTHEMECLASS_SET_STRING(hslider);
}

bool MMSWidgetClass::getHSlider(string &hslider) {
	MMSTHEMECLASS_GET_STRING(hslider);
}

bool MMSWidgetClass::isImagesOnDemand() {
	MMSTHEMECLASS_ISSET(imagesondemand);
}

void MMSWidgetClass::initImagesOnDemand() {
	MMSTHEMECLASS_INIT_BASIC(imagesondemand);
}

void MMSWidgetClass::freeImagesOnDemand() {
	MMSTHEMECLASS_FREE_BASIC(imagesondemand);
}

void MMSWidgetClass::unsetImagesOnDemand() {
	MMSTHEMECLASS_UNSET(imagesondemand);
}

void MMSWidgetClass::setImagesOnDemand(bool imagesondemand) {
	MMSTHEMECLASS_SET_BASIC(imagesondemand);
}

bool MMSWidgetClass::getImagesOnDemand(bool &imagesondemand) {
	MMSTHEMECLASS_GET_BASIC(imagesondemand);
}

void MMSWidgetClass::initBlend() {
	MMSTHEMECLASS_INIT_BASIC(blend);
}

void MMSWidgetClass::freeBlend() {
	MMSTHEMECLASS_FREE_BASIC(blend);
}

bool MMSWidgetClass::isBlend() {
	MMSTHEMECLASS_ISSET(blend);
}

void MMSWidgetClass::unsetBlend() {
	MMSTHEMECLASS_UNSET(blend);
}

void MMSWidgetClass::setBlend(unsigned int blend) {
	MMSTHEMECLASS_SET_BASIC(blend);
}

bool MMSWidgetClass::getBlend(unsigned int &blend) {
	MMSTHEMECLASS_GET_BASIC(blend);
}

void MMSWidgetClass::initBlendFactor() {
	MMSTHEMECLASS_INIT_BASIC(blendfactor);
}

void MMSWidgetClass::freeBlendFactor() {
	MMSTHEMECLASS_FREE_BASIC(blendfactor);
}

bool MMSWidgetClass::isBlendFactor() {
	MMSTHEMECLASS_ISSET(blendfactor);
}

void MMSWidgetClass::unsetBlendFactor() {
	MMSTHEMECLASS_UNSET(blendfactor);
}

void MMSWidgetClass::setBlendFactor(double blendfactor) {
	MMSTHEMECLASS_SET_BASIC(blendfactor);
}

bool MMSWidgetClass::getBlendFactor(double &blendfactor) {
	MMSTHEMECLASS_GET_BASIC(blendfactor);
}

bool MMSWidgetClass::isScrollOnFocus() {
	MMSTHEMECLASS_ISSET(scrollonfocus);
}

void MMSWidgetClass::initScrollOnFocus() {
	MMSTHEMECLASS_INIT_BASIC(scrollonfocus);
}

void MMSWidgetClass::freeScrollOnFocus() {
	MMSTHEMECLASS_FREE_BASIC(scrollonfocus);
}

void MMSWidgetClass::unsetScrollOnFocus() {
	MMSTHEMECLASS_UNSET(scrollonfocus);
}

void MMSWidgetClass::setScrollOnFocus(bool scrollonfocus) {
	MMSTHEMECLASS_SET_BASIC(scrollonfocus);
}

bool MMSWidgetClass::getScrollOnFocus(bool &scrollonfocus) {
	MMSTHEMECLASS_GET_BASIC(scrollonfocus);
}

void MMSWidgetClass::initClickable() {
    MMSTHEMECLASS_INIT_BASIC(clickable);
}

void MMSWidgetClass::freeClickable() {
    MMSTHEMECLASS_FREE_BASIC(clickable);
}

bool MMSWidgetClass::isClickable() {
	MMSTHEMECLASS_ISSET(clickable);
}

void MMSWidgetClass::unsetClickable() {
    MMSTHEMECLASS_UNSET(clickable);
}

void MMSWidgetClass::setClickable(bool clickable) {
	MMSTHEMECLASS_SET_BASIC(clickable);
}

bool MMSWidgetClass::getClickable(bool &clickable) {
	MMSTHEMECLASS_GET_BASIC(clickable);
}

void MMSWidgetClass::initReturnOnScroll() {
    MMSTHEMECLASS_INIT_BASIC(returnonscroll);
}

void MMSWidgetClass::freeReturnOnScroll() {
    MMSTHEMECLASS_FREE_BASIC(returnonscroll);
}

bool MMSWidgetClass::isReturnOnScroll() {
	MMSTHEMECLASS_ISSET(returnonscroll);
}

void MMSWidgetClass::unsetReturnOnScroll() {
    MMSTHEMECLASS_UNSET(returnonscroll);
}

void MMSWidgetClass::setReturnOnScroll(bool returnonscroll) {
	MMSTHEMECLASS_SET_BASIC(returnonscroll);
}

bool MMSWidgetClass::getReturnOnScroll(bool &returnonscroll) {
	MMSTHEMECLASS_GET_BASIC(returnonscroll);
}

void MMSWidgetClass::initInputMode() {
	MMSTHEMECLASS_INIT_STRING(inputmode);
}

void MMSWidgetClass::freeInputMode() {
	MMSTHEMECLASS_FREE_STRING(inputmode);
}

bool MMSWidgetClass::isInputMode() {
	MMSTHEMECLASS_ISSET(inputmode);
}

void MMSWidgetClass::unsetInputMode() {
	MMSTHEMECLASS_UNSET(inputmode);
}

void MMSWidgetClass::setInputMode(const string &inputmode) {
	MMSTHEMECLASS_SET_STRING(inputmode);
}

bool MMSWidgetClass::getInputMode(string &inputmode) {
	MMSTHEMECLASS_GET_STRING(inputmode);
}



void MMSWidgetClass::initJoinedWidget() {
	MMSTHEMECLASS_INIT_STRING(joinedwidget);
}

void MMSWidgetClass::freeJoinedWidget() {
	MMSTHEMECLASS_FREE_STRING(joinedwidget);
}

bool MMSWidgetClass::isJoinedWidget() {
	MMSTHEMECLASS_ISSET(joinedwidget);
}

void MMSWidgetClass::unsetJoinedWidget() {
	MMSTHEMECLASS_UNSET(joinedwidget);
}

void MMSWidgetClass::setJoinedWidget(const string &joinedwidget) {
	MMSTHEMECLASS_SET_STRING(joinedwidget);
}

bool MMSWidgetClass::getJoinedWidget(string &joinedwidget) {
	MMSTHEMECLASS_GET_STRING(joinedwidget);
}


void MMSWidgetClass::initActivated() {
    MMSTHEMECLASS_INIT_BASIC(activated);
}

void MMSWidgetClass::freeActivated() {
    MMSTHEMECLASS_FREE_BASIC(activated);
}

bool MMSWidgetClass::isActivated() {
	MMSTHEMECLASS_ISSET(activated);
}

void MMSWidgetClass::unsetActivated() {
    MMSTHEMECLASS_UNSET(activated);
}

void MMSWidgetClass::setActivated(bool activated) {
	MMSTHEMECLASS_SET_BASIC(activated);
}

bool MMSWidgetClass::getActivated(bool &activated) {
	MMSTHEMECLASS_GET_BASIC(activated);
}

