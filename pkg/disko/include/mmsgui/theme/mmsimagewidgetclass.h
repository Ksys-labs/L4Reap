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

#ifndef MMSIMAGEWIDGETCLASS_H_
#define MMSIMAGEWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSImageWidget which are additional to the MMSWidgetClass
namespace MMSGUI_IMAGEWIDGET_ATTR {

	#define MMSGUI_IMAGEWIDGET_ATTR_ATTRDESC \
		{ "image", TAFF_ATTRTYPE_STRING }, \
		{ "image.path", TAFF_ATTRTYPE_STRING }, \
		{ "image.name", TAFF_ATTRTYPE_STRING }, \
		{ "selimage", TAFF_ATTRTYPE_STRING }, \
		{ "selimage.path", TAFF_ATTRTYPE_STRING }, \
		{ "selimage.name", TAFF_ATTRTYPE_STRING }, \
		{ "image_p", TAFF_ATTRTYPE_STRING }, \
		{ "image_p.path", TAFF_ATTRTYPE_STRING }, \
		{ "image_p.name", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_p", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_p.path", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_p.name", TAFF_ATTRTYPE_STRING }, \
		{ "image_i", TAFF_ATTRTYPE_STRING }, \
		{ "image_i.path", TAFF_ATTRTYPE_STRING }, \
		{ "image_i.name", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_i", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_i.path", TAFF_ATTRTYPE_STRING }, \
		{ "selimage_i.name", TAFF_ATTRTYPE_STRING }, \
		{ "useratio", TAFF_ATTRTYPE_BOOL }, \
		{ "fit_width", TAFF_ATTRTYPE_BOOL }, \
		{ "fit_height", TAFF_ATTRTYPE_BOOL }, \
		{ "alignment", TAFF_ATTRTYPE_STRING }, \
		{ "mirror_size", TAFF_ATTRTYPE_INT }, \
		{ "gen_taff", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_IMAGEWIDGET_ATTR_IDS \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_p_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_p_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_image_i_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i_path, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_selimage_i_name, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_useratio, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_fit_width, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_fit_height, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_alignment, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_mirror_size, \
		MMSGUI_IMAGEWIDGET_ATTR_IDS_gen_taff

	#define MMSGUI_IMAGEWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_IMAGEWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_IMAGEWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_IMAGEWIDGET_ATTR_I[];


//! A data access class for the image widget.
/*!
This class is the base for the MMSImageWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSImageWidget.
\author Jens Schneider
*/
class MMSImageWidgetClass {
    private:
    	//! name of the theme class
        string       className;

        //! is imagepath set?
        bool         isimagepath;

        //! path to the image if the widget is not selected
        string       imagepath;

        //! is imagename set?
        bool         isimagename;

        //! image filename if the widget is not selected
        string       imagename;

        //! is selimagepath set?
        bool         isselimagepath;

        //! path to the image if the widget is selected
        string       selimagepath;

        //! is selimagename set?
        bool         isselimagename;

        //! image filename if the widget is selected
        string       selimagename;

        //! is pressed imagepath set?
        bool         isimagepath_p;

        //! path to the pressed image if the widget is not selected
        string       imagepath_p;

        //! is pressed imagename set?
        bool         isimagename_p;

        //! pressed image filename if the widget is not selected
        string       imagename_p;

        //! is pressed selimagepath set?
        bool         isselimagepath_p;

        //! path to the pressed image if the widget is selected
        string       selimagepath_p;

        //! is pressed selimagename set?
        bool         isselimagename_p;

        //! pressed image filename if the widget is selected
        string       selimagename_p;

        //! is inactive imagepath set?
        bool         isimagepath_i;

        //! path to the inactive image if the widget is not selected
        string       imagepath_i;

        //! is inactive imagename set?
        bool         isimagename_i;

        //! inactive image filename if the widget is not selected
        string       imagename_i;

        //! is inactive selimagepath set?
        bool         isselimagepath_i;

        //! path to the inactive image if the widget is selected
        string       selimagepath_i;

        //! is inactive selimagename set?
        bool         isselimagename_i;

        //! inactive image filename if the widget is selected
        string       selimagename_i;

        //! is use aspect ratio flag set?
        bool         isuseratio;

        //! use aspect ratio of the images (true/false)
        bool         useratio;

        //! is fitwidth set?
        bool         isfitwidth;

        //! image width should be the same as widget width
        bool         fitwidth;

        //! is fitheight set?
        bool         isfitheight;

        //! image height should be the same as widget height
        bool         fitheight;

        //! is the alignment set?
        bool         isalignment;

        //! the alignment of the image within the widget
        MMSALIGNMENT alignment;

        //! is the mirror size set?
        bool         ismirrorsize;

        //! the size of the mirror effect
        unsigned int mirrorsize;

        //! is gen taff flag set?
        bool         isgentaff;

        //! generate taff files while reading images true/false
        bool         gentaff;


        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   		pointer to the TAFF buffer
        \param prefix  		optional, prefix to all attribute names (<prefix><attrname>=<attrvalue>)
        \param path    		optional, path needed for empty path values from the TAFF buffer
        \param reset_paths  optional, should reset all path attributes?
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix = NULL, string *path = NULL,
								   bool reset_paths = false);

    public:
    	//! stores base widget attributes
        MMSWidgetClass widgetClass;

        //! Constructor of class MMSImageWidgetClass.
        MMSImageWidgetClass();

        //! Mark all attributes as not set.
        void unsetAll();

        //! Set the name of the theme class.
        /*!
        \param classname  name of the class
        */
        void setClassName(string className);

        //! Get the name of the theme class.
        /*!
        \return name of the class
        */
        string getClassName();

        //! Check if the imagepath is set. This path will be used for the unselected widget.
        bool isImagePath();

        //! Set the imagepath which is used to draw the unselected widget.
        /*!
        \param imagepath  path to unselected image
        */
        void setImagePath(string imagepath);

        //! Mark the imagepath as not set.
        void unsetImagePath();

        //! Get the imagepath which is used to draw the unselected widget.
        /*!
        \return path to the unselected image
        */
        string getImagePath();

        //! Check if the imagename is set. This name will be used for the unselected widget.
        bool isImageName();

        //! Set the imagename which is used to draw the unselected widget.
        /*!
        \param imagename  name of the unselected image
        */
        void setImageName(string imagename);

        //! Mark the imagename as not set.
        void unsetImageName();

        //! Get the imagename which is used to draw the unselected widget.
        /*!
        \return name of the unselected image
        */
        string getImageName();

        //! Check if the selimagepath is set. This path will be used for the selected widget.
        bool isSelImagePath();

        //! Set the selimagepath which is used to draw the selected widget.
        /*!
        \param selimagepath  path to selected image
        */
        void setSelImagePath(string selimagepath);

        //! Mark the selimagepath as not set.
        void unsetSelImagePath();

        //! Get the selimagepath which is used to draw the selected widget.
        /*!
        \return path to the selected image
        */
        string getSelImagePath();

        //! Check if the selimagename is set. This name will be used for the selected widget.
        bool isSelImageName();

        //! Set the selimagename which is used to draw the selected widget.
        /*!
        \param selimagename  name of the selected image
        */
        void setSelImageName(string selimagename);

        //! Mark the selimagename as not set.
        void unsetSelImageName();

        //! Get the selimagename which is used to draw the selected widget.
        /*!
        \return name of the selected image
        */
        string getSelImageName();

        //! Check if the pressed imagepath is set. This path will be used for the unselected widget.
        bool isImagePath_p();

        //! Set the pressed imagepath which is used to draw the unselected widget.
        /*!
        \param imagepath_p  path to pressed unselected image
        */
        void setImagePath_p(string imagepath_p);

        //! Mark the pressed imagepath as not set.
        void unsetImagePath_p();

        //! Get the pressed imagepath which is used to draw the unselected widget.
        /*!
        \return path to the pressed unselected image
        */
        string getImagePath_p();

        //! Check if the pressed imagename is set. This name will be used for the unselected widget.
        bool isImageName_p();

        //! Set the pressed imagename which is used to draw the unselected widget.
        /*!
        \param imagename_p  name of the pressed unselected image
        */
        void setImageName_p(string imagename_p);

        //! Mark the pressed imagename as not set.
        void unsetImageName_p();

        //! Get the pressed imagename which is used to draw the unselected widget.
        /*!
        \return name of the pressed unselected image
        */
        string getImageName_p();

        //! Check if the pressed selimagepath is set. This path will be used for the selected widget.
        bool isSelImagePath_p();

        //! Set the pressed selimagepath which is used to draw the selected widget.
        /*!
        \param selimagepath_p  path to pressed selected image
        */
        void setSelImagePath_p(string selimagepath_p);

        //! Mark the pressed selimagepath as not set.
        void unsetSelImagePath_p();

        //! Get the pressed selimagepath which is used to draw the selected widget.
        /*!
        \return path to the pressed selected image
        */
        string getSelImagePath_p();

        //! Check if the pressed selimagename is set. This name will be used for the selected widget.
        bool isSelImageName_p();

        //! Set the pressed selimagename which is used to draw the selected widget.
        /*!
        \param selimagename_p  name of the pressed selected image
        */
        void setSelImageName_p(string selimagename_p);

        //! Mark the pressed selimagename as not set.
        void unsetSelImageName_p();

        //! Get the pressed selimagename which is used to draw the selected widget.
        /*!
        \return name of the pressed selected image
        */
        string getSelImageName_p();

        //! Check if the inactive imagepath is set. This path will be used for the unselected widget.
        bool isImagePath_i();

        //! Set the inactive imagepath which is used to draw the unselected widget.
        /*!
        \param imagepath_i  path to inactive unselected image
        */
        void setImagePath_i(string imagepath_i);

        //! Mark the inactive imagepath as not set.
        void unsetImagePath_i();

        //! Get the inactive imagepath which is used to draw the unselected widget.
        /*!
        \return path to the inactive unselected image
        */
        string getImagePath_i();

        //! Check if the inactive imagename is set. This name will be used for the unselected widget.
        bool isImageName_i();

        //! Set the inactive imagename which is used to draw the unselected widget.
        /*!
        \param imagename_i  name of the inactive unselected image
        */
        void setImageName_i(string imagename_i);

        //! Mark the inactive imagename as not set.
        void unsetImageName_i();

        //! Get the inactive imagename which is used to draw the unselected widget.
        /*!
        \return name of the inactive unselected image
        */
        string getImageName_i();

        //! Check if the inactive selimagepath is set. This path will be used for the selected widget.
        bool isSelImagePath_i();

        //! Set the inactive selimagepath which is used to draw the selected widget.
        /*!
        \param selimagepath_i  path to inactive selected image
        */
        void setSelImagePath_i(string selimagepath_i);

        //! Mark the inactive selimagepath as not set.
        void unsetSelImagePath_i();

        //! Get the inactive selimagepath which is used to draw the selected widget.
        /*!
        \return path to the inactive selected image
        */
        string getSelImagePath_i();

        //! Check if the inactive selimagename is set. This name will be used for the selected widget.
        bool isSelImageName_i();

        //! Set the inactive selimagename which is used to draw the selected widget.
        /*!
        \param selimagename_i  name of the inactive selected image
        */
        void setSelImageName_i(string selimagename_i);

        //! Mark the inactive selimagename as not set.
        void unsetSelImageName_i();

        //! Get the inactive selimagename which is used to draw the selected widget.
        /*!
        \return name of the inactive selected image
        */
        string getSelImageName_i();

        //! Check if the use acpect ratio is set.
        bool isUseRatio();

        //! Set the use aspect ratio flag.
        /*!
        \param useratio  use aspect ratio true/false
        */
        void setUseRatio(bool useratio);

        //! Mark the use aspect ratio as not set.
        void unsetUseRatio();

        //! Get the use aspect ratio flag.
        /*!
        \return use aspect ratio flag
        */
        bool getUseRatio();

        //! Check if the fitwidth flag is set.
        bool isFitWidth();

        //! Set the fitwidth flag.
        /*!
        \param fitwidth  fit width true/false
        */
        void setFitWidth(bool fitwidth);

        //! Mark the fitwidth as not set.
        void unsetFitWidth();

        //! Get the fitwidth flag.
        /*!
        \return fitwidth flag
        */
        bool getFitWidth();

        //! Check if the fitheight flag is set.
        bool isFitHeight();

        //! Set the fitheight flag.
        /*!
        \param fitheight  fit height true/false
        */
        void setFitHeight(bool fitheight);

        //! Mark the fitheight as not set.
        void unsetFitHeight();

        //! Get the fitheight flag.
        /*!
        \return fitheight flag
        */
        bool getFitHeight();

        //! Check if the alignment of the image is set.
        bool isAlignment();

        //! Set the alignment of the image.
        /*!
        \param alignment	alignment of the image within the widget area
        */
        void setAlignment(MMSALIGNMENT alignment);

        //! Mark the alignment as not set.
        void unsetAlignment();

        //! Get the alignment.
        /*!
        \return alignment of the image within the widget
        */
        MMSALIGNMENT getAlignment();

        //! Check if the mirror size is set.
        bool isMirrorSize();

        //! Set the mirror size.
        /*!
        \param mirrorsize size of the mirror effect in pixel
        */
        void setMirrorSize(unsigned int mirrorsize);

        //! Mark the mirror size as not set.
        void unsetMirrorSize();

        //! Get the mirror size.
        /*!
        \return size of the mirror in pixel
        */
        unsigned int getMirrorSize();

        //! Check if the gen taff flag is set.
        bool isGenTaff();

        //! Set the gen taff flag.
        /*!
        \param gentaff  generate taff files while reading images if set to true
        \note Taff files will never generated, if you load images e.g. from http:// or ftp://.
        */
        void setGenTaff(bool gentaff);

        //! Mark the gen taff flag as not set.
        void unsetGenTaff();

        //! Get the gen taff flag.
        /*!
        \return gen taff flag
        */
        bool getGenTaff();


    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSIMAGEWIDGETCLASS_H_*/
