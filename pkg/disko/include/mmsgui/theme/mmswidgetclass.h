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

#ifndef MMSWIDGETCLASS_H_
#define MMSWIDGETCLASS_H_

#include "mmsgui/theme/mmsborderclass.h"

//! describe attributes for MMSWidget which are additional to the MMSBorderClass
namespace MMSGUI_WIDGET_ATTR {

	#define MMSGUI_WIDGET_ATTR_ATTRDESC \
	{ "bgcolor", TAFF_ATTRTYPE_COLOR }, \
	{ "bgcolor.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor", TAFF_ATTRTYPE_COLOR }, \
	{ "selbgcolor.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_p", TAFF_ATTRTYPE_COLOR }, \
	{ "bgcolor_p.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_p.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_p.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_p.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_p", TAFF_ATTRTYPE_COLOR }, \
	{ "selbgcolor_p.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_p.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_p.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_p.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_i", TAFF_ATTRTYPE_COLOR }, \
	{ "bgcolor_i.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_i.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_i.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgcolor_i.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_i", TAFF_ATTRTYPE_COLOR }, \
	{ "selbgcolor_i.a", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_i.r", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_i.g", TAFF_ATTRTYPE_UCHAR }, \
	{ "selbgcolor_i.b", TAFF_ATTRTYPE_UCHAR }, \
	{ "bgimage", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage.path", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage.name", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage.path", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage.name", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_p", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_p.path", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_p.name", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_p", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_p.path", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_p.name", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_i", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_i.path", TAFF_ATTRTYPE_STRING }, \
	{ "bgimage_i.name", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_i", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_i.path", TAFF_ATTRTYPE_STRING }, \
	{ "selbgimage_i.name", TAFF_ATTRTYPE_STRING }, \
	{ "margin", TAFF_ATTRTYPE_UCHAR }, \
	{ "focusable", TAFF_ATTRTYPE_BOOL }, \
	{ "selectable", TAFF_ATTRTYPE_BOOL }, \
	{ "up_arrow", TAFF_ATTRTYPE_STRING }, \
	{ "down_arrow", TAFF_ATTRTYPE_STRING }, \
	{ "left_arrow", TAFF_ATTRTYPE_STRING }, \
	{ "right_arrow", TAFF_ATTRTYPE_STRING }, \
	{ "data", TAFF_ATTRTYPE_STRING }, \
	{ "navigate_up", TAFF_ATTRTYPE_STRING }, \
	{ "navigate_down", TAFF_ATTRTYPE_STRING }, \
	{ "navigate_left", TAFF_ATTRTYPE_STRING }, \
	{ "navigate_right", TAFF_ATTRTYPE_STRING }, \
	{ "vslider", TAFF_ATTRTYPE_STRING }, \
	{ "hslider", TAFF_ATTRTYPE_STRING }, \
	{ "imagesondemand", TAFF_ATTRTYPE_BOOL }, \
	{ "blend", TAFF_ATTRTYPE_UCHAR }, \
	{ "blend_factor", TAFF_ATTRTYPE_STRING }, \
	{ "scroll_onfocus", TAFF_ATTRTYPE_BOOL }, \
	{ "clickable", TAFF_ATTRTYPE_BOOL }, \
	{ "return_onscroll", TAFF_ATTRTYPE_BOOL }, \
	{ "inputmode", TAFF_ATTRTYPE_STRING }, \
	{ "joined_widget", TAFF_ATTRTYPE_STRING }, \
	{ "activated", TAFF_ATTRTYPE_BOOL }


