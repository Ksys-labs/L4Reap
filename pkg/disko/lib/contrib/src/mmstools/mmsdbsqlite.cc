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
 * @file  mmsdbsqlite.cpp
 *
 * @brief Source file for sqlite3 database functions.
 *
 * @author Stefan Schwarzer <stefan.schwarzer@diskohq.org>
 * @author Guido Madaus     <guido.madaus@diskohq.org>
 * @author Jens Schneider   <pupeider@gmx.de>
 *
 * @ingroup mmstools
 */
#ifdef __ENABLE_SQLITE__

#include "mmstools/mmsdbsqlite.h"
#include "mmstools/mmslogger.h"

/**
 * @brief ????
 *
 */
MMSDBSQLite::MMSDBSQLite(DataSource *_datasource) : IMMSDB(_datasource) {
	if(!this->datasource)
		throw MMSError(0, "Cannot instantiate MMSDBSQLite without datasource");
};

/**
 * @brief ????
 *
 */
MMSDBSQLite::~MMSDBSQLite() {
    this->disconnect();
}

/**
 * @brief ????
 *
 */
int MMSDBSQLite::getResults(void *rs, int numCols, char **results, char **columnNames)
{
    int             i=0;
    MMSRecordSet   *myrs;

    myrs = (MMSRecordSet *) rs;
    myrs->addRow();
    for(i=0; i < numCols; i++)
    {
        if(results[i] != NULL)
            (*myrs)[columnNames[i]]=results[i];
    }

    return 0;
}

/**
 * @brief ????
 *
 */
void MMSDBSQLite::startTransaction() {
    int     rc=0;
    char    *errmsg=NULL;

    //open transaction
    if((rc = sqlite3_exec((sqlite3 *)this->dbhandle, "BEGIN TRANSACTION", NULL, NULL, &errmsg)) != SQLITE_OK)
    {
        throw MMSError(rc, errmsg);
    }

    return;
}

/**
 * @brief ????
 *
 */
void MMSDBSQLite::commitTransaction() {
    int     rc=0;
    char    *errmsg=NULL;

    //open transaction
    if((rc = sqlite3_exec((sqlite3 *)this->dbhandle, "COMMIT", NULL, NULL, &errmsg)) != SQLITE_OK)
    {
        throw MMSError(rc, errmsg);
    }

    return;
}

/**
 * @brief ????
 *
 */
void MMSDBSQLite::rollbackTransaction() {
    int     rc=0;
    char    *errmsg=NULL;

    //open transaction
    if((rc = sqlite3_exec((sqlite3 *)this->dbhandle, "ROLLBACK", NULL, NULL, &errmsg)) != SQLITE_OK)
    {
        throw MMSError(rc, errmsg);
    }

    return;
}

/**
 * @brief Opens connection to database.
 *
 * @param datasource DataSource object which contains required information for database
 *
 * @return void
 */
void MMSDBSQLite::connect() {
   int     rc=0;

   // Open database connection
   if((rc = sqlite3_open(datasource->getDatabaseName().c_str(), &this->dbhandle)) != SQLITE_OK)
   {
       this->disconnect();
       string err = sqlite3_errmsg(dbhandle);
       sqlite3_close(this->dbhandle);
       throw MMSError(rc, err);
   }

   connected = true;

   return;
}

/**
 * @brief Close connection to database.
 *
 * @return void
 */
void MMSDBSQLite::disconnect() {

    if(connected) {
        sqlite3_close((sqlite3 *)dbhandle);
        this->connected = false;
    }

    this->dbname = "";

    return;
}

/**
 * @brief This function executes given database query and puts the results in MMSRecordSet.
 *        This method is used for select statements
 *
 * @param statement buffer with database query
 *
 * @return Returns the number of affected rows
 */
int MMSDBSQLite::query(string statement, MMSRecordSet *rs) {
    int     rc=0;
    char    *errmsg=NULL;
    string 	message;

    rs->reset();

    if(!this->connected) {
    	message = "Query called but no connection established." + string(" [query was: ") + statement + string("]");
        throw MMSError(rc, message);
    }

    if((rc = sqlite3_exec((sqlite3 *)dbhandle, statement.c_str(), &(this->getResults), (void *) rs, &errmsg)) != SQLITE_OK)
    {
        message = string(errmsg) + string(" [query was: ") + statement + string("]");
        sqlite3_free(errmsg);
        throw MMSError(rc, message);
    }

    //rewind
    rs->setRecordNum(0);

    return (sqlite3_changes((sqlite3 *)dbhandle));
}

/**
 * @brief This function executes given database query.
 *        This method is used for insert, update and delete statements
 *
 * @param statement buffer with database query
 *
 * @return Returns the number of affected rows
 */
int MMSDBSQLite::query(string statement) {

    int     rc=0;
    char    *errmsg=NULL;
    string 	message;

    if(!this->connected) {
    	message = "Query called but no connection established." + string(" [query was: ") + statement + string("]");
        throw MMSError(rc, message);
    }

    if((rc = sqlite3_exec((sqlite3 *)dbhandle, statement.c_str(), NULL, NULL, &errmsg)) != SQLITE_OK)
    {
        message = string(errmsg) + string(" [query was: ") + statement + string("]");
        sqlite3_free(errmsg);
        throw MMSError(rc, message);
    }

    // return the number of affected rows
    return (sqlite3_changes((sqlite3 *)dbhandle));
}

/**
 * @brief Returns the ID of the last inserted record
 *
 * @return Returns the ID of the last inserted record
 */
int MMSDBSQLite::getLastInsertedID() {
    return sqlite3_last_insert_rowid((sqlite3 *)this->dbhandle);
}

#endif /*__ENABLE_SQLITE__*/
