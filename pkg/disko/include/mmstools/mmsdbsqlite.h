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

/**
 * @file  mmsdbsqlite.h
 *
 * @brief Header file for sqlite3 database functions.
 *
 * @author Stefan Schwarzer <stefan.schwarzer@diskohq.org>
 * @author Guido Madaus     <guido.madaus@diskohq.org>
 * @author Jens Schneider   <pupeider@gmx.de>
 *
 * @ingroup mmstools
 */
#ifndef MMSDBSQLITE3_H_
#define MMSDBSQLITE3_H_

#ifdef __ENABLE_SQLITE__

#include "mmstools/mmserror.h"
#include "mmstools/interfaces/immsdb.h"

#include <sqlite3.h>

class MMSDBSQLite : public IMMSDB {
    private:
        sqlite3    *dbhandle;
        static int getResults(void *rs, int numCols, char **results, char **columnNames);

    public:
    	MMSDBSQLite(DataSource *datasource = NULL);
        virtual ~MMSDBSQLite();

        void connect();
        void disconnect();
        void startTransaction();
        void commitTransaction();
        void rollbackTransaction();
        int query(string statement, MMSRecordSet *rs);
        int query(string statement);
        int getLastInsertedID();
};

#endif /*__ENABLE_SQLITE__*/

#endif /*MMSDBSQLITE3_H_*/
