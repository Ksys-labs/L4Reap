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

#include "mmscore/mmstranslator.h"
#include "mmsconfig/mmsconfigdata.h"
#include "mmsconfig/mmspluginservice.h"
#include "mmstools/tools.h"
#include "mmstools/mmsfile.h"
#include "mmstools/mmsfilesearch.h"

#include <string.h>
#include <stdexcept>

bool MMSTranslator::firsttime = true;
MMSLanguage MMSTranslator::sourcelang = MMSLANG_NONE;
MMSLanguage MMSTranslator::targetlang = MMSLANG_NONE;
int MMSTranslator::sourceIdx = -1;
int MMSTranslator::targetIdx = -1;
MMSTranslator::MMSTRANSLATION_INDEX MMSTranslator::transIdx;
MMSTranslator::MMSTRANSLATION_MAP MMSTranslator::transmap;
MMSTranslator::MMSTRANSLATION_FILES MMSTranslator::files;

sigc::signal<void, MMSLanguage>  MMSTranslator::onTargetLangChanged;
bool MMSTranslator::addtranslations;

MMSTranslator::MMSTranslator() {
	if (this->firsttime) {
		MMSConfigData config;
		this->sourcelang = config.getSourceLang();
		this->targetlang = config.getDefaultTargetLang();
		this->addtranslations = config.getAddTranslations();
		this->firsttime = false;
	}

/*
QUESTION: sourcelang not used? why???
	if (this->sourcelang != MMSLANG_NONE && this->targetlang != MMSLANG_NONE)
		loadTranslations();
*/

	if (this->targetlang != MMSLANG_NONE)
		loadTranslations();
}

MMSTranslator::~MMSTranslator() {

}

void MMSTranslator::loadTranslations() {
	MMSConfigData config;
	DataSource source(config.getConfigDBDBMS(),
					  config.getConfigDBDatabase(),
					  config.getConfigDBAddress(),
					  config.getConfigDBPort(),
					  config.getConfigDBUser(),
					  config.getConfigDBPassword());

	try {
		MMSPluginService service(&source);
		vector<MMSPluginData *> data = service.getAllPlugins();

		for(vector<MMSPluginData *>::iterator it = data.begin();it!=data.end();it++) {
			MMSFileSearch search((*it)->getPath(), "translation.??", true);
			MMSFILEENTRY_LIST ret =  search.execute();
			for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
				processFile((*it2)->name);
			}
		}
	} catch (MMSError &err) {
		DEBUGMSG("MMSTranslator", "No plugins database found for translation.");
	}

	MMSFileSearch search(config.getLanguagefileDir(), "translation.??", false);
	MMSFILEENTRY_LIST ret =  search.execute();
	for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
		processFile((*it2)->name);
	}

//	this->sourceIdx = this->transIdx.find(this->sourcelang)->second;
	this->targetIdx = this->transIdx.find(this->targetlang)->second;
}

void MMSTranslator::addMissing(const string &phrase, const bool completemiss) {
	if(phrase.empty()) {
		return;
	}

	size_t size = this->files.size();

	if(completemiss) {
		//add to all language files;
		for(unsigned int idx = 0; idx < size; ++idx) {
			MMSFile file(this->files.at(idx), MMSFM_APPEND, false);
			char line[1024];
			snprintf(line, sizeof(line) - 4, "%s===\n", phrase.c_str());
			file.writeBuffer(line, NULL, strlen(line), 1);

			MMSTRANSLATION_MAP::iterator transit = this->transmap.find(phrase);
			if(transit != this->transmap.end()) {
				transit->second.at(idx) = phrase;
			} else {
				vector<string> trans(this->files.size());
				trans.at(idx) = phrase;
				transmap.insert(make_pair(phrase, trans));
			}
		}
	} else {
		//check the single languages...
		MMSTRANSLATION_MAP::iterator transit = this->transmap.find(phrase);
		for(unsigned int idx = 0; idx < size; ++idx) {
			if(transit->second.at(idx).empty()) {
				MMSFile file(this->files.at(idx), MMSFM_APPEND, false);
				char line[1024];
				snprintf(line, sizeof(line) - 4, "%s===\n", phrase.c_str());
				file.writeBuffer(line, NULL, strlen(line), 1);
				transit->second.at(idx) = phrase;
			}
		}
	}
}


void MMSTranslator::translate(const string &source, string &dest) {
	if((this->targetIdx == -1) || source.empty()) {
		dest = source;
		return;
	}

	MMSTRANSLATION_MAP::iterator it = this->transmap.find(source);
	if(it == this->transmap.end()) {
		dest = source;
		if(this->addtranslations) {
			addMissing(source, true);
		}
	} else {
		if(it->second.size() > this->targetIdx) {
			dest = it->second.at(this->targetIdx);
			if(dest.empty()) {
				dest = source;
			}
		} else {
			dest = source;
		}
	}

	size_t lookHere = 0;
    size_t foundHere;
    string from = "\\n";
    string to = "\n";
    while((foundHere = dest.find(from, lookHere)) != string::npos) {
		dest.replace(foundHere, from.size(), to);
		lookHere = foundHere + to.size();
    }

}

bool MMSTranslator::setTargetLang(MMSLanguage lang) {
	MMSTRANSLATION_INDEX::iterator it = this->transIdx.find(lang);
	if (it == this->transIdx.end())
		return false;

	this->targetlang = lang;
	this->targetIdx = it->second;
	this->onTargetLangChanged.emit(this->targetlang);

	return true;
}

MMSLanguage MMSTranslator::getTargetLang() {
	return this->targetlang;
}

void MMSTranslator::processFile(const string &file) {
	MMSFile transfile(file,MMSFM_READ,false);
	string line;
	string from, to;
	size_t idx;
	string countryCode = file.substr(file.find("translation")+12);
	MMSLanguage lang = getMMSLanguageFromString(strToUpr(countryCode));

	MMSTRANSLATION_INDEX::iterator it = this->transIdx.find(lang);
	if(it == this->transIdx.end()) {
		idx = this->files.size();
		this->transIdx[lang] = idx;
		this->files.push_back(file);
		for(MMSTRANSLATION_MAP::iterator it = this->transmap.begin(); it != this->transmap.end(); ++it) {
			it->second.resize(idx + 1, "");
		}
	} else {
		idx = it->second;
	}

	while(transfile.getLine(line)) {
		size_t pos = line.find("===");
		if(pos != string::npos) {
			from = line.substr(0, pos);
			to = line.substr(pos+3, string::npos);

			MMSTRANSLATION_MAP::iterator f = this->transmap.find(from);
			if(f != this->transmap.end()) {
				//already have the source
				DEBUGMSG("MMSTranslator", "insert: '%s'", from.c_str());
				try {
					f->second.at(idx) = to;
				} catch(std::exception &ex) {
					f->second.resize(idx + 1, "");
					f->second.at(idx) = to;
				}
			} else {
				DEBUGMSG("MMSTranslator", "fresh insert: '%s'", from.c_str());
				vector<string> trans(idx + 1);
				trans.at(idx) = to;
				transmap.insert(make_pair(from, trans));
			}
		}
	}
}
