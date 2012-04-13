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
 * @file  mmsdbmysql.cpp
 *
 * @brief Source file for mysql database functions.
 *
 * @author Stefan Schwarzer <stefan.schwarzer@diskohq.org>
 * @author Guido Madaus     <guido.madaus@diskohq.org>
 * @author Jens Schneider   <pupeider@gmx.de>
 * @author Matthias Hardt   <matthias.hardt@diskohq.org>
 *
 * @ingroup mmstools
 */
#ifdef __ENABLE_MYSQL__

#include <stdlib.h>
#include "mmstools/mmsdbmysql.h"
#include "mmstools/mmslogger.h"

/**
 * Constructor
 *
 * It just sets the datasource and the reconnect parameters
 * for later use in the connect() method.
 *
 * @param	_datasource		[in]	datasource object containing definitions for database connection
 * @param	autoreconnect 	[in]	if set to true, connection to the database server will be reestablished, if lost (needs mysql >= 5.0.13)
 *
 * @exception MMSError	No datasource given (Look at the exception message).
 *
 * @see MMSDBMySQL::~MMSDBMySQL()
 * @see MMSError::getMessage()
 */
MMSDBMySQL::MMSDBMySQL(DataSource *_datasource, bool autoreconnect) :
	IMMSDB(_datasource),
	autoreconnect(autoreconnect) {
	if(!this->datasource)
		throw MMSError(0, "Cannot instantiate MMSDBMySQL without datasource");
};

/**
 * Destructor
 *
 * Handles disconnection from the mysql database.
 *
 * @see MMSDBMySQL::MMSDBMySQL()
 */
MMSDBMySQL::~MMSDBMySQL() {
    this->disconnect();
}

/**
 * Starts a transaction.
 *
 * If you are using transaction based queries, call
 * this method to start a transaction. To commit it,
 * call commitTransaction().
 *
 * @see MMSDBMySQL::commitTransaction()
 * @see MMSError::getMessage()
 *
 * @exception MMSError	Error while trying to start transaction (Look at the exception message).
 */
void MMSDBMySQL::startTransaction() {
    if(mysql_query(&this->dbhandle, "START TRANSACTION") != 0)
        throw MMSError(0, mysql_error(&this->dbhandle));
}

/**
 * Commits a transaction.
 *
 * If you are using transaction based queries, call
 * this method to commit a transaction. To start it,
 * call startTransaction().
 *
 * @see MMSDBMySQL::startTransaction()
 * @see MMSError::getMessage()
 *
 * @exception MMSError	Error while trying to commit transaction (Look at the exception message).
 */
void MMSDBMySQL::commitTransaction() {
#if MYSQL_VERSION_ID >= 40100
    if(mysql_commit(&this->dbhandle) != 0)
#else
    if(mysql_query(&this->dbhandle, "COMMIT") != 0)
#endif
        throw MMSError(0, mysql_error(&this->dbhandle));
}

/**
 * Does a rollback on the current transaction.
 *
 * If you are using transaction based queries, call
 * this method to rollback a transaction.
 *
 * @see MMSDBMySQL::startTransaction()
 * @see MMSDBMySQL::commitTransaction()
 * @see MMSError::getMessage()
 *
 * @exception MMSError	Error while trying to rollback the transaction (Look at the exception message).
 */
void MMSDBMySQL::rollbackTransaction() {
#if MYSQL_VERSION_ID >= 40100
    if(mysql_rollback(&this->dbhandle) != 0)
#else
    if(mysql_query(&this->dbhandle, "ROLLBACK") != 0)
#endif
        throw MMSError(0, mysql_error(&this->dbhandle));
}

/**
 * Connects to the mysql database.
 *
 * @see MMSDBMySQL::disconnect()
 * @see MMSError::getMessage()
 *
 * @exception MMSError	Error while trying to connect (Look at the exception message).
 */
void MMSDBMySQL::connect() {
	mysql_init(&this->dbhandle);

#ifdef MYSQL_OPT_RECONNECT
#  if defined(MYSQL_VERSION_ID) && (MYSQL_VERSION_ID >= 50013)
	// reconnect?
	if(this->autoreconnect) {
		my_bool r = 1;
		mysql_options(&this->dbhandle, MYSQL_OPT_RECONNECT, &r);
	}
#  endif
#endif

	if(!mysql_real_connect(&this->dbhandle,
			               this->datasource->getAddress().c_str(),
			               this->datasource->getUser().c_str(),
			               this->datasource->getPassword().c_str(),
			               this->datasource->getDatabaseName().c_str(),
			               this->datasource->getPort(),
			               NULL, 0)) {
	       string err = mysql_error(&this->dbhandle);
	       mysql_close(&this->dbhandle);
	       throw MMSError(0, err);
	}

   this->connected = true;
}

