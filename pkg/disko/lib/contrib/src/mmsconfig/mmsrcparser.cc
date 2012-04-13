/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

using namespace std;

#include <iostream>

#include <cstring>
#include "mmsconfig/mmsrcparser.h"
#include "mmstools/tools.h"
#include "diskoversion.h"

#define WRONG_VALUE(parname, parvalue, validvals, addmsg) throw MMSRcParserError(1, "wrong value '" + string(parvalue) + "' for parameter '" + string((const char *)parname) + "'\n valid value(s): " + validvals + "\n " + addmsg);


MMSRcParser::MMSRcParser() {
    this->configdb.database = "/tmp/mmsconfigdb";
	this->datadb.database   = "/tmp/mmsdatadb";
}

MMSRcParser::~MMSRcParser() {

}

void MMSRcParser::parseFile(string filename) {

	try {
		xmlDoc *parser = NULL;

		LIBXML_TEST_VERSION

#ifdef __ENABLE_LOG__
		parser = xmlReadFile((char *)filename.c_str(), NULL, XML_PARSE_NOBLANKS | XML_PARSE_NONET);
#else
		parser = xmlReadFile((char *)filename.c_str(), NULL, XML_PARSE_NOBLANKS | XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
#endif

		if(parser == NULL) {
			throw MMSRcParserError(1,"Could not parse file:" + filename);
		}

		if(parser) {
			//Walk the tree:
			xmlNode* pNode = xmlDocGetRootElement(parser);
	  		if(xmlStrcmp(pNode->name, (const xmlChar *) "mmsrc")) {
		  		if(xmlStrcmp(pNode->name, (const xmlChar *) "diskorc")) {
					std::cout << "invalid configuration file (" << filename << ") - does not contain mmsrc root node" << std::endl;
					throw MMSRcParserError(1, "invalid file");
		  		}
	  		}
	      	this->throughFile(pNode);

            if (this->graphics.vrect.w <= 0)
                this->graphics.vrect.w = this->graphics.graphicslayer.rect.w;
            if (this->graphics.vrect.h <= 0)
                this->graphics.vrect.h = this->graphics.graphicslayer.rect.h;
            if ((this->graphics.vrect.x < 0)||(this->graphics.vrect.x > this->graphics.graphicslayer.rect.w))
                this->graphics.vrect.x = 0;
            if ((this->graphics.vrect.y < 0)||(this->graphics.vrect.y > this->graphics.graphicslayer.rect.h))
                this->graphics.vrect.y = 0;
            if (this->graphics.vrect.w - this->graphics.vrect.x > this->graphics.graphicslayer.rect.w)
                this->graphics.vrect.w = this->graphics.graphicslayer.rect.w - this->graphics.vrect.x;
            if (this->graphics.vrect.h - this->graphics.vrect.y > this->graphics.graphicslayer.rect.h)
                this->graphics.vrect.h = this->graphics.graphicslayer.rect.h - this->graphics.vrect.y;


    	    /*free the document */
    	    xmlFreeDoc(parser);
		}
	}
	catch(MMSError &error)
	{
		std::cout << "RcParser exception: " << error.getMessage() << std::endl;
		throw MMSRcParserError(1, error.getMessage());
	}

}

void MMSRcParser::getMMSRc(MMSConfigDataGlobal 		**global,
		                   MMSConfigDataDB     		**configdb,
		                   MMSConfigDataDB     		**datadb,
		                   MMSConfigDataGraphics    **graphics,
		                   MMSConfigDataLanguage    **language) {
	if (global)		*global   = &this->global;
	if (configdb)	*configdb = &this->configdb;
	if (datadb)		*datadb   = &this->datadb;
	if (graphics)	*graphics = &this->graphics;
	if (language)	*language = &this->language;
}

void MMSRcParser::getMMSRc(MMSConfigDataGlobal 		*global,
		                   MMSConfigDataDB     		*configdb,
		                   MMSConfigDataDB     		*datadb,
		                   MMSConfigDataGraphics    *graphics,
		                   MMSConfigDataLanguage    *language) {
	if (global)		*global   = this->global;
	if (configdb)	*configdb = this->configdb;
	if (datadb)		*datadb   = this->datadb;
	if (graphics)	*graphics = this->graphics;
	if (language)	*language = this->language;
}

/**
 * Checks for version of the configuration file.
 *
 * If the version does not match, an exception is thrown.
 *
 * @param	node	should be the mmsrc root node
 */
void MMSRcParser::checkVersion(xmlNode* node) {

	xmlChar *version;

	version = xmlGetProp(node, (const xmlChar*)"version");

	if(!version) {
		std::cout << "Configuration file misses version entity!" << std::endl;
		throw MMSRcParserError(1, "missing version");
	}

	// sep version
	version[1]=0;
	version[3]=0;
	int mav = atoi((char*)&version[0]);
	int miv = atoi((char*)&version[2]);

	// disko version check
	if ((mav > DISKO_MAJOR_VERSION )||(miv > DISKO_MINOR_VERSION)) {
		std::cout << "Version of configuration file does not match!" << std::endl;
		xmlFree(version);
		throw MMSRcParserError(1, "version mismatch");
	}

	xmlFree(version);
}

void MMSRcParser::throughGlobal(xmlNode* node) {
	xmlNode *cur_node = NULL;
	xmlChar *parname;
	xmlChar *parvalue;

	node = node->xmlChildrenNode;

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "text")) continue;
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "comment"))	continue;
		if(xmlStrcmp(cur_node->name, (const xmlChar *) "parameter")) {
			printf("RcParser: ignoring tag <%s/>\n", cur_node->name);
			continue;
		}

    	parname  = xmlGetProp(cur_node, (const xmlChar*)"name");
    	parvalue = xmlGetProp(cur_node, (const xmlChar*)"value");

    	if(parname == NULL && parvalue == NULL)
    		continue;

	    if(!xmlStrcmp(parname, (const xmlChar *) "logfile"))
		    this->global.logfile = string((const char *)parvalue);
		else if(!xmlStrcmp(parname, (const xmlChar *) "inputmap"))
			this->global.inputmap = string((const char *)parvalue);
		else if(!xmlStrcmp(parname, (const xmlChar *) "prefix"))
			this->global.prefix = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "theme"))
            this->global.theme = string((const char *)parvalue);
		else if(!xmlStrcmp(parname, (const xmlChar *) "sysconfig"))
			this->global.sysconfig = string((const char *)parvalue);
		else if(!xmlStrcmp(parname, (const xmlChar *) "data"))
			this->global.data = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "inputinterval"))
            this->global.inputinterval = strToInt(string((const char *)parvalue));
		else if(!xmlStrcmp(parname, (const xmlChar *) "firstplugin"))
			this->global.firstplugin = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "shutdown"))
            this->global.shutdown = strToBool(string((const char *)parvalue));
        else if(!xmlStrcmp(parname, (const xmlChar *) "shutdowncmd"))
            this->global.shutdowncmd = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "inputmode"))
            this->global.inputmode = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "actmonaddress"))
            this->global.actmonaddress = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "actmonport"))
        	this->global.actmonport = atoi((const char*)parvalue);
		else
			printf("RcParser: ignoring parameter '%s' in tag <global/>\n", parname);

	    xmlFree(parname);
	    xmlFree(parvalue);
	}
}

