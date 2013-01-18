/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 Berlinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer <SSchwarzer@berlinux-solutions.de>,               *
 *      Matthias Hardt   <MHardt@berlinux-solutions.de>,                   *
 *      Jens Schneider   <pupeider@gmx.de>                                 *
 *      Guido Madaus     <GMadaus@berlinux-solutions.de>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <disko.h>
#include <cstdio>

#ifndef __X86__
#define DISKORC "rom/diskorc_arm.xml"
#else
#define DISKORC "rom/diskorc.xml"
#endif

void onMenuReturn(MMSWidget *wid) {
	//cast to menu
	MMSMenuWidget *menu = dynamic_cast<MMSMenuWidget*>(wid);

	if(menu) {
		MMSWidget *button = menu->getSelectedItem();

		string widgetdata;
		// extract data string set in xml from pressed button
		// and print it to stdout
		if(button->getData(widgetdata)) {
			printf("Button pressed: %s\n", widgetdata.c_str());
		}
	}
}

void onSubscription(MMSInputSubscription *sub) {
	printf("subscribed key pressed...\n");
}

int main(int argc, char *argv[]) {

	// initialize disko
	if(!mmsInit(MMSINIT_WINDOWS|MMSINIT_INPUTS, argc, argv, DISKORC,
			"Disko Tutorial: firststeps/05", "DT: firststeps/05"))
		return -1;


	try {

        // configure TS
        MMSConfigData *config = new MMSConfigData();

        TsCalibration cal_const(
            0.260736, 0.002595, -29.626526,
            0.002376, -0.156178, 288.105164
        );
        config->setTsCalibration(cal_const);

        // one dialog manager for each window loaded from xml
		MMSDialogManager dm;

		// load the window
		MMSWindow *window  = dm.loadDialog("root.xml");

		//search for the menu and connect the Return callback
		//this works for clicks too
		MMSWidget *wid = window->searchForWidget("mymenu");
		wid->onReturn->connect(sigc::ptr_fun(onMenuReturn));

		//create and register an input subscription
		MMSInputSubscription subs(MMSKEY_SMALL_A);
		subs.callback.connect(sigc::ptr_fun(onSubscription));
		subs.registerMe();


		//show the window
		window->show();

		//pause();
		while (1) sleep(1);
		return 0;
	}
	catch(MMSError *error) {
		fprintf(stderr, "Abort due to: %s\n", error->getMessage().c_str());
		return 1;
	}
}