	#define MMSGUI_WIDGET_ATTR_IDS \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_a, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_r, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_g, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_b, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_a, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_r, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_g, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_b, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_p, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_a, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_r, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_g, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_p_b, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_a, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_r, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_g, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_p_b, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_i, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_a, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_r, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_g, \
		MMSGUI_WIDGET_ATTR_IDS_bgcolor_i_b, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_a, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_r, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_g, \
		MMSGUI_WIDGET_ATTR_IDS_selbgcolor_i_b, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_path, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_name, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_path, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_name, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_p, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_p_path, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_p_name, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_p, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_p_path, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_p_name, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_i, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_i_path, \
		MMSGUI_WIDGET_ATTR_IDS_bgimage_i_name, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_i, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_i_path, \
		MMSGUI_WIDGET_ATTR_IDS_selbgimage_i_name, \
		MMSGUI_WIDGET_ATTR_IDS_margin, \
		MMSGUI_WIDGET_ATTR_IDS_focusable, \
		MMSGUI_WIDGET_ATTR_IDS_selectable, \
		MMSGUI_WIDGET_ATTR_IDS_up_arrow, \
		MMSGUI_WIDGET_ATTR_IDS_down_arrow, \
		MMSGUI_WIDGET_ATTR_IDS_left_arrow, \
		MMSGUI_WIDGET_ATTR_IDS_right_arrow, \
		MMSGUI_WIDGET_ATTR_IDS_data, \
		MMSGUI_WIDGET_ATTR_IDS_navigate_up, \
		MMSGUI_WIDGET_ATTR_IDS_navigate_down, \
		MMSGUI_WIDGET_ATTR_IDS_navigate_left, \
		MMSGUI_WIDGET_ATTR_IDS_navigate_right, \
		MMSGUI_WIDGET_ATTR_IDS_vslider, \
		MMSGUI_WIDGET_ATTR_IDS_hslider, \
		MMSGUI_WIDGET_ATTR_IDS_imagesondemand, \
		MMSGUI_WIDGET_ATTR_IDS_blend, \
		MMSGUI_WIDGET_ATTR_IDS_blend_factor, \
		MMSGUI_WIDGET_ATTR_IDS_scroll_onfocus, \
		MMSGUI_WIDGET_ATTR_IDS_clickable, \
		MMSGUI_WIDGET_ATTR_IDS_return_onscroll, \
		MMSGUI_WIDGET_ATTR_IDS_inputmode, \
		MMSGUI_WIDGET_ATTR_IDS_joined_widget, \
		MMSGUI_WIDGET_ATTR_IDS_activated

	#define MMSGUI_WIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS
	} ids;
}



//! A data access class as base for all widgets.
/*!
This class is the base for all widget classes.
With this data store you have access to all changeable widget attributes
used for all widgets.
\note This class will be internally used by class MMSWidget.
\author Jens Schneider
*/
class MMSWidgetClass {
    private:
    	struct {
    		//! is bgcolor set?
	    	bool            isbgcolor;

	    	//! background color if the widget is not selected
	        MMSFBColor      bgcolor;

	        //! is selbgcolor set?
	        bool            isselbgcolor;

	        //! background color if the widget is selected
	        MMSFBColor      selbgcolor;

	        //! is pressed bgcolor set?
	    	bool            isbgcolor_p;

	    	//! pressed background color if the widget is not selected
	        MMSFBColor      bgcolor_p;

	        //! is pressed selbgcolor set?
	        bool            isselbgcolor_p;

	        //! pressed background color if the widget is selected
	        MMSFBColor      selbgcolor_p;

	        //! is inactive bgcolor set?
	        bool            isbgcolor_i;

	        //! inactive background color if the widget is not selected
	        MMSFBColor      bgcolor_i;

	        //! is inactive selbgcolor set?
	        bool            isselbgcolor_i;

	        //! inactive background color if the widget is selected
	        MMSFBColor      selbgcolor_i;

	        //! is bgimagepath set?
	        bool            isbgimagepath;

	        //! is bgimagename set?
	        bool            isbgimagename;

	        //! is selbgimagepath set?
	        bool            isselbgimagepath;

	        //! is selbgimagename set?
	        bool            isselbgimagename;

	        //! is pressed bgimagepath set?
	        bool            isbgimagepath_p;

	        //! is pressed bgimagename set?
	        bool            isbgimagename_p;

	        //! is pressed selbgimagepath set?
	        bool            isselbgimagepath_p;

	        //! is pressed selbgimagename set?
	        bool            isselbgimagename_p;

	        //! is inactive bgimagepath set?
	        bool            isbgimagepath_i;

	        //! is inactive bgimagename set?
	        bool            isbgimagename_i;

	        //! is inactive selbgimagepath set?
	        bool            isselbgimagepath_i;

	        //! is inactive selbgimagename set?
	        bool            isselbgimagename_i;

	        //! is margin set?
	        bool            ismargin;

	        //! margin in pixel
	        unsigned int    margin;

	        //! is the focusable flag set?
	        bool            isfocusable;

	        //! widget can get the focus true/false
	        bool            focusable;

	        //! is the selectable flag set?
	        bool            isselectable;

	        //! widget can be selected true/false
	        bool            selectable;

	        //! is the uparrow set?
	        bool            isuparrow;

	        //! is the downarrow set?
	        bool            isdownarrow;

	        //! is the leftarrow set?
	        bool            isleftarrow;

	        //! is the rightarrow set?
	        bool            isrightarrow;

	        //! is the data value set?
	        bool            isdata;

	        //! is the navigateup set?
	        bool            isnavigateup;

	        //! is the navigatedown set?
	        bool            isnavigatedown;

	        //! is the navigateleft set?
	        bool            isnavigateleft;

