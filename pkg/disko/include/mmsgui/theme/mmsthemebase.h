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

#ifndef MMSTHEMEBASE_H_
#define MMSTHEMEBASE_H_

#include "mmstools/mmstafffile.h"
#include "mmsgui/mmsguitools.h"



//! alignment type, used e.g. for positioning of windows, text in labels or textboxes, ...
typedef enum {
	//! not set
    MMSALIGNMENT_NOTSET=0,
    //! centered, horizontal and vertical
    MMSALIGNMENT_CENTER,
    //! left aligned, vertical centered
    MMSALIGNMENT_LEFT,
    //! right aligned, vertical centered
    MMSALIGNMENT_RIGHT,
    //! horizontal justified, vertical centered
    MMSALIGNMENT_JUSTIFY,
    //! horizontal centered, top aligned
    MMSALIGNMENT_TOP_CENTER,
    //! left and top aligned
    MMSALIGNMENT_TOP_LEFT,
    //! right and top aligned
    MMSALIGNMENT_TOP_RIGHT,
    //! horizontal justified, top aligned
    MMSALIGNMENT_TOP_JUSTIFY,
    //! horizontal centered, bottom aligned
    MMSALIGNMENT_BOTTOM_CENTER,
    //! left and bottom aligned
    MMSALIGNMENT_BOTTOM_LEFT,
    //! right and bottom aligned
    MMSALIGNMENT_BOTTOM_RIGHT,
    //! horizontal justified, bottom aligned
    MMSALIGNMENT_BOTTOM_JUSTIFY
} MMSALIGNMENT;


MMSALIGNMENT getAlignmentFromString(string inputstr);
MMSALIGNMENT swapAlignmentHorizontal(MMSALIGNMENT alignment);

typedef enum {
    MMSDIRECTION_NOTSET=0,
    MMSDIRECTION_LEFT,
    MMSDIRECTION_RIGHT,
    MMSDIRECTION_UP,
    MMSDIRECTION_DOWN,
    MMSDIRECTION_UP_LEFT,
    MMSDIRECTION_UP_RIGHT,
    MMSDIRECTION_DOWN_LEFT,
    MMSDIRECTION_DOWN_RIGHT,
    MMSDIRECTION_SIZE
} MMSDIRECTION;


MMSDIRECTION getDirectionFromString(string inputstr);


typedef enum {
    MMSPOSITION_NOTSET=0,
    MMSPOSITION_LEFT,
    MMSPOSITION_RIGHT,
    MMSPOSITION_TOP,
    MMSPOSITION_BOTTOM,
    MMSPOSITION_TOP_LEFT,
    MMSPOSITION_TOP_RIGHT,
    MMSPOSITION_BOTTOM_LEFT,
    MMSPOSITION_BOTTOM_RIGHT,
    MMSPOSITION_SIZE
} MMSPOSITION;


MMSPOSITION getPositionFromString(string inputstr);


#define MMSTHEMECLASS_INIT_STRING(x) \
	this->ed.x = NULL; \
    this->id.is##x = false;

#define MMSTHEMECLASS_FREE_STRING(x) \
	if (this->ed.x) delete this->ed.x; \
	this->ed.x = NULL; \
    this->id.is##x = false;

#define MMSTHEMECLASS_SET_STRING(x) \
	if (!this->ed.x) this->ed.x = new string(x); \
    else *(this->ed.x) = x; \
	this->id.is##x = true;

#define MMSTHEMECLASS_GET_STRING(x) \
	if (!this->id.is##x) return false; \
	x = *(this->ed.x); \
	return true;



#define MMSTHEMECLASS_INIT_STRINGS(x,c) \
	for (int cc=0;cc<c;cc++) \
		this->ed.x[cc] = NULL; \
    this->id.is##x = false;

#define MMSTHEMECLASS_FREE_STRINGS(x,c) \
	for (int cc=0;cc<c;cc++) { \
		if (this->ed.x[cc]) delete this->ed.x[cc]; \
		this->ed.x[cc] = NULL; \
	} \
    this->id.is##x = false;

#define MMSTHEMECLASS_SET_STRINGS(x,c,y) \
	if (!this->ed.x[c]) this->ed.x[c] = new string(y); \
	else *(this->ed.x[c]) = y; \
	this->id.is##x = true;

#define MMSTHEMECLASS_GET_STRINGS(x,c,y) \
	if (!this->id.is##x) return false; \
	if (!this->ed.x[c]) return false; \
	y = *(this->ed.x[c]); \
	return true;

#define MMSTHEMECLASS_UNSET_STRINGS(x,c) \
	for (int cc=0;cc<c;cc++) \
		if (this->ed.x[cc]) *(this->ed.x[cc]) = ""; \
	this->id.is##x = false;



#define MMSTHEMECLASS_INIT_BASIC(x) \
    this->id.is##x = false;

#define MMSTHEMECLASS_FREE_BASIC(x) \
    this->id.is##x = false;

#define MMSTHEMECLASS_SET_BASIC(x) \
    this->id.x = x; \
    this->id.is##x = true;

#define MMSTHEMECLASS_GET_BASIC(x) \
    if (!this->id.is##x) return false; \
    x = this->id.x; \
    return true;


#define MMSTHEMECLASS_UNSET(x) \
    this->id.is##x = false;

#define MMSTHEMECLASS_ISSET(x) \
    return this->id.is##x;




#define startTAFFScan \
	{ bool first=true; int attrid; char *attrval_str; int attrval_int; \
	while (1) { \
		if (first) { attrid=tafff->getFirstAttribute(&attrval_str, &attrval_int); first=false; } else \
		attrid=tafff->getNextAttribute(&attrval_str, &attrval_int); \
		if (attrid<0) break;

#define endTAFFScan } }


#define startTAFFScan_WITHOUT_ID \
	{ bool first=true; int attrid; char *attrval_str; int attrval_int; char *attrname; \
	while (1) { \
		if (first) { attrid=tafff->getFirstAttribute(&attrval_str, &attrval_int, &attrname); first=false; } else \
		attrid=tafff->getNextAttribute(&attrval_str, &attrval_int, &attrname); \
		while((attrid>=0)&&(attrid!=MMSTAFF_ATTR_WITHOUT_ID)) attrid=tafff->getNextAttribute(&attrval_str,&attrval_int,&attrname); \
		if (attrid<0) break;

#define endTAFFScan_WITHOUT_ID } }


//! macro for widget specific setAttributesFromTAFF() implementation
#define ISATTRNAME(aname) ((strcmp(attrname, GETATTRNAME(aname))==0)?(tafff->convertString2TaffAttributeType(GETATTRTYPE(aname), attrval_str, &attrval_str_valid, &int_val_set, &byte_val_set, p_int_val, attrname, attrid, tafff->getCurrentTagName())):(0))




namespace MMSGUI_MMSDIALOG_ATTR {

	#define MMSGUI_MMSDIALOG_ATTR_ATTRDESC \
		{ "name", TAFF_ATTRTYPE_STRING }

	#define MMSGUI_MMSDIALOG_ATTR_IDS \
		MMSGUI_MMSDIALOG_ATTR_IDS_name

	#define MMSGUI_MMSDIALOG_ATTR_INIT { \
		MMSGUI_MMSDIALOG_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_MMSDIALOG_ATTR_IDS
	} ids;
}


namespace MMSGUI_BASE_ATTR {

	#define MMSGUI_BASE_ATTR_ATTRDESC \
		{ "name", TAFF_ATTRTYPE_NE_STRING }, \
		{ "type", TAFF_ATTRTYPE_NE_STRING }, \
		{ "class", TAFF_ATTRTYPE_NE_STRING }, \
		{ "size", TAFF_ATTRTYPE_STRING }, \
		{ "min_width", TAFF_ATTRTYPE_STRING }, \
		{ "min_height", TAFF_ATTRTYPE_STRING }, \
		{ "max_width", TAFF_ATTRTYPE_STRING }, \
		{ "max_height", TAFF_ATTRTYPE_STRING }, \
		{ "show", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_BASE_ATTR_IDS \
		MMSGUI_BASE_ATTR_IDS_name, \
		MMSGUI_BASE_ATTR_IDS_type, \
		MMSGUI_BASE_ATTR_IDS_class, \
		MMSGUI_BASE_ATTR_IDS_size, \
		MMSGUI_BASE_ATTR_IDS_min_width, \
		MMSGUI_BASE_ATTR_IDS_min_height, \
		MMSGUI_BASE_ATTR_IDS_max_width, \
		MMSGUI_BASE_ATTR_IDS_max_height, \
		MMSGUI_BASE_ATTR_IDS_show

	#define MMSGUI_BASE_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS
	} ids;
}





#endif /*MMSTHEMEBASE_H_*/


