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

#include "mmsconfig/mmsimportsourceservice.h"
#include "mmsconfig/mmsimportsourcedao.h"
#include "mmstools/mmsdbconnmgr.h"
#include "mmstools/tools.h"

MMSImportSourceService::MMSImportSourceService(DataSource *datasource) :
    dbconn(NULL) {
    MMSDBConnMgr connMgr(datasource);
    if((this->dbconn = connMgr.getConnection()))
    	this->dbconn->connect();
}

MMSImportSourceService::~MMSImportSourceService() {
	if(this->dbconn) {
		this->dbconn->disconnect();
		delete this->dbconn;
	}
}

void MMSImportSourceService::setImportSource(vector<MMSImportSourceData *> dataList) {
    MMSImportSourceDAO myImportSourceDAO(this->dbconn);
    myImportSourceDAO.saveOrUpdate(dataList);
}

vector<MMSImportSourceData *> MMSImportSourceService::getImportSourcesByPlugin(MMSPluginData *plugin) {
    MMSImportSourceDAO myImportSourceDAO(this->dbconn);
    vector<MMSImportSourceData *> importSources;
    importSources = myImportSourceDAO.findImportSourcesByPlugin(plugin);

    /* substitute env vars in the sources */
    for (unsigned i = 0; i < importSources.size(); i++) {
        importSources.at(i)->setSource(substituteEnvVars(importSources.at(i)->getSource()));
    }

    return importSources;
}

MMSImportSourceData * MMSImportSourceService::getImportSourcesByID(int id) {
    MMSImportSourceDAO myImportSourceDAO(this->dbconn);
    return myImportSourceDAO.findImportSourcesByID(id);
}

MMSImportSourceData * MMSImportSourceService::getImportSourcesByName(string name) {
    MMSImportSourceDAO myImportSourceDAO(this->dbconn);
    return myImportSourceDAO.findImportSourcesByName(name);
}