	        //! is the navigateright set?
	        bool            isnavigateright;

	        //! is the vslider set?
	        bool            isvslider;

	        //! is the hslider set?
	        bool            ishslider;

	        //! is images on demand flag set?
	        bool            isimagesondemand;

	        //! use images on demand (true/false)
	        bool            imagesondemand;

	        //! is blend set?
	        bool            isblend;

	        //! blend 0..255, default 0
	        unsigned int    blend;

	        //! is blend factor set?
	        bool            isblendfactor;

	        //! blend factor 0.0.., default 0.0
	        double          blendfactor;

	        //! is scroll on focus flag set?
	        bool            isscrollonfocus;

	        //! use scroll on focus (true/false)
	        bool            scrollonfocus;

	        //! is the clickable flag set?
	        bool            isclickable;

	        //! user can click onto widget true/false
	        bool            clickable;

	        //! is the returnonscroll flag set?
	        bool            isreturnonscroll;

	        //! emit on return callback (true/false) if user changes the selection e.g. in a menu
	        bool            returnonscroll;

	        //! is inputmode set?
	        bool            isinputmode;

	        //! is joined_widget set?
	        bool            isjoinedwidget;

	        //! is the activated flag set?
	        bool            isactivated;

	        //! widget is active / inactive
	        bool            activated;


    	} id;

    	struct {
    		//! path to the background image if the widget is not selected
            string          *bgimagepath;

            //! background image filename if the widget is not selected
            string          *bgimagename;

            //! path to the background image if the widget is selected
            string          *selbgimagepath;

            //! background image filename if the widget is selected
            string          *selbgimagename;

            //! path to the pressed background image if the widget is not selected
            string          *bgimagepath_p;

            //! pressed background image filename if the widget is not selected
            string          *bgimagename_p;

            //! path to the pressed background image if the widget is selected
            string          *selbgimagepath_p;

            //! pressed background image filename if the widget is selected
            string          *selbgimagename_p;

            //! path to the inactive background image if the widget is not selected
            string          *bgimagepath_i;

            //! inactive background image filename if the widget is not selected
            string          *bgimagename_i;

            //! path to the inactive background image if the widget is selected
            string          *selbgimagepath_i;

            //! inactive background image filename if the widget is selected
            string          *selbgimagename_i;

            //! the name of the widget which represents the scroll up arrow
            string          *uparrow;

            //! the name of the widget which represents the scroll down arrow
            string          *downarrow;

            //! the name of the widget which represents the scroll left arrow
            string          *leftarrow;

            //! the name of the widget which represents the scroll right arrow
            string          *rightarrow;

            //! any string which can store additional information (will not displayed)
            string          *data;

            //! the name of the widget to which should navigate up
            string          *navigateup;

            //! the name of the widget to which should navigate down
            string          *navigatedown;

            //! the name of the widget to which should navigate left
            string          *navigateleft;

            //! the name of the widget to which should navigate right
            string          *navigateright;

            //! the name of the widget which represents the vertical slider
            string          *vslider;

            //! the name of the widget which represents the horizontal slider
            string          *hslider;

    		//! input mode
            string          *inputmode;

            //! name of widget which is joined to this widget
            string          *joinedwidget;
    	} ed;

    	/* init routines */
        void initBgColor();
        void initSelBgColor();
        void initBgColor_p();
        void initSelBgColor_p();
        void initBgColor_i();
        void initSelBgColor_i();

        void initBgImagePath();
        void initBgImageName();
        void initSelBgImagePath();
        void initSelBgImageName();
        void initBgImagePath_p();
        void initBgImageName_p();
        void initSelBgImagePath_p();
        void initSelBgImageName_p();
        void initBgImagePath_i();
        void initBgImageName_i();
        void initSelBgImagePath_i();
        void initSelBgImageName_i();

        void initMargin();
        void initFocusable();
        void initSelectable();

        void initUpArrow();
        void initDownArrow();
        void initLeftArrow();
        void initRightArrow();

        void initData();

        void initNavigateUp();
        void initNavigateDown();
        void initNavigateLeft();
        void initNavigateRight();

        void initVSlider();
        void initHSlider();

        void initImagesOnDemand();

        void initBlend();
        void initBlendFactor();

        void initScrollOnFocus();
        void initClickable();
        void initReturnOnScroll();

        void initInputMode();
        void initJoinedWidget();

        void initActivated();


        /* free routines */
        void freeBgColor();
        void freeSelBgColor();
        void freeBgColor_p();
        void freeSelBgColor_p();
        void freeBgColor_i();
        void freeSelBgColor_i();

