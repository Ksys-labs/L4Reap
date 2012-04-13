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

#include "mmsconfig/mmsimportpropertyservice.h"
#include "mmstools/interfaces/immsdb.h"
#include "mmstools/mmsdbconnmgr.h"
#include "mmsconfig/mmsimportpropertydao.h"

MMSImportPropertyService::MMSImportPropertyService(DataSource *datasource) :
    dbconn(NULL) {
    MMSDBConnMgr connMgr(datasource);
    if((this->dbconn = connMgr.getConnection()))
        this->dbconn->connect();
}

MMSImportPropertyService::~MMSImportPropertyService() {
	if(this->dbconn) {
		this->dbconn->disconnect();
		delete this->dbconn;
	}
}

void MMSImportPropertyService::setImportProperty(MMSImportPropertyData *data) {
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    myImportPropertyDAO.saveOrUpdate(data);
}

MMSImportPropertyData *MMSImportPropertyService::getImportPropertyByPlugin(MMSPluginData *plugin) {
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    MMSImportPropertyData *importProperty = new MMSImportPropertyData();
    importProperty = myImportPropertyDAO.findImportPropertyByPlugin(plugin);

    return importProperty;
}
