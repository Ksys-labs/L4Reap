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

#include "mmsgui/mmsvboxwidget.h"

MMSVBoxWidget::MMSVBoxWidget(MMSWindow *root) : MMSWidget::MMSWidget() {
    create(root);
}

bool MMSVBoxWidget::create(MMSWindow *root) {
	this->type = MMSWIDGETTYPE_VBOX;
    return MMSWidget::create(root, false, true, false, false, true, true, false);
}

MMSWidget *MMSVBoxWidget::copyWidget() {
    // create widget
    MMSVBoxWidget *newWidget = new MMSVBoxWidget(this->rootwindow);

    // copy base widget
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    return newWidget;
}

void MMSVBoxWidget::add(MMSWidget *widget) {
	widget->setParent(this);
    this->children.push_back(widget);
    if (this->getRootWindow())
        this->getRootWindow()->add(widget);
    this->recalculateChildren();
}



void MMSVBoxWidget::calcSize(int *num_spacers, int *last_spacer,
							 int *required_pix, int *remain_pix, int *avail_pix, int *fixed_pix, int *dyn_pix, int *min_dyn_pix,
							 float dyn_reduce_factor) {
	*num_spacers  = 0;
	*last_spacer  = -1;
	*required_pix = 0;
	*remain_pix   = 0;
	*avail_pix    = 0;
	*fixed_pix    = 0;
    *dyn_pix      = 0;
    *min_dyn_pix  = 0;

    // through all my children
	for(unsigned int i = 0; i < this->children.size(); i++) {
		int content_width;
		int content_height;
		if (!children.at(i)->getContentSize(&content_width, &content_height)) {
			// size of content not set, use sizehint
			string sizehint = children.at(i)->getSizeHint();

			if (sizehint == "") {
				// have no sizehint
				(*num_spacers)++;
				*last_spacer = i;
			}
			else {
				// calculate length based on sizehint
				int len;
				getPixelFromSizeHint(&len, sizehint, this->geom.h, this->geom.w);
				(*fixed_pix)+= len;
			}
		}
		else
		if (dyn_reduce_factor < 0.0001) {
			// use fixed min size
			(*fixed_pix)+= children.at(i)->getMinHeightPix();
		}
		else {
			// use content height
			content_height = (int)((float)content_height * dyn_reduce_factor + 0.5);
			if (content_height <= children.at(i)->getMinHeightPix()) {
				// use fixed min size
				(*fixed_pix)+= children.at(i)->getMinHeightPix();
			}
			else {
				// use dynamic height
				(*dyn_pix)+= content_height;
				(*min_dyn_pix)+= children.at(i)->getMinHeightPix();
			}
		}
	}

	// minimal size needed
	*required_pix = *fixed_pix + *dyn_pix;

	// remaining pixels
	*remain_pix = this->geom.h - *required_pix;

	// available pixels
	*avail_pix = this->geom.h - *fixed_pix;
}


void MMSVBoxWidget::recalculateChildren() {

    // check something
    if(this->children.empty())
        return;

    if(this->geomset == false)
        return;

    // first pass: check if content fits into box, start with factor 1.0
    int num_spacers, last_spacer;
    int required_pix, remain_pix, avail_pix, fixed_pix, dyn_pix, min_dyn_pix;
    float dyn_reduce_factor = 1.0f;
    while (1) {
    	// calculate content size of box
    	calcSize(&num_spacers, &last_spacer,
				 &required_pix, &remain_pix, &avail_pix, &fixed_pix, &dyn_pix, &min_dyn_pix,
    			 dyn_reduce_factor);

    	if (remain_pix >= 0) {
    		// fine, all widgets can be put into this box
    		break;
    	}

    	// negative remaining pixels, so try to reduce something
    	if (avail_pix > min_dyn_pix) {
    		// available pixels for dynamic widget, so calculate reduce factor and recalc content size
    		dyn_reduce_factor = (float)((float)avail_pix) / ((float)dyn_pix / dyn_reduce_factor);
			continue;
    	}
    	else
    	if (avail_pix == min_dyn_pix) {
    		// there are no free pixels for dynamic widgets, so set reduce factor to zero and recalc content size
    		dyn_reduce_factor = 0.0f;
			continue;
    	}
    	else {
    		printf("VBOX: cannot calculate geometry (not enough free pixels)\n");
    		return;
    		// fixed content of box does not fit into it
    		//do not throw exception as this will left surface locks behind
    		//throw MMSWidgetError(0,"VBOX: cannot calculate geometry (not enough free pixels)");
    	}
    }


    // second pass: calculate geometry of all children
    int next_pos = this->geom.y;
    int safe_len = (num_spacers) ? remain_pix / num_spacers : 0;
    for (unsigned int i = 0; i < this->children.size(); i++) {
        MMSFBRectangle rect;
    	int content_width, content_height;
    	if (!children.at(i)->getContentSize(&content_width, &content_height)) {
    		// size of content not set, use sizehint
			string sizehint = children.at(i)->getSizeHint();

			if (sizehint == "") {
				// calculate height based on remaining space
				rect.h = safe_len;
				if (i == last_spacer)
					rect.h+= remain_pix % num_spacers;
			}
			else {
				// calculate height based on sizehint
				getPixelFromSizeHint(&rect.h, sizehint, this->geom.h, this->geom.w);
			}
    	}
    	else {
    		// use content height
    		rect.h = (int)((float)content_height * dyn_reduce_factor + 0.5);
    		if (rect.h < children.at(i)->getMinHeightPix())
    			rect.h = children.at(i)->getMinHeightPix();
    	}

        // set geometry of child widget
        rect.y = next_pos;
     	rect.x = this->geom.x;
        rect.w = this->geom.w;
        this->children.at(i)->setGeometry(rect);

        // next position
        next_pos+= rect.h;
    }
}

void MMSVBoxWidget::setContentSizeFromChildren() {
	if (!this->minmax_set) {
		return;
	}

	if (!this->parent)
		return;

	// determine width and height of my content
	int mycw = getMinWidthPix();
	int mych = 0;
    for (unsigned int i = 0; i < this->children.size(); i++) {
    	int content_width, content_height;

    	if (!children.at(i)->getContentSize(&content_width, &content_height)) {
    		// size of content not set, use sizehint
			string sizehint = children.at(i)->getSizeHint();

			if (sizehint == "") {
				// have no sizehint, we assume lowest height of 0
				content_height = 0;
			}
			else {
				// calculate height based on sizehint
				getPixelFromSizeHint(&content_height, sizehint, this->geom.h, this->geom.w);
			}
    	}
    	else {
    		// content size of child set, so we can set lowest width of my content
    		if (mycw < content_width)
    			mycw = content_width;
    	}

    	mych+= content_height;
    }

    if (mycw > 0 && mych > 0) {
    	// width and height of my content set, check min/max ranges
    	if (mych < getMinHeightPix())
    		mych = getMinHeightPix();
    	if (getMaxWidthPix() > 0 && mycw > getMaxWidthPix())
    		mycw = getMaxWidthPix();
    	if (getMaxHeightPix() > 0 && mych > getMaxHeightPix())
    		mych = getMaxHeightPix();

    	// set my size
		this->content_width_child = mycw;
		this->content_height_child = mych;
		this->parent->setContentSizeFromChildren();
    }
}

