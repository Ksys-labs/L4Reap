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

#ifndef MMSPROGRESSBARWIDGET_H_
#define MMSPROGRESSBARWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can display a progress bar.
/*!
Shows the progress of any action.
The progressbar widget cannot be focused.
\author Jens Schneider
*/
class MMSProgressBarWidget : public MMSWidget {

	private:
        string              		className;
        MMSProgressBarWidgetClass 	*progressBarWidgetClass;
        MMSProgressBarWidgetClass 	myProgressBarWidgetClass;

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

	public:
        MMSProgressBarWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSProgressBarWidget();

        MMSWidget *copyWidget();

    public:
        /* theme access methods */
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        unsigned int getProgress();

        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setProgress(unsigned int progress, bool refresh = true);

        void updateFromThemeClass(MMSProgressBarWidgetClass *themeClass);
};

#endif /*MMSPROGRESSBARWIDGET_H_*/
