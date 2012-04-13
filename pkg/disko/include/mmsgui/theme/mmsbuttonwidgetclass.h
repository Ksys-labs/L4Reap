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

#ifndef MMSBUTTONWIDGETCLASS_H_
#define MMSBUTTONWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSButtonWidget which are additional to the MMSWidgetClass
namespace MMSGUI_BUTTONWIDGET_ATTR {

	#define MMSGUI_BUTTONWIDGET_ATTR_INIT { \
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

extern TAFF_ATTRDESC MMSGUI_BUTTONWIDGET_ATTR_I[];


//! A data access class for the button widget.
/*!
This class is the base for the MMSButtonWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSButtonWidget.
\author Jens Schneider
*/
class MMSButtonWidgetClass {
    private:
    	//! name of the theme class
        string className;

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

        //! Constructor of class MMSButtonWidgetClass.
        MMSButtonWidgetClass();

        //! Mark all attributes as not set.
        void unsetAll();

        //! Set the name of the theme class.
        /*!
        \param className  name of the class
        */
        void setClassName(string className);

        //! Get the name of the theme class.
        /*!
        \return name of the class
        */
        string getClassName();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSBUTTONWIDGETCLASS_H_*/
