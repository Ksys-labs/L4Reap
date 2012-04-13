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

#ifndef MMSTEXTBASECLASS_H_
#define MMSTEXTBASECLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

// all definitions around fonts
///////////////////////////////////////////////////////////////////////////////

//! FONT macro to address font attribute names
#define GETFONTATTRNAME(w, aname) w##_I[w::MMSGUI_FONT_ATTR_IDS_##aname].name

//! FONT macro to address font attribute types
#define GETFONTATTRTYPE(w, aname) w##_I[w::MMSGUI_FONT_ATTR_IDS_##aname].type

//! FONT macro for widget specific setAttributesFromTAFF() implementation
#define ISFONTATTRNAME(w, aname) ((strcmp(attrname, GETFONTATTRNAME(w, aname))==0)?(tafff->convertString2TaffAttributeType(GETFONTATTRTYPE(w, aname), attrval_str, &attrval_str_valid, &int_val_set, &byte_val_set, p_int_val, attrname, attrid, tafff->getCurrentTagName())):(0))


//! XML attributes for fonts
namespace MMSGUI_FONT_ATTR {

	#define MMSGUI_FONT_ATTR_ATTRDESC \
		{ "font.path", TAFF_ATTRTYPE_STRING }, \
		{ "font.size", TAFF_ATTRTYPE_UCHAR }, \
		{ "font.name", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.de", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.en", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.dk", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.es", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.fi", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.fr", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.it", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.nl", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.no", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.se", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.tr", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.cn", TAFF_ATTRTYPE_STRING }, \
		{ "font.name.il", TAFF_ATTRTYPE_STRING }

	#define MMSGUI_FONT_ATTR_IDS \
		MMSGUI_FONT_ATTR_IDS_font_path, \
		MMSGUI_FONT_ATTR_IDS_font_size, \
		MMSGUI_FONT_ATTR_IDS_font_name, \
		MMSGUI_FONT_ATTR_IDS_font_name_de, \
		MMSGUI_FONT_ATTR_IDS_font_name_en, \
		MMSGUI_FONT_ATTR_IDS_font_name_dk, \
		MMSGUI_FONT_ATTR_IDS_font_name_es, \
		MMSGUI_FONT_ATTR_IDS_font_name_fi, \
		MMSGUI_FONT_ATTR_IDS_font_name_fr, \
		MMSGUI_FONT_ATTR_IDS_font_name_it, \
		MMSGUI_FONT_ATTR_IDS_font_name_nl, \
		MMSGUI_FONT_ATTR_IDS_font_name_no, \
		MMSGUI_FONT_ATTR_IDS_font_name_se, \
		MMSGUI_FONT_ATTR_IDS_font_name_tr, \
		MMSGUI_FONT_ATTR_IDS_font_name_cn, \
		MMSGUI_FONT_ATTR_IDS_font_name_il

	#define MMSGUI_FONT_ATTR_INIT { \
		MMSGUI_FONT_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_FONT_ATTR_IDS
	} ids;
}


//! FONT macro for widget specific setAttributesFromTAFF() implementation
#define SET_FONT_FROM_TAFF(w) \
	case w::MMSGUI_FONT_ATTR_IDS_font_path: \
		if (*attrval_str) \
			setFontPath(attrval_str); \
		else \
			setFontPath((path)?*path:""); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_size: \
		setFontSize(attrval_int); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name: \
		setFontName(attrval_str); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_de: \
		setFontName(attrval_str, MMSLANG_DE); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_en: \
		setFontName(attrval_str, MMSLANG_EN); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_dk: \
		setFontName(attrval_str, MMSLANG_DK); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_es: \
		setFontName(attrval_str, MMSLANG_ES); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_fi: \
		setFontName(attrval_str, MMSLANG_FI); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_fr: \
		setFontName(attrval_str, MMSLANG_FR); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_it: \
		setFontName(attrval_str, MMSLANG_IT); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_nl: \
		setFontName(attrval_str, MMSLANG_NL); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_no: \
		setFontName(attrval_str, MMSLANG_NO); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_se: \
		setFontName(attrval_str, MMSLANG_SE); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_tr: \
		setFontName(attrval_str, MMSLANG_TR); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_cn: \
		setFontName(attrval_str, MMSLANG_CN); \
		break; \
	case w::MMSGUI_FONT_ATTR_IDS_font_name_il: \
		setFontName(attrval_str, MMSLANG_IL); \
		break;


//! FONT macro for widget specific setAttributesFromTAFF() implementation
#define SET_FONT_FROM_TAFF_WITH_PREFIX(w) \
	if (ISFONTATTRNAME(w, font_path)) { \
		if (*attrval_str) \
			setFontPath(attrval_str); \
		else \
			setFontPath((path)?*path:""); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_size)) { \
		setFontSize(attrval_int); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name)) { \
		setFontName(attrval_str); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_de)) { \
		setFontName(attrval_str, MMSLANG_DE); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_en)) { \
		setFontName(attrval_str, MMSLANG_EN); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_dk)) { \
		setFontName(attrval_str, MMSLANG_DK); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_es)) { \
		setFontName(attrval_str, MMSLANG_ES); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_fi)) { \
		setFontName(attrval_str, MMSLANG_FI); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_fr)) { \
		setFontName(attrval_str, MMSLANG_FR); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_it)) { \
		setFontName(attrval_str, MMSLANG_IT); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_nl)) { \
		setFontName(attrval_str, MMSLANG_NL); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_no)) { \
		setFontName(attrval_str, MMSLANG_NO); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_se)) { \
		setFontName(attrval_str, MMSLANG_SE); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_tr)) { \
		setFontName(attrval_str, MMSLANG_TR); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_cn)) { \
		setFontName(attrval_str, MMSLANG_CN); \
	} \
	else \
	if (ISFONTATTRNAME(w, font_name_il)) { \
		setFontName(attrval_str, MMSLANG_IL); \
	}




// all definitions around shadows
///////////////////////////////////////////////////////////////////////////////

//! SHADOW macro to address shadow attribute names
#define GETSHADOWATTRNAME(w, aname) w##_I[w::MMSGUI_SHADOW_ATTR_IDS_##aname].name

//! SHADOW macro to address shadow attribute types
#define GETSHADOWATTRTYPE(w, aname) w##_I[w::MMSGUI_SHADOW_ATTR_IDS_##aname].type

//! SHADOW macro for widget specific setAttributesFromTAFF() implementation
#define ISSHADOWATTRNAME(w, aname) ((strcmp(attrname, GETSHADOWATTRNAME(w, aname))==0)?(tafff->convertString2TaffAttributeType(GETSHADOWATTRTYPE(w, aname), attrval_str, &attrval_str_valid, &int_val_set, &byte_val_set, p_int_val, attrname, attrid, tafff->getCurrentTagName())):(0))


//! XML attributes for shadows
namespace MMSGUI_SHADOW_ATTR {

	#define MMSGUI_SHADOW_ATTR_ATTRDESC \
		{ "shadow.top.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.bottom.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.right.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.top-left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.top-right.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.bottom-left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "shadow.bottom-right.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.top.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.bottom.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.right.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.top-left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.top-right.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.bottom-left.color", TAFF_ATTRTYPE_COLOR }, \
		{ "selshadow.bottom-right.color", TAFF_ATTRTYPE_COLOR }

	#define MMSGUI_SHADOW_ATTR_IDS \
		MMSGUI_SHADOW_ATTR_IDS_shadow_top_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_right_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_top_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_top_right_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_right_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_top_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_right_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_top_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_top_right_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_left_color, \
		MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_right_color

	#define MMSGUI_SHADOW_ATTR_INIT { \
		MMSGUI_SHADOW_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_SHADOW_ATTR_IDS
	} ids;
}


