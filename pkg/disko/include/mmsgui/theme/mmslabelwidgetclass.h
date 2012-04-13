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

#ifndef MMSLABELWIDGETCLASS_H_
#define MMSLABELWIDGETCLASS_H_

#include "mmsgui/theme/mmstextbaseclass.h"

//! describe attributes for MMSLabelWidget which are additional to the MMSWidgetClass
namespace MMSGUI_LABELWIDGET_ATTR {

	#define MMSGUI_LABELWIDGET_ATTR_ATTRDESC \
		{ "slidable", TAFF_ATTRTYPE_BOOL }, \
		{ "slide_speed", TAFF_ATTRTYPE_UCHAR }, \
		{ "translate", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_LABELWIDGET_ATTR_IDS \
		MMSGUI_LABELWIDGET_ATTR_IDS_slidable, \
		MMSGUI_LABELWIDGET_ATTR_IDS_slide_speed, \
		MMSGUI_LABELWIDGET_ATTR_IDS_translate

	#define MMSGUI_LABELWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_FONT_ATTR_ATTRDESC, \
		MMSGUI_SHADOW_ATTR_ATTRDESC, \
		MMSGUI_TEXTINFO_ATTR_ATTRDESC, \
		MMSGUI_LABELWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_FONT_ATTR_IDS,
		MMSGUI_SHADOW_ATTR_IDS,
		MMSGUI_TEXTINFO_ATTR_IDS,
		MMSGUI_LABELWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_LABELWIDGET_ATTR_I[];


//! A data access class for the label widget.
/*!
This class is the base for the MMSLabelWidget class and is derived from
MMSTextBaseClass which is the base for all widgets with text output.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSLabelWidget.
\author Jens Schneider
*/
class MMSLabelWidgetClass : public MMSTextBaseClass {
    private:
    	//! name of the theme class
        string          className;

        //! is slidable set?
        bool            isslidable;

        //! if true and size of the text string is greater than widget dimension, the text will slide over the widget
        bool          	slidable;

        //! is slide speed set?
        bool            isslidespeed;

        //! slide speed used if slidable set to true
        unsigned int   	slidespeed;

        //! is translate set?
        bool            istranslate;

        //! if true the text will be translated before displayed
        bool          	translate;


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

        //! Constructor of class MMSLabelWidgetClass.
        MMSLabelWidgetClass();

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


        //! Check if the slidable flag is set.
        bool isSlidable();

        //! Set the slidable flag.
        /*!
        \param slidable  true/false
        */
        void setSlidable(bool slidable);

        //! Mark the slidable flag as not set.
        void unsetSlidable();

        //! Get the slidable flag.
        /*!
        \return true/false
        */
        bool getSlidable();

        //! Check if the slide speed is set.
        bool isSlideSpeed();

        //! Set the slide speed.
        /*!
        \param slidespeed  speed in pixel per second
        */
        void setSlideSpeed(unsigned char slidespeed);

        //! Mark the slide speed as not set.
        void unsetSlideSpeed();

        //! Get the slide speed.
        /*!
        \return slide speed
        */
        unsigned char getSlideSpeed();


        //! Check if the translate flag is set.
        bool isTranslate();

        //! Set the translate flag.
        /*!
        \param translate  true/false
        */
        void setTranslate(bool translate);

        //! Mark the translate flag as not set.
        void unsetTranslate();

        //! Get the translate flag.
        /*!
        \return true/false
        */
        bool getTranslate();


    // friends
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSLABELWIDGETCLASS_H_*/
