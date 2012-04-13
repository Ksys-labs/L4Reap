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

#include "mmsgui/mmsbuttonwidget.h"

MMSButtonWidget::MMSButtonWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    create(root, className, theme);
}

MMSButtonWidget::~MMSButtonWidget() {
}

bool MMSButtonWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_BUTTON;
    this->className = className;

    // init attributes for drawable widgets
    this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->buttonWidgetClass = this->da->theme->getButtonWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->buttonWidgetClass.widgetClass);
    if (this->buttonWidgetClass) this->da->widgetClass = &(this->buttonWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    return MMSWidget::create(root, true, false, true, true, true, true, true);
}

MMSWidget *MMSButtonWidget::copyWidget() {
    /* create widget */
    MMSButtonWidget *newWidget = new MMSButtonWidget(this->rootwindow, className);

    //copy _only_!!!!!!!! my attributes

    newWidget->className = this->className;
    newWidget->buttonWidgetClass = this->buttonWidgetClass;
    newWidget->myButtonWidgetClass = this->myButtonWidgetClass;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    return newWidget;
}

bool MMSButtonWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    return true;
}

bool MMSButtonWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    return true;
}

bool MMSButtonWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    /* lock */
    this->surface->lock();

    /* draw widget basics */
    if (MMSWidget::draw(backgroundFilled)) {
        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* unlock */
    this->surface->unlock();

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSButtonWidget::updateFromThemeClass(MMSButtonWidgetClass *themeClass) {
    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