void MMSRcParser::throughLanguage(xmlNode* node) {
	xmlNode *cur_node = NULL;
	xmlChar *parname;
	xmlChar *parvalue;

	node = node->xmlChildrenNode;

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "text")) continue;
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "comment"))	continue;
		if(xmlStrcmp(cur_node->name, (const xmlChar *) "parameter")) {
			printf("RcParser: ignoring tag <%s/>\n", cur_node->name);
			continue;
		}

    	parname  = xmlGetProp(cur_node, (const xmlChar*)"name");
    	parvalue = xmlGetProp(cur_node, (const xmlChar*)"value");

    	if(parname == NULL && parvalue == NULL)
    		continue;

	    if(!xmlStrcmp(parname, (const xmlChar *) "sourcelang"))
	    	this->language.sourcelang = getMMSLanguageFromString((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "defaultdestlang"))
            this->language.defaulttargetlang = getMMSLanguageFromString((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "addtranslations"))
            this->language.addtranslations = strToBool(string((const char *)parvalue));
        else if(!xmlStrcmp(parname, (const xmlChar *) "languagefiledir"))
        	this->language.languagefiledir = string((const char *)parvalue);
		else
			printf("RcParser: ignoring parameter '%s' in tag <global/>\n", parname);

	    xmlFree(parname);
	    xmlFree(parvalue);
	}
}

void MMSRcParser::throughDBSettings(xmlNode* node) {
	MMSConfigDataDB *db;
	xmlChar *type;

	type = xmlGetProp(node, (const xmlChar*)"type");

	if(!xmlStrcmp(type, (const xmlChar *) "config")) {
		db = &this->configdb;
	}
	else if(!xmlStrcmp(type, (const xmlChar *) "data")) {
		db = &this->datadb;
	}
	else {
		throw MMSRcParserError(1, "unknown database type (" + string((const char*)type) + ")");
	}

	xmlFree(type);

	xmlNode *cur_node = NULL;
	xmlChar *parname;
	xmlChar *parvalue;

	for (cur_node = node->xmlChildrenNode; cur_node; cur_node = cur_node->next) {
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "text")) continue;
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "comment"))	continue;
		if(xmlStrcmp(cur_node->name, (const xmlChar *) "parameter")) {
			printf("RcParser: ignoring tag <%s/>\n", cur_node->name);
			continue;
		}

		parname  = xmlGetProp(cur_node, (const xmlChar*)"name");
		parvalue = xmlGetProp(cur_node, (const xmlChar*)"value");

		if(!xmlStrcmp(parname, (const xmlChar *) "dbms"))
            db->dbms = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "address"))
            db->address = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "port"))
            db->port = atoi((const char*)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "user"))
            db->user = string((const char *)parvalue);
        else if(!xmlStrcmp(parname, (const xmlChar *) "password"))
            db->password = string((const char *)parvalue);
		else if(!xmlStrcmp(parname, (const xmlChar *) "database"))
		    db->database = string((const char *)parvalue);
		else
			printf("RcParser: ignoring parameter '%s' in tag <dbsettings/>\n", parname);

	    xmlFree(parname);
	    xmlFree(parvalue);
	}
}



void MMSRcParser::check_outputtype(MMSFBOutputType outputtype, xmlChar *parname, xmlChar *parvalue) {
	string val = string((const char *)parvalue);

	// check value against backend type
	if (this->graphics.backend == MMSFB_BE_DFB) {
		switch (outputtype) {
		case MMSFB_OT_STDFB:
		case MMSFB_OT_MATROXFB:
		case MMSFB_OT_VIAFB:
		case MMSFB_OT_X11:
		case MMSFB_OT_DAVINCIFB:
		case MMSFB_OT_OMAPFB:
			// okay
			break;
		default:
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_DFB, "-> this depends on backend=\"DFB\"");
		}
	}
	else
	if (this->graphics.backend == MMSFB_BE_X11) {
		switch (outputtype) {
		case MMSFB_OT_X11:
		case MMSFB_OT_XSHM:
		case MMSFB_OT_XVSHM:
		case MMSFB_OT_OGL:
			// okay
			break;
		default:
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_X11, "-> this depends on backend=\"X11\"");
		}
	}
	else
	if (this->graphics.backend == MMSFB_BE_FBDEV) {
		switch (outputtype) {
		case MMSFB_OT_STDFB:
		case MMSFB_OT_MATROXFB:
		case MMSFB_OT_DAVINCIFB:
		case MMSFB_OT_OMAPFB:
		case MMSFB_OT_OGL:
			// okay
			break;
		default:
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_FBDEV, "-> this depends on backend=\"FBDEV\"");
		}
	}
	else
    if (this->graphics.backend == MMSFB_BE_L4RE) {
        switch (outputtype) {
        case MMSFB_OT_L4FB:
            // okay
            break;
        default:
            WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_L4RE, "-> this depends on backend=\"L4RE\"");
        }
    }
}


