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
 * @file  mmsdbfreetds.h
 *
 * @brief Header file for freetds database functions.
 *
 * @author Stefan Schwarzer <stefan.schwarzer@diskohq.org>
 * @author Guido Madaus     <guido.madaus@diskohq.org>
 * @author Jens Schneider   <pupeider@gmx.de>
 *
 * @ingroup mmstools
 */
#ifndef MMSDBFREETDS_H_
#define MMSDBFREETDS_H_

#ifdef __ENABLE_FREETDS__

#include "mmstools/base.h"
#include "mmstools/mmserror.h"
#include "mmstools/mmsrecordset.h"
#include "mmstools/datasource.h"
#include "mmstools/interfaces/immsdb.h"

#include <sql.h>
#include <map>
#include <sqlext.h>

#define DIAG_TYPE_ENV   1
#define DIAG_TYPE_DBC   2
#define DIAG_TYPE_STMT  3

#define FREETDS_SP_EXEC_CMD		"CALL "

class MMSDBFreeTDS : public IMMSDB {
    private:
        string  	dbname;
        SQLHDBC 	*dbhandle;
        SQLHENV 	henv;
   	 	SQLHSTMT	hstmt;
   	 	DataSource  *datasource;
        bool    	connected;

    public:
    	MMSDBFreeTDS(DataSource *datasource);
        virtual ~MMSDBFreeTDS();

        void connect();
        void disconnect();
        void startTransaction() {};
        void commitTransaction() {};
        void rollbackTransaction() {};
        string getDBName();
        int query(string statement, MMSRecordSet *rs);
        int query(string statement);
        int executeSP(string spName, MMSDB_SP_ARGLIST argList, MMSRecordSet *rs);
        int executeSP(string spName, MMSDB_SP_ARGLIST argList);
        int getLastInsertedID();
};

#endif /* __ENABLE_FREETDS__ */

#endif /*MMSDBFREETDS_H_*/
