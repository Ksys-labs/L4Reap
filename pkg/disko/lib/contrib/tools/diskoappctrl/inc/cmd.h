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

#ifndef MMSCMD_H_
#define MMSCMD_H_

#include <mms.h>

#include "console.h"

#define MMSCMD_LIST    "list"
#define MMSCMD_UPDATE  "update"
#define MMSCMD_ACT	   "act"
#define MMSCMD_DEACT   "deact"
#define MMSCMD_EXEC    "execute"
#define MMSCMD_EVENT   "event"
#define MMSCMD_ADD     "add"
#define MMSCMD_VERSION "version"


#define MMSCMD_CMD       "cmd"
#define MMSCMD_PLUGIN    "plugin"
#define MMSCMD_SOURCE    "source"
#define MMSCMD_PARAM     "param"
#define MMSCMD_IMPORT    "import"
#define MMSCMD_DEBUG     "debug"
#define MMSCMD_TYPE      "type"
#define MMSCMD_VALUE     "value"
#define MMSCMD_CONFIG    "config"
#define MMSCMD_HEADING   "heading"
#define MMSCMD_PORT      "port"
#define MMSCMD_TYPE      "type"
#define MMSCMD_MINVALUE  "minvalue"
#define MMSCMD_MAXVALUE  "maxvalue"
#define MMSCMD_VALUELIST "valuelist"
#define MMSCMD_SEPARATOR "separator"

typedef enum  {
	UNKNOWN = 0,
	LIST,
	UPDATE,
	ACT,
	DEACT,
    EXEC,
    EVENT,
    ADD,
    VERSION
} MMSCMD_COMMANDS;

typedef map<string,string> CmdLine;

class Cmd {

	private:
		bool debug;
		string configfile;
		CmdLine cmdline;
		DataSource *datasource;
		Console cons;

		void executeExec();
		void executeEvent();
		void executeList();
		void executeUpdate();
		void executeAct();
		void executeDeact();
		void executeVersion();

		void prepareDb();

		void updateSource();
		void updateParameter();

		MMSCMD_COMMANDS getCommand();

		void printPlugin(MMSPluginData *plugin, bool full);
		void printImportproperty(MMSImportPropertyData *prop);
		void printImportsource(MMSImportSourceData *src);
		void printWorkingdata(MMSConfigData *config);

	public:
		Cmd(CmdLine &cmdline);
		~Cmd();

		void handleRequest();
};

#endif /*MMSCMD_H_*/
