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

#ifndef MMSARROWWIDGET_H_
#define MMSARROWWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can draw an arrow.
/*!
The arrow here is nothing more than a triangle with a specified direction.
The arrow widget cannot be focused.
But if you click on it (e.g. mouse or touch screen), the arrow widget submits an input
event (MMSKEY_CURSOR_LEFT, MMSKEY_CURSOR_RIGHT, MMSKEY_CURSOR_UP, MMSKEY_CURSOR_DOWN)
to the toplevel parent window according to the direction parameter (see setDirection()).
\author Jens Schneider
*/
class MMSArrowWidget : public MMSWidget {
    private:
        string         		className;
        MMSArrowWidgetClass *arrowWidgetClass;
        MMSArrowWidgetClass myArrowWidgetClass;

        bool last_pressed;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground color
        MMSFBColor		current_fgcolor;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        bool init();
        bool release();

        void getForeground(MMSFBColor *color);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);

        void handleInput(MMSInputEvent *inputevent);

    public:
        MMSArrowWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSArrowWidget();

        MMSWidget *copyWidget();

    public:
        /* theme access methods */
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        MMSDIRECTION getDirection();
        bool getCheckSelected();

        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setDirection(MMSDIRECTION direction, bool refresh = true);
        void setCheckSelected(bool checkselected);

        void updateFromThemeClass(MMSArrowWidgetClass *themeClass);
};

#endif /*MMSARROWWIDGET_H_*/
