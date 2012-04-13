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

#include "mmstools/mmsdbconnmgr.h"

#ifdef __ENABLE_SQLITE__
#include "mmstools/mmsdbsqlite.h"
#endif
#ifdef __ENABLE_MYSQL__
#include "mmstools/mmsdbmysql.h"
#endif
#ifdef __ENABLE_FREETDS__
#include "mmstools/mmsdbfreetds.h"
#endif

MMSDBConnMgr::MMSDBConnMgr(DataSource *datasource) {
	this->datasource = datasource;
}

IMMSDB *MMSDBConnMgr::getConnection() {
	#ifdef __ENABLE_SQLITE__
		if((datasource->getDBMS()==DBMS_SQLITE3) || datasource->getDBMS()=="")
			return new MMSDBSQLite(datasource);
	#endif

	#ifdef __ENABLE_MYSQL__
		if(datasource->getDBMS()==DBMS_MYSQL)
			return new MMSDBMySQL(datasource);
	#endif

	#ifdef __ENABLE_FREETDS__
		if(datasource->getDBMS()==DBMS_FREETDS)
			return new MMSDBFreeTDS(datasource);
	#endif

	return NULL;

}

