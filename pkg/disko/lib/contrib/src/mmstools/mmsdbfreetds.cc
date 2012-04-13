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
 * @file  mmsdbfreetds.cpp
 *
 * @brief Source file for freetds database functions.
 *
 * @author Stefan Schwarzer <stefan.schwarzer@diskohq.org>
 * @author Guido Madaus     <guido.madaus@diskohq.org>
 * @author Jens Schneider   <pupeider@gmx.de>
 *
 * @ingroup mmstools
 */
#ifdef __ENABLE_FREETDS__

#include "mmstools/mmsdbfreetds.h"
#include "mmstools/mmstools.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief ????
 *
 */
MMSDBFreeTDS::MMSDBFreeTDS(DataSource *_datasource) : IMMSDB(_datasource) {
	if(!this->datasource)
		throw MMSError(0, "Cannot instantiate MMSDBFreeTDS without datasource");
}

/**
 * @brief ????
 *
 */
MMSDBFreeTDS::~MMSDBFreeTDS() {
    this->disconnect();
}

/**
 * @brief Gets and creates a databases error message.
 *
 * @param rc return code of last called sql function
 * @param htype ???
 * @param handle database connection handle
 *
 * @return last error message.
 */
string errmsg(SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle) {
	MMSLogger 		logger("MMSFREETDS");

    SQLCHAR szSqlState[6],szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER  pfNativeError;
    SQLSMALLINT pcbErrorMsg;

    rc = SQLGetDiagRec(htype, handle,1,
                       (SQLCHAR *)&szSqlState,
                       (SQLINTEGER *)&pfNativeError,
                       (SQLCHAR *)&szErrorMsg,
                       SQL_MAX_MESSAGE_LENGTH-1,
                       (SQLSMALLINT *)&pcbErrorMsg);


    logger.writeLog(string((char*)szSqlState) + "|" + string((char*)szErrorMsg));

    return (string((char*)szSqlState) + "|" + string((char*)szErrorMsg));
}

/**
 * @brief Opens connection to database.
 *
 * @return void
 */
