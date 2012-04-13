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

#include "mmsconfig/mmsimportsourcedao.h"
#include "mmstools/tools.h"
#include <stdlib.h>

MMSImportSourceDAO::MMSImportSourceDAO(IMMSDB *connection) {
    setMMSDBConnection(connection);
}

void MMSImportSourceDAO::setMMSDBConnection(IMMSDB *connection) {
    this->dbConnection = connection;
}

IMMSDB *MMSImportSourceDAO::getMMSDBConnection() {
    return this->dbConnection;
}

void MMSImportSourceDAO::deleteImportSource(MMSImportSourceData *source) {
    return;
}

void MMSImportSourceDAO::save(MMSImportSourceData *data) {
    /* do the insert */
    this->getMMSDBConnection()->query(
        "insert into ImportSource(PluginID,Name,Source,LifeTime) values('"
        + iToStr(data->getPluginId()) + "','"
        + data->getName() + "','"
        + data->getSource() + "','"
        + iToStr(data->getLifeTime()) + "')");
/*TODO:return over stack!!!*/
    /* set the ID */
    data->setId(this->getMMSDBConnection()->getLastInsertedID());
}

void MMSImportSourceDAO::update(MMSImportSourceData *data) {
    /* do the update */
    this->getMMSDBConnection()->query(
        "update ImportSource set Name='" + data->getName() + "',"
        + "Source='" + data->getSource() + "',"
        + "LifeTime='" + iToStr(data->getLifeTime()) + "' "
        "where ID = '" + iToStr(data->getId()) + "'");
}

void MMSImportSourceDAO::saveOrUpdate(MMSImportSourceData *data) {
    /* check if ID is set */
    if (data->getId()<0)
        /* no, have to save */
        save(data);
    else
        /* yes, have to update */
        update(data);
}

void MMSImportSourceDAO::saveOrUpdate(vector<MMSImportSourceData *> dataList) {
    /* for each item */
    for (unsigned i=0;i<dataList.size();i++) {
        /* check if ID is set */
        if (dataList.at(i)->getId()<0)
            /* no, have to save */
            save(dataList.at(i));
        else
            /* yes, have to update */
            update(dataList.at(i));
    }
}

MMSImportSourceData *MMSImportSourceDAO::moveRecordToData(MMSRecordSet &rs) {
    MMSImportSourceData *data = new MMSImportSourceData();

    data->setId(atoi(rs["ID"].c_str()));
    data->setPluginId(atoi(rs["PluginID"].c_str()));
    data->setName(rs["Name"]);
    data->setSource(rs["Source"]);
    data->setLifeTime(atoi(rs["LifeTime"].c_str()));

    return data;
}

vector<MMSImportSourceData *> MMSImportSourceDAO::findImportSourcesByPlugin(MMSPluginData *plugin) {
    vector<MMSImportSourceData *>   sourceList;
    MMSRecordSet                    rs;

    /* do query */
    this->getMMSDBConnection()->query(
        "select * from ImportSource where PluginID = " + iToStr(plugin->getId()), &rs);

    /* check if result is empty */
    if (rs.getCount()==0) return sourceList;

    /* for each result record */
    do {
        /* set the values */
        MMSImportSourceData *source = new MMSImportSourceData;
        source = moveRecordToData(rs);

        /* push to list */
        sourceList.push_back(source);
    } while(rs.next() == true);

    return sourceList;
}
MMSImportSourceData *   MMSImportSourceDAO::findImportSourcesByID(int id) {
    MMSRecordSet                    rs;

    /* do query */
    this->getMMSDBConnection()->query(
        "select * from ImportSource where ID = " + iToStr(id),&rs);

    /* check if result is empty */
    if (rs.getCount()==0) return NULL;

    MMSImportSourceData *source = moveRecordToData(rs);


    return source;

}

MMSImportSourceData *   MMSImportSourceDAO::findImportSourcesByName(string name) {
    MMSRecordSet                    rs;

    /* do query */
    this->getMMSDBConnection()->query(
        "select * from ImportSource where Name = '" + name  + "'",&rs);

    /* check if result is empty */
    if (rs.getCount()==0) return NULL;

    MMSImportSourceData *source = moveRecordToData(rs);


    return source;
}