//! SHADOW macro for widget specific setAttributesFromTAFF() implementation
#define SET_SHADOW_FROM_TAFF(w) \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_top_color: \
		setShadowColor(MMSPOSITION_TOP, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_color: \
		setShadowColor(MMSPOSITION_BOTTOM, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_left_color: \
		setShadowColor(MMSPOSITION_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_right_color: \
		setShadowColor(MMSPOSITION_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_top_left_color: \
		setShadowColor(MMSPOSITION_TOP_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_top_right_color: \
		setShadowColor(MMSPOSITION_TOP_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_left_color: \
		setShadowColor(MMSPOSITION_BOTTOM_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_shadow_bottom_right_color: \
		setShadowColor(MMSPOSITION_BOTTOM_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_top_color: \
		setSelShadowColor(MMSPOSITION_TOP, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_color: \
		setSelShadowColor(MMSPOSITION_BOTTOM, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_left_color: \
		setSelShadowColor(MMSPOSITION_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_right_color: \
		setSelShadowColor(MMSPOSITION_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_top_left_color: \
		setSelShadowColor(MMSPOSITION_TOP_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_top_right_color: \
		setSelShadowColor(MMSPOSITION_TOP_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_left_color: \
		setSelShadowColor(MMSPOSITION_BOTTOM_LEFT, MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_SHADOW_ATTR_IDS_selshadow_bottom_right_color: \
		setSelShadowColor(MMSPOSITION_BOTTOM_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
		break;


//! SHADOW macro for widget specific setAttributesFromTAFF() implementation
#define SET_SHADOW_FROM_TAFF_WITH_PREFIX(w) \
	if (ISSHADOWATTRNAME(w, shadow_top_color)) { \
		setShadowColor(MMSPOSITION_TOP, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_bottom_color)) { \
		setShadowColor(MMSPOSITION_BOTTOM, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_left_color)) { \
		setShadowColor(MMSPOSITION_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_right_color)) { \
		setShadowColor(MMSPOSITION_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_top_left_color)) { \
		setShadowColor(MMSPOSITION_TOP_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_top_right_color)) { \
		setShadowColor(MMSPOSITION_TOP_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_bottom_left_color)) { \
		setShadowColor(MMSPOSITION_BOTTOM_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, shadow_bottom_right_color)) { \
		setShadowColor(MMSPOSITION_BOTTOM_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_top_color)) { \
		setSelShadowColor(MMSPOSITION_TOP, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_bottom_color)) { \
		setSelShadowColor(MMSPOSITION_BOTTOM, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_left_color)) { \
		setSelShadowColor(MMSPOSITION_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_right_color)) { \
		setSelShadowColor(MMSPOSITION_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_top_left_color)) { \
		setSelShadowColor(MMSPOSITION_TOP_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_top_right_color)) { \
		setSelShadowColor(MMSPOSITION_TOP_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_bottom_left_color)) { \
		setSelShadowColor(MMSPOSITION_BOTTOM_LEFT, MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISSHADOWATTRNAME(w, selshadow_bottom_right_color)) { \
		setSelShadowColor(MMSPOSITION_BOTTOM_RIGHT, MMSFBColor((unsigned int)attrval_int)); \
	}




// all definitions around textinfos
///////////////////////////////////////////////////////////////////////////////

//! TEXTINFO macro to address textinfo attribute names
#define GETTEXTINFOATTRNAME(w, aname) w##_I[w::MMSGUI_TEXTINFO_ATTR_IDS_##aname].name

//! TEXTINFO macro to address textinfos attribute types
#define GETTEXTINFOATTRTYPE(w, aname) w##_I[w::MMSGUI_TEXTINFO_ATTR_IDS_##aname].type

//! TEXTINFO macro for widget specific setAttributesFromTAFF() implementation
#define ISTEXTINFOATTRNAME(w, aname) ((strcmp(attrname, GETTEXTINFOATTRNAME(w, aname))==0)?(tafff->convertString2TaffAttributeType(GETTEXTINFOATTRTYPE(w, aname), attrval_str, &attrval_str_valid, &int_val_set, &byte_val_set, p_int_val, attrname, attrid, tafff->getCurrentTagName())):(0))


//! XML attributes for textinfos
namespace MMSGUI_TEXTINFO_ATTR {

	#define MMSGUI_TEXTINFO_ATTR_ATTRDESC \
		{ "alignment", TAFF_ATTRTYPE_STRING }, \
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
		{ "color_p", TAFF_ATTRTYPE_COLOR }, \
		{ "color_p.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_p.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_p.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_p.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_p", TAFF_ATTRTYPE_COLOR }, \
		{ "selcolor_p.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_p.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_p.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_p.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_i", TAFF_ATTRTYPE_COLOR }, \
		{ "color_i.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_i.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_i.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "color_i.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_i", TAFF_ATTRTYPE_COLOR }, \
		{ "selcolor_i.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_i.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_i.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor_i.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "text", TAFF_ATTRTYPE_STRING }


	#define MMSGUI_TEXTINFO_ATTR_IDS \
		MMSGUI_TEXTINFO_ATTR_IDS_alignment, \
		MMSGUI_TEXTINFO_ATTR_IDS_color, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_p, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_p_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_p_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_p_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_p_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_i, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_i_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_i_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_i_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_color_i_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_a, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_r, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_g, \
		MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_b, \
		MMSGUI_TEXTINFO_ATTR_IDS_text

	#define MMSGUI_TEXTINFO_ATTR_INIT { \
		MMSGUI_TEXTINFO_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_TEXTINFO_ATTR_IDS
	} ids;
}


//! TEXTINFO macro for widget specific setAttributesFromTAFF() implementation
#define SET_TEXTINFO_FROM_TAFF(w) \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_alignment: \
		setAlignment(getAlignmentFromString(attrval_str)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color: \
		setColor(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.a = attrval_int; \
		setColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.r = attrval_int; \
		setColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.g = attrval_int; \
		setColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.b = attrval_int; \
		setColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor: \
		setSelColor(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.a = attrval_int; \
		setSelColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.r = attrval_int; \
		setSelColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.g = attrval_int; \
		setSelColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.b = attrval_int; \
		setSelColor(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_p: \
		setColor_p(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_p_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.a = attrval_int; \
		setColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_p_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.r = attrval_int; \
		setColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_p_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.g = attrval_int; \
		setColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_p_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.b = attrval_int; \
		setColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p: \
		setSelColor_p(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.a = attrval_int; \
		setSelColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.r = attrval_int; \
		setSelColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.g = attrval_int; \
		setSelColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_p_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.b = attrval_int; \
		setSelColor_p(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_i: \
		setColor_i(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_i_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.a = attrval_int; \
		setColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_i_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.r = attrval_int; \
		setColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_i_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.g = attrval_int; \
		setColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_color_i_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.b = attrval_int; \
		setColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i: \
		setSelColor_i(MMSFBColor((unsigned int)attrval_int)); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_a: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.a = attrval_int; \
		setSelColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_r: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.r = attrval_int; \
		setSelColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_g: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.g = attrval_int; \
		setSelColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_selcolor_i_b: \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.b = attrval_int; \
		setSelColor_i(color); \
		break; \
	case w::MMSGUI_TEXTINFO_ATTR_IDS_text: \
		setText(attrval_str); \
		break;



//! TEXTINFO macro for widget specific setAttributesFromTAFF() implementation
#define SET_TEXTINFO_FROM_TAFF_WITH_PREFIX(w) \
	if (ISTEXTINFOATTRNAME(w, alignment)) { \
		setAlignment(getAlignmentFromString(attrval_str)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color)) { \
		setColor(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.a = attrval_int; \
		setColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.r = attrval_int; \
		setColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.g = attrval_int; \
		setColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor()) color = getColor(); \
		color.b = attrval_int; \
		setColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor)) { \
		setSelColor(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.a = attrval_int; \
		setSelColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.r = attrval_int; \
		setSelColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.g = attrval_int; \
		setSelColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor()) color = getSelColor(); \
		color.b = attrval_int; \
		setSelColor(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_p)) { \
		setColor_p(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_p_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.a = attrval_int; \
		setColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_p_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.r = attrval_int; \
		setColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_p_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.g = attrval_int; \
		setColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_p_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_p()) color = getColor_p(); \
		color.b = attrval_int; \
		setColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_p)) { \
		setSelColor_p(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_p_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.a = attrval_int; \
		setSelColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_p_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.r = attrval_int; \
		setSelColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_p_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.g = attrval_int; \
		setSelColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_p_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_p()) color = getSelColor_p(); \
		color.b = attrval_int; \
		setSelColor_p(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_i)) { \
		setColor_i(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_i_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.a = attrval_int; \
		setColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_i_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.r = attrval_int; \
		setColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_i_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.g = attrval_int; \
		setColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, color_i_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isColor_i()) color = getColor_i(); \
		color.b = attrval_int; \
		setColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_i)) { \
		setSelColor_i(MMSFBColor((unsigned int)attrval_int)); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_i_a)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.a = attrval_int; \
		setSelColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_i_r)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.r = attrval_int; \
		setSelColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_i_g)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.g = attrval_int; \
		setSelColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, selcolor_i_b)) { \
		color.a = color.r = color.g = color.b = 0; \
		if (isSelColor_i()) color = getSelColor_i(); \
		color.b = attrval_int; \
		setSelColor_i(color); \
	} \
	else \
	if (ISTEXTINFOATTRNAME(w, text)) { \
		setText(attrval_str); \
	}




//! A base class for MMSLabelWidgetClass, MMSTextBoxWidgetClass and MMSInputWidgetClass.
/*!
\note This class will be internally used by MMSLabelWidgetClass, MMSTextBoxWidgetClass and MMSInputWidgetClass.
\author Jens Schneider
*/
class MMSTextBaseClass {
    private:
        //! is fontpath set?
        bool            isfontpath;

        //! path to the font
        string          fontpath;

        //! is fontsize set?
        bool            isfontsize;

        //! size of the font
        unsigned int    fontsize;


        //! describes name of a font
        typedef struct {
            //! is fontname set?
            bool            isfontname;

            //! name of the font
            string          fontname;
        } MMSTEXTBASEFONTNAME;

        //! language dependent font filenames
        MMSTEXTBASEFONTNAME	fontname[MMSLANG_SIZE];



        //! is alignment set?
        bool            isalignment;

        //! alignment of the text
        MMSALIGNMENT    alignment;

        //! is color set?
        bool            iscolor;

        //! color of the text if the widget is not selected
        MMSFBColor      color;

        //! is selcolor set?
        bool            isselcolor;

        //! color of the text if the widget is selected
        MMSFBColor      selcolor;


        //! is color_p set?
        bool            iscolor_p;

        //! color of the text if the widget is not selected but pressed
        MMSFBColor      color_p;

        //! is selcolor_p set?
        bool            isselcolor_p;

        //! color of the text if the widget is selected and pressed
        MMSFBColor      selcolor_p;


        //! is color_i set?
        bool            iscolor_i;

        //! color of the text if the widget is not selected and disabled
        MMSFBColor      color_i;

        //! is selcolor_i set?
        bool            isselcolor_i;

        //! color of the text if the widget is selected and disabled
        MMSFBColor      selcolor_i;


        //! is text set?
        bool            istext;

        //! text to draw
        string          text;



        //! describes shadow of a text
        typedef struct {
            //! is color set?
            bool            iscolor;

            //! color of the text if the widget is not selected
            MMSFBColor      color;

            //! is selcolor set?
            bool            isselcolor;

            //! color of the text if the widget is selected
            MMSFBColor      selcolor;
        } MMSTEXTBASESHADOW;

        //! text shadows (eight directions)
        MMSTEXTBASESHADOW	shadow[MMSPOSITION_SIZE];


    public:
        //! Constructor of class MMSTextBaseClass.
        MMSTextBaseClass();

        //! Mark all attributes as not set.
        virtual void unsetAll();


        //! Check if the fontpath is set.
        bool isFontPath();

        //! Set the fontpath which is used to draw the text.
        /*!
        \param fontpath  path to the font
        */
        void setFontPath(string fontpath);

        //! Mark the fontpath as not set.
        void unsetFontPath();

        //! Get the fontpath which is used to draw the text.
        /*!
        \return path to the font
        */
        string getFontPath();


        //! Check if the fontsize is set.
        bool isFontSize();

        //! Set the fontsize which is used to draw the text.
        /*!
        \param fontsize  size of the font
        */
        void setFontSize(unsigned int fontsize);

        //! Mark the fontsize as not set.
        void unsetFontSize();

        //! Get the fontsize which is used to draw the text.
        /*!
        \return size of the font
        */
        unsigned int getFontSize();


        //! Check if the fontname is set.
        /*!
        \param lang  optional language
        */
        bool isFontName(MMSLanguage lang = MMSLANG_NONE);

        //! Set the fontname which is used to draw the text.
        /*!
        \param fontname  name of the font
        \param lang      optional language
        */
        void setFontName(string fontname, MMSLanguage lang = MMSLANG_NONE);

        //! Mark the fontname as not set.
        /*!
        \param lang  optional language
        */
        void unsetFontName(MMSLanguage lang = MMSLANG_NONE);

        //! Mark all fontnames as not set.
        void unsetFontNames();

        //! Get the fontname which is used to draw the text.
        /*!
        \param lang  optional language
        \return name of the font
        */
        string getFontName(MMSLanguage lang = MMSLANG_NONE);


        //! Check if alignment is set.
        bool isAlignment();

        //! Set the alignment of the text (see MMSALIGNMENT values).
        /*!
        \param alignment  text alignment
        */
        void setAlignment(MMSALIGNMENT alignment);

        //! Mark the alignment as not set.
        void unsetAlignment();

        //! Get the alignment of the text (see MMSALIGNMENT values).
        /*!
        \return size of the font
        */
        MMSALIGNMENT getAlignment();

        //! Check if the color is set. This color will be used for the unselected text.
        bool isColor();

        //! Set the color which is used to draw the unselected text.
        /*!
        \param color  color for unselected text
        */
        void setColor(MMSFBColor color);

        //! Mark the color as not set.
        void unsetColor();

        //! Get the color which is used to draw the unselected text.
        /*!
        \return color for unselected text
        */
        MMSFBColor getColor();

        //! Check if the color is set. This color will be used for the selected text.
        bool isSelColor();

        //! Set the color which is used to draw the selected text.
        /*!
        \param selcolor  color for selected text
        */
        void setSelColor(MMSFBColor selcolor);

        //! Mark the color as not set.
        void unsetSelColor();

        //! Get the color which is used to draw the selected text.
        /*!
        \return color for selected text
        */
        MMSFBColor getSelColor();


        //! Check if the color_p is set. This color will be used for the unselected, pressed text.
        bool isColor_p();

        //! Set the color which is used to draw the unselected, pressed text.
        /*!
        \param color_p  color for unselected, pressed text
        */
        void setColor_p(MMSFBColor color_p);

        //! Mark the color_p as not set.
        void unsetColor_p();

        //! Get the color which is used to draw the unselected, pressed text.
        /*!
        \return color for unselected, pressed text
        */
        MMSFBColor getColor_p();

        //! Check if the color is set. This color will be used for the selected, pressed text.
        bool isSelColor_p();

        //! Set the color which is used to draw the selected, pressed text.
        /*!
        \param selcolor_p  color for selected, pressed text
        */
        void setSelColor_p(MMSFBColor selcolor_p);

        //! Mark the color_p as not set.
        void unsetSelColor_p();

        //! Get the color which is used to draw the selected, pressed text.
        /*!
        \return color for selected, pressed text
        */
        MMSFBColor getSelColor_p();



        //! Check if the color_i is set. This color will be used for the unselected, inactive text.
        bool isColor_i();

        //! Set the color which is used to draw the unselected, inactive text.
        /*!
        \param color_i  color for unselected, inactive text
        */
        void setColor_i(MMSFBColor color_i);

        //! Mark the color_i as not set.
        void unsetColor_i();

        //! Get the color which is used to draw the unselected, inactive text.
        /*!
        \return color for unselected, inactive text
        */
        MMSFBColor getColor_i();

        //! Check if the color is set. This color will be used for the selected, inactive text.
        bool isSelColor_i();

        //! Set the color which is used to draw the selected, inactive text.
        /*!
        \param selcolor_i  color for selected, inactive text
        */
        void setSelColor_i(MMSFBColor selcolor_i);

        //! Mark the color_i as not set.
        void unsetSelColor_i();

        //! Get the color which is used to draw the selected, inactive text.
        /*!
        \return color for selected, inactive text
        */
        MMSFBColor getSelColor_i();


        //! Check if the text is set.
        bool isText();

        //! Set the text to be drawn.
        /*!
        \param text  pointer to any text string
        */
        void setText(string *text);

        //! Set the text to be drawn.
        /*!
        \param text  any text string
        */
        void setText(string text);

        //! Mark the text as not set.
        void unsetText();

        //! Get the current text.
        /*!
        \return text string
        */
        string getText();



        bool isShadowColor(MMSPOSITION position);
        void setShadowColor(MMSPOSITION position, MMSFBColor color);
        void unsetShadowColor(MMSPOSITION position);
        void unsetShadowColors();
        MMSFBColor getShadowColor(MMSPOSITION position);

        bool isSelShadowColor(MMSPOSITION position);
        void setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor);
        void unsetSelShadowColor(MMSPOSITION position);
        void unsetSelShadowColors();
        MMSFBColor getSelShadowColor(MMSPOSITION position);


    // friends
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSTEXTBASECLASS_H_*/
