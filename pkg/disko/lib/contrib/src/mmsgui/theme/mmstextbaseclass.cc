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

#include "mmsgui/theme/mmstextbaseclass.h"
#include <string.h>

MMSTextBaseClass::MMSTextBaseClass() {
    unsetAll();
}

void MMSTextBaseClass::unsetAll() {
    unsetFontPath();
    unsetFontSize();
    unsetFontNames();
    unsetAlignment();
    unsetColor();
    unsetSelColor();
    unsetColor_p();
    unsetSelColor_p();
    unsetColor_i();
    unsetSelColor_i();
    unsetText();
    unsetShadowColors();
    unsetSelShadowColors();
}


bool MMSTextBaseClass::isFontPath() {
    return this->isfontpath;
}

void MMSTextBaseClass::setFontPath(string fontpath) {
    this->fontpath = fontpath;
    this->isfontpath = true;
}

void MMSTextBaseClass::unsetFontPath() {
    this->isfontpath = false;
}

string MMSTextBaseClass::getFontPath() {
    return this->fontpath;
}

bool MMSTextBaseClass::isFontSize() {
    return this->isfontsize;
}

void MMSTextBaseClass::setFontSize(unsigned int fontsize) {
    this->fontsize = fontsize;
    this->isfontsize = true;
}

void MMSTextBaseClass::unsetFontSize() {
    this->isfontsize = false;
}

unsigned int MMSTextBaseClass::getFontSize() {
    return this->fontsize;
}


bool MMSTextBaseClass::isFontName(MMSLanguage lang) {
	if (lang < MMSLANG_NONE || lang >= MMSLANG_SIZE) return false;
    return this->fontname[lang].isfontname;
}

void MMSTextBaseClass::setFontName(string fontname, MMSLanguage lang) {
	if (lang < MMSLANG_NONE || lang >= MMSLANG_SIZE) return;
	this->fontname[lang].fontname = fontname;
	this->fontname[lang].isfontname = true;
}

void MMSTextBaseClass::unsetFontName(MMSLanguage lang) {
	if (lang < MMSLANG_NONE || lang >= MMSLANG_SIZE) return;
	this->fontname[lang].isfontname = false;
}

void MMSTextBaseClass::unsetFontNames() {
	for (unsigned int i = MMSLANG_NONE; i < MMSLANG_SIZE; i++) {
		unsetFontName((MMSLanguage)i);
	}
}

string MMSTextBaseClass::getFontName(MMSLanguage lang) {
	if (lang < MMSLANG_NONE || lang >= MMSLANG_SIZE) return "";
	return this->fontname[lang].fontname;
}


bool MMSTextBaseClass::isAlignment() {
    return this->isalignment;
}

void MMSTextBaseClass::setAlignment(MMSALIGNMENT alignment) {
    this->alignment = alignment;
    this->isalignment = true;
}

void MMSTextBaseClass::unsetAlignment() {
    this->isalignment = false;
}

MMSALIGNMENT MMSTextBaseClass::getAlignment() {
    return this->alignment;
}

bool MMSTextBaseClass::isColor() {
    return this->iscolor;
}

void MMSTextBaseClass::setColor(MMSFBColor color) {
    this->color = color;
    this->iscolor = true;
}

void MMSTextBaseClass::unsetColor() {
    this->iscolor = false;
}

MMSFBColor MMSTextBaseClass::getColor() {
    return this->color;
}

bool MMSTextBaseClass::isSelColor() {
    return this->isselcolor;
}

void MMSTextBaseClass::setSelColor(MMSFBColor selcolor) {
    this->selcolor = selcolor;
    this->isselcolor = true;
}

void MMSTextBaseClass::unsetSelColor() {
    this->isselcolor = false;
}

MMSFBColor MMSTextBaseClass::getSelColor() {
    return this->selcolor;
}


bool MMSTextBaseClass::isColor_p() {
    return this->iscolor_p;
}

void MMSTextBaseClass::setColor_p(MMSFBColor color_p) {
    this->color_p = color_p;
    this->iscolor_p = true;
}

void MMSTextBaseClass::unsetColor_p() {
    this->iscolor_p = false;
}

MMSFBColor MMSTextBaseClass::getColor_p() {
    return this->color_p;
}

bool MMSTextBaseClass::isSelColor_p() {
    return this->isselcolor_p;
}

void MMSTextBaseClass::setSelColor_p(MMSFBColor selcolor_p) {
    this->selcolor_p = selcolor_p;
    this->isselcolor_p = true;
}

void MMSTextBaseClass::unsetSelColor_p() {
    this->isselcolor_p = false;
}

MMSFBColor MMSTextBaseClass::getSelColor_p() {
    return this->selcolor_p;
}


bool MMSTextBaseClass::isColor_i() {
    return this->iscolor_i;
}

void MMSTextBaseClass::setColor_i(MMSFBColor color_i) {
    this->color_i = color_i;
    this->iscolor_i = true;
}

void MMSTextBaseClass::unsetColor_i() {
    this->iscolor_i = false;
}

MMSFBColor MMSTextBaseClass::getColor_i() {
    return this->color_i;
}

bool MMSTextBaseClass::isSelColor_i() {
    return this->isselcolor_i;
}

void MMSTextBaseClass::setSelColor_i(MMSFBColor selcolor_i) {
    this->selcolor_i = selcolor_i;
    this->isselcolor_i = true;
}

void MMSTextBaseClass::unsetSelColor_i() {
    this->isselcolor_i = false;
}

MMSFBColor MMSTextBaseClass::getSelColor_i() {
    return this->selcolor_i;
}



bool MMSTextBaseClass::isText() {
    return this->istext;
}

void MMSTextBaseClass::setText(string *text) {
    this->text = *text;
    this->istext = true;
}

void MMSTextBaseClass::setText(string text) {
    setText(&text);
}

void MMSTextBaseClass::unsetText() {
    this->istext = false;
}

string MMSTextBaseClass::getText() {
    return this->text;
}



bool MMSTextBaseClass::isShadowColor(MMSPOSITION position) {
    return this->shadow[position].iscolor;
}

void MMSTextBaseClass::setShadowColor(MMSPOSITION position, MMSFBColor color) {
	this->shadow[position].color = color;
	this->shadow[position].iscolor = true;
}

void MMSTextBaseClass::unsetShadowColor(MMSPOSITION position) {
	this->shadow[position].iscolor = false;
}

void MMSTextBaseClass::unsetShadowColors() {
	for (int i = 0; i < MMSPOSITION_SIZE; i++) {
		unsetShadowColor((MMSPOSITION)i);
	}
}

MMSFBColor MMSTextBaseClass::getShadowColor(MMSPOSITION position) {
    return this->shadow[position].color;
}

bool MMSTextBaseClass::isSelShadowColor(MMSPOSITION position) {
    return this->shadow[position].isselcolor;
}

void MMSTextBaseClass::setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor) {
	this->shadow[position].selcolor = selcolor;
	this->shadow[position].isselcolor = true;
}

void MMSTextBaseClass::unsetSelShadowColor(MMSPOSITION position) {
	this->shadow[position].isselcolor = false;
}

void MMSTextBaseClass::unsetSelShadowColors() {
	for (int i = 0; i < MMSPOSITION_SIZE; i++) {
		unsetSelShadowColor((MMSPOSITION)i);
	}
}

MMSFBColor MMSTextBaseClass::getSelShadowColor(MMSPOSITION position) {
    return this->shadow[position].selcolor;
}



