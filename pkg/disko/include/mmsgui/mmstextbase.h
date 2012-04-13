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

#ifndef MMSTEXTBASE_H_
#define MMSTEXTBASE_H_

#define MMSTEXTBASE_UPDATE_FROM_THEME_CLASS(widget, themeClass) \
    if (themeClass->isFontPath()) \
        widget->setFontPath(themeClass->getFontPath()); \
    for (unsigned int i = MMSLANG_NONE; i < MMSLANG_SIZE; i++) { \
    	if (themeClass->isFontName((MMSLanguage)i)) \
    		widget->setFontName(themeClass->getFontName((MMSLanguage)i), (MMSLanguage)i); \
    } \
    if (themeClass->isFontSize()) \
        widget->setFontSize(themeClass->getFontSize()); \
    for (int position = 0; position < MMSPOSITION_SIZE; position++) { \
		if (themeClass->isShadowColor((MMSPOSITION)position)) \
			widget->setShadowColor((MMSPOSITION)position, themeClass->getShadowColor((MMSPOSITION)position)); \
		if (themeClass->isSelShadowColor((MMSPOSITION)position)) \
			widget->setSelShadowColor((MMSPOSITION)position, themeClass->getSelShadowColor((MMSPOSITION)position)); \
    } \
    if (themeClass->isAlignment()) \
        widget->setAlignment(themeClass->getAlignment()); \
    if (themeClass->isColor()) \
        widget->setColor(themeClass->getColor()); \
    if (themeClass->isSelColor()) \
        widget->setSelColor(themeClass->getSelColor()); \
    if (themeClass->isColor_p()) \
        widget->setColor_p(themeClass->getColor_p()); \
    if (themeClass->isSelColor_p()) \
        widget->setSelColor_p(themeClass->getSelColor_p()); \
	if (themeClass->isColor_i()) \
		widget->setColor_i(themeClass->getColor_i()); \
	if (themeClass->isSelColor_i()) \
		widget->setSelColor_i(themeClass->getSelColor_i()); \
	if (themeClass->isText()) \
        widget->setText(themeClass->getText());

#endif /*MMSTEXTBASE_H_*/
