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

#ifndef MMSIMAGEWIDGET_H_
#define MMSIMAGEWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can display pictures and animations.
/*!
All widgets derived from the base widget class can display pictures at its background.
This image class additionally can display pictures over the background.
As special feature the image class can play animations. Currently the GIF format will be supported.
The image widget cannot be focused.
\author Jens Schneider
*/
class MMSImageWidget : public MMSWidget {
    private:
        string         		className;
        MMSImageWidgetClass *imageWidgetClass;
        MMSImageWidgetClass myImageWidgetClass;

        bool imagepath_set;
        bool selimagepath_set;

        bool imagepath_p_set;
        bool selimagepath_p_set;

        bool imagepath_i_set;
        bool selimagepath_i_set;

        MMSFBSurface    *image;
        MMSIM_DESC_SUF  *image_suf;
        unsigned int    image_curr_index;
        MMSFBSurface    *selimage;
        MMSIM_DESC_SUF  *selimage_suf;
        unsigned int    selimage_curr_index;

        MMSFBSurface    *image_p;
        MMSIM_DESC_SUF  *image_p_suf;
        unsigned int    image_p_curr_index;
        MMSFBSurface    *selimage_p;
        MMSIM_DESC_SUF  *selimage_p_suf;
        unsigned int    selimage_p_curr_index;

        MMSFBSurface    *image_i;
        MMSIM_DESC_SUF  *image_i_suf;
        unsigned int    image_i_curr_index;
        MMSFBSurface    *selimage_i;
        MMSIM_DESC_SUF  *selimage_i_suf;
        unsigned int    selimage_i_curr_index;

        bool    image_loaded;
        bool    image_p_loaded;
        bool    image_i_loaded;
        bool    selimage_loaded;
        bool    selimage_p_loaded;
        bool    selimage_i_loaded;

        class MMSImageWidgetThread  *imageThread;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground image
        MMSFBSurface	*current_fgimage;

        //! current foreground image2
        MMSFBSurface	*current_fgimage2;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        bool init();
        bool release();

        void getForeground(MMSFBSurface	**image, MMSFBSurface **image2);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);

        void loadMyImage(string path, string filename, MMSFBSurface **surface, MMSIM_DESC_SUF **surfdesc,
        				 unsigned int *index, unsigned int mirror_size, bool gen_taff);

        void workWithRatio(MMSFBSurface *suf, MMSFBRectangle *surfaceGeom);

    public:
        MMSImageWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        virtual ~MMSImageWidget();

        MMSWidget *copyWidget();

        void setVisible(bool visible, bool refresh = true);

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
        bool getUseRatio();
        bool getFitWidth();
        bool getFitHeight();
        MMSALIGNMENT getAlignment();
        unsigned int getMirrorSize();
        bool getGenTaff();

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
        void setUseRatio(bool useratio, bool refresh = true);
        void setFitWidth(bool fitwidth, bool refresh = true);
        void setFitHeight(bool fitheight, bool refresh = true);
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true);
        void setMirrorSize(unsigned int mirrorsize, bool refresh = true);
        void setGenTaff(bool gentaff, bool refresh = true);

        void updateFromThemeClass(MMSImageWidgetClass *themeClass);

    friend class MMSImageWidgetThread;
};

#endif /*MMSIMAGEWIDGET_H_*/
