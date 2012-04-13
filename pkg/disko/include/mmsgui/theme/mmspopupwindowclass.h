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

#ifndef MMSPOPUPWINDOWCLASS_H_
#define MMSPOPUPWINDOWCLASS_H_

#include "mmsgui/theme/mmswindowclass.h"

//! describe attributes for MMSPopupWindow which are additional to the MMSWindowClass
namespace MMSGUI_POPUPWINDOW_ATTR {

	#define MMSGUI_POPUPWINDOW_ATTR_ATTRDESC \
		{ "duration", TAFF_ATTRTYPE_UCHAR }

	#define MMSGUI_POPUPWINDOW_ATTR_IDS \
		MMSGUI_POPUPWINDOW_ATTR_IDS_duration

	#define MMSGUI_POPUPWINDOW_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WINDOW_ATTR_ATTRDESC, \
		MMSGUI_POPUPWINDOW_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WINDOW_ATTR_IDS,
		MMSGUI_POPUPWINDOW_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_POPUPWINDOW_ATTR_I[];


class MMSPopupWindowClass {
    private:
        string       className;
        bool         isduration;
        unsigned int duration;

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   		pointer to the TAFF buffer
        \param path    		optional, path needed for empty path values from the TAFF buffer
        \param reset_paths  optional, should reset all path attributes?
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *path = NULL, bool reset_paths = false);

    public:
        MMSWindowClass windowClass;

        MMSPopupWindowClass();
        //
        void unsetAll();

        void setClassName(string className);
        string getClassName();
        //
        bool isDuration();
        void setDuration(unsigned int duration);
        void unsetDuration();
        unsigned int getDuration();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSPOPUPWINDOWCLASS_H_*/
