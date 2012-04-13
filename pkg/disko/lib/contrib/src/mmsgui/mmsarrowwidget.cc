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

#include "mmsgui/mmsarrowwidget.h"

MMSArrowWidget::MMSArrowWidget(MMSWindow *root, string className, MMSTheme *theme) {
    create(root, className, theme);
}

MMSArrowWidget::~MMSArrowWidget() {
}

bool MMSArrowWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_ARROW;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->arrowWidgetClass = this->da->theme->getArrowWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->arrowWidgetClass.widgetClass);
    if (this->arrowWidgetClass) this->da->widgetClass = &(this->arrowWidgetClass->widgetClass); else this->da->widgetClass = NULL;

	this->last_pressed = false;
	this->current_fgset = false;

    return MMSWidget::create(root, true, false, false, true, true, true, true);
}

MMSWidget *MMSArrowWidget::copyWidget() {
    /* create widget */
    MMSArrowWidget *newWidget = new MMSArrowWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->arrowWidgetClass = this->arrowWidgetClass;
    newWidget->myArrowWidgetClass = this->myArrowWidgetClass;
    newWidget->last_pressed = this->last_pressed;
    newWidget->current_fgset = this->current_fgset;
    newWidget->current_fgcolor = this->current_fgcolor;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    return newWidget;
}

bool MMSArrowWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    return true;
}

bool MMSArrowWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    return true;
}

void MMSArrowWidget::getForeground(MMSFBColor *color) {
	color->a = 0;

	if (isSelected()) {
        *color = getSelColor();
	}
    else {
        *color = getColor();
    }
}

bool MMSArrowWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSArrowWidget::checkRefreshStatus() {
	if (MMSWidget::checkRefreshStatus()) return true;

	if (this->current_fgset) {
		// current foreground initialized
		MMSFBColor color;
		getForeground(&color);

		if (color == this->current_fgcolor) {
			// foreground color not changed, so we do not enable refreshing
			return false;
		}
	}

	// (re-)enable refreshing
	enableRefresh();

	return true;
}

bool MMSArrowWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    // draw widget basics
    if (MMSWidget::draw(backgroundFilled)) {

        // lock
        this->surface->lock();

        // draw my things
        MMSFBRectangle surfaceGeom = getSurfaceGeometry();

        // get color
        MMSFBColor color;
        getForeground(&color);
        this->current_fgcolor   = color;
        this->current_fgset     = true;

        if (color.a) {
            /* prepare for drawing */
            this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(color, getBrightness(), getOpacity());

            /* draw triangle */
            switch (getDirection()) {
                case MMSDIRECTION_LEFT:
                case MMSDIRECTION_NOTSET:
                    /* draw triangle */
                    this->surface->drawTriangle(
                                    surfaceGeom.x,                     surfaceGeom.y + surfaceGeom.h/2,
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y,
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y + surfaceGeom.h - 1 + (surfaceGeom.h % 2 - 1));
                    /* fill triangle */
                    this->surface->fillTriangle(
                                    surfaceGeom.x,                     surfaceGeom.y + surfaceGeom.h/2,
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y,
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y + surfaceGeom.h - 1 + (surfaceGeom.h % 2 - 1));
                    break;
                case MMSDIRECTION_RIGHT:
                    /* draw triangle */
                    this->surface->drawTriangle(
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y + surfaceGeom.h/2,
                                    surfaceGeom.x,                     surfaceGeom.y,
                                    surfaceGeom.x,                     surfaceGeom.y + surfaceGeom.h - 1 + (surfaceGeom.h % 2 - 1));
                    /* fill triangle */
                    this->surface->fillTriangle(
                                    surfaceGeom.x + surfaceGeom.w - 1, surfaceGeom.y + surfaceGeom.h/2,
                                    surfaceGeom.x,                     surfaceGeom.y,
                                    surfaceGeom.x,                     surfaceGeom.y + surfaceGeom.h - 1 + (surfaceGeom.h % 2 - 1));
                    break;
                case MMSDIRECTION_UP:
                    /* draw triangle */
                    this->surface->drawTriangle(
                                    surfaceGeom.x + surfaceGeom.w/2,                            surfaceGeom.y,
                                    surfaceGeom.x,                                              surfaceGeom.y + surfaceGeom.h - 1,
                                    surfaceGeom.x + surfaceGeom.w - 1 + (surfaceGeom.w % 2 - 1),surfaceGeom.y + surfaceGeom.h - 1);
                    /* fill triangle */
                    this->surface->fillTriangle(
                                    surfaceGeom.x + surfaceGeom.w/2,                            surfaceGeom.y,
                                    surfaceGeom.x,                                              surfaceGeom.y + surfaceGeom.h - 1,
                                    surfaceGeom.x + surfaceGeom.w - 1 + (surfaceGeom.w % 2 - 1),surfaceGeom.y + surfaceGeom.h - 1);
                    break;
                case MMSDIRECTION_DOWN:
                    /* draw triangle */
                    this->surface->drawTriangle(
                                    surfaceGeom.x + surfaceGeom.w/2,                            surfaceGeom.y + surfaceGeom.h - 1,
                                    surfaceGeom.x,                                              surfaceGeom.y,
                                    surfaceGeom.x + surfaceGeom.w - 1 + (surfaceGeom.w % 2 - 1),surfaceGeom.y);
                    /* fill triangle */
                    this->surface->fillTriangle(
                                    surfaceGeom.x + surfaceGeom.w/2,                            surfaceGeom.y + surfaceGeom.h - 1,
                                    surfaceGeom.x,                                              surfaceGeom.y,
                                    surfaceGeom.x + surfaceGeom.w - 1 + (surfaceGeom.w % 2 - 1),surfaceGeom.y);
                    break;
            }
        }

        /* unlock */
        this->surface->unlock();

        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}


