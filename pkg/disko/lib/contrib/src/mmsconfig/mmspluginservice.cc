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

#include "mmsconfig/mmspluginservice.h"
#include "mmstools/mmsdbconnmgr.h"
#include "mmstools/tools.h"
#include "mmsconfig/mmsplugindao.h"
#include "mmsconfig/mmspluginpropertydao.h"
#include "mmsconfig/mmsimportpropertydao.h"
#include "mmsconfig/mmsplugincategorydao.h"
#include "mmsconfig/mmsplugintypedao.h"

MMSPluginService::MMSPluginService(DataSource *datasource) :
    dbconn(NULL) {
    MMSDBConnMgr connMgr(datasource);
    if((this->dbconn = connMgr.getConnection()))
        this->dbconn->connect();
}

MMSPluginService::~MMSPluginService() {
	if(this->dbconn) {
	    this->dbconn->disconnect();
	    delete this->dbconn;
	}
}

void MMSPluginService::setPlugin(MMSPluginData *data) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    myPluginDAO.saveOrUpdate(data);

    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    myPropertyDAO.saveOrUpdate(data);
}

void MMSPluginService::setPlugin(vector<MMSPluginData *> dataList) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    myPluginDAO.saveOrUpdate(dataList);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    myPropertyDAO.saveOrUpdate(dataList);
}

const inline void MMSPluginService::setPluginProperties(MMSPluginData *plugin) const {
	if(plugin) {
		MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
		vector <MMSPropertyData *> properties;
		properties = myPropertyDAO.findAllPluginPropertiesByPlugin(plugin);
		plugin->setProperties(properties);
	}
}

const inline void MMSPluginService::setPluginProperties(vector<MMSPluginData*> &pluginList) const {
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    for(vector<MMSPluginData*>::iterator i = pluginList.begin(); i != pluginList.end(); ++i) {
        (*i)->setProperties(myPropertyDAO.findAllPluginPropertiesByPlugin(*i));
    }
}

MMSPluginData *MMSPluginService::getPluginByName(string name) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    MMSPluginData *plugin = myPluginDAO.findPluginByName(name);
    setPluginProperties(plugin);

    return plugin;
}

MMSPluginData *MMSPluginService::getPluginByID(int id) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    MMSPluginData *plugin = myPluginDAO.findPluginByID(id);
    setPluginProperties(plugin);

    return plugin;
}

vector<MMSPluginData*> MMSPluginService::getAllPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPlugins(inactiveToo);
    setPluginProperties(pluginList);

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getOSDPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_OSD_PLUGIN, inactiveToo);
    setPluginProperties(pluginList);

    DEBUGMSG("PLUGINSERVICE", "Working with %d OSD plugins", pluginList.size());

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getCentralPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_CENTRAL_PLUGIN, inactiveToo);
    setPluginProperties(pluginList);

    DEBUGMSG("PLUGINSERVICE", "Working with %d Central plugins", pluginList.size());

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getImportPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_IMPORT_PLUGIN, inactiveToo);

    for(vector<MMSPluginData*>::iterator i = pluginList.begin(); i != pluginList.end(); ++i) {
        (*i)->setProperties(myPropertyDAO.findAllPluginPropertiesByPlugin(*i));
        (*i)->setImportProperties(myImportPropertyDAO.findImportPropertyByPlugin(*i));
    }

    DEBUGMSG("PLUGINSERVICE", "Working with %d Import plugins", pluginList.size());

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getBackendPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_BACKEND_PLUGIN, inactiveToo);
    setPluginProperties(pluginList);

    DEBUGMSG("PLUGINSERVICE", "Working with %d Backend plugins", pluginList.size());

    return pluginList;
}

/* getAllPluginsByCategory */
vector<MMSPluginData*> MMSPluginService::getPluginsByCategory(MMSPluginCategoryData *category, const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByCategory(category, inactiveToo);

    for(vector<MMSPluginData*>::iterator i = pluginList.begin(); i != pluginList.end(); ++i) {
        if((*i)->getType()->getName() == PT_IMPORT_PLUGIN) {
            (*i)->setImportProperties(myImportPropertyDAO.findImportPropertyByPlugin(*i));
        }
        (*i)->setProperties(myPropertyDAO.findAllPluginPropertiesByPlugin(*i));
    }

    return pluginList;
}

vector<MMSPluginData*> MMSPluginService::getPluginsByType(MMSPluginTypeData *type, const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(type, inactiveToo);

    for(vector<MMSPluginData*>::iterator i = pluginList.begin(); i != pluginList.end(); ++i) {
        if((*i)->getType()->getName() == PT_IMPORT_PLUGIN) {
            (*i)->setImportProperties(myImportPropertyDAO.findImportPropertyByPlugin(*i));
        }
        (*i)->setProperties(myPropertyDAO.findAllPluginPropertiesByPlugin(*i));
    }

    return pluginList;
}

MMSPluginCategoryData* MMSPluginService::getPluginCategoryByName(string name) {
    MMSPluginCategoryDAO categoryDAO(this->dbconn);
    return categoryDAO.findCategoryByName(name);
}

MMSPluginTypeData* MMSPluginService::getPluginTypeByName(string name) {
	MMSPluginTypeDAO typeDAO(this->dbconn);
    return typeDAO.findTypeByName(name);
}

vector<MMSPluginCategoryData*> MMSPluginService::getPluginCategories() {
    MMSPluginCategoryDAO categoryDAO(this->dbconn);
    return categoryDAO.findAllCategories();
}

void MMSPluginService::getSystemProperties(std::map<string,MMSPropertyData*> &result) {
    MMSPluginDAO myPluginDAO(this->dbconn);

    MMSPluginData *plugin = myPluginDAO.findPluginByID(-2);
    if(plugin) {
        MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
        vector <MMSPropertyData *> properties;
		properties = myPropertyDAO.findAllPluginPropertiesByPlugin(plugin);
		for(vector <MMSPropertyData *>::iterator it = properties.begin(); it!= properties.end(); it++) {
			result.insert(std::make_pair(string((*it)->getParameter()),*it));
		}
    }
}
