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

#ifndef MMSTRANSLATOR_H_
#define MMSTRANSLATOR_H_

#include <map>
#include <string>
#include <sigc++/sigc++.h>

#include "mmsconfig/mmsconfigdata.h"

#define TRANSLATION_FILE_NAME "translation"

class MMSTranslator {
	private:
		/**
		 * This maps country codes (i.e. de for germany) to
		 * an index for vectors in MMSTRANSLATION_FILES and
		 * MMSTRANSLATION_MAP.
		 */
		//typedef std::map<std::string, int> MMSTRANSLATION_INDEX;
		typedef std::map<MMSLanguage, int> MMSTRANSLATION_INDEX;

		/**
		 * Vector containing all processed translation files.
		 */
		typedef std::vector<std::string> MMSTRANSLATION_FILES;

		/**
		 * Map that associates a string to translate to a vector
		 * containing the translation of all processed languages.
		 */
		typedef std::map<std::string, vector<std::string> > MMSTRANSLATION_MAP;

		//! first time MMSTranslator will be constructed?
		static bool 				firsttime;

		static MMSLanguage 			sourcelang, targetlang;
    	static int 					sourceIdx, targetIdx;
    	static MMSTRANSLATION_INDEX transIdx;
    	static MMSTRANSLATION_MAP 	transmap;
    	static bool 				addtranslations;
    	static MMSTRANSLATION_FILES files;

    	void loadTranslations();
    	void processFile(const string &file);
    	void addMissing(const string &phrase, const bool completemiss = false);

	public:
		MMSTranslator();
		~MMSTranslator();

		void translate(const std::string &source, std::string &dest);
		bool setTargetLang(MMSLanguage lang);
		MMSLanguage getTargetLang();

        static sigc::signal<void, MMSLanguage> onTargetLangChanged;
};

#endif /* MMSTRANSLATOR_H_ */
