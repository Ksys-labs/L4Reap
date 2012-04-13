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

#include <cstring>
#include "mms.h"

void help() {
	printf("\nActivity Monitor\n\n");
	printf("parameter:\n\n");
	printf("--reset    if used, statistic infos will be reset\n");
	printf("\nexamples:\n\n");
	printf("actmon\n\n");
	printf("actmon --disko:config=./etc/diskorc.xml\n\n");
	printf("actmon --reset\n\n");
	printf("actmon --disko:config=./etc/diskorc.xml --reset\n\n");
}

bool getparams(int argc, char *argv[], bool &reset) {

	reset = false;

    //check if --reset is given per commandline
    for (int i = 1; i < argc; i++) {
    	if (strcmp(argv[i], "--reset")==0) {
    		// yes
    		reset = true;
    	}
    	else
		if (memcmp(argv[i], "--disko", 7)!=0) {
			printf("Error: unknown parameter %s\n", argv[i]);
			return false;
		}
    }

	return true;
}

int main(int argc, char *argv[]) {
	bool reset;

	// get cmd parameters
	if (!getparams(argc, argv, reset)) {
		help();
		return 1;
	}

	// init disko in silent mode
	if (mmsInit(MMSINIT_SILENT, argc, argv)) {
		MMSConfigData config;

		// init tcp client instance
		MMSTCPClient *tcl = new MMSTCPClient(config.getActMonAddress(), config.getActMonPort());

		// build command
		string cmd;
		if (!reset)
			cmd = "GET_STATINFO RESET(FALSE)";
		else
			cmd = "GET_STATINFO RESET(TRUE)";

		// send command and receive answer
		string ret;
		tcl->sendAndReceive(cmd, &ret);
		if (!ret.empty()) {
			// success
			printf(ret.c_str());
		}
		else {
			// command failed
			printf("Error: command '%s' failed\n", cmd.c_str());
			return 3;
		}

		// free tcp client instance
		delete tcl;

		return 0;
	}

	return 2;
}


