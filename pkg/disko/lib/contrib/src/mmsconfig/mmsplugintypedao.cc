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

#include "mmsconfig/mmsplugintypedao.h"
#include <stdlib.h>

MMSPluginTypeDAO::MMSPluginTypeDAO(IMMSDB *myConnection) {
    MMSPluginTypeDAO::setMMSDBConnection(myConnection);
}

IMMSDB *MMSPluginTypeDAO::getMMSDBConnection() {
    return this->dbConnection;
}

void MMSPluginTypeDAO::setMMSDBConnection(IMMSDB *connection) {
    this->dbConnection = connection;
}

MMSPluginTypeData *MMSPluginTypeDAO::findTypeByName(string name) {
    MMSRecordSet            rs;
    MMSPluginTypeData   *type;
    /* do query */
    this->getMMSDBConnection()->query(
        "select * from PluginTypes where PluginTypeName = '" + name + "'", &rs);

    /* check if result is empty */
    if (rs.getCount()==0)
        return NULL;

    /* create object */
    type = new MMSPluginTypeData();
    type->setID(atoi(rs["ID"].c_str()));
    type->setName(rs["PluginTypeName"]);

    return type;
}

vector<MMSPluginTypeData *> MMSPluginTypeDAO::findAllTypes() {
    MMSRecordSet            rs;
    vector<MMSPluginTypeData   *> types;
    MMSPluginTypeData   *type;

    /* do query */
    this->getMMSDBConnection()->query(
        "select * from PluginTypes", &rs);

    /* check if result is empty */
    if (rs.getCount()==0)
        return types;

    /* create object */
    do {

        type = new MMSPluginTypeData;
        type->setID(atoi(rs["ID"].c_str()));
        type->setName(rs["PluginTypeName"]);
        types.push_back(type);

    } while (rs.next());

    return types;

}