void MMSRcParser::get_outputtype(THROUGH_GRAPHICS_MODE mode, xmlChar *parname, xmlChar *parvalue) {
	string val = string((const char *)parvalue);

	// get/check value
	switch (mode) {
	case THROUGH_GRAPHICS_MODE_NORMAL:
		this->graphics.videolayer.outputtype = this->graphics.graphicslayer.outputtype = getMMSFBOutputTypeFromString(strToUpr(val));
		if (this->graphics.videolayer.outputtype == MMSFB_OT_NONE)
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES, "");
		check_outputtype(this->graphics.graphicslayer.outputtype, parname, parvalue);
		break;
	case THROUGH_GRAPHICS_MODE_VIDEOLAYER:
		this->graphics.videolayer.outputtype = getMMSFBOutputTypeFromString(strToUpr(val));
		if (this->graphics.videolayer.outputtype == MMSFB_OT_NONE)
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES, "");
		check_outputtype(this->graphics.videolayer.outputtype, parname, parvalue);
		break;
	case THROUGH_GRAPHICS_MODE_GRAPHICSLAYER:
		this->graphics.graphicslayer.outputtype = getMMSFBOutputTypeFromString(strToUpr(val));
		if (this->graphics.graphicslayer.outputtype == MMSFB_OT_NONE)
			WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES, "");
		check_outputtype(this->graphics.graphicslayer.outputtype, parname, parvalue);
		break;
	}
}

void MMSRcParser::get_xres(THROUGH_GRAPHICS_MODE mode, xmlChar *parvalue) {
	switch (mode) {
	case THROUGH_GRAPHICS_MODE_NORMAL:
		this->graphics.videolayer.rect.w = this->graphics.graphicslayer.rect.w = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_VIDEOLAYER:
		this->graphics.videolayer.rect.w = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_GRAPHICSLAYER:
		this->graphics.graphicslayer.rect.w = strToInt(string((const char *)parvalue));
		break;
	}
}


void MMSRcParser::get_yres(THROUGH_GRAPHICS_MODE mode, xmlChar *parvalue) {
	switch (mode) {
	case THROUGH_GRAPHICS_MODE_NORMAL:
		this->graphics.videolayer.rect.h = this->graphics.graphicslayer.rect.h = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_VIDEOLAYER:
		this->graphics.videolayer.rect.h = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_GRAPHICSLAYER:
		this->graphics.graphicslayer.rect.h = strToInt(string((const char *)parvalue));
		break;
	}
}


void MMSRcParser::get_xpos(THROUGH_GRAPHICS_MODE mode, xmlChar *parvalue) {
	switch (mode) {
	case THROUGH_GRAPHICS_MODE_NORMAL:
		this->graphics.videolayer.rect.x = this->graphics.graphicslayer.rect.x = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_VIDEOLAYER:
		this->graphics.videolayer.rect.x = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_GRAPHICSLAYER:
		this->graphics.graphicslayer.rect.x = strToInt(string((const char *)parvalue));
		break;
	}
}


void MMSRcParser::get_ypos(THROUGH_GRAPHICS_MODE mode, xmlChar *parvalue) {
	switch (mode) {
	case THROUGH_GRAPHICS_MODE_NORMAL:
		this->graphics.videolayer.rect.y = this->graphics.graphicslayer.rect.y = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_VIDEOLAYER:
		this->graphics.videolayer.rect.y = strToInt(string((const char *)parvalue));
		break;
	case THROUGH_GRAPHICS_MODE_GRAPHICSLAYER:
		this->graphics.graphicslayer.rect.y = strToInt(string((const char *)parvalue));
		break;
	}
}


