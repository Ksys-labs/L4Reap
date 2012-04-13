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

#include <libxml/xmlversion.h>

#ifdef LIBXML_READER_ENABLED

#include "mmsbase/mmsxmlserverinterface.h"
#include "mmsbase/mmsevent.h"
#include "mmstools/tools.h"

MMSXMLServerInterface::MMSXMLServerInterface() : MMSServerInterface("MMSXMLServerInterface") {
    LIBXML_TEST_VERSION;
}

MMSXMLServerInterface::~MMSXMLServerInterface() {
}

bool MMSXMLServerInterface::processRequest(string *request, string *answer) {
    if(!request || !answer) {
        DEBUGMSG("MMSXMLServerInterface","processRequest() error in cmdline");
    	return false;
    }
    DEBUGMSG("MMSXMLServerInterface","got request %s", request->c_str());

	*answer = "";

    DEBUGMSG("MMSXMLServerInterface","new reader");
    xmlDocPtr doc = xmlReadMemory(request->c_str(),request->length(),"memory.xml",NULL,0);

    if(!doc) {
        *answer = "<ret error=\"Problems with xml request.\"/>";
        DEBUGMSG("MMSXMLServerInterface", "Error initializing xmlReader()");
        return false;
    }

    DEBUGMSG("MMSXMLServerInterface","throughdoc");
    throughDoc(doc, answer);
    if(*answer == "")
    		*answer = "<ret error=\"Unknown error.\"/>";

    xmlFreeDoc(doc);

    return true;
}

bool MMSXMLServerInterface::throughDoc(xmlDocPtr doc, string *answer) {

    if(!doc|| !answer)  {
        DEBUGMSG("MMSXMLServerInterface","throughdoc, error in cmdline");
        return false;
    }

    xmlNodePtr root =  xmlDocGetRootElement(doc);
    if(!root)
    	return false;

    if(xmlStrEqual(root->name, (const xmlChar*)"func")==0) {
        DEBUGMSG("MMSXMLServerInterface", "The root element must be <func> and not <%s>.", root->name);
        return false;
    }

    return throughFunc(root, answer);
}

bool MMSXMLServerInterface::throughFunc(xmlNodePtr node, string *answer) {
    xmlChar *name;

    if(!node|| !answer) return false;

    /* get name of function to be called */
    name = xmlGetProp(node, (const xmlChar*)"name");
    if(!name) {
		/* function not specified */
        *answer = "<ret error=\"Function not specified.\"/>";
        DEBUGMSG("MMSXMLServerInterface", "Function not specified.");
		return false;
	}

	/* check function */
    if(!xmlStrEqual(name, (const xmlChar*)"SendEvent")) {
		/* unknown function */
        *answer = "<ret error=\"Unknown function '" + string((const char*)name) + "'.\"/>";
        DEBUGMSG("MMSXMLServerInterface", "Unknown function '%s'.", name);
		return false;
	}

    return funcSendEvent(node, answer);
}

bool MMSXMLServerInterface::funcSendEvent(xmlNodePtr node, string *answer) {
    xmlChar  *heading, *name, *value;/**pluginid*/
    xmlTextReader *reader;
	MMSEvent *event;

    if(!node || !answer) return false;

    /* get attributes */
    heading  = xmlGetProp(node, (const xmlChar*)"heading");
    reader = xmlReaderWalker(node->doc);
    //pluginid = xmlTextReaderGetAttribute(reader, (const xmlChar*)"pluginid");

    if(!heading /*|| !pluginid*/) return false;

    event = new MMSEvent((const char*)heading);

    /* through <func/> childs */
    while(xmlTextReaderRead(reader)) {
        name = (xmlChar*)xmlTextReaderConstName(reader);
        if(name && xmlStrEqual(name, (const xmlChar*)"param")) {
            while(xmlTextReaderMoveToNextAttribute(reader)) {
                name  = xmlTextReaderName(reader);
                value = xmlTextReaderValue(reader);
                event->setData((const char*)name, (const char*)value);
                xmlFree(name);
                xmlFree(value);
            }
        }
    }
	/* build answer */
	*answer = "<ret/>";

    event->send();


    return true;
}

#endif /* LIBXML_READER_ENABLED */