        void freeBgImagePath();
        void freeBgImageName();
        void freeSelBgImagePath();
        void freeSelBgImageName();
        void freeBgImagePath_p();
        void freeBgImageName_p();
        void freeSelBgImagePath_p();
        void freeSelBgImageName_p();
        void freeBgImagePath_i();
        void freeBgImageName_i();
        void freeSelBgImagePath_i();
        void freeSelBgImageName_i();

        void freeMargin();
        void freeFocusable();
        void freeSelectable();

        void freeUpArrow();
        void freeDownArrow();
        void freeLeftArrow();
        void freeRightArrow();

        void freeData();

        void freeNavigateUp();
        void freeNavigateDown();
        void freeNavigateLeft();
        void freeNavigateRight();

        void freeVSlider();
        void freeHSlider();

        void freeImagesOnDemand();

        void freeBlend();
        void freeBlendFactor();

        void freeScrollOnFocus();
        void freeClickable();
        void freeReturnOnScroll();

        void freeInputMode();
        void freeJoinedWidget();

        void freeActivated();

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
    	//! stores base border attributes
        MMSBorderClass border;

        //! Constructor of class MMSWidgetClass.
        MMSWidgetClass();

        //! Destructor of class MMSWidgetClass.
        ~MMSWidgetClass();

        //! operator=
        MMSWidgetClass &operator=(const MMSWidgetClass &c);

        //! Mark all attributes as not set.
        void unsetAll();

        //! Check if the background color is set. This color will be used for the unselected widget.
        bool isBgColor();

        //! Mark the bgcolor as not set.
        void unsetBgColor();

        //! Set the background color which is used to draw the unselected widget.
        /*!
        \param bgcolor  color for unselected background
        */
        void setBgColor(const MMSFBColor &bgcolor);

        //! Get the background color which is used to draw the unselected widget.
        /*!
        \param bgcolor  background color
        \return true if set
        */
        bool getBgColor(MMSFBColor &bgcolor);

        //! Check if the background color is set. This color will be used for the selected widget.
        bool isSelBgColor();

        //! Mark the selbgcolor as not set.
        void unsetSelBgColor();

        //! Set the background color which is used to draw the selected widget.
        /*!
        \param selbgcolor  color for selected background
        */
        void setSelBgColor(const MMSFBColor &selbgcolor);

        //! Get the background color which is used to draw the selected widget.
        /*!
        \param selbgcolor  background color
        \return true if set
        */
        bool getSelBgColor(MMSFBColor &selbgcolor);

        //! Check if the pressed background color is set. This color will be used for the unselected widget.
        bool isBgColor_p();

        //! Mark the pressed bgcolor as not set.
        void unsetBgColor_p();

        //! Set the pressed background color which is used to draw the unselected widget.
        /*!
        \param bgcolor_p  pressed background color
        */
        void setBgColor_p(const MMSFBColor &bgcolor_p);

        //! Get the pressed background color which is used to draw the unselected widget.
        /*!
        \param bgcolor_p  pressed background color
        \return true if set
        */
        bool getBgColor_p(MMSFBColor &bgcolor_p);

        //! Check if the pressed background color is set. This color will be used for the selected widget.
        bool isSelBgColor_p();

        //! Mark the pressed selbgcolor as not set.
        void unsetSelBgColor_p();

        //! Set the pressed background color which is used to draw the selected widget.
        /*!
        \param selbgcolor_p  pressed color for selected background
        */
        void setSelBgColor_p(const MMSFBColor &selbgcolor_p);

        //! Get the pressed background color which is used to draw the selected widget.
        /*!
        \param selbgcolor_p  pressed background color
        \return true if set
        */
        bool getSelBgColor_p(MMSFBColor &selbgcolor_p);

        //! Check if the inactive background color is set. This color will be used for the unselected widget.
        bool isBgColor_i();

        //! Mark the inactive bgcolor as not set.
        void unsetBgColor_i();

        //! Set the inactive background color which is used to draw the unselected widget.
        /*!
        \param bgcolor_i  color for inactive unselected background
        */
        void setBgColor_i(const MMSFBColor &bgcolor_i);

        //! Get the inactive background color which is used to draw the unselected widget.
        /*!
        \param bgcolor_i  inactive background color
        \return true if set
        */
        bool getBgColor_i(MMSFBColor &bgcolor_i);

        //! Check if the inactive background color is set. This color will be used for the selected widget.
        bool isSelBgColor_i();

        //! Mark the inactive selbgcolor as not set.
        void unsetSelBgColor_i();

        //! Set the inactive background color which is used to draw the selected widget.
        /*!
        \param selbgcolor_i  color for inactive selected background
        */
        void setSelBgColor_i(const MMSFBColor &selbgcolor_i);

