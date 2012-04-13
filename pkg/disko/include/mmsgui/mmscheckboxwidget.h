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

#ifndef MMSCHECKBOXWIDGET_H_
#define MMSCHECKBOXWIDGET_H_

#include "mmsgui/mmswidget.h"


//! With this class you can display an on/off switch.
/*!
The checkbox is focusable.
\author Jens Schneider
*/
class MMSCheckBoxWidget : public MMSWidget {
    private:
        string          		className;
        MMSCheckBoxWidgetClass 	*checkBoxWidgetClass;
        MMSCheckBoxWidgetClass 	myCheckBoxWidgetClass;

        MMSFBSurface	*checked_bgimage;
        MMSFBSurface	*checked_selbgimage;
        MMSFBSurface	*checked_bgimage_p;
        MMSFBSurface	*checked_selbgimage_p;
        MMSFBSurface	*checked_bgimage_i;
        MMSFBSurface	*checked_selbgimage_i;

        //! current checked background values set?
        bool			current_checked_bgset;

        //! current checked background color
        MMSFBColor		current_checked_bgcolor;

        //! current checked background image
        MMSFBSurface	*current_checked_bgimage;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        bool init();
        bool release();

        void getCheckedBackground(MMSFBColor *color, MMSFBSurface **image);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);

        void handleInput(MMSInputEvent *inputevent);

    public:
        MMSCheckBoxWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSCheckBoxWidget();

        MMSWidget *copyWidget();

    public:
        // theme access methods
        bool 	getCheckedBgColor(MMSFBColor &checked_bgcolor);
        bool 	getCheckedSelBgColor(MMSFBColor &checked_selbgcolor);
        bool	getCheckedBgColor_p(MMSFBColor &checked_bgcolor_p);
        bool	getCheckedSelBgColor_p(MMSFBColor &checked_selbgcolor_p);
        bool	getCheckedBgColor_i(MMSFBColor &checked_bgcolor_i);
        bool	getCheckedSelBgColor_i(MMSFBColor &checked_selbgcolor_i);
        bool    getCheckedBgImagePath(string &checked_bgimagepath);
        bool    getCheckedBgImageName(string &checked_bgimagename);
        bool    getCheckedSelBgImagePath(string &checked_selbgimagepath);
        bool    getCheckedSelBgImageName(string &checked_selbgimagename);
        bool    getCheckedBgImagePath_p(string &checked_bgimagepath_p);
        bool    getCheckedBgImageName_p(string &checked_bgimagename_p);
        bool    getCheckedSelBgImagePath_p(string &checked_selbgimagepath_p);
        bool    getCheckedSelBgImageName_p(string &checked_selbgimagename_p);
        bool    getCheckedBgImagePath_i(string &checked_bgimagepath_i);
        bool    getCheckedBgImageName_i(string &checked_bgimagename_i);
        bool    getCheckedSelBgImagePath_i(string &checked_selbgimagepath_i);
        bool    getCheckedSelBgImageName_i(string &checked_selbgimagename_i);
        bool 	getChecked(bool &checked);

        void setCheckedBgColor(MMSFBColor checked_bgcolor, bool refresh = true);
        void setCheckedSelBgColor(MMSFBColor checked_selbgcolor, bool refresh = true);
        void setCheckedBgColor_p(MMSFBColor checked_bgcolor_p, bool refresh = true);
        void setCheckedSelBgColor_p(MMSFBColor checked_selbgcolor_p, bool refresh = true);
        void setCheckedBgColor_i(MMSFBColor checked_bgcolor_i, bool refresh = true);
        void setCheckedSelBgColor_i(MMSFBColor checked_selbgcolor_i, bool refresh = true);
        void setCheckedBgImagePath(string checked_bgimagepath, bool load = true, bool refresh = true);
        void setCheckedBgImageName(string checked_bgimagename, bool load = true, bool refresh = true);
        void setCheckedSelBgImagePath(string checked_selbgimagepath, bool load = true, bool refresh = true);
        void setCheckedSelBgImageName(string checked_selbgimagename, bool load = true, bool refresh = true);
        void setCheckedBgImagePath_p(string checked_bgimagepath_p, bool load = true, bool refresh = true);
        void setCheckedBgImageName_p(string checked_bgimagename_p, bool load = true, bool refresh = true);
        void setCheckedSelBgImagePath_p(string checked_selbgimagepath_p, bool load = true, bool refresh = true);
        void setCheckedSelBgImageName_p(string checked_selbgimagename_p, bool load = true, bool refresh = true);
        void setCheckedBgImagePath_i(string checked_bgimagepath_i, bool load = true, bool refresh = true);
        void setCheckedBgImageName_i(string checked_bgimagename_i, bool load = true, bool refresh = true);
        void setCheckedSelBgImagePath_i(string checked_selbgimagepath_i, bool load = true, bool refresh = true);
        void setCheckedSelBgImageName_i(string checked_selbgimagename_i, bool load = true, bool refresh = true);
        void setChecked(bool checked, bool refresh = true);

        void updateFromThemeClass(MMSCheckBoxWidgetClass *themeClass);
};

#endif /*MMSCHECKBOXWIDGET_H_*/
