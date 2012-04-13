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

#ifndef MMSSLIDERWIDGET_H_
#define MMSSLIDERWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can display a slider.
/*!
The slider consists of an image which will be positioned within the slider widget area.
The slider widget cannot be focused.
\author Jens Schneider
*/
class MMSSliderWidget : public MMSWidget {
    private:
        string         			className;
        MMSSliderWidgetClass 	*sliderWidgetClass;
        MMSSliderWidgetClass 	mySliderWidgetClass;

        bool imagepath_set;
        bool selimagepath_set;

        bool imagepath_p_set;
        bool selimagepath_p_set;

        bool imagepath_i_set;
        bool selimagepath_i_set;

        bool barimagepath_set;
        bool selbarimagepath_set;

        MMSFBSurface    *image;
        MMSFBSurface    *selimage;
        MMSFBSurface    *image_p;
        MMSFBSurface    *selimage_p;
        MMSFBSurface    *image_i;
        MMSFBSurface    *selimage_i;

        MMSFBSurface    *barimage;
        MMSFBSurface    *selbarimage;

        bool	vertical;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground image
        MMSFBSurface	*current_fgimage;

        //! current foreground bar image
        MMSFBSurface	*current_fgbarimage;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        bool init();
        bool release();

        void getImage(MMSFBSurface **suf);
        void getBarImage(MMSFBSurface **suf);
        void calcPos(MMSFBSurface *suf, MMSFBRectangle *surfaceGeom, bool *vertical,
					 MMSFBSurface *barsuf = NULL, MMSFBRectangle *src_barGeom = NULL, MMSFBRectangle *dst_barGeom = NULL);

        void getForeground(MMSFBSurface	**image, MMSFBSurface **barimage);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);
        void switchArrowWidgets();

    public:
        MMSSliderWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSSliderWidget();

        MMSWidget *copyWidget();

        bool scrollDown(unsigned int count = 1, bool refresh = true, bool test = false, bool leave_selection = false);
        bool scrollUp(unsigned int count = 1, bool refresh = true, bool test = false, bool leave_selection = false);
        bool scrollRight(unsigned int count = 1, bool refresh = true, bool test = false, bool leave_selection = false);
        bool scrollLeft(unsigned int count = 1, bool refresh = true, bool test = false, bool leave_selection = false);

        bool scrollTo(int posx, int posy, bool refresh = true, bool *changed = NULL);

        sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator> *onSliderIncrement;
        sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator> *onSliderDecrement;

    public:
        /* theme access methods */
        string getImagePath();
        string getImageName();
        string getSelImagePath();
        string getSelImageName();
        string getImagePath_p();
        string getImageName_p();
        string getSelImagePath_p();
        string getSelImageName_p();
        string getImagePath_i();
        string getImageName_i();
        string getSelImagePath_i();
        string getSelImageName_i();
        unsigned int getPosition();
        string getBarImagePath();
        string getBarImageName();
        string getSelBarImagePath();
        string getSelBarImageName();

        void setImagePath(string imagepath, bool load = true, bool refresh = true);
        void setImageName(string imagename, bool load = true, bool refresh = true);
        void setImage(string imagepath, string imagename, bool load = true, bool refresh = true);
        void setSelImagePath(string selimagepath, bool load = true, bool refresh = true);
        void setSelImageName(string selimagename, bool load = true, bool refresh = true);
        void setSelImage(string selimagepath, string selimagename, bool load = true, bool refresh = true);
        void setImagePath_p(string imagepath_p, bool load = true, bool refresh = true);
        void setImageName_p(string imagename_p, bool load = true, bool refresh = true);
        void setImage_p(string imagepath_p, string imagename_p, bool load = true, bool refresh = true);
        void setSelImagePath_p(string selimagepath_p, bool load = true, bool refresh = true);
        void setSelImageName_p(string selimagename_p, bool load = true, bool refresh = true);
        void setSelImage_p(string selimagepath_p, string selimagename_p, bool load = true, bool refresh = true);
        void setImagePath_i(string imagepath_i, bool load = true, bool refresh = true);
        void setImageName_i(string imagename_i, bool load = true, bool refresh = true);
        void setImage_i(string imagepath_i, string imagename_i, bool load = true, bool refresh = true);
        void setSelImagePath_i(string selimagepath_i, bool load = true, bool refresh = true);
        void setSelImageName_i(string selimagename_i, bool load = true, bool refresh = true);
        void setSelImage_i(string selimagepath_i, string selimagename_i, bool load = true, bool refresh = true);
        void setPosition(unsigned int pos, bool refresh = true);
        void setBarImagePath(string barimagepath, bool load = true, bool refresh = true);
        void setBarImageName(string barimagename, bool load = true, bool refresh = true);
        void setBarImage(string barimagepath, string barimagename, bool load = true, bool refresh = true);
        void setSelBarImagePath(string selbarimagepath, bool load = true, bool refresh = true);
        void setSelBarImageName(string selbarimagename, bool load = true, bool refresh = true);
        void setSelBarImage(string selbarimagepath, string selbarimagename, bool load = true, bool refresh = true);

        void updateFromThemeClass(MMSSliderWidgetClass *themeClass);
};

#endif /*MMSSLIDERWIDGET_H_*/