        //! Get the inactive background color which is used to draw the selected widget.
        /*!
        \param selbgcolor_i  inactive background color
        \return true if set
        */
        bool getSelBgColor_i(MMSFBColor &selbgcolor_i);

        //! Check if the imagepath for background is set. This path will be used for the unselected widget.
        bool isBgImagePath();

        //! Mark the bgimagepath as not set.
        void unsetBgImagePath();

        //! Set the imagepath for background which is used to draw the unselected widget.
        /*!
        \param bgimagepath  path to unselected background image
        */
        void setBgImagePath(const string &bgimagepath);

        //! Get the imagepath for background which is used to draw the unselected widget.
        /*!
        \param bgimagepath  path to unselected background image
        \return true if set
        */
        bool getBgImagePath(string &bgimagepath);

        //! Check if the imagename for background is set. This name will be used for the unselected widget.
        bool isBgImageName();

        //! Mark the bgimagename as not set.
        void unsetBgImageName();

        //! Set the imagename for background which is used to draw the unselected widget.
        /*!
        \param bgimagename  name of the unselected background image
        */
        void setBgImageName(const string &bgimagename);

        //! Get the imagename for background which is used to draw the unselected widget.
        /*!
        \param bgimagename  name of the unselected background image
        \return true if set
        */
        bool getBgImageName(string &bgimagename);

        //! Check if the selimagepath for background is set. This path will be used for the selected widget.
        bool isSelBgImagePath();

        //! Mark the selbgimagepath as not set.
        void unsetSelBgImagePath();

        //! Set the selimagepath for background which is used to draw the selected widget.
        /*!
        \param selbgimagepath  path to selected background image
        */
        void setSelBgImagePath(const string &selbgimagepath);

        //! Get the selimagepath for background which is used to draw the selected widget.
        /*!
        \param selbgimagepath  path to the selected background image
        \return true if set
        */
        bool getSelBgImagePath(string &selbgimagepath);

        //! Check if the selimagename for background is set. This name will be used for the selected widget.
        bool isSelBgImageName();

        //! Mark the selbgimagename as not set.
        void unsetSelBgImageName();

        //! Set the selimagename for background which is used to draw the selected widget.
        /*!
        \param selbgimagename  name of the selected background image
        */
        void setSelBgImageName(const string &selbgimagename);

        //! Get the selimagename for background which is used to draw the selected widget.
        /*!
        \param selbgimagename  name of the selected background image
        \return true if set
        */
        bool getSelBgImageName(string &selbgimagename);


        //! Check if the pressed imagepath for background is set. This path will be used for the unselected widget.
        bool isBgImagePath_p();

        //! Mark the pressed bgimagepath as not set.
        void unsetBgImagePath_p();

        //! Set the pressed imagepath for background which is used to draw the unselected widget.
        /*!
        \param bgimagepath_p  path to pressed & unselected background image
        */
        void setBgImagePath_p(const string &bgimagepath_p);

        //! Get the pressed imagepath for background which is used to draw the unselected widget.
        /*!
        \param bgimagepath_p  path to pressed & unselected background image
        \return true if set
        */
        bool getBgImagePath_p(string &bgimagepath_p);

        //! Check if the pressed imagename for background is set. This name will be used for the unselected widget.
        bool isBgImageName_p();

        //! Mark the pressed bgimagename as not set.
        void unsetBgImageName_p();

        //! Set the pressed imagename for background which is used to draw the unselected widget.
        /*!
        \param bgimagename_p  name of the pressed & unselected background image
        */
        void setBgImageName_p(const string &bgimagename_p);

        //! Get the pressed imagename for background which is used to draw the unselected widget.
        /*!
        \param bgimagename_p  name of the pressed & unselected background image
        \return true if set
        */
        bool getBgImageName_p(string &bgimagename_p);

        //! Check if the pressed selimagepath for background is set. This path will be used for the selected widget.
        bool isSelBgImagePath_p();

        //! Mark the pressed selbgimagepath as not set.
        void unsetSelBgImagePath_p();

        //! Set the pressed selimagepath for background which is used to draw the selected widget.
        /*!
        \param selbgimagepath_p  path to pressed & selected background image
        */
        void setSelBgImagePath_p(const string &selbgimagepath_p);

        //! Get the pressed selimagepath for background which is used to draw the selected widget.
        /*!
        \param selbgimagepath_p  path to the pressed & selected background image
        \return true if set
        */
        bool getSelBgImagePath_p(string &selbgimagepath_p);

