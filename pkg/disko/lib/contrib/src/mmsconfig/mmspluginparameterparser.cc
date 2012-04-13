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

#include "mmsconfig/mmspluginparameterparser.h"
#include "mmstools/tools.h"

#include <libxml/parser.h>

MMSPluginParameterParser::MMSPluginParameterParser() {
}

MMSPluginParameterParser::~MMSPluginParameterParser() {

}

bool MMSPluginParameterParser::validate(MMSPluginData *plugin) {
    string parameterfile;

#ifdef __NOT_USED__
    xmlpp::Document *doc;

    if(plugin== NULL)
        return false;

    logger.writeLog("validate property");

    //all is well if there are no parameters
    if(plugin->getProperties().empty()) {
        return true;
    }

    parameterfile = plugin->getPath() + "/parameter.xml";
    logger.writeLog("parse file: " +plugin->getPath() + "/parameter.xml");
    try {
        parser->parse_file(parameterfile);
        if(parser) {
            doc = parser->get_document();
            xmlpp::Node *root = doc->get_root_node();

            vector<MMSPropertyData *> properties = plugin->getProperties();
            for(vector<MMSPropertyData *>::iterator it= properties.begin();it!=properties.end();it++) {
                Glib::ustring query;
                query.append("/plugin/parameter[@name=\"");
                query.append((*it)->getParameter());
                query.append("\"]");
                xmlpp::NodeSet set = root->find(query);
                if(set.empty())
                    throw MMSPluginParameterParserError(0,"Plugin " + plugin->getName() + " has no parameter named " + (*it)->getParameter());

            }
            delete doc;
            return true;


        }
    }
    catch(const std::exception& ex) {
        //std::cout << "Exception caught: " << ex.what() << std::endl;
        return false;
    }
#endif
    return false;
}

void MMSPluginParameterParser::createProperty(MMSPluginData *plugin,string name) {
    string parameterfile;
	xmlDoc *parser = NULL;

    if(plugin== NULL)
        return;

    DEBUGMSG("PLUGINPARAMETERPARSER", "CreateProperty");

    if(plugin->getProperty(name)!=NULL) {
        return;
    }

	LIBXML_TEST_VERSION

    parameterfile = plugin->getPath() + "/parameter.xml";
    try {
    	parser = xmlReadFile((char *)parameterfile.c_str(), NULL, 0);

		if(parser == NULL) {
			throw MMSPluginParameterParserError(1,"Could not parse file:" + parameterfile);
		}
		else {
			xmlNode* pNode = xmlDocGetRootElement(parser);

            string query;
            query.append("//plugin/parameter[@name=\"");
            query.append(name);
            query.append("\"]");

	  		if(xmlStrcmp(pNode->name, (const xmlChar *) query.c_str())) {
	  			DEBUGMSG("PLUGINPARAMETERPARSER", "invalid configuration file (%s) - does not contain correct root node", parameterfile.c_str());
                throw MMSPluginParameterParserError(0,"Plugin " + plugin->getName() + " has no parameter named " + name);
	  		}

            // ok we  have a parameter description create a new property
            MMSPropertyData *data = new MMSPropertyData();
            data->setParameter(name);

        	xmlChar *type;
        	type = xmlGetProp(pNode, (const xmlChar*) "type");

        	if(type!=NULL) {
                if((string((char*)type) == MMSPROPERTYTYPE_STRING) || (string((char*)type) == MMSPROPERTYTYPE_INTEGER))
                    data->setType((char *)type);
                else {
            	    /*free the document */
            	    xmlFreeDoc(parser);
                    throw MMSPluginParameterParserError(0,"the data type \"" + string((char*)type) + "\" defined in the parameter.xml of " + plugin->getName() + " is unknown.");
                }
            }
        	xmlFree(type);

        	xmlChar *min;
        	min = xmlGetProp(pNode, (const xmlChar*)"min");
            if(min!=NULL) {
                data->setMin(atoi((char*)min));
            }
            xmlFree(min);

        	xmlChar *max;
        	max = xmlGetProp(pNode, (const xmlChar*)"max");
            if(max!=NULL) {
                data->setMax(atoi((char*)max));
            }
            xmlFree(max);

    	    /*free the document */
    	    xmlFreeDoc(parser);

            data->setisSetinDb(false);
            vector<MMSPropertyData *> result = plugin->getProperties();
            result.push_back(data);
            plugin->setProperties(result);

            return;


        }
    }
    catch(const std::exception& ex) {
        return;
    }
}


//fills properites from the db whith unset properties from xml
void MMSPluginParameterParser::fillProperties(MMSPluginData *plugin) {
    string parameterfile;
#ifdef __NOT_USED__
	xmlDoc *parser = NULL;

	logger.writeLog("Fillproperies");

    if(plugin== NULL)
        throw MMSPluginParameterParserError(0,"no plugin given");

    LIBXML_TEST_VERSION

    parameterfile = plugin->getPath() + "/parameter.xml";
    logger.writeLog("parse file: " +parameterfile );

    try {
    	parser = xmlReadFile((char *)parameterfile.c_str(), NULL, 0);

    	if(parser == NULL) {
    			throw MMSPluginParameterParserError(1,"Could not parse file:" + parameterfile);
    	}
    	else {
    		xmlNode* pNode = xmlDocGetRootElement(parser);

            string query;
            query.append("/plugin/parameter");

            if(xmlStrcmp(pNode->name, (const xmlChar *) query.c_str())) {
	  			logger.writeLog("invalid configuration file (" + parameterfile + ") - does not contain correct root node");
                throw MMSPluginParameterParserError(0,"Plugin " + plugin->getName() + " has no parameter named " + string((char *)pNode->name));
	  		}

            if(myset.empty())
                throw MMSPluginParameterParserError(MMSPLUGINPARAMETERPARSER_ERROR_NOPARAMETERS,"Plugin has no parameters");
            else
                logger.writeLog("got " + iToStr(myset.size()) + " parameters");

            vector<MMSPropertyData *> properties = plugin->getProperties();
            //go through the set
            for(unsigned int i= 0;i<myset.size();i++) {
                logger.writeLog("vor get_name()");
                //logger.writeLog(((xmlpp::Element *)myset.at(i))->get_name());
                logger.writeLog("nach get_name()");

                xmlpp::Attribute *attr = ((xmlpp::Element*)myset.at(i))->get_attribute("name");
                logger.writeLog(attr->get_name());
                logger.writeLog(attr->get_value());

                //see if we have the parameter already in the list
                bool found = false;
                for(vector<MMSPropertyData *>::iterator props=properties.begin();props!=properties.end();props++) {
                    if(((MMSPropertyData *)(*props))->getParameter()==attr->get_value()) {
                        found=true;
                        break;
                    }
                }

                //add parameter if not found
                if(found == false) {
                    logger.writeLog("create missing property");
                    this->createProperty(plugin,attr->get_value());
                }

            }
            //delete doc;
            return;

        }
    }
    catch(const std::exception& ex) {
        return;
//        throw MMSPluginParameterParserError(0,ex.what());
    }
#endif

}