void MMSRcParser::throughGraphics(xmlNode* node, THROUGH_GRAPHICS_MODE mode) {
	xmlNode *cur_node = NULL;
	xmlChar *parname;
	xmlChar *parvalue;
	MMSConfigDataLayer vl, gl;

	if (mode == THROUGH_GRAPHICS_MODE_NORMAL) {
		// save the old settings and set id to -1
		vl = this->graphics.videolayer;
		gl = this->graphics.graphicslayer;
		this->graphics.videolayer.id = -1;
		this->graphics.graphicslayer.id = -1;
	}

	node = node->xmlChildrenNode;

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "text")) continue;
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "comment"))	continue;
		if(xmlStrcmp(cur_node->name, (const xmlChar *) "parameter")) {
			if (mode == THROUGH_GRAPHICS_MODE_NORMAL) {
				if(!xmlStrcmp(cur_node->name, (const xmlChar *) "videolayer")) {
					// we detect the <videolayer/> tag inside the graphics section
					throughGraphics(cur_node, THROUGH_GRAPHICS_MODE_VIDEOLAYER);
					continue;
				}
				else
				if(!xmlStrcmp(cur_node->name, (const xmlChar *) "graphicslayer")) {
					// we detect the <graphicslayer/> tag inside the graphics section
					throughGraphics(cur_node, THROUGH_GRAPHICS_MODE_GRAPHICSLAYER);
					continue;
				}
			}
			printf("RcParser: ignoring tag <%s/>\n", cur_node->name);
			continue;
		}

		// get next parameter/value pair
    	parname  = xmlGetProp(cur_node, (const xmlChar*)"name");
    	parvalue = xmlGetProp(cur_node, (const xmlChar*)"value");

    	// checking all supported parameters
		if(!xmlStrcmp(parname, (const xmlChar *) "outputtype")) {
        	// get the outputtype of the layer(s)
        	get_outputtype(mode, parname, parvalue);
		}
		else if(!xmlStrcmp(parname, (const xmlChar *) "xres")) {
        	// get the width of the layer(s)
        	get_xres(mode, parvalue);
		}
	    else if(!xmlStrcmp(parname, (const xmlChar *) "yres")) {
        	// get the height of the layer(s)
        	get_yres(mode, parvalue);
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "xpos")) {
        	// get the x-position of the layer(s)
        	get_xpos(mode, parvalue);
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "ypos")) {
        	// get the y-position of the layer(s)
        	get_ypos(mode, parvalue);
	    } else if(!xmlStrcmp(parname, (const xmlChar *) "graphicslayerid")) {
			this->graphics.graphicslayer.id = atoi((const char *)parvalue);
	    }
		else if(!xmlStrcmp(parname, (const xmlChar *) "graphicslayerpixelformat")) {
        	string val = string((const char *)parvalue);
            if ((this->graphics.graphicslayer.pixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE)
            	WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_LAYER, "");
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "graphicslayeroptions")) {
	        this->graphics.graphicslayer.options = strToUpr(string((const char *)parvalue));
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "graphicslayerbuffermode")) {
	    	if((this->graphics.graphicslayer.outputtype == MMSFB_OT_X11) &&
	    	   xmlStrEqual(parvalue, (const xmlChar *) "FRONTONLY")) {
	    		WRONG_VALUE(parname, string((const char *)parvalue), MMSFB_PF_VALID_BUFFERMODES_X11, "-> this depends on graphicslayer.outputtype=\"X11\"");
	    	} else if((xmlStrcasecmp(parvalue, (const xmlChar *) "BACKSYSTEM") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "BACKVIDEO") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "FRONTONLY") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "TRIPLE") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "WINDOWS") != 0)) {
            	WRONG_VALUE(parname, string((const char *)parvalue), MMSFB_PF_VALID_BUFFERMODES, "");
	    	} else {
	    		this->graphics.graphicslayer.buffermode = strToUpr((const char *)parvalue);
	    	}
	    }
		else if(!xmlStrcmp(parname, (const xmlChar *) "videolayerid")) {
			this->graphics.videolayer.id = atoi((const char *)parvalue);
		}
	    else if(!xmlStrcmp(parname, (const xmlChar *) "videolayerpixelformat")) {
        	string val = string((const char *)parvalue);
            if ((this->graphics.videolayer.pixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE)
            	WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_LAYER, "");
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "videolayeroptions")) {
	        this->graphics.videolayer.options = strToUpr(string((const char *)parvalue));
	    }
	    else if(!xmlStrcmp(parname, (const xmlChar *) "videolayerbuffermode")) {
	    	if((this->graphics.videolayer.outputtype == MMSFB_OT_X11) &&
	    	   xmlStrEqual(parvalue, (const xmlChar *) "FRONTONLY")) {
	    		WRONG_VALUE(parname, string((const char *)parvalue), MMSFB_PF_VALID_BUFFERMODES_X11, "-> this depends on videolayer.outputtype=\"X11\"");
	    	} else if((xmlStrcasecmp(parvalue, (const xmlChar *) "BACKSYSTEM") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "BACKVIDEO") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "FRONTONLY") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "TRIPLE") != 0) &&
	    	   (xmlStrcasecmp(parvalue, (const xmlChar *) "WINDOWS") != 0)) {
            	WRONG_VALUE(parname, string((const char *)parvalue), MMSFB_PF_VALID_BUFFERMODES, "");
	    	} else {
	    		this->graphics.videolayer.buffermode = strToUpr((const char *)parvalue);
	    	}
		} else if (mode == THROUGH_GRAPHICS_MODE_NORMAL) {
			// through parameters which are used outside <graphicslayer/> and <videolayer/>
			if(!xmlStrcmp(parname, (const xmlChar *) "backend")) {
				string val = string((const char *)parvalue);
				// get/check value
				if ((this->graphics.backend = getMMSFBBackendFromString(strToUpr(val))) == MMSFB_BE_NONE)
					WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES, "");
/*
				// check value against output type
				if (this->graphics.outputtype == MMSFB_OT_STDFB) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_FBDEV:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_FB, "-> this depends on outputtype=\"STDFB\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_MATROXFB) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_FBDEV:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_FB, "-> this depends on outputtype=\"MATROXFB\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_VIAFB) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_FBDEV:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_FB, "-> this depends on outputtype=\"VIAFB\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_X11) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_X11:
					case MMSFB_BE_OGL:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_X11, "-> this depends on outputtype=\"X11\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_XSHM) {
					switch (this->graphics.backend) {
					case MMSFB_BE_X11:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_X, "-> this depends on outputtype=\"XSHM\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_XVSHM) {
					switch (this->graphics.backend) {
					case MMSFB_BE_X11:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_X, "-> this depends on outputtype=\"XVSHM\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_DAVINCIFB) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_FBDEV:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_FB, "-> this depends on outputtype=\"DAVINCIFB\"");
					}
				}
				else
				if (this->graphics.outputtype == MMSFB_OT_OMAPFB) {
					switch (this->graphics.backend) {
					case MMSFB_BE_DFB:
					case MMSFB_BE_FBDEV:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_BE_VALID_VALUES_OT_FB, "-> this depends on outputtype=\"OMAPFB\"");
					}
				}*/

			}