        //! Check if the pressed selimagename for background is set. This name will be used for the selected widget.
        bool isSelBgImageName_p();

        //! Mark the pressed selbgimagename as not set.
        void unsetSelBgImageName_p();

        //! Set the pressed selimagename for background which is used to draw the selected widget.
        /*!
        \param selbgimagename_p  name of the pressed & selected background image
        */
        void setSelBgImageName_p(const string &selbgimagename_p);

        //! Get the pressed selimagename for background which is used to draw the selected widget.
        /*!
        \param selbgimagename_p  name of the pressed & selected background image
        \return true if set
        */
        bool getSelBgImageName_p(string &selbgimagename_p);

        //! Check if the inactive imagepath for background is set. This path will be used for the unselected widget.
        bool isBgImagePath_i();

        //! Mark the inactive bgimagepath as not set.
        void unsetBgImagePath_i();

        //! Set the imagepath for inactive background which is used to draw the unselected widget.
        /*!
        \param bgimagepath_i  path to inactive unselected background image
        */
        void setBgImagePath_i(const string &bgimagepath_i);

        //! Get the imagepath for inactive background which is used to draw the unselected widget.
        /*!
        \param bgimagepath_i  path to the inactive unselected background image
        \return true if set
        */
        bool getBgImagePath_i(string &bgimagepath_i);

        //! Check if the imagename for inactive background is set. This name will be used for the unselected widget.
        bool isBgImageName_i();

        //! Mark the inactive bgimagename as not set.
        void unsetBgImageName_i();

        //! Set the imagename for inactive background which is used to draw the unselected widget.
        /*!
        \param bgimagename_i  name of the inactive unselected background image
        */
        void setBgImageName_i(const string &bgimagename_i);

        //! Get the imagename for inactive background which is used to draw the unselected widget.
        /*!
        \param bgimagename_i  name of the inactive unselected background image
        \return true if set
        */
        bool getBgImageName_i(string &bgimagename_i);

        //! Check if the selimagepath for inactive background is set. This path will be used for the selected widget.
        bool isSelBgImagePath_i();

        //! Mark the inactive selbgimagepath as not set.
        void unsetSelBgImagePath_i();

        //! Set the selimagepath for inactive background which is used to draw the selected widget.
        /*!
        \param selbgimagepath_i  path to inactive selected background image
        */
        void setSelBgImagePath_i(const string &selbgimagepath_i);

        //! Get the selimagepath for inactive background which is used to draw the selected widget.
        /*!
        \param selbgimagepath_i  path to the inactive selected background image
        \return true if set
        */
        bool getSelBgImagePath_i(string &selbgimagepath_i);

        //! Check if the selimagename for inactive background is set. This name will be used for the selected widget.
        bool isSelBgImageName_i();

        //! Mark the inactive selbgimagename as not set.
        void unsetSelBgImageName_i();

        //! Set the selimagename for inactive background which is used to draw the selected widget.
        /*!
        \param selbgimagename  name of the inactive selected background image
        */
        void setSelBgImageName_i(const string &selbgimagename_i);

        //! Get the selimagename for inactive background which is used to draw the selected widget.
        /*!
        \param selbgimagename_i  name of the inactive selected background image
        \return true if set
        */
        bool getSelBgImageName_i(string &selbgimagename_i);

        //! Check if the margin is set.
        bool isMargin();

        //! Mark the margin as not set.
        void unsetMargin();

        //! Set the margin.
        /*!
        \param margin  margin in pixel
        */
        void setMargin(unsigned int margin);

        //! Get the margin.
        /*!
        \param margin  margin
        \return true if set
        */
        bool getMargin(unsigned int &margin);

        //! Check if the focusable flag is set.
        bool isFocusable();

        //! Mark the focusable flag as not set.
        void unsetFocusable();

        //! Set the focusable flag.
        /*!
        \param focusable  the widget can get the focus if set to true
        \note There is a difference between focused and selected. Only one widget
              can get the focus at the same time. The focused widget gets the keyboard input.
              A focused widget is also selected.
        */
        void setFocusable(bool focusable);

        //! Get the focusable flag.
        /*!
        \param focusable  focusable true or false
        \return true if set
        */
        bool getFocusable(bool &focusable);

        //! Check if the selectable flag is set.
        bool isSelectable();

        //! Mark the selectable flag as not set.
        void unsetSelectable();

        //! Set the selectable flag.
        /*!
        \param selectable  the widget can be selected if set to true
        \note There is a difference between focused and selected. Only one widget
              can get the focus at the same time. But all other widgets can be switched
              between selected and unselected independently.
        */
        void setSelectable(bool selectable);

