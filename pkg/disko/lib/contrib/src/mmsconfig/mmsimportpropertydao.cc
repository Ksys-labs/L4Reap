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

#include "mmsconfig/mmsimportpropertydao.h"
#include "mmstools/tools.h"
#include <stdlib.h>

MMSImportPropertyDAO::MMSImportPropertyDAO(IMMSDB *connection) {
    setMMSDBConnection(connection);
}

void MMSImportPropertyDAO::setMMSDBConnection(IMMSDB *connection) {
    this->dbConnection = connection;
}

IMMSDB *MMSImportPropertyDAO::getMMSDBConnection() {
    return this->dbConnection;
}

void MMSImportPropertyDAO::save(MMSImportPropertyData *data) {
    /* do the insert */
    this->getMMSDBConnection()->query(
        "insert into ImportProperties(PluginID,onStartUp,Time,Interval) values('"
        + iToStr(data->getPluginId()) + "','"
        + ((data->getOnStartUp())?"Y":"N") + "','"
        + iToStr(data->getTime()) + "','"
        + iToStr(data->getInterval()) + "')");
/*TODO:return over stack!!!*/
    /* set the ID */
    data->setId(this->getMMSDBConnection()->getLastInsertedID());
}

void MMSImportPropertyDAO::update(MMSImportPropertyData *data) {
    /* do the update */
    this->getMMSDBConnection()->query(
        "update ImportProperties set Time='" + iToStr(data->getTime()) + "',"
        + "onStartUp='" + ((data->getOnStartUp())?"Y":"N") + "',"
        + "Interval='" + iToStr(data->getInterval()) + "' "
        "where ID = '" + iToStr(data->getId()) + "'");
}

void MMSImportPropertyDAO::saveOrUpdate(MMSImportPropertyData *data) {
    /* check if ID is set */
    if (data->getId()<0)
        /* no, have to save */
        save(data);
    else
        /* yes, have to update */
        update(data);
}

MMSImportPropertyData *MMSImportPropertyDAO::moveRecordToData(MMSRecordSet &rs) {
    MMSImportPropertyData *data = new MMSImportPropertyData();

    data->setId(atoi(rs["ID"].c_str()));
    data->setPluginId(atoi(rs["PluginID"].c_str()));
    data->setOnStartUp((rs["onStartUp"]=="Y"));
    data->setTime(atoi(rs["Time"].c_str()));
    data->setInterval(atoi(rs["Interval"].c_str()));

    return data;
}

MMSImportPropertyData *MMSImportPropertyDAO::findImportPropertyByPlugin(MMSPluginData *plugin) {
    MMSRecordSet    rs;

    this->getMMSDBConnection()->query(
        "select * from ImportProperties where PluginID = " + iToStr(plugin->getId()),
        &rs);

    /* check if result is empty */
    if (rs.getCount()==0)
        throw MMSImportPropertyDAOError(0,"ImportProperties for PluginID " + iToStr(plugin->getId()) + " not found");

    /* fill the return data */
    return moveRecordToData(rs);
}

