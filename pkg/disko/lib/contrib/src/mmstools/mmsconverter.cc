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

#include "mmstools/mmsconverter.h"
#include "mmstools/tools.h"
#include <errno.h>
#include <string.h>

map<string, iconv_t> MMSConverter::trans;

MMSConverter::MMSConverter(string codepage) {
	map<string, iconv_t>::iterator it;
	iconv_t newtrans;

	it = trans.find(codepage);
	if(it==trans.end()) {
		// TODO: memory is never freed by calling iconv_close()
		newtrans = iconv_open("UTF-8",codepage.c_str());
		if(newtrans !=(iconv_t)(-1)) {
			trans.insert(make_pair(codepage, newtrans));
		}else {
			throw MMSConverterError(0,"cannot create translation descriptor");
		}
	}

}

MMSConverter::~MMSConverter() {

}

string MMSConverter::convert(string frompage, string buffer) {
	map<string, iconv_t>::iterator it;
	char *lineptr;
	char *retlineptr;
	char line[32000];
	char retline[32000];
	size_t sizein, sizeout, sizeret;
	mutex.lock();
	it=trans.find(frompage);
	if(it!=trans.end()) {
		sizeout=32000;
		retlineptr=retline;
		memset(line,0,32000);
		memset(retline,0,32000);
		sprintf(line,"%s",buffer.c_str());
		sizein = strlen(line);
		lineptr=line;
		//logger.writeLog("before sizein(" + iToStr(sizein) + ") sizeout(" + iToStr(sizeout) + ")");
		//logger.writeLog("string before: |BEGIN|" + string(line) + "|END|");
		sizeret=iconv(it->second,&lineptr, &sizein,&retlineptr,&sizeout);
		//logger.writeLog("before sizein(" + iToStr(sizein) + ") sizeout(" + iToStr(sizeout) + ") sizeret(" + iToStr(sizeret) + ") " + strerror(errno));
		//logger.writeLog("string after: |BEGIN|" + string(retline) + "|END|");
		mutex.unlock();
		return retline;
	} else {
			mutex.unlock();
			throw MMSConverterError(0,"have no translation descriptor");
	}
	mutex.unlock();
	return "";
}

MMSMutex MMSConverter::mutex;
