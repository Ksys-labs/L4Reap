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

#include "mmstools/mmsdatetime.h"
#include "mmstools/tools.h"
#include <stdlib.h>

MMSDateTime::MMSDateTime() {
	MMSDateTime(time(NULL));
}

MMSDateTime::MMSDateTime(string timestr, string format) {
	struct tm mytime;
	if(format == "YYYY-MM-DD hh:mm:ss") {
		this->yearstr = timestr.substr(0,4);
		this->yearnum = strToInt(this->yearstr);
		this->monthnum = atoi(timestr.substr(5,2).c_str()) -1;
		this->daynum = atoi(timestr.substr(8,2).c_str());
		this->hour = atoi(timestr.substr(11,2).c_str());
		this->minute = atoi(timestr.substr(14,2).c_str());
		this->seconds = atoi(timestr.substr(17,2).c_str());
	}

	mytime.tm_mday = this->daynum;
	mytime.tm_mon = this->monthnum;
	mytime.tm_year = this->yearnum -1900;
	mytime.tm_hour = this->hour;
	mytime.tm_min = this->minute;
	mytime.tm_sec = this->seconds;

	this->timest = mktime(&mytime);
}

MMSDateTime::MMSDateTime(time_t stamp) {
	this->timest = stamp;
	struct tm mytime;


	localtime_r(&(this->timest),&mytime);
	this->daynum = mytime.tm_mday;
	this->monthnum = mytime.tm_mon;
	this->yearnum = 1900 + mytime.tm_year;
	this->dayofweek = mytime.tm_wday;
	this->hour = mytime.tm_hour;
	this->minute = mytime.tm_min;
	this->seconds = mytime.tm_sec;

}

MMSDateTime::~MMSDateTime() {

}

string &MMSDateTime::getDay() {
	return this->daystr;
}

string &MMSDateTime::getMonth() {
	return this->monthstr;
}

string &MMSDateTime::getYear() {
	return this->yearstr;
}

string &MMSDateTime::getDbDate() {
	this->dbdate = iToStr(this->yearnum) + "-"
			+ ((this->monthnum < 10) ? "0" : "") + iToStr(this->monthnum + 1) + "-"
			+ ((this->daynum < 10) ? "0" : "") + iToStr(this->daynum) + " "
			+ ((this->hour < 10) ? "0" : "") + iToStr(this->hour) + ":"
			+ ((this->minute < 10) ? "0" : "") + iToStr(this->minute) + ":"
			+ ((this->seconds < 10) ? "0" : "") + iToStr(this->seconds);

	return this->dbdate;
}

int	MMSDateTime::getDayNum() {
	return this->daynum;
}

int MMSDateTime::getMonthNum() {
	return this->monthnum;
}

int MMSDateTime::getYearNum() {
	return this->yearnum;
}
