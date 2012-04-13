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

#include "mmstools/datasource.h"

DataSource::DataSource(const string _dbms,
    		           const string _dbName,
    			       const string _address,
    			       const unsigned int _port,
    			       const string _user,
    			       const string _password) :
    dbms(_dbms),
    address(_address),
    port(_port),
    dbName(_dbName),
    user(_user),
    password(_password) {

}

DataSource::DataSource(const DataSource& d) :
    dbms(d.dbms),
    address(d.address),
    port(d.port),
    dbName(d.dbName),
    user(d.user),
    password(d.password) {
}

DataSource::~DataSource() {

}


void DataSource::setDBMS(const string dbms) {
    this->dbms = dbms;
}

const string DataSource::getDBMS() {
    return this->dbms;
}

void DataSource::setAddress(const string address) {
    this->address = address;
}

const string DataSource::getAddress() {
    return this->address;
}

void DataSource::setPort(const unsigned int port) {
    this->port = port;
}

const unsigned int DataSource::getPort() {
    return this->port;
}

void DataSource::setDatabaseName(const string dbName) {
    this->dbName = dbName;
}

const string DataSource::getDatabaseName() {
    return this->dbName;
}

void DataSource::setUser(const string user) {
    this->user = user;
}

const string DataSource::getUser() {
    return this->user;
}

void DataSource::setPassword(const string password) {
    this->password = password;
}

const string DataSource::getPassword() {
    return this->password;
}
