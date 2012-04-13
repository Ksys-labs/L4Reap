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

#ifndef MMSDESCRIPTIONCLASS_H_
#define MMSDESCRIPTIONCLASS_H_

#include "mmsgui/theme/mmsthemebase.h"

//! describe attributes
namespace MMSGUI_DESCRIPTION_ATTR {

	#define MMSGUI_DESCRIPTION_ATTR_ATTRDESC \
		{ "author", TAFF_ATTRTYPE_STRING }, \
		{ "email", TAFF_ATTRTYPE_STRING }, \
		{ "desc", TAFF_ATTRTYPE_STRING }

	#define MMSGUI_DESCRIPTION_ATTR_IDS \
		MMSGUI_DESCRIPTION_ATTR_IDS_author, \
		MMSGUI_DESCRIPTION_ATTR_IDS_email, \
		MMSGUI_DESCRIPTION_ATTR_IDS_desc

	#define MMSGUI_DESCRIPTION_ATTR_INIT { \
		MMSGUI_DESCRIPTION_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_DESCRIPTION_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_DESCRIPTION_ATTR_I[];


class MMSDescriptionClass {
    private:
        bool    isauthor;
        string  author;
        bool    isemail;
        string  email;
        bool    isdesc;
        string  desc;

    public:
        MMSDescriptionClass();
        //
        void unsetAll();

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   pointer to the TAFF buffer
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff);

        bool isAuthor();
        void setAuthor(string author);
        void unsetAuthor();
        string getAuthor();
        //
        bool isEmail();
        void setEmail(string email);
        void unsetEmail();
        string getEmail();
        //
        bool isDesc();
        void setDesc(string desc);
        void unsetDesc();
        string getDesc();
};

#endif /*MMSDESCRIPTIONCLASS_H_*/
