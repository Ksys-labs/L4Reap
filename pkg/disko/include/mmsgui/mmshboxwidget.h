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

#ifndef MMSHBOXWIDGET_H_
#define MMSHBOXWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can get a horizontal box.
/*!
The hbox widget is a container for other widgets.
You can add more than one child widget to a hbox. Widgets different from MMSHBoxWidget/MMSVBoxWidget can only have one child widget.
The child widgets will be horizontal arranged.
The hbox widget cannot be focused and displays nothing.
\author Jens Schneider
*/
class MMSHBoxWidget : public MMSWidget {
    private:
        bool create(MMSWindow *root);

        void calcSize(int *num_spacers, int *last_spacer,
    			      int *required_pix, int *remain_pix, int *avail_pix, int *fixed_pix, int *dyn_pix, int *min_dyn_pix,
    			      float dyn_reduce_factor);

    	void recalculateChildren();

        void setContentSizeFromChildren();

    public:
        MMSHBoxWidget(MMSWindow *root);

        MMSWidget *copyWidget();

        void add(MMSWidget *widget);
};

#endif /*MMSHBOXWIDGET_H_*/