        //! Get the selectable flag.
        /*!
        \param selectable  selectable true or false
        \return true if set
        */
        bool getSelectable(bool &selectable);

        //! Check if the uparrow is set.
        bool isUpArrow();

        //! Mark the uparrow as not set.
        void unsetUpArrow();

        //! Set the uparrow.
        /*!
        \param uparrow  the name of the widget which represents the scroll up arrow
        */
        void setUpArrow(const string &uparrow);

        //! Get the uparrow.
        /*!
        \param uparrow  name of the up arrow widget
        \return true if set
        */
        bool getUpArrow(string &uparrow);

        //! Check if the downarrow is set.
        bool isDownArrow();

		//! Mark the downarrow as not set.
		void unsetDownArrow();

        //! Set the downarrow.
        /*!
        \param downarrow  the name of the widget which represents the scroll down arrow
        */
        void setDownArrow(const string &downarrow);

        //! Get the downarrow.
        /*!
        \param downarrow  name of the down arrow widget
        \return true if set
        */
        bool getDownArrow(string &downarrow);

        //! Check if the leftarrow is set.
        bool isLeftArrow();

        //! Mark the leftarrow as not set.
        void unsetLeftArrow();

        //! Set the leftarrow.
        /*!
        \param leftarrow  the name of the widget which represents the scroll left arrow
        */
        void setLeftArrow(const string &leftarrow);

        //! Get the leftarrow.
        /*!
        \param leftarrow  name of the left arrow widget
        \return true if set
        */
        bool getLeftArrow(string &leftarrow);

        //! Check if the rightarrow is set.
        bool isRightArrow();

        //! Mark the rightarrow as not set.
        void unsetRightArrow();

        //! Set the rightarrow.
        /*!
        \param rightarrow  the name of the widget which represents the scroll right arrow
        */
        void setRightArrow(const string &rightarrow);


        //! Get the rightarrow.
        /*!
        \param rightarrow  name of the right arrow widget
        \return true if set
        */
        bool getRightArrow(string &rightarrow);

        //! Check if the data is set.
        bool isData();

        //! Mark the data as not set.
        void unsetData();

        //! Set the additional data value.
        /*!
        \param data  any string which can store additional information (will not displayed)
        */
        void setData(const string &data);

        //! Get the data.
        /*!
        \param data  additional data string
        \return true if set
        */
        bool getData(string &data);

        //! Check if the navigateup is set.
        bool isNavigateUp();

        //! Mark the navigateup as not set.
        void unsetNavigateUp();

        //! Set the navigateup widget.
        /*!
        \param navigateup  the name of the widget to which should navigate up
        */
        void setNavigateUp(const string &navigateup);

        //! Get the navigateup widget.
        /*!
        \param navigateup name of the navigate up widget
        \return true if set
        */
        bool getNavigateUp(string &navigateup);

        //! Check if the navigatedown is set.
        bool isNavigateDown();

        //! Mark the navigatedown as not set.
        void unsetNavigateDown();

        //! Set the navigatedown widget.
        /*!
        \param navigatedown  the name of the widget to which should navigate down
        */
        void setNavigateDown(const string &navigatedown);

        //! Get the navigatedown widget.
        /*!
        \param navigatedown  name of the navigate down widget
        \return true if set
        */
        bool getNavigateDown(string &navigatedown);

        //! Check if the navigateleft is set.
        bool isNavigateLeft();

        //! Mark the navigateleft as not set.
        void unsetNavigateLeft();

        //! Set the navigateleft widget.
        /*!
        \param navigateleft  the name of the widget to which should navigate left
        */
        void setNavigateLeft(const string &navigateleft);

        //! Get the navigateleft widget.
        /*!
        \param navigateleft  name of the navigate left widget
        \return true if set
        */
        bool getNavigateLeft(string &navigateleft);

        //! Check if the navigateright is set.
        bool isNavigateRight();

        //! Mark the navigateright as not set.
        void unsetNavigateRight();

        //! Set the navigateright.
        /*!
        \param navigateright  the name of the widget to which should navigate right
        */
        void setNavigateRight(const string &navigateright);

        //! Get the navigateright widget.
        /*!
        \param navigateright  name of the navigate right widget
        \return true if set
        */
        bool getNavigateRight(string &navigateright);

        //! Check if the vslider is set.
        bool isVSlider();

        //! Mark the vslider as not set.
        void unsetVSlider();

        //! Set the vslider.
        /*!
        \param vslider  the name of the widget which represents the vertical slider
        */
        void setVSlider(const string &vslider);

        //! Get the vslider.
        /*!
        \param vslider  name of the vslider widget
        \return true if set
        */
        bool getVSlider(string &vslider);