/*			else if(!xmlStrcmp(parname, (const xmlChar *) "outputtype")) {
				string val = string((const char *)parvalue);
				// get/check value
				if ((this->graphics.outputtype = getMMSFBOutputTypeFromString(strToUpr(val))) == MMSFB_OT_NONE)
					WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES, "");

				// check value against backend type
				if (this->graphics.backend == MMSFB_BE_DFB) {
					switch (this->graphics.outputtype) {
					case MMSFB_OT_STDFB:
					case MMSFB_OT_MATROXFB:
					case MMSFB_OT_VIAFB:
					case MMSFB_OT_X11:
					case MMSFB_OT_DAVINCIFB:
					case MMSFB_OT_OMAPFB:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_DFB, "-> this depends on backend=\"DFB\"");
					}
				}
				else
				if (this->graphics.backend == MMSFB_BE_X11) {
					switch (this->graphics.outputtype) {
					case MMSFB_OT_X11:
					case MMSFB_OT_XSHM:
					case MMSFB_OT_XVSHM:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_X11, "-> this depends on backend=\"X11\"");
					}
				}
				else
				if (this->graphics.backend == MMSFB_BE_FBDEV) {
					switch (this->graphics.outputtype) {
					case MMSFB_OT_STDFB:
					case MMSFB_OT_MATROXFB:
					case MMSFB_OT_DAVINCIFB:
					case MMSFB_OT_OMAPFB:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_FBDEV, "-> this depends on backend=\"FBDEV\"");
					}
				}
				else
				if (this->graphics.backend == MMSFB_BE_OGL) {
					switch (this->graphics.outputtype) {
					case MMSFB_OT_X11:
						// okay
						break;
					default:
						WRONG_VALUE(parname, val, MMSFB_OT_VALID_VALUES_BE_OGL, "-> this depends on backend=\"OGL\"");
					}
				}
			}*/
			else if(!xmlStrcmp(parname, (const xmlChar *) "vrect.x")) {
				this->graphics.vrect.x = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "vrect.y")) {
				this->graphics.vrect.y = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "vrect.w")) {
				this->graphics.vrect.w = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "vrect.h")) {
				this->graphics.vrect.h = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "touchrect.x")) {
				this->graphics.touchrect.x = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "touchrect.y")) {
				this->graphics.touchrect.y = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "touchrect.w")) {
				this->graphics.touchrect.w = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "touchrect.h")) {
				this->graphics.touchrect.h = strToInt(string((const char *)parvalue));
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "pointer")) {
				string val = string((const char *)parvalue);
				if ((this->graphics.pointer = getMMSFBPointerModeFromString(strToUpr(val))) == MMSFB_PM_NONE)
					WRONG_VALUE(parname, val, MMSFB_PM_VALID_VALUES, "");
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "graphicswindowpixelformat")) {
				string val = string((const char *)parvalue);
				if ((this->graphics.graphicswindowpixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE) {
					string val2 = strToUpr(val);
					if ((val2 != "") && (val2 != "AUTODETECT") && (val2 != "AUTO"))
						WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_WINDOWS, "");
				}
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "graphicssurfacepixelformat")) {
				string val = string((const char *)parvalue);
				if ((this->graphics.graphicssurfacepixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE) {
					string val2 = strToUpr(val);
					if ((val2 != "") && (val2 != "AUTODETECT") && (val2 != "AUTO"))
						WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_SURFACES, "");
				}
			}
			else if(!xmlStrcmp(parname, (const xmlChar *) "extendedaccel"))
				this->graphics.extendedaccel = strToBool(string((const char *)parvalue));
			else if(!xmlStrcmp(parname, (const xmlChar *) "allocmethod"))
				this->graphics.allocmethod = strToUpr(string((const char *)parvalue));
			else if(!xmlStrcmp(parname, (const xmlChar *) "fullscreen")) {
				string val = string((const char *)parvalue);
				if ((this->graphics.fullscreen = getMMSFBFullScreenModeFromString(strToUpr(val))) == MMSFB_FSM_NONE)
					WRONG_VALUE(parname, val, MMSFB_FSM_VALID_VALUES, "");
			} else if(!xmlStrcmp(parname, (const xmlChar *) "rotatescreen")) {
				string val = string((const char *)parvalue);
				this->graphics.rotatescreen = strToInt(val);
				if (this->graphics.rotatescreen != 0 && this->graphics.rotatescreen != 180)
					WRONG_VALUE(parname, val, "0, 180", "");
			} else if(!xmlStrcmp(parname, (const xmlChar *) "hideapplication")) {
				this->graphics.hideapplication = strToBool(string((const char *)parvalue));
			} else if(!xmlStrcmp(parname, (const xmlChar *) "initialload")) {
				this->graphics.initialload = strToBool(string((const char *)parvalue));
			} else if(!xmlStrcmp(parname, (const xmlChar *) "debugframes")) {
				this->graphics.debugframes = strToBool(string((const char *)parvalue));
			} else if(!xmlStrcmp(parname, (const xmlChar *) "touchSwapX")) {
				this->graphics.touchSwapX = strToBool(string((const char *)parvalue));
			} else if(!xmlStrcmp(parname, (const xmlChar *) "touchSwapY")) {
				this->graphics.touchSwapY = strToBool(string((const char *)parvalue));
			} else if(!xmlStrcmp(parname, (const xmlChar *) "touchSwapXY")) {
				this->graphics.touchSwapXY = strToBool(string((const char *)parvalue));
			}
			else
				printf("RcParser: ignoring parameter '%s' in tag <graphics/>\n", parname);
	    }
	    else {
	    	if (mode == THROUGH_GRAPHICS_MODE_GRAPHICSLAYER) {
				// through parameters which are used inside <graphicslayer/>
	    		if(!xmlStrcmp(parname, (const xmlChar *) "id")) {
	    			this->graphics.graphicslayer.id = atoi((const char *)parvalue);
	    		}
	    		else if(!xmlStrcmp(parname, (const xmlChar *) "pixelformat")) {
	            	string val = string((const char *)parvalue);
	                if ((this->graphics.graphicslayer.pixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE)
	                	WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_LAYER, "");
	    	    }
	    	    else if(!xmlStrcmp(parname, (const xmlChar *) "options")) {
	    	        this->graphics.graphicslayer.options = strToUpr(string((const char *)parvalue));
	    	    }
	    	    else if(!xmlStrcmp(parname, (const xmlChar *) "buffermode")) {
	    	        this->graphics.graphicslayer.buffermode = strToUpr(string((const char *)parvalue));
	    	    }
	    	    else {
	    	    	printf("RcParser: ignoring parameter '%s' in tag <graphicslayer/>\n", parname);
	    	    }
	    	}
	    	else if (mode == THROUGH_GRAPHICS_MODE_VIDEOLAYER) {
				// through parameters which are used inside <videolayer/>
	    		if(!xmlStrcmp(parname, (const xmlChar *) "id")) {
	    			this->graphics.videolayer.id = atoi((const char *)parvalue);
	    		}
	    	    else if(!xmlStrcmp(parname, (const xmlChar *) "pixelformat")) {
	            	string val = string((const char *)parvalue);
	                if ((this->graphics.videolayer.pixelformat = getMMSFBPixelFormatFromString(strToUpr(val))) == MMSFB_PF_NONE)
	                	WRONG_VALUE(parname, val, MMSFB_PF_VALID_VALUES_LAYER, "");
	    	    }
	    	    else if(!xmlStrcmp(parname, (const xmlChar *) "options")) {
	    	        this->graphics.videolayer.options = strToUpr(string((const char *)parvalue));
	    	    }
	    	    else if(!xmlStrcmp(parname, (const xmlChar *) "buffermode")) {
	    	        this->graphics.videolayer.buffermode = strToUpr(string((const char *)parvalue));
	    	    }
	    	    else {
	    	    	printf("RcParser: ignoring parameter '%s' in tag <videolayer/>\n", parname);
	    	    }
	    	}
			else {
				printf("RcParser: ignoring parameter '%s' in tag <.../>\n", parname);
			}
	    }

	    xmlFree(parname);
	    xmlFree(parvalue);

	}

	// special modes?
	if (mode != THROUGH_GRAPHICS_MODE_NORMAL)
		return;

	// doing final checks...................
	// layer ids set?
	if (this->graphics.videolayer.id < 0) {
		// video layer id not set!
		if (this->graphics.graphicslayer.id < 0) {
			// graphics layer id not set, using defaults
			this->graphics.videolayer = vl;
			this->graphics.graphicslayer = gl;
			printf("RcParser: video and graphics layer not correctly set, using defaults\n");
		}
		else {
			// using graphics layer settings also for video layer
			this->graphics.videolayer = this->graphics.graphicslayer;
			printf("RcParser: video layer not correctly set, using graphics layer settings for both\n");
		}
	}
	else {
		if (this->graphics.graphicslayer.id < 0) {
			// using video layer settings also for graphics layer
			this->graphics.graphicslayer = this->graphics.videolayer;
			printf("RcParser: graphics layer not correctly set, using video layer settings for both\n");
		}
		else {
			if (this->graphics.videolayer.id == this->graphics.graphicslayer.id) {
				if   (this->graphics.videolayer.rect.w != this->graphics.graphicslayer.rect.w
					||this->graphics.videolayer.rect.h != this->graphics.graphicslayer.rect.h
					||this->graphics.videolayer.rect.x != this->graphics.graphicslayer.rect.x
					||this->graphics.videolayer.rect.y != this->graphics.graphicslayer.rect.y
					||this->graphics.videolayer.outputtype != this->graphics.graphicslayer.outputtype
					||this->graphics.videolayer.pixelformat != this->graphics.graphicslayer.pixelformat
					||this->graphics.videolayer.options != this->graphics.graphicslayer.options
					||this->graphics.videolayer.buffermode != this->graphics.graphicslayer.buffermode) {
					// same layers, but different definitions!
					this->graphics.videolayer = this->graphics.graphicslayer;
					printf("RcParser: video and graphics layer are the same, but definitions are not consistent, using graphics layer settings for both\n");
				}
			}
		}
	}

	// checking layer ids
	if (this->graphics.backend == MMSFB_BE_X11) {
		if (this->graphics.videolayer.id != 0 && this->graphics.videolayer.id != 1) {
			WRONG_VALUE("videolayer.id", iToStr(this->graphics.videolayer.id), "0, 1", "-> this depends on backend=\"X11\", outputtype=\"X11/XVSHM\"");
		}
		if ((this->graphics.videolayer.id == 1) && (this->graphics.videolayer.outputtype != MMSFB_OT_XVSHM)) {
			WRONG_VALUE("videolayer.outputtype", getMMSFBOutputTypeString(this->graphics.videolayer.outputtype), "XVSHM", "-> this depends on backend=\"X11\", videolayer.id=\"1\"");
		}
		if (this->graphics.graphicslayer.id != 0) {
			WRONG_VALUE("graphicslayer.id", iToStr(this->graphics.graphicslayer.id), "0", "-> this depends on backend=\"X11\", outputtype=\"X11/XSHM/XVSHM/OGL\"");
		}

/*		switch (this->graphics.outputtype) {
		case MMSFB_OT_X11:
		case MMSFB_OT_XVSHM:
			if (this->graphics.videolayer.id != 0 && this->graphics.videolayer.id != 1)
				WRONG_VALUE("videolayerid", iToStr(this->graphics.videolayer.id), "0, 1", "-> this depends on backend=\"X11\", outputtype=\"X11/XVSHM\"");
			if (this->graphics.graphicslayer.id != 0)
				WRONG_VALUE("graphicslayerid", iToStr(this->graphics.graphicslayer.id), "0", "-> this depends on backend=\"X11\", outputtype=\"X11/XVSHM\"");
			break;
		case MMSFB_OT_XSHM:
			if (this->graphics.videolayer.id != 0 && this->graphics.videolayer.id != 1)
				WRONG_VALUE("videolayerid", iToStr(this->graphics.videolayer.id), "0, 1", "-> this depends on backend=\"X11\", outputtype=\"XSHM\"");
			if (this->graphics.graphicslayer.id != 0)
				WRONG_VALUE("graphicslayerid", iToStr(this->graphics.graphicslayer.id), "0", "-> this depends on backend=\"X11\", outputtype=\"XSHM\"");
			break;
		default:
			break;
		}*/
	}
	else
	if (this->graphics.backend == MMSFB_BE_FBDEV) {
		switch (this->graphics.graphicslayer.outputtype) {
		case MMSFB_OT_DAVINCIFB:
			if (this->graphics.videolayer.id != this->graphics.graphicslayer.id) {
				if ((this->graphics.videolayer.id != 1)&&(this->graphics.videolayer.id != 2))
					WRONG_VALUE("videolayerid", iToStr(this->graphics.videolayer.id), "1, 2", "-> this depends on backend=\"FBDEV\", outputtype=\"DAVINCIFB\"");
				if (this->graphics.graphicslayer.id != 0)
					WRONG_VALUE("graphicslayerid", iToStr(this->graphics.graphicslayer.id), "0", "-> this depends on backend=\"FBDEV\", outputtype=\"DAVINCIFB\"");
			}
			break;
/*		case MMSFB_OT_OMAPFB:
			if (this->graphics.videolayer.id != this->graphics.graphicslayer.id) {
				if (this->graphics.videolayer.id != 1)
					WRONG_VALUE("videolayerid", iToStr(this->graphics.videolayer.id), "1", "-> this depends on backend=\"FBDEV\", outputtype=\"OMAPFB\"");
				if ((this->graphics.graphicslayer.id != 0)&&(this->graphics.graphicslayer.id != 2))
					WRONG_VALUE("graphicslayerid", iToStr(this->graphics.graphicslayer.id), "0, 2", "-> this depends on backend=\"FBDEV\", outputtype=\"OMAPFB\"");
			}
			break;*/
		default:
			break;
		}
	}

	// checking pixelformats
	if (this->graphics.backend == MMSFB_BE_X11) {
		switch (this->graphics.graphicslayer.outputtype) {
		case MMSFB_OT_X11:
		case MMSFB_OT_XVSHM:
		case MMSFB_OT_XSHM:
			if (this->graphics.graphicslayer.pixelformat != MMSFB_PF_YV12 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB32 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB24 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_ARGB)
				WRONG_VALUE("graphicslayer.pixelformat", getMMSFBPixelFormatString(this->graphics.graphicslayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_X11_OT_XSHM, "-> this depends on backend=\"X11\", outputtype=\"XSHM\"");
			break;
		case MMSFB_OT_OGL:
			if (this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB32 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_ARGB
					 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_ABGR)
				WRONG_VALUE("graphicslayer.pixelformat", getMMSFBPixelFormatString(this->graphics.graphicslayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OGL, "-> this depends on backend=\"FBDEV\", outputtype=\"OGL\"");
			break;
		default:
			break;
		}
		if (this->graphics.graphicslayer.id != this->graphics.videolayer.id) {
			switch (this->graphics.videolayer.outputtype) {
			case MMSFB_OT_X11:
			case MMSFB_OT_XVSHM:
			case MMSFB_OT_XSHM:
				if (this->graphics.videolayer.pixelformat != MMSFB_PF_YV12 && this->graphics.videolayer.pixelformat != MMSFB_PF_RGB32 && this->graphics.videolayer.pixelformat != MMSFB_PF_ARGB)
					WRONG_VALUE("videolayer.pixelformat", getMMSFBPixelFormatString(this->graphics.videolayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_X11_OT_XVSHM, "-> this depends on backend=\"X11\", outputtype=\"X11/XVSHM\"");
				break;
			default:
				break;
			}
		}
	}
	else
	if (this->graphics.backend == MMSFB_BE_FBDEV) {
		switch (this->graphics.graphicslayer.outputtype) {
		case MMSFB_OT_DAVINCIFB:
			if (this->graphics.videolayer.pixelformat != MMSFB_PF_YUY2)
				WRONG_VALUE("videolayer.pixelformat", getMMSFBPixelFormatString(this->graphics.videolayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_DAVINCIFB_LAYER_1, "-> this depends on backend=\"FBDEV\", outputtype=\"DAVINCIFB\"");
			if   ((this->graphics.graphicslayer.pixelformat != MMSFB_PF_ARGB3565)
				&&(this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB16))
				WRONG_VALUE("graphicslayer.pixelformat", getMMSFBPixelFormatString(this->graphics.graphicslayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_DAVINCIFB_LAYER_0, "-> this depends on backend=\"FBDEV\", outputtype=\"DAVINCIFB\"");
			break;
/*		case MMSFB_OT_OMAPFB:
			if   ((this->graphics.videolayer.pixelformat != MMSFB_PF_YUY2)
				&&(this->graphics.videolayer.pixelformat != MMSFB_PF_RGB32))
				WRONG_VALUE("videolayerpixelformat", getMMSFBPixelFormatString(this->graphics.videolayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OMAPFB_LAYER_1, "-> this depends on backend=\"FBDEV\", outputtype=\"OMAPFB\"");
			if   ((this->graphics.graphicslayer.pixelformat != MMSFB_PF_ARGB)
				&&(this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB32)
				&&(this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB16))
				WRONG_VALUE("graphicslayerpixelformat", getMMSFBPixelFormatString(this->graphics.graphicslayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OMAPFB_LAYER_0, "-> this depends on backend=\"FBDEV\", outputtype=\"OMAPFB\"");
			break;*/

		case MMSFB_OT_OGL:
			if (this->graphics.graphicslayer.pixelformat != MMSFB_PF_RGB32 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_ARGB
					 && this->graphics.graphicslayer.pixelformat != MMSFB_PF_ABGR)
				WRONG_VALUE("graphicslayer.pixelformat", getMMSFBPixelFormatString(this->graphics.graphicslayer.pixelformat), MMSFB_PF_VALID_VALUES_BE_FBDEV_OT_OGL, "-> this depends on backend=\"FBDEV\", outputtype=\"OGL\"");
			break;

		default:
			break;
		}

		if (this->graphics.graphicslayer.id != this->graphics.videolayer.id) {
			//TODO...
			switch (this->graphics.videolayer.outputtype) {
			default:
				break;
			}
		}
	}

    if ((this->graphics.backend == MMSFB_BE_X11)||(this->graphics.backend == MMSFB_BE_FBDEV)||(this->graphics.backend == MMSFB_BE_L4RE)) {
		// overwite values needed for this backends
		this->graphics.extendedaccel = true;
		this->graphics.allocmethod = "MALLOC";

		if (this->graphics.graphicslayer.outputtype == MMSFB_OT_OGL) {
			// overwite values needed for this outputtype
			this->graphics.extendedaccel = false;
			this->graphics.allocmethod = "OGL";
		}
	}
}

void MMSRcParser::throughFile(xmlNode* node) {

	xmlNode *cur_node = NULL;

	if(node==NULL)
		return;

	if(!xmlStrcmp(node->name, (const xmlChar *) "mmsrc") || !xmlStrcmp(node->name, (const xmlChar *) "diskorc")) {
		checkVersion(node);

		node = node->xmlChildrenNode;
    }
	else {
		this->throughFile(node->next);
		return;
	}

	for (cur_node = node; cur_node; cur_node = cur_node->next) {

		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "text")) continue;
		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "comment"))	continue;

		if(!xmlStrcmp(cur_node->name, (const xmlChar *) "global")) {
			throughGlobal(cur_node);
		}
		else if(!xmlStrcmp(cur_node->name, (const xmlChar *) "dbsettings")) {
			throughDBSettings(cur_node);
		}
		else if((!xmlStrcmp(cur_node->name, (const xmlChar *) "dfbsettings"))
			  ||(!xmlStrcmp(cur_node->name, (const xmlChar *) "graphics"))) {
			throughGraphics(cur_node);
		}
		else if(!xmlStrcmp(cur_node->name, (const xmlChar *) "language")) {
			throughLanguage(cur_node);
		}
		else {
			printf("RcParser: ignoring tag <%s/>\n", cur_node->name);
		}
	}
}


