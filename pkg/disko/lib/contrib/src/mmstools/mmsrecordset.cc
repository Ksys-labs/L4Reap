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

#include "mmstools/mmsrecordset.h"

MMSRecordSet::~MMSRecordSet() {
	for(vector< map<string, string>* >::iterator it = rows.begin();it!=rows.end();it++) {
		if(*it) delete *it;
		*it=NULL;
	}
    rows.clear();
}

void MMSRecordSet::addRow() {
    this->count++;
    this->recnum = this->count-1;
    rows.push_back(new map<string, string> );
}

bool MMSRecordSet::next() {
    if(this->recnum < this->count - 1) {
        this->recnum++;
        return true;
    }

    return false;
}

bool MMSRecordSet::previous() {
    if(this->recnum > 0) {
        this->recnum--;
        return true;
    }

    return false;
}

const unsigned int MMSRecordSet::getCount() const {
    return (unsigned int) this->count;
}

const string MMSRecordSet::getInfo() const {
    return this->info;
}

bool MMSRecordSet::setRecordNum(int num) {
    if(num < this->count) {
        this->recnum = num;
        return true;
    }

    return false;
}

const int MMSRecordSet::getRecordNum() const {
    return this->recnum;
}

static string defret = "";

string &MMSRecordSet::operator[](string key) {
	if(this->recnum == -1)
		return defret;

	map<string,string>::iterator found;
	found = rows.at(this->recnum)->find(key);
	if (found != rows.at(this->recnum)->end())
		return found->second;
	else {
		pair< map<string,string>::iterator, bool > ret = rows.at(this->recnum)->insert(make_pair(key,string("")));
		return ret.first->second;
	}
}

const string &MMSRecordSet::operator[](const string key) const {
	if(this->recnum == -1)
		return defret;

	map<string,string>::iterator found;
	found = rows.at(this->recnum)->find(key);
	if (found != rows.at(this->recnum)->end())
		return found->second;

	return defret;
}

bool MMSRecordSet::reset() {
	for(vector< map<string, string>* >::iterator it = rows.begin();it!=rows.end();it++) {
		if(*it) delete *it;
		*it=NULL;
	}

    rows.clear();

    this->recnum = -1;
    this->count  = 0;
    this->info   = "not specified";

    return true;
}
