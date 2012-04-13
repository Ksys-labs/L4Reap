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
#include <string.h>

#ifdef LIBXML_READER_ENABLED
#include "mmsbase/mmsxmlclientinterface.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "mmstools/tools.h"

MMSXMLClientInterface::MMSXMLClientInterface(string host, unsigned int port) {
    LIBXML_TEST_VERSION;

	this->tcl = new MMSTCPClient(host, port);
}

bool MMSXMLClientInterface::parseAnswer(string *answer, int *rc, string *error) {
    bool             ret = false;
    xmlDocPtr		 doc;

    doc = xmlReadMemory(answer->c_str(),answer->length(),"memory.xml",NULL,0);

    if(!doc) {
        DEBUGMSG("MMSXMLClientInterface", "Error initializing doc()");
        return false;
    }
    if(checkRoot(doc, rc, error)) {
        ret = true;
	}

	xmlFreeDoc(doc);

    return ret;
}

bool MMSXMLClientInterface::checkRoot(xmlDocPtr doc, int *rc, string *error) {
    //xmlChar *name, *attr;

    if (!doc)
    	return false;
#if 0
    /* check root element */
    name = (xmlChar*)xmlTextReaderConstName(reader);
    if(!name || !xmlStrEqual(name, (const xmlChar*)"ret")) {
        DEBUGMSG("MMSXMLClientInterface", "The root element must be <ret> and not <%s>.", name);
        return false;
    }

    /* get attribute rc */
    *rc = 0;
    attr = xmlTextReaderGetAttribute(reader, (const xmlChar*)"rc");
    if(attr) *rc = atoi((const char*)attr);
    xmlFree(attr);

    /* get attribute error */
    *error = "";
    attr = xmlTextReaderGetAttribute(reader, (const xmlChar*)"error");
    if(attr) *error = strdup((const char*)attr);
    xmlFree(attr);
#endif
    return true;
}

bool MMSXMLClientInterface::funcSendEvent(string heading, int pluginid, int *rc, string *error) {
  return funcSendEvent(heading, NULL, pluginid, rc, error);
}

bool MMSXMLClientInterface::funcSendEvent(string heading, map<string, string> *params, int pluginid, int *rc, string *error) {
	string rbuf, abuf;

	/* build request */
	rbuf = "<func name=\"SendEvent\" heading=\"" + heading + "\"";
	if (pluginid>=0) rbuf+= " pluginid=\"" + iToStr(pluginid) + "\"";
	if (NULL == params || 0 == params->size()) {
	  rbuf+= "/>";
	}
	else {
	  rbuf +=">";
	  for (map<string, string>::iterator iter = params->begin(); iter != params->end(); iter++) {
	    rbuf += "<param "+iter->first+"=\""+iter->second+"\" />";
	  }
	  rbuf += "</func>";
	}

	/* call server */
	if(!tcl->connectToServer()) {
		DEBUGMSG("MMSBASE", "connection to server failed");
	}

	tcl->sendAndReceive(rbuf, &abuf);

	DEBUGMSG("MMSBASE", "got response %s", abuf.c_str());
	/* parse answer */
	if(parseAnswer(&abuf, rc, error)) {
		/* parse for more values here */

		return true;
	}

	return false;
}

#endif /* LIBXML_READER_ENABLED */