        //! Check if the hslider is set.
        bool isHSlider();

        //! Mark the hslider as not set.
        void unsetHSlider();

        //! Set the hslider.
        /*!
        \param hslider  the name of the widget which represents the horizontal slider
        */
        void setHSlider(const string &hslider);

        //! Get the hslider.
        /*!
        \param hslider  name of the hslider widget
        \return true if set
        */
        bool getHSlider(string &hslider);

        //! Check if the images on demand is set.
        bool isImagesOnDemand();

        //! Mark the images on demand flag as not set.
        void unsetImagesOnDemand();

        //! Set the images on demand flag.
        /*!
        \param imagesondemand  use imagesondemand true/false
        */
        void setImagesOnDemand(bool imagesondemand);

        //! Get the images on demand flag.
        /*!
        \return images on demand flag
        \return true if set
        */
        bool getImagesOnDemand(bool &imagesondemand);

        //! Check if the blend is set.
        bool isBlend();

        //! Mark the blend as not set.
        void unsetBlend();

        //! Set the blend.
        /*!
        \param blend  blend with values 0..255
        */
        void setBlend(unsigned int blend);

        //! Get the blend.
        /*!
        \param blend  blend
        \return true if set
        */
        bool getBlend(unsigned int &blend);

        //! Check if the blend factor is set.
        bool isBlendFactor();

        //! Mark the blend factor as not set.
        void unsetBlendFactor();

        //! Set the blend factor.
        /*!
        \param blendfactor  blend factor with values 0.0..
        */
        void setBlendFactor(double blendfactor);

        //! Get the blend factor.
        /*!
        \param blendfactor  blend factor
        \return true if set
        */
        bool getBlendFactor(double &blendfactor);


        //! Check if the scroll on focus is set.
        bool isScrollOnFocus();

        //! Mark the scroll on focus flag as not set.
        void unsetScrollOnFocus();

        //! Set the scroll on focus flag.
        /*!
        \param scrollonfocus  use scrollonfocus true/false
        */
        void setScrollOnFocus(bool scrollonfocus);

        //! Get the scroll on focus flag.
        /*!
        \return scroll on focus flag
        \return true if set
        */
        bool getScrollOnFocus(bool &scrollonfocus);

        //! Check if the clickable flag is set.
        bool isClickable();

        //! Mark the clickable flag as not set.
        void unsetClickable();

        //! Set the clickable flag.
        /*!
        \param clickable  user can click onto the widget if set to true
        \note Widgets which are not focusable can be clickable anyway. See for example the MMSArrowWidget.
        */
        void setClickable(bool clickable);

        //! Get the clickable flag.
        /*!
        \param clickable  clickable true or false
        \return true if set
        */
        bool getClickable(bool &clickable);

        //! Check if the returnonscroll flag is set.
        bool isReturnOnScroll();

        //! Mark the returnonscroll flag as not set.
        void unsetReturnOnScroll();

        //! Set the returnonscroll flag.
        /*!
        \param returnonscroll  if true emit on return callback if user changes the selection e.g. in a menu
        */
        void setReturnOnScroll(bool returnonscroll);

        //! Get the returnonscroll flag.
        /*!
        \param returnonscroll  returnonscroll true or false
        \return true if set
        */
        bool getReturnOnScroll(bool &returnonscroll);


        //! Check if the input mode is set.
        bool isInputMode();

        //! Mark the input mode as not set.
        void unsetInputMode();

        //! Set the input mode.
        /*!
        \param inputmode  input mode ("move" or "click")
        */
        void setInputMode(const string &inputmode);

        //! Get the input mode.
        /*!
        \param inputmode  input mode
        \return true if set
        */
        bool getInputMode(string &inputmode);


        //! Check if the joined widget is set.
        bool isJoinedWidget();

        //! Mark the joined widget as not set.
        void unsetJoinedWidget();

        //! Set the joined widget.
        /*!
        \param joinedwidget  name of widget which is to join
        */
        void setJoinedWidget(const string &joinedwidget);

        //! Get the joined widget.
        /*!
        \param joinedwidget  name of widget
        \return true if set
        */
        bool getJoinedWidget(string &joinedwidget);


        //! Check if the activated flag is set.
        bool isActivated();

        //! Mark the activated flag as not set.
        void unsetActivated();

        //! Set the activated flag.
        /*!
        \param activated  widget is active / inactive
        */
        void setActivated(bool activated);

        //! Get the activated flag.
        /*!
        \param activated  activated true or false
        \return true if set
        */
        bool getActivated(bool &activated);



    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSWIDGETCLASS_H_*/
