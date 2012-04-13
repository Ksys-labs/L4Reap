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

#ifndef MMSPLUGINDAO_H_
#define MMSPLUGINDAO_H_

#include "mmsconfig/mmsplugindata.h"
#include "mmstools/interfaces/immsdb.h"

class MMSPluginDAO {
    private:
        IMMSDB *dbConnection;

        void setMMSDBConnection(IMMSDB *connection);
        IMMSDB *getMMSDBConnection();

        void save(MMSPluginData *data);
        void update(MMSPluginData *data);

        MMSPluginData *moveRecordToData(MMSRecordSet &rs);

    public:
        MMSPluginDAO(IMMSDB *connection);
        void deletePlugin(MMSPluginData *plugin);

        void saveOrUpdate(MMSPluginData *data);
        void saveOrUpdate(vector<MMSPluginData *> dataList);

        MMSPluginData *findPluginByID(int ID);
        MMSPluginData *findPluginByName(string name);
        vector<MMSPluginData *> findAllPlugins(const bool inactiveToo = "false");
        vector<MMSPluginData *> findAllPluginsByCategory(MMSPluginCategoryData *category, const bool inactiveToo = "false");
        vector<MMSPluginData *> findAllPluginsByType(MMSPluginTypeData *type, const bool inactiveToo = "false");
        vector<MMSPluginData *> findAllPluginsByType(string typeName, const bool inactiveToo = "false");
};

#endif /*MMSPLUGINDAO_H_*/