void MMSRcParser::updateConfigParms(MMSConfigData *config, char *ap) {
// helpers
#define MMSRC_CPP_GET_PARAMETER(str,len) if(memcmp(ap,str,len)==0){char *val=ap+len;while(*val&&*val==' ')val++;char *vb=valbuf;*vb=0;while(*val&&*val!=' '){*vb=*val;vb++;*vb=0;val++;}val=valbuf;
#define MMSRC_CPP_PROCESS_PIXELFORMAT_PARAMETER(str,len,setter) MMSRC_CPP_GET_PARAMETER(str,len)MMSFBSurfacePixelFormat pf=getMMSFBPixelFormatFromString(strToUpr(val));if(pf!=MMSFB_PF_NONE)setter else printf("DISKO: Parameter --disko:%s must be a valid pixelformat!\n",str);}
#define MMSRC_CPP_PROCESS_BOOL_PARAMETER(str,len,truecall,falsecall) MMSRC_CPP_GET_PARAMETER(str,len)if((!strcmp(val,"true"))||(!strcmp(val,"TRUE")))truecall else if((!strcmp(val,"false"))||(!strcmp(val,"FALSE")))falsecall else printf("DISKO: Parameter --disko:%s must be true or false!\n",str);}

// available commandline parameters
#define MMSRC_CCP_GRAPHICS_VIDEOLAYER_PIXELFORMAT_STR		"graphics.videolayer.pixelformat="
#define MMSRC_CCP_GRAPHICS_VIDEOLAYER_PIXELFORMAT_LEN		32
#define MMSRC_CCP_GRAPHICS_GRAPHICSLAYER_PIXELFORMAT_STR	"graphics.graphicslayer.pixelformat="
#define MMSRC_CCP_GRAPHICS_GRAPHICSLAYER_PIXELFORMAT_LEN	35
#define MMSRC_CCP_GRAPHICS_FULLSCREEN_STR					"graphics.fullscreen="
#define MMSRC_CCP_GRAPHICS_FULLSCREEN_LEN					20
#define MMSRC_CCP_GRAPHICS_HIDEAPPLICATION_STR				"graphics.hideapplication="
#define MMSRC_CCP_GRAPHICS_HIDEAPPLICATION_LEN				25

	//  buffer for value
	char valbuf[256];

	// check all my parameters
	MMSRC_CPP_PROCESS_PIXELFORMAT_PARAMETER(MMSRC_CCP_GRAPHICS_VIDEOLAYER_PIXELFORMAT_STR, MMSRC_CCP_GRAPHICS_VIDEOLAYER_PIXELFORMAT_LEN,
											{MMSConfigDataLayer l=config->getVideoLayer();l.pixelformat=pf;config->setVideoLayer(l);})
	else
	MMSRC_CPP_PROCESS_PIXELFORMAT_PARAMETER(MMSRC_CCP_GRAPHICS_GRAPHICSLAYER_PIXELFORMAT_STR, MMSRC_CCP_GRAPHICS_GRAPHICSLAYER_PIXELFORMAT_LEN,
											{MMSConfigDataLayer l=config->getGraphicsLayer();l.pixelformat=pf;config->setGraphicsLayer(l);})
	else
	MMSRC_CPP_GET_PARAMETER(MMSRC_CCP_GRAPHICS_FULLSCREEN_STR, MMSRC_CCP_GRAPHICS_FULLSCREEN_LEN) {
		MMSFBFullScreenMode fsm = getMMSFBFullScreenModeFromString(strToUpr(val));
		if (fsm != MMSFB_FSM_NONE)
			config->setFullScreen(fsm);
		else
			printf("DISKO: Parameter --disko:%s must be a valid fullscreen mode (%s)!\n",
					MMSRC_CCP_GRAPHICS_FULLSCREEN_STR, MMSFB_FSM_VALID_VALUES);
	}}
	else
	MMSRC_CPP_PROCESS_BOOL_PARAMETER(MMSRC_CCP_GRAPHICS_HIDEAPPLICATION_STR, MMSRC_CCP_GRAPHICS_HIDEAPPLICATION_LEN,
									 {config->setHideApplication(true);},{config->setHideApplication(false);});
}

void MMSRcParser::updateConfig(MMSConfigData *config, string args, int argc, char *argv[]) {

	if (!config) return;

	// args...
	char *ap = (char*)args.c_str();
	while (*ap) {
		// find parameter
		if (!(ap = strstr(ap, "--disko:"))) break;
		ap+= 8;

		// update parameter
		updateConfigParms(config, ap);

		// go to next parameter
		if (!(ap = strstr(ap, " "))) break;
		ap++;
	}

	// argv...
	for (int i = 1; i < argc; i++) {
		// find parameter
		char *ap= argv[i];
		if (memcmp(ap, "--disko:", 8)) continue;
		ap+= 8;

		// update parameter
		updateConfigParms(config, ap);
	}
}

