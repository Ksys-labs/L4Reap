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

#include "mmsconfig/mmspluginpropertydao.h"
#include "mmstools/tools.h"
#include "mmstools/mmslogger.h"
#include "mmsconfig/mmsconfigqueries.h"
#include <stdlib.h>

MMSPluginPropertyDAO::MMSPluginPropertyDAO(IMMSDB *myConnection) {
    MMSPluginPropertyDAO::setMMSDBConnection(myConnection);
}

IMMSDB *MMSPluginPropertyDAO::getMMSDBConnection() {
    return this->dbConnection;
}

void MMSPluginPropertyDAO::setMMSDBConnection(IMMSDB *connection) {
    this->dbConnection = connection;
}

void MMSPluginPropertyDAO::saveOrUpdate(vector<MMSPluginData *> plugins) {
    for(vector<MMSPluginData *>::iterator it = plugins.begin();it!=plugins.end();it++)
        this->saveOrUpdate(*it);
}

void MMSPluginPropertyDAO::saveOrUpdate(MMSPluginData *plugin) {
    string                      query;
    vector<MMSPropertyData *>   pluginPropertyList;
    MMSRecordSet                rs;
    MMSLogger                   logger;
    string                      tmpValList = "";

    vector <string>             vallist;

    pluginPropertyList = plugin->getProperties();
    for(vector<MMSPropertyData *>::iterator it = pluginPropertyList.begin(); it!=pluginPropertyList.end();it++) {
        query = "select * from PluginProperties where PluginID = '" +
                iToStr(plugin->getId())  + "' and Parameter = '" +
                (*it)->getParameter() + "';";

        this->getMMSDBConnection()->query(query, &rs);
        /* check if result is empty */
        if (rs.getCount()==0) {
            //insert...

            //create value list string
            vector<string> vallist = (*it)->getVallist();
            string valliststr = "";
            for(vector<string>::iterator val = vallist.begin();val!=vallist.end();val++) {
                valliststr = valliststr + cToStr((*it)->getSeparator()) + *val;
            }

            query = "insert into PluginProperties (Parameter,Value,TYPE,MAX,MIN,VALLIST,SEPARATOR,PluginID) VALUES('" +
                    (*it)->getParameter() + "','" +
                    (*it)->getValue() + "','" +
                    (*it)->getType() + "','" +
                    iToStr((*it)->getMax()) + "','" +
                    iToStr((*it)->getMin()) + "','" +
                    valliststr + "','" +
                    cToStr((*it)->getSeparator()) + "','" +
                    iToStr(plugin->getId()) + "');";

                    this->getMMSDBConnection()->query(query);
        }
        else {
            //update...

            //create value list string
            vector<string> vallist = (*it)->getVallist();
            string valliststr = "";
            for(vector<string>::iterator val = vallist.begin();val!=vallist.end();val++) {
                valliststr = valliststr + cToStr((*it)->getSeparator()) + *val;
            }

            query = "update PluginProperties Set Value='" +
                    (*it)->getValue() +
                    "',TYPE='" + (*it)->getType() +
                    "',MAX='" + iToStr((*it)->getMax()) +
                    "',MIN='" + iToStr((*it)->getMin()) +
                    "',VALLIST='" + valliststr +
                    "',SEPARATOR='" + cToStr((*it)->getSeparator()) +
                    "' where PluginID='" + iToStr(plugin->getId()) +
                    "' and Parameter='" + (*it)->getParameter() + "';";

                    this->getMMSDBConnection()->query(query);
        }
    }
}


vector <MMSPropertyData *> MMSPluginPropertyDAO::findAllPluginPropertiesByPlugin(MMSPluginData *plugin)
{
    string 						query;
    vector<MMSPropertyData *> 	pluginPropertyList;
    MMSRecordSet            	rs;
    string						tmpValList = "";

    vector <string>             vallist;
    size_t      			    tmpBegin;
    size_t		                tmpEnd;

    this->getMMSDBConnection()->query(PLUGINPROPERTYDAO_FIND_ALL_PLUGIN_PROPERTIES_BY_PLUGIN(iToStr(plugin->getId())), &rs);
        /* check if result is empty */

    if (rs.getCount()==0) {
    	return pluginPropertyList;
    }

	/* for each result record */
    do {
        /* set the values */
        MMSPropertyData *pluginProperty = new MMSPropertyData;
        if(!rs["ID"].empty())
        	pluginProperty->setID(atoi(rs["ID"].c_str()));
        pluginProperty->setParameter(rs["Parameter"]);
        pluginProperty->setValue(rs["Value"]);
        pluginProperty->setType(rs["TYPE"]);
        if(!rs["MAX"].empty())
	        pluginProperty->setMax(atoi(rs["MAX"].c_str()));
		else
	        pluginProperty->setMax(0);
        if(!rs["MIN"].empty())
        	pluginProperty->setMin(atoi(rs["Min"].c_str()));
        else
        	pluginProperty->setMin(0);

        if(!rs["SEPARATOR"].empty())
        	pluginProperty->setSeparator(rs["SEPARATOR"].c_str()[0]);

		tmpValList = rs["VALLIST"];

		tmpBegin = 0;

		while ((tmpEnd = tmpValList.find(pluginProperty->getSeparator(), tmpBegin)) != string::npos) {
			vallist.push_back(tmpValList.substr(tmpBegin, tmpEnd - tmpBegin));
			tmpBegin = tmpEnd + 1;
		}
		vallist.push_back(tmpValList.substr(tmpBegin, tmpValList.length() - tmpBegin));

        pluginProperty->setVallist(vallist);
		vallist.clear();

        /* push to list */
        pluginPropertyList.push_back(pluginProperty);
    } while(rs.next() == true);

    return pluginPropertyList;
}
