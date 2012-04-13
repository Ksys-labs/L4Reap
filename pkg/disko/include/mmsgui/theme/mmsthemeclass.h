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

#ifndef MMSTHEMECLASS_H_
#define MMSTHEMECLASS_H_

#include "mmsgui/theme/mmsthemebase.h"

//! describe attributes
namespace MMSGUI_MMSTHEME_ATTR {

	#define MMSGUI_MMSTHEME_ATTR_ATTRDESC \
		{ "name", TAFF_ATTRTYPE_STRING }, \
		{ "fadein", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_MMSTHEME_ATTR_IDS \
		MMSGUI_MMSTHEME_ATTR_IDS_name, \
		MMSGUI_MMSTHEME_ATTR_IDS_fadein

	#define MMSGUI_MMSTHEME_ATTR_INIT { \
		MMSGUI_MMSTHEME_ATTR_ATTRDESC, \
		{NULL, TAFF_ATTRTYPE_NONE} \
	}

	typedef enum {
		MMSGUI_MMSTHEME_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_MMSTHEME_ATTR_I[];


class MMSThemeClass {
    private:
        bool    isname;
        string  name;
        bool    isfadein;
        bool    fadein;

    public:
        MMSThemeClass();
        //
        void unsetAll();

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   pointer to the TAFF buffer
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff);

        bool isName();
        void setName(string name);
        void unsetName();
        string getName();

        bool isFadeIn();
        void setFadeIn(bool fadein);
        void unsetFadeIn();
        bool getFadeIn();
};

#endif /*MMSTHEMECLASS_H_*/