/**
 * Disconnects from a mysql database.
 *
 * @see MMSDBMySQL::connect()
 */
void MMSDBMySQL::disconnect() {
    if(this->connected) {
        mysql_close(&this->dbhandle);
        this->connected = false;
    }

    this->dbname = "";

    return;
}

/**
 * @brief This function executes given database query and puts the results in MMSRecordSet.
 *        This method is used for select statements
 *
 * @param statement	[in]	buffer with database query
 * @param rs		[out]	MMSRecordSet that holds the results
 *
 * @see MMSRecordSet
 * @see MMSError::getMessage()
 *
 * @return Returns the number of affected rows
 *
 * @exception	MMSError	Query couldn't be executed (Look at the exception message).
 */
int MMSDBMySQL::query(string statement, MMSRecordSet *rs) {
	int     numRows = 0;
    string 	message;

    rs->reset();

    if(!this->connected) {
    	message = "Query called but no connection established." + string(" [query was: ") + statement + string("]");
        throw MMSError(0, message);
    }

    if(mysql_query(&this->dbhandle, statement.c_str()) != 0) {
        message = mysql_error(&this->dbhandle) + string(" [query was: ") + statement + string("]");
        throw MMSError(0, message);
    }

    // get results
    MYSQL_RES *results = mysql_use_result(&this->dbhandle);
    if(results) {
    	unsigned int count = mysql_num_fields(results);
    	if(count > 0) {
			MYSQL_FIELD *fields = mysql_fetch_fields(results);
			MYSQL_ROW row;
			while((row = mysql_fetch_row(results))) {
				rs->addRow();
				for(unsigned int i = 0; i < count; i++) {
					if(row[i])
						(*rs)[fields[i].name] = row[i];
				}
			}
    	}
    	numRows = mysql_num_rows(results);
    	mysql_free_result(results);
    }

    //rewind
    rs->setRecordNum(0);

    return numRows;
}

/**
 * @brief This function executes given database query.
 *        This method is used for insert, update and delete statements
 *
 * @param statement buffer with database query
 *
 * @see MMSError::getMessage()
 *
 * @return Returns the number of affected rows
 *
 * @exception	MMSError	Query couldn't be executed (Look at the exception message).
 */
int MMSDBMySQL::query(string statement) {
	int     numRows = 0;
    string 	message;

    if(!this->connected) {
    	message = "Query called but no connection established." + string(" [query was: ") + statement + string("]");
        throw MMSError(0, message);
    }

    if(mysql_query(&this->dbhandle, statement.c_str()) != 0) {
        message = mysql_error(&this->dbhandle) + string(" [query was: ") + statement + string("]");
        throw MMSError(0, message);
    }

    // fetch results if there are some
    MYSQL_RES *results = mysql_store_result(&this->dbhandle);
	if(results) {
		numRows = mysql_num_rows(results);
		mysql_free_result(results);
	}

    // return the number of affected rows
    return numRows;
}

/**
 * @brief Returns the ID of the last inserted record
 *
 * @return Returns the ID of the last inserted record
 *
 * @exception	MMSError	Couldn't fetch last ID (Look at the exception message).
 */
int MMSDBMySQL::getLastInsertedID() {
	int ret = 0;

    if(!this->connected)
        throw MMSError(0, "No connection established. Cannot fetch last inserted id.");

    if(mysql_query(&this->dbhandle, "SELECT LAST_INSERT_ID();") != 0) {
        string message = mysql_error(&this->dbhandle) + string(" [query was: SELECT LAST_INSERT_ID();]");
        throw MMSError(0, message);
    }

    // fetch results if there are some
    MYSQL_RES *results = mysql_store_result(&this->dbhandle);
	if(results) {
		MYSQL_ROW row = mysql_fetch_row(results);
		if(row)
			ret = atoi(row[0]);
		mysql_free_result(results);
	}

	return ret;
}

#endif /*__ENABLE_MYSQL__*/