void MMSArrowWidget::handleInput(MMSInputEvent *inputevent) {
	MMSWidget::handleInput(inputevent);

	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		this->last_pressed = isPressed();
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		if (this->last_pressed) {
			if (this->parent_rootwindow) {
				bool submitinput = true;
				if (getCheckSelected())
					submitinput = (isSelected());
				if (submitinput) {
					// if selected the arrow widget submits an input event
					// according to its direction
					MMSInputEvent ievt;
					ievt.type = MMSINPUTEVENTTYPE_KEYPRESS;
					switch (getDirection()) {
					case MMSDIRECTION_LEFT:
						ievt.key = MMSKEY_CURSOR_LEFT;
						break;
					case MMSDIRECTION_RIGHT:
						ievt.key = MMSKEY_CURSOR_RIGHT;
						break;
					case MMSDIRECTION_UP:
						ievt.key = MMSKEY_CURSOR_UP;
						break;
					case MMSDIRECTION_DOWN:
						ievt.key = MMSKEY_CURSOR_DOWN;
						break;
					default:
						ievt.key = MMSKEY_UNKNOWN;
						break;
					}
					if (ievt.key != MMSKEY_UNKNOWN) {
						this->parent_rootwindow->handleInput(&ievt);
					}
				}
			}
			this->last_pressed = false;
		}
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_AXISMOTION) {
		this->last_pressed = isPressed();
	}
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETARROW(x) \
    if (this->myArrowWidgetClass.is##x()) return myArrowWidgetClass.get##x(); \
    else if ((arrowWidgetClass)&&(arrowWidgetClass->is##x())) return arrowWidgetClass->get##x(); \
    else return this->da->theme->arrowWidgetClass.get##x();

MMSFBColor MMSArrowWidget::getColor() {
    GETARROW(Color);
}

MMSFBColor MMSArrowWidget::getSelColor() {
    GETARROW(SelColor);
}

MMSDIRECTION MMSArrowWidget::getDirection() {
    GETARROW(Direction);
}

bool MMSArrowWidget::getCheckSelected() {
    GETARROW(CheckSelected);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSArrowWidget::setColor(MMSFBColor color, bool refresh) {
    myArrowWidgetClass.setColor(color);

	// refresh required?
	enableRefresh((color != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSArrowWidget::setSelColor(MMSFBColor selcolor, bool refresh) {
    myArrowWidgetClass.setSelColor(selcolor);

	// refresh required?
	enableRefresh((selcolor != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSArrowWidget::setDirection(MMSDIRECTION direction, bool refresh) {
    myArrowWidgetClass.setDirection(direction);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSArrowWidget::setCheckSelected(bool checkselected) {
    myArrowWidgetClass.setCheckSelected(checkselected);
}

void MMSArrowWidget::updateFromThemeClass(MMSArrowWidgetClass *themeClass) {
    if (themeClass->isColor())
        setColor(themeClass->getColor());
    if (themeClass->isSelColor())
        setSelColor(themeClass->getSelColor());
    if (themeClass->isDirection())
        setDirection(themeClass->getDirection());
    if (themeClass->isCheckSelected())
        setCheckSelected(themeClass->getCheckSelected());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
