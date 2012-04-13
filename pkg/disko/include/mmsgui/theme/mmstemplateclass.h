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

#ifndef MMSTEMPLATECLASS_H_
#define MMSTEMPLATECLASS_H_

#include "mmsgui/theme/mmsthemebase.h"

//! describe attributes
namespace MMSGUI_TEMPLATE_ATTR {

	#define MMSGUI_TEMPLATE_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_TEMPLATE_ATTR_I[];


class MMSTemplateClass {
    private:
    	//! name of the theme class
        string          className;

        //! the copied TAFF buffer
        MMSTaffFile		*tafff;

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   pointer to the TAFF buffer
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff);

        //! Saves a copy of an TAFF buffer including all child nodes.
        /*!
        \param tafff   pointer to the TAFF buffer
        */
        void duplicateTAFF(MMSTaffFile *tafff);

    public:
        MMSTemplateClass();
        ~MMSTemplateClass();
        //
        void unsetAll();

        //! Get the copied TAFF buffer, see duplicateTAFF().
        /*!
        \return pointer to the TAFF buffer
        */
        MMSTaffFile *getTAFF();

        void setClassName(string className);
        string getClassName();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSTEMPLATECLASS_H_*/
