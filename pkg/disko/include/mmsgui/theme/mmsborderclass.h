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

#ifndef MMSBORDERCLASS_H_
#define MMSBORDERCLASS_H_

#include "mmsgui/theme/mmsthemebase.h"

//! describe attributes for MMSBorder
namespace MMSGUI_BORDER_ATTR {

	#define MMSGUI_BORDER_ATTR_ATTRDESC \
		{ "border.color", TAFF_ATTRTYPE_COLOR }, \
		{ "border.color.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.color.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.color.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.color.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.selcolor", TAFF_ATTRTYPE_COLOR }, \
		{ "border.selcolor.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.selcolor.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.selcolor.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.selcolor.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.image.path", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.top-left", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.top", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.top-right", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.right", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.bottom-right", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.bottom", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.bottom-left", TAFF_ATTRTYPE_STRING }, \
		{ "border.image.left", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.path", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.top-left", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.top", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.top-right", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.right", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.bottom-right", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.bottom", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.bottom-left", TAFF_ATTRTYPE_STRING }, \
		{ "border.selimage.left", TAFF_ATTRTYPE_STRING }, \
		{ "border.thickness", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.margin", TAFF_ATTRTYPE_UCHAR }, \
		{ "border.rcorners", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_BORDER_ATTR_IDS \
		MMSGUI_BORDER_ATTR_IDS_border_color, \
		MMSGUI_BORDER_ATTR_IDS_border_color_a, \
		MMSGUI_BORDER_ATTR_IDS_border_color_r, \
		MMSGUI_BORDER_ATTR_IDS_border_color_g, \
		MMSGUI_BORDER_ATTR_IDS_border_color_b, \
		MMSGUI_BORDER_ATTR_IDS_border_selcolor, \
		MMSGUI_BORDER_ATTR_IDS_border_selcolor_a, \
		MMSGUI_BORDER_ATTR_IDS_border_selcolor_r, \
		MMSGUI_BORDER_ATTR_IDS_border_selcolor_g, \
		MMSGUI_BORDER_ATTR_IDS_border_selcolor_b, \
		MMSGUI_BORDER_ATTR_IDS_border_image_path, \
		MMSGUI_BORDER_ATTR_IDS_border_image_top_left, \
		MMSGUI_BORDER_ATTR_IDS_border_image_top, \
		MMSGUI_BORDER_ATTR_IDS_border_image_top_right, \
		MMSGUI_BORDER_ATTR_IDS_border_image_right, \
		MMSGUI_BORDER_ATTR_IDS_border_image_bottom_right, \
		MMSGUI_BORDER_ATTR_IDS_border_image_bottom, \
		MMSGUI_BORDER_ATTR_IDS_border_image_bottom_left, \
		MMSGUI_BORDER_ATTR_IDS_border_image_left, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_path, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_top_left, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_top, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_top_right, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_right, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom_right, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_bottom_left, \
		MMSGUI_BORDER_ATTR_IDS_border_selimage_left, \
		MMSGUI_BORDER_ATTR_IDS_border_thickness, \
		MMSGUI_BORDER_ATTR_IDS_border_margin, \
		MMSGUI_BORDER_ATTR_IDS_border_rcorners

	#define MMSGUI_BORDER_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS
	} ids;
}

//! 8 possible border images
#define MMSBORDER_IMAGE_NUM_SIZE	8

//! describes the 8 possible border images
typedef enum {
	MMSBORDER_IMAGE_NUM_TOP_LEFT = 0,
	MMSBORDER_IMAGE_NUM_TOP,
	MMSBORDER_IMAGE_NUM_TOP_RIGHT,
	MMSBORDER_IMAGE_NUM_RIGHT,
	MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT,
	MMSBORDER_IMAGE_NUM_BOTTOM,
	MMSBORDER_IMAGE_NUM_BOTTOM_LEFT,
	MMSBORDER_IMAGE_NUM_LEFT
} MMSBORDER_IMAGE_NUM;


//! A data access class for the border of widgets and windows.
/*!
This class is the base for the MMSBorder class.
With this data store you have access to all changeable border attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSBorder.
\author Jens Schneider
*/
class MMSBorderClass {
    private:
    	struct {
	    	bool         iscolor;
	        MMSFBColor   color;
	        bool         isselcolor;
	        MMSFBColor   selcolor;
	        bool         isimagepath;
	        bool         isimagenames;
	        bool         isselimagepath;
	        bool         isselimagenames;
	        bool         isthickness;
	        unsigned int thickness;
	        bool         ismargin;
	        unsigned int margin;
	        bool         isrcorners;
	        bool         rcorners;
    	} id;

    	struct {
            string       *imagepath;
            string       *imagenames[MMSBORDER_IMAGE_NUM_SIZE];
            string       *selimagepath;
            string       *selimagenames[MMSBORDER_IMAGE_NUM_SIZE];
		} ed;

        /* init routines */
        void initColor();
        void initSelColor();

        void initImagePath();
        void initImageNames();
        void initSelImagePath();
        void initSelImageNames();

        void initThickness();
        void initMargin();
        void initRCorners();

        /* free routines */
        void freeColor();
        void freeSelColor();

        void freeImagePath();
        void freeImageNames();
        void freeSelImagePath();
        void freeSelImageNames();

        void freeThickness();
        void freeMargin();
        void freeRCorners();

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
        MMSBorderClass();
        ~MMSBorderClass();
        MMSBorderClass &operator=(const MMSBorderClass &c);
        //
        void unsetAll();

        bool isColor();
        void unsetColor();
        void setColor(const MMSFBColor &color);
        bool getColor(MMSFBColor &color);
        //
        bool isSelColor();
        void unsetSelColor();
        void setSelColor(const MMSFBColor &selcolor);
        bool getSelColor(MMSFBColor &selcolor);
        //
        bool isImagePath();
        void unsetImagePath();
        void setImagePath(const string &imagepath);
        bool getImagePath(string &imagepath);
        //
        bool isImageNames();
        void unsetImageNames();
        void setImageNames(const string &imagename_1, const string &imagename_2, const string &imagename_3, const string &imagename_4,
        				   const string &imagename_5, const string &imagename_6, const string &imagename_7, const string &imagename_8);
        void setImageNames(MMSBORDER_IMAGE_NUM num, const string &imagename);
        bool getImageNames(MMSBORDER_IMAGE_NUM num, string &imagename);
        //
        bool isSelImagePath();
        void unsetSelImagePath();
        void setSelImagePath(const string &selimagepath);
        bool getSelImagePath(string &selimagepath);
        //
        bool isSelImageNames();
        void unsetSelImageNames();
        void setSelImageNames(const string &selimagename_1, const string &selimagename_2, const string &selimagename_3, const string &selimagename_4,
        					  const string &selimagename_5, const string &selimagename_6, const string &selimagename_7, const string &selimagename_8);
        void setSelImageNames(MMSBORDER_IMAGE_NUM num, const string &selimagename);
        bool getSelImageNames(MMSBORDER_IMAGE_NUM num, string &selimagename);
        //
        bool isThickness();
        void unsetThickness();
        void setThickness(unsigned int thickness);
        bool getThickness(unsigned int &thickness);
        //
        bool isMargin();
        void unsetMargin();
        void setMargin(unsigned int margin);
        bool getMargin(unsigned int &margin);
        //
        bool isRCorners();
        void unsetRCorners();
        void setRCorners(bool rcorners);
        bool getRCorners(bool &rcorners);

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSBORDERCLASS_H_*/