void MMSDBFreeTDS::connect() {
	int rc;
	char connection_string[256] = "";
	MMSLogger logger("MMSFREETDS");

	this->dbhandle = (SQLHDBC*)calloc(sizeof(SQLHDBC), 1);

	if((rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE, &this->henv)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

    if((rc = SQLSetEnvAttr(this->henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLSetEnvAttr() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

    if((rc = SQLAllocHandle(SQL_HANDLE_DBC, this->henv, this->dbhandle)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

    snprintf(connection_string,sizeof(connection_string),
		    "Server=%s;UID=%s;PWD=%s;Database=%s;Port=%d;TDS_Version=8.0;",
		    	datasource->getAddress().c_str(),
		    	datasource->getUser().c_str(),
		    	datasource->getPassword().c_str(),
		    	datasource->getDatabaseName().c_str(),
		    	datasource->getPort());

    logger.writeLog("try to connect to database: ");
    logger.writeLog(connection_string);

    if((rc = SQLDriverConnect(*(this->dbhandle), NULL, (SQLCHAR *)connection_string, SQL_NTS,
   	    (SQLCHAR *)connection_string,sizeof(connection_string),NULL, SQL_DRIVER_COMPLETE)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLDriverConnect() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

    logger.writeLog("connect to database successful");
	return;
}

/**
 * @brief Close connection to database.
 *
 * @return void
 */
void MMSDBFreeTDS::disconnect() {
	int rc;

	rc = SQLDisconnect(this->dbhandle);

	rc = SQLFreeConnect(this->dbhandle);

    return;
}

/**
 * @brief Returns the name of the connected database
 *
 * @return The name of the connected database
 */
string MMSDBFreeTDS::getDBName() {

    return this->dbname;
}

/**
 * @brief This function executes given database query and puts the results in MMSRecordSet.
 *        This method is used for select statements
 *
 * @param statement buffer with database query
 * @param rs		recordset containing result of query
 *
 * @return Returns the number of affected rows
 */
int MMSDBFreeTDS::query(string statement, MMSRecordSet *rs) {
	int 			rc=0;;
	SQLSMALLINT 	columns=0;
    MMSRecordSet   *myrs;
	MMSLogger 		logger("MMSFREETDS");

	myrs = (MMSRecordSet *) rs;
	myrs->reset();
	myrs->setRecordNum(0);

    if((rc = SQLAllocHandle(SQL_HANDLE_STMT, *this->dbhandle, &this->hstmt)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

	logger.writeLog(statement);

	// start a stored procedure
	rc = SQLExecDirect(this->hstmt, (SQLCHAR *) statement.c_str(), SQL_NTS);

	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        throw MMSError(rc, "Execution of query failed with rc " + iToStr(rc) + ". [" + errmsg(rc, DIAG_TYPE_STMT, this->hstmt) +"]");

	SQLNumResultCols(this->hstmt, &columns);

	logger.writeLog("Debug: Query returned " + iToStr(columns) + " columns");

    int ret, rows=0;

    while (SQL_SUCCEEDED(ret = SQLFetch(this->hstmt))) {
    	SQLUSMALLINT i;

        /* add a new row to resultset*/
    	myrs->addRow();
    	rows++;

    	/* Loop through the columns */
    	for (i = 1; i <= columns; i++) {
    		SQLLEN indicator;
    		char buf[512];
    		SQLCHAR         colname[32];
    		SQLSMALLINT     coltype;
    		SQLSMALLINT     colnamelen;
    		SQLSMALLINT     nullable;
    		SQLSMALLINT     scale;
    		SQLULEN     collen[1024];

    		SQLDescribeCol (this->hstmt, i, colname, sizeof (colname),
    		&colnamelen, &coltype, &collen[i], &scale, &nullable);

    		/* retrieve column data as a string */
    		ret = SQLGetData(hstmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
    		if (SQL_SUCCEEDED(ret)) {
    				/* Handle null columns */
    				if (indicator == SQL_NULL_DATA) strcpy(buf, "NULL");

    				(*myrs)[(char*)colname]=(char *)buf;
    		}
    	}
	}

    //rewind
    rs->setRecordNum(0);
    SQLCloseCursor(hstmt);

    logger.writeLog(iToStr(rows) + " rows returned.");
	return (rows);
}

/**
 * @brief This function executes given database query.
 *        This method is used for insert, update and delete statements
 *
 * @param statement buffer with database query
 *
 * @return Returns the number of affected rows
 */
int MMSDBFreeTDS::query(string statement) {
	int 		rc=0;;
	SQLSMALLINT columns=0;
	MMSLogger 	logger("MMSFREETDS");

	// start the query
    if((rc = SQLAllocHandle(SQL_HANDLE_STMT, *this->dbhandle, &this->hstmt)) != SQL_SUCCESS)
        throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");

	logger.writeLog(statement);
    // start a stored procedure
	if((rc = SQLExecDirect(this->hstmt, (SQLCHAR *) statement.c_str(), statement.length())) != SQL_SUCCESS)
        throw MMSError(rc, "Execution of query failed. [" + errmsg(rc, DIAG_TYPE_STMT, this->hstmt) +"]");

	SQLNumResultCols(this->hstmt, &columns);

	SQLCloseCursor(hstmt);

	return (columns);
}

/**
 * @brief Returns the ID of the last inserted record
 *
 * @return Returns the ID of the last inserted record
 */
int MMSDBFreeTDS::getLastInsertedID() {
    return (0);
}

/**
 * @brief This function executes given stored procedure and puts the results in MMSRecordSet.
 *        This method is used for insert, update and delete statements
 *
 * @param spName 	name of stored procedure
 * @param argList	arguments for stored procedure
 * @param rs		recordset containing result
 *
 * @return Returns the number of affected rows
 */
int MMSDBFreeTDS::executeSP(string spName, MMSDB_SP_ARGLIST argList, MMSRecordSet *rs) {
	string query;
	MMSDB_SP_ARGLIST::iterator it;

	//Build my query
	query="{" + string(FREETDS_SP_EXEC_CMD) + spName + " (";

	for(it = argList.begin();it!=argList.end();it++) {
		if(it!=argList.begin())
			query += ",";

		query += it->first + "=" + it->second;
	}

	query=")}";

	return(this->query(query, rs));
}

/**
 * @brief This function executes a stored procedure.
 *        This method is used for insert, update and delete statements
 *
 * @param spName 	name of stored procedure
 * @param argList	arguments for stored procedure
 *
 * @return Returns the number of affected rows
 */
int MMSDBFreeTDS::executeSP(string spName, MMSDB_SP_ARGLIST argList) {
	string query;
	MMSDB_SP_ARGLIST::iterator it;

	//Build my query
	query="{" + string(FREETDS_SP_EXEC_CMD) + spName + " (";

	for(it = argList.begin();it!=argList.end();it++) {
		if(it!=argList.begin())
			query += ",";

		query += it->first + "=" + it->second;
	}

	query=")}";

	return(this->query(query));
}

#endif /*__ENABLE_FREETDS__*/
