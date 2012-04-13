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

#include "mmsinput/mmsinputmapper.h"
#include "mmstools/mmstools.h"
#include <iostream>
#include <stdlib.h>
#include <cerrno>

MMSInputMapper::MMSInputMapper(string mapfile, string name) {
	xmlNode *walkNode = NULL;
	xmlNode *curNode = NULL;

	/* map the keys */
	try {
		LIBXML_TEST_VERSION

		parser = xmlReadFile((char *)mapfile.c_str(), NULL, 0);

		if(parser == NULL) {
			throw MMSError(1, "Could not parse file:" + mapfile);
		}
		else {

			xmlNode* pNode = xmlDocGetRootElement(parser);

			// check if incorrent input mapfile found
			if(xmlStrcmp(pNode->name, (const xmlChar *) "mmsinputmaps")) {
				std::cout << "invalid mapfile (" << mapfile << ") - does not contain mmsinputmaps root node" << std::endl;
				throw MMSError(1, "invalid file");
			}

			pNode = pNode->xmlChildrenNode;

			// need char buffers fpr mapping information
			xmlChar *keyName;
			xmlChar *mapTo;

			// walk trough the child node, until found the correct map node
			for (walkNode = pNode; walkNode; walkNode = walkNode->next) {
				xmlChar *mapName;

				if(xmlStrcmp(walkNode->name, (const xmlChar*) "map"))
					continue;

				mapName   = xmlGetProp(walkNode, (const xmlChar*)"name");
				if(!mapName)
				    continue;

				if(!xmlStrcmp(mapName, (const xmlChar *) name.c_str())) {
					DEBUGMSG("MMSINPUTMANAGER", "using mapping set of " + name + " node");

					walkNode = walkNode->xmlChildrenNode;
					for (curNode = walkNode; curNode; curNode = curNode->next) {
						if(xmlStrcmp(curNode->name, (const xmlChar *) "key"))
							continue;

						keyName = xmlGetProp(curNode, (const xmlChar*)"name");
						mapTo   = xmlGetProp(curNode, (const xmlChar*)"mapto");

					    this->keyMap.insert(pair<string, string> (strToUpr(string((const char *) keyName)), strToUpr(string((const char *)mapTo))));

						xmlFree(keyName);
						xmlFree(mapTo);
					}
				}
				else {
					//ignore this node
					DEBUGMSG("MMSINPUTMANAGER", "Ignore mapping set of " + string((const char *) mapName) + " node");
				}

				xmlFree(mapName);
			}
        }
	}
	catch(MMSError &error) {
		DEBUGMSG("MMSINPUTMANAGER", "Error loading inputmaps (" + mapfile + "." + name + ") [" + error.getMessage() + "]");
	}

	DEBUGMSG("MMSINPUTMANAGER", "Parsing inputmap finished.");
}

MMSInputMapper::~MMSInputMapper() {

	this->keyMap.clear();
	delete this->parser;
}

void MMSInputMapper::mapkey(MMSInputEvent *inputevent) {
    string symbol = mmskeys[inputevent->key];

	/* parse the result nodes */
    typedef multimap<string, string>::iterator MI;
    pair<MI,MI> iter = this->keyMap.equal_range(symbol);

    for( MI run = iter.first; run != iter.second; ++run ) {
   		string foundkeyname = run->second;
   	    MMSKeySymbol foundkey = mmskeys[foundkeyname];
   	    if(foundkey) {
   	    	DEBUGMSG("MMSINPUTMANAGER", "mapped to key '" + foundkeyname + "', id: " + iToStr(foundkey));
//   	        evt.type = MMSINPUTEVENTTYPE_KEYPRESS;
   	        inputevent->key = foundkey;
   	        return;
   	    }
    }
}

