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

#ifndef MMSCHECKBOXWIDGETCLASS_H_
#define MMSCHECKBOXWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSCheckBoxWidget which are additional to the MMSWidgetClass
namespace MMSGUI_CHECKBOXWIDGET_ATTR {

	#define MMSGUI_CHECKBOXWIDGET_ATTR_ATTRDESC \
	{ "checked_bgcolor", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_bgcolor.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_selbgcolor.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_p", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_bgcolor_p.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_p.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_p.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_p.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_p", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_selbgcolor_p.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_p.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_p.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_p.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_i", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_bgcolor_i.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_i.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_i.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgcolor_i.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_i", TAFF_ATTRTYPE_COLOR }, \
	{ "checked_selbgcolor_i.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_i.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_i.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_selbgcolor_i.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "checked_bgimage", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_p", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_p.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_p.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_p", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_p.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_p.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_i", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_i.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_bgimage_i.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_i", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_i.path", TAFF_ATTRTYPE_STRING }, \
	{ "checked_selbgimage_i.name", TAFF_ATTRTYPE_STRING }, \
	{ "checked", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_CHECKBOXWIDGET_ATTR_IDS \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_p_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_p_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgcolor_i_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_a, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_r, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_g, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgcolor_i_b, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_p_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_p_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_bgimage_i_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i_path, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked_selbgimage_i_name, \
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS_checked

	#define MMSGUI_CHECKBOXWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_CHECKBOXWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_CHECKBOXWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_CHECKBOXWIDGET_ATTR_I[];


//! A data access class for the checkbox widget.
/*!
This class is the base for the MMSCheckBoxWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSCheckBoxWidget.
\author Jens Schneider
*/
class MMSCheckBoxWidgetClass {
    private:
    	//! name of the theme class
        string          className;

    	struct {
    		//! is checked_bgcolor set?
	    	bool            ischecked_bgcolor;

	    	//! background color if the widget is checked and not selected
	        MMSFBColor      checked_bgcolor;

	        //! is checked_selbgcolor set?
	        bool            ischecked_selbgcolor;

	        //! background color if the widget is checked and selected
	        MMSFBColor      checked_selbgcolor;

	        //! is pressed checked_bgcolor set?
	    	bool            ischecked_bgcolor_p;

	    	//! pressed background color if the widget is checked and not selected
	        MMSFBColor      checked_bgcolor_p;

	        //! is pressed selbgcolor set?
	        bool            ischecked_selbgcolor_p;

	        //! pressed background color if the widget is checked and selected
	        MMSFBColor      checked_selbgcolor_p;

	        //! is inactive checked_bgcolor set?
	        bool            ischecked_bgcolor_i;

	        //! inactive background color if the widget is checked and not selected
	        MMSFBColor      checked_bgcolor_i;

	        //! is inactive checked_selbgcolor set?
	        bool            ischecked_selbgcolor_i;

	        //! inactive background color if the widget is checked and selected
	        MMSFBColor      checked_selbgcolor_i;

	        //! is checked_bgimagepath set?
	        bool            ischecked_bgimagepath;

	        //! is checked_bgimagename set?
	        bool            ischecked_bgimagename;

	        //! is checked_selbgimagepath set?
	        bool            ischecked_selbgimagepath;

	        //! is checked_selbgimagename set?
	        bool            ischecked_selbgimagename;

	        //! is pressed checked_bgimagepath set?
	        bool            ischecked_bgimagepath_p;

	        //! is pressed checked_bgimagename set?
	        bool            ischecked_bgimagename_p;

	        //! is pressed checked_selbgimagepath set?
	        bool            ischecked_selbgimagepath_p;

	        //! is pressed checked_selbgimagename set?
	        bool            ischecked_selbgimagename_p;

	        //! is inactive checked_bgimagepath set?
	        bool            ischecked_bgimagepath_i;

	        //! is inactive checked_bgimagename set?
	        bool            ischecked_bgimagename_i;

	        //! is inactive checked_selbgimagepath set?
	        bool            ischecked_selbgimagepath_i;

	        //! is inactive checked_selbgimagename set?
	        bool            ischecked_selbgimagename_i;

	        //! is the checked flag set?
	        bool            ischecked;

	        //! widget's checked state true/false
	        bool            checked;
    	} id;

    	struct {
    		//! path to the background image if the widget is checked and not selected
            string          *checked_bgimagepath;

            //! background image filename if the widget is checked and not selected
            string          *checked_bgimagename;

            //! path to the background image if the widget is checked and selected
            string          *checked_selbgimagepath;

            //! background image filename if the widget is checked and selected
            string          *checked_selbgimagename;

            //! path to the pressed background image if the widget is checked and not selected
            string          *checked_bgimagepath_p;

            //! pressed background image filename if the widget is checked and not selected
            string          *checked_bgimagename_p;

            //! path to the pressed background image if the widget is checked and selected
            string          *checked_selbgimagepath_p;

            //! pressed background image filename if the widget is checked and selected
            string          *checked_selbgimagename_p;

            //! path to the inactive background image if the widget is checked and not selected
            string          *checked_bgimagepath_i;

            //! inactive background image filename if the widget is checked and not selected
            string          *checked_bgimagename_i;

            //! path to the inactive background image if the widget is checked and selected
            string          *checked_selbgimagepath_i;

            //! inactive background image filename if the widget is checked and selected
            string          *checked_selbgimagename_i;
    	} ed;

    	/* init routines */
        void initCheckedBgColor();
        void initCheckedSelBgColor();
        void initCheckedBgColor_p();
        void initCheckedSelBgColor_p();
        void initCheckedBgColor_i();
        void initCheckedSelBgColor_i();

        void initCheckedBgImagePath();
        void initCheckedBgImageName();
        void initCheckedSelBgImagePath();
        void initCheckedSelBgImageName();
        void initCheckedBgImagePath_p();
        void initCheckedBgImageName_p();
        void initCheckedSelBgImagePath_p();
        void initCheckedSelBgImageName_p();
        void initCheckedBgImagePath_i();
        void initCheckedBgImageName_i();
        void initCheckedSelBgImagePath_i();
        void initCheckedSelBgImageName_i();

        void initChecked();

        /* free routines */
        void freeCheckedBgColor();
        void freeCheckedSelBgColor();
        void freeCheckedBgColor_p();
        void freeCheckedSelBgColor_p();
        void freeCheckedBgColor_i();
        void freeCheckedSelBgColor_i();

        void freeCheckedBgImagePath();
        void freeCheckedBgImageName();
        void freeCheckedSelBgImagePath();
        void freeCheckedSelBgImageName();
        void freeCheckedBgImagePath_p();
        void freeCheckedBgImageName_p();
        void freeCheckedSelBgImagePath_p();
        void freeCheckedSelBgImageName_p();
        void freeCheckedBgImagePath_i();
        void freeCheckedBgImageName_i();
        void freeCheckedSelBgImagePath_i();
        void freeCheckedSelBgImageName_i();

        void freeChecked();

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

        //! Constructor of class MMSCheckBoxWidgetClass.
        MMSCheckBoxWidgetClass();

        //! Destructor of class MMSCheckBoxWidgetClass.
        ~MMSCheckBoxWidgetClass();

        //! operator=
        MMSCheckBoxWidgetClass &operator=(const MMSCheckBoxWidgetClass &c);

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

        //! Check if the background color is set. This color will be used for the checked, unselected widget.
        bool isCheckedBgColor();

        //! Mark the checked_bgcolor as not set.
        void unsetCheckedBgColor();

        //! Set the background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor  color for checked, unselected background
        */
        void setCheckedBgColor(const MMSFBColor &checked_bgcolor);

        //! Get the background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor  background color
        \return true if set
        */
        bool getCheckedBgColor(MMSFBColor &checked_bgcolor);

        //! Check if the background color is set. This color will be used for the checked, selected widget.
        bool isCheckedSelBgColor();

        //! Mark the checked_selbgcolor as not set.
        void unsetCheckedSelBgColor();

        //! Set the background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor  color for selected background
        */
        void setCheckedSelBgColor(const MMSFBColor &checked_selbgcolor);

        //! Get the background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor  background color
        \return true if set
        */
        bool getCheckedSelBgColor(MMSFBColor &checked_selbgcolor);

        //! Check if the pressed background color is set. This color will be used for the checked, unselected widget.
        bool isCheckedBgColor_p();

        //! Mark the pressed checked_bgcolor as not set.
        void unsetCheckedBgColor_p();

        //! Set the pressed background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor_p  pressed background color
        */
        void setCheckedBgColor_p(const MMSFBColor &checked_bgcolor_p);

        //! Get the pressed background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor_p  pressed background color
        \return true if set
        */
        bool getCheckedBgColor_p(MMSFBColor &checked_bgcolor_p);

        //! Check if the pressed background color is set. This color will be used for the checked, selected widget.
        bool isCheckedSelBgColor_p();

        //! Mark the pressed checked_selbgcolor as not set.
        void unsetCheckedSelBgColor_p();

        //! Set the pressed background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor_p  pressed color for selected background
        */
        void setCheckedSelBgColor_p(const MMSFBColor &checked_selbgcolor_p);

        //! Get the pressed background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor_p  pressed background color
        \return true if set
        */
        bool getCheckedSelBgColor_p(MMSFBColor &checked_selbgcolor_p);

        //! Check if the inactive background color is set. This color will be used for the checked, unselected widget.
        bool isCheckedBgColor_i();

        //! Mark the inactive checked_bgcolor as not set.
        void unsetCheckedBgColor_i();

        //! Set the inactive background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor_i  color for inactive unselected background
        */
        void setCheckedBgColor_i(const MMSFBColor &checked_bgcolor_i);

        //! Get the inactive background color which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgcolor_i  inactive background color
        \return true if set
        */
        bool getCheckedBgColor_i(MMSFBColor &checked_bgcolor_i);

        //! Check if the inactive background color is set. This color will be used for the checked, selected widget.
        bool isCheckedSelBgColor_i();

        //! Mark the inactive checked_selbgcolor as not set.
        void unsetCheckedSelBgColor_i();

        //! Set the inactive background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor_i  color for inactive selected background
        */
        void setCheckedSelBgColor_i(const MMSFBColor &checked_selbgcolor_i);

        //! Get the inactive background color which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgcolor_i  inactive background color
        \return true if set
        */
        bool getCheckedSelBgColor_i(MMSFBColor &checked_selbgcolor_i);

        //! Check if the imagepath for background is set. This path will be used for the checked, unselected widget.
        bool isCheckedBgImagePath();

        //! Mark the checked_bgimagepath as not set.
        void unsetCheckedBgImagePath();

        //! Set the imagepath for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath  path to unselected background image
        */
        void setCheckedBgImagePath(const string &checked_bgimagepath);

        //! Get the imagepath for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath  path to unselected background image
        \return true if set
        */
        bool getCheckedBgImagePath(string &checked_bgimagepath);

        //! Check if the imagename for background is set. This name will be used for the checked, unselected widget.
        bool isCheckedBgImageName();

        //! Mark the checked_bgimagename as not set.
        void unsetCheckedBgImageName();

        //! Set the imagename for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename  name of the unselected background image
        */
        void setCheckedBgImageName(const string &checked_bgimagename);

        //! Get the imagename for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename  name of the unselected background image
        \return true if set
        */
        bool getCheckedBgImageName(string &checked_bgimagename);

        //! Check if the selimagepath for background is set. This path will be used for the checked, selected widget.
        bool isCheckedSelBgImagePath();

        //! Mark the checked_selbgimagepath as not set.
        void unsetCheckedSelBgImagePath();

        //! Set the selimagepath for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath  path to selected background image
        */
        void setCheckedSelBgImagePath(const string &checked_selbgimagepath);

        //! Get the selimagepath for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath  path to the selected background image
        \return true if set
        */
        bool getCheckedSelBgImagePath(string &checked_selbgimagepath);

        //! Check if the selimagename for background is set. This name will be used for the checked, selected widget.
        bool isCheckedSelBgImageName();

        //! Mark the checked_selbgimagename as not set.
        void unsetCheckedSelBgImageName();

        //! Set the selimagename for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename  name of the selected background image
        */
        void setCheckedSelBgImageName(const string &checked_selbgimagename);

        //! Get the selimagename for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename  name of the selected background image
        \return true if set
        */
        bool getCheckedSelBgImageName(string &checked_selbgimagename);


        //! Check if the pressed imagepath for background is set. This path will be used for the checked, unselected widget.
        bool isCheckedBgImagePath_p();

        //! Mark the pressed checked_bgimagepath as not set.
        void unsetCheckedBgImagePath_p();

        //! Set the pressed imagepath for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath_p  path to pressed & unselected background image
        */
        void setCheckedBgImagePath_p(const string &checked_bgimagepath_p);

        //! Get the pressed imagepath for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath_p  path to pressed & unselected background image
        \return true if set
        */
        bool getCheckedBgImagePath_p(string &checked_bgimagepath_p);

        //! Check if the pressed imagename for background is set. This name will be used for the checked, unselected widget.
        bool isCheckedBgImageName_p();

        //! Mark the pressed checked_bgimagename as not set.
        void unsetCheckedBgImageName_p();

        //! Set the pressed imagename for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename_p  name of the pressed & unselected background image
        */
        void setCheckedBgImageName_p(const string &checked_bgimagename_p);

        //! Get the pressed imagename for background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename_p  name of the pressed & unselected background image
        \return true if set
        */
        bool getCheckedBgImageName_p(string &checked_bgimagename_p);

        //! Check if the pressed selimagepath for background is set. This path will be used for the checked, selected widget.
        bool isCheckedSelBgImagePath_p();

        //! Mark the pressed checked_selbgimagepath as not set.
        void unsetCheckedSelBgImagePath_p();

        //! Set the pressed selimagepath for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath_p  path to pressed & selected background image
        */
        void setCheckedSelBgImagePath_p(const string &checked_selbgimagepath_p);

        //! Get the pressed selimagepath for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath_p  path to the pressed & selected background image
        \return true if set
        */
        bool getCheckedSelBgImagePath_p(string &checked_selbgimagepath_p);

        //! Check if the pressed selimagename for background is set. This name will be used for the checked, selected widget.
        bool isCheckedSelBgImageName_p();

        //! Mark the pressed checked_selbgimagename as not set.
        void unsetCheckedSelBgImageName_p();

        //! Set the pressed selimagename for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename_p  name of the pressed & selected background image
        */
        void setCheckedSelBgImageName_p(const string &checked_selbgimagename_p);

        //! Get the pressed selimagename for background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename_p  name of the pressed & selected background image
        \return true if set
        */
        bool getCheckedSelBgImageName_p(string &checked_selbgimagename_p);

        //! Check if the inactive imagepath for background is set. This path will be used for the checked, unselected widget.
        bool isCheckedBgImagePath_i();

        //! Mark the inactive checked_bgimagepath as not set.
        void unsetCheckedBgImagePath_i();

        //! Set the imagepath for inactive background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath_i  path to inactive unselected background image
        */
        void setCheckedBgImagePath_i(const string &checked_bgimagepath_i);

        //! Get the imagepath for inactive background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagepath_i  path to the inactive unselected background image
        \return true if set
        */
        bool getCheckedBgImagePath_i(string &checked_bgimagepath_i);

        //! Check if the imagename for inactive background is set. This name will be used for the checked, unselected widget.
        bool isCheckedBgImageName_i();

        //! Mark the inactive checked_bgimagename as not set.
        void unsetCheckedBgImageName_i();

        //! Set the imagename for inactive background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename_i  name of the inactive unselected background image
        */
        void setCheckedBgImageName_i(const string &checked_bgimagename_i);

        //! Get the imagename for inactive background which is used to draw the checked, unselected widget.
        /*!
        \param checked_bgimagename_i  name of the inactive unselected background image
        \return true if set
        */
        bool getCheckedBgImageName_i(string &checked_bgimagename_i);

        //! Check if the selimagepath for inactive background is set. This path will be used for the checked, selected widget.
        bool isCheckedSelBgImagePath_i();

        //! Mark the inactive checked_selbgimagepath as not set.
        void unsetCheckedSelBgImagePath_i();

        //! Set the selimagepath for inactive background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath_i  path to inactive selected background image
        */
        void setCheckedSelBgImagePath_i(const string &checked_selbgimagepath_i);

        //! Get the selimagepath for inactive background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagepath_i  path to the inactive selected background image
        \return true if set
        */
        bool getCheckedSelBgImagePath_i(string &checked_selbgimagepath_i);

        //! Check if the selimagename for inactive background is set. This name will be used for the checked, selected widget.
        bool isCheckedSelBgImageName_i();

        //! Mark the inactive checked_selbgimagename as not set.
        void unsetCheckedSelBgImageName_i();

        //! Set the selimagename for inactive background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename  name of the inactive selected background image
        */
        void setCheckedSelBgImageName_i(const string &checked_selbgimagename_i);

        //! Get the selimagename for inactive background which is used to draw the checked, selected widget.
        /*!
        \param checked_selbgimagename_i  name of the inactive selected background image
        \return true if set
        */
        bool getCheckedSelBgImageName_i(string &checked_selbgimagename_i);

        //! Check if the checked flag is set.
        bool isChecked();

        //! Mark the checked flag as not set.
        void unsetChecked();

        //! Set the checked flag.
        /*!
        \param checked  the widget will be displayed as checked if set to true
        */
        void setChecked(bool checked);

        //! Get the checked flag.
        /*!
        \param checked  checked true or false
        \return true if set
        */
        bool getChecked(bool &checked);



    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSCHECKBOXWIDGETCLASS_H_*/
