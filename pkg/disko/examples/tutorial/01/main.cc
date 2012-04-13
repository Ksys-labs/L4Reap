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

#include <mms.h>

#ifndef __X86__
#define DISKORC "rom/diskorc_arm.xml"
#else
#define DISKORC "rom/diskorc.xml"
#endif

int main(int argc, char *argv[]) {

	// initialize disko
	if(!mmsInit(MMSINIT_WINDOWS, argc, argv, DISKORC,
			"Disko Tutorial: firststeps/01", "DT: firststeps/01")) {
		printf("\tCheck your diskorc.xml\n");
		return -1;
	}

	// create a root window
	printf("create a root window\n");
	MMSRootWindow *window = new MMSRootWindow("","100%","100%");

	// create and add hello world label
    printf("create and add hello world label\n");
	MMSLabel *mylabel = new MMSLabel(window, "");
	mylabel->setFont("rom","DejaVuSansMono.ttf",16);
	mylabel->setText("Hello World");
	window->add(mylabel);

	// show the window
	window->show();

	// until user press <ctrl+c> or <power> button on the remote control
	while (1) sleep(1);
	return 0;
}
