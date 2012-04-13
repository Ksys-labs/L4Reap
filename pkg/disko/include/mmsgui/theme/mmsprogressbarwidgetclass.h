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

#ifndef MMSPROGRESSBARWIDGETCLASS_H_
#define MMSPROGRESSBARWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSProgressBarWidget which are additional to the MMSWidgetClass
namespace MMSGUI_PROGRESSBARWIDGET_ATTR {

	#define MMSGUI_PROGRESSBARWIDGET_ATTR_ATTRDESC \
		{ "color", TAFF_ATTRTYPE_COLOR }, \
		{ "color.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor", TAFF_ATTRTYPE_COLOR }, \
		{ "selcolor.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "progress", TAFF_ATTRTYPE_UCHAR100 }

	#define MMSGUI_PROGRESSBARWIDGET_ATTR_IDS \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_color, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_color_a, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_color_r, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_color_g, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_color_b, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_selcolor, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_selcolor_a, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_selcolor_r, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_selcolor_g, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_selcolor_b, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS_progress

	#define MMSGUI_PROGRESSBARWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_PROGRESSBARWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_PROGRESSBARWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_PROGRESSBARWIDGET_ATTR_I[];


//! A data access class for the progressbar widget.
/*!
This class is the base for the MMSProgressBarWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSProgressBarWidget.
\author Jens Schneider
*/
class MMSProgressBarWidgetClass {
    private:
    	//! name of the theme class
        string          className;

        //! is color set?
        bool            iscolor;

        //! color if the widget is not selected
        MMSFBColor      color;

        //! is selcolor set?
        bool            isselcolor;

        //! color if the widget is selected
        MMSFBColor      selcolor;

        //! is progress value set?
        bool            isprogress;

        //! progress value in percent
        unsigned int    progress;

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

        //! Constructor of class MMSProgressBarWidgetClass.
        MMSProgressBarWidgetClass();

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

        //! Check if the color is set. This color will be used to draw the unselected progress bar.
        bool isColor();

        //! Set the color which is used to draw the unselected progress bar.
        /*!
        \param color  color for unselected bar
        */
        void setColor(MMSFBColor color);

        //! Mark the color as not set.
        void unsetColor();

        //! Get the color which is used to draw the unselected progress bar.
        /*!
        \return color which is used for the unselected progress bar
        */
        MMSFBColor getColor();

        //! Check if the color is set. This color will be used for the selected progress bar.
        bool isSelColor();

        //! Set the color which is used to draw the selected progress bar.
        /*!
        \param selcolor  color for selected bar
        */
        void setSelColor(MMSFBColor selcolor);

        //! Mark the color as not set.
        void unsetSelColor();

        //! Get the color which is used to draw the selected progress bar.
        /*!
        \return color which is used for the selected progress bar
        */
        MMSFBColor getSelColor();

        //! Check if the progress value is set.
        bool isProgress();

        //! Set the progress.
        /*!
        \param progress  progress in percent (0-100)
        */
        void setProgress(unsigned int progress);

        //! Mark the progress as not set.
        void unsetProgress();

        //! Get the progress.
        /*!
        \return progress
        */
        unsigned int getProgress();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSPROGRESSBARWIDGETCLASS_H_*/
