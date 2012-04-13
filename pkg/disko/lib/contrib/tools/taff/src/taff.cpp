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

/*
 * taff.cpp
 *
 *  Created on: 01.04.2008
 *      Author: Jens Schneider
 */

#include <cstring>
#include "mms.h"


void help() {
	printf("\nTAFF Converter\n\n");
	printf("parameter:\n\n");
	printf("--mode <mode>               specifies what is to do\n");
	printf("  --mode c2t                convert external file to taff file\n");
	printf("  --mode c2e                convert taff file to external file\n");
	printf("  --mode tc2t               test the convertion of external file to taff file\n");
	printf("  --mode tc2e               test the convertion of taff file to external file\n");
	printf("--tafffile <filename>       file for reading/writing the taff format\n");
	printf("--tafftype mmsgui           type of the taff file, default is mmsgui\n");
	printf("--extfile <filename>        file for reading/writing the external format\n");
#if defined(__HAVE_PNG__) or defined(__HAVE_JPEG__)
	printf("--exttype xml|image         type of the external file, default is xml\n");
	printf("                            (image support:");
#ifdef __HAVE_PNG__
	printf(" png");
#endif
#ifdef __HAVE_JPEG__
	printf(" jpeg");
#endif
#ifdef __HAVE_TIFF__
	printf(" tiff");
#endif
	printf(")\n");
	printf("--image:pf ARGB|ABGR|       pixelformat of the taff image (see --exttype image)\n");
    printf("           AiRGB|AYUV|      default is ARGB\n");
    printf("           ARGB4444|RGB16\n");
    printf("--image:premulti yes|no     generate premultiplied images?, default yes\n");
	printf("--image:mirror_size <size>  size of the mirror effect in pixels, default is 0\n");
#endif
	printf("--ignore_blanks yes|no      ignore blank values?, default no\n");
	printf("--trace yes|no              print trace messages?, default no\n");
	printf("--warnings yes|no           print warnings?, default yes\n");
	printf("--silent yes|no             ignore errors if extfile is not supp.? default no\n");
	printf("\nexamples:\n\n");
	printf("taff --mode c2t --tafffile theme.taff --extfile theme.xml\n\n");
	printf("taff --mode c2t --exttype image --tafffile myimage.taff --extfile myimage.png\n\n");
}

bool getparams(int argc, char *argv[],
			   string &mode, string &tafffile, TAFF_DESCRIPTION **tafftype,
			   string &extfile, MMSTAFF_EXTERNAL_TYPE *exttype,
			   MMSTAFF_PF &pf, int &mirror_size, bool &premultiplied,
			   bool &ignore_blanks, bool &trace, bool &print_warnings, bool &silent) {

	if ((argc<3)||((argc-1)%2)) {
		printf("Error: wrong parameter list\n");
		return false;
	}

	*tafftype = &mmsgui_taff_description;
	*exttype = MMSTAFF_EXTERNAL_TYPE_XML;
	ignore_blanks = false;
	trace = false;
	print_warnings = true;
	silent = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--mode")==0)
		{
			i++;
			if   ((strcmp(argv[i], "c2t")==0)
				||(strcmp(argv[i], "c2e")==0)
				||(strcmp(argv[i], "tc2t")==0)
				||(strcmp(argv[i], "tc2e")==0))
				mode = argv[i];
			else {
				printf("Error: --mode is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--tafffile")==0)
		{
			i++;
			tafffile = argv[i];
		}
		else
		if (strcmp(argv[i], "--tafftype")==0)
		{
			i++;
			if (strcmp(argv[i], "mmsgui")==0)
				*tafftype = &mmsgui_taff_description;
			else {
				printf("Error: --tafftype is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--extfile")==0)
		{
			i++;
			extfile = argv[i];
		}
		else
		if (strcmp(argv[i], "--exttype")==0)
		{
			i++;
			if (strcmp(argv[i], "xml")==0)
				*exttype = MMSTAFF_EXTERNAL_TYPE_XML;
			else
			if (strcmp(argv[i], "image")==0) {
				*exttype = MMSTAFF_EXTERNAL_TYPE_IMAGE;
				*tafftype = NULL;
			}
			else {
				printf("Error: --exttype is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--image:pf")==0)
		{
			i++;
			if (strcmp(argv[i], "ARGB")==0)
				pf = MMSTAFF_PF_ARGB;
			else
			if (strcmp(argv[i], "ABGR")==0) {
				pf = MMSTAFF_PF_ABGR;
			}
			else
			if (strcmp(argv[i], "AiRGB")==0) {
				pf = MMSTAFF_PF_AiRGB;
			}
			else
			if (strcmp(argv[i], "AYUV")==0) {
				pf = MMSTAFF_PF_AYUV;
			}
			else
			if (strcmp(argv[i], "ARGB4444")==0) {
				pf = MMSTAFF_PF_ARGB4444;
			}
			else
			if (strcmp(argv[i], "RGB16")==0) {
				pf = MMSTAFF_PF_RGB16;
			}
			else {
				printf("Error: --image:pf is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--image:mirror_size")==0)
		{
			i++;
			mirror_size = atoi(argv[i]);
		}
		else
		if (strcmp(argv[i], "--image:premulti")==0)
		{
			i++;
			if (strcmp(argv[i], "yes")==0)
				premultiplied = true;
			else
			if (strcmp(argv[i], "no")==0)
				premultiplied = false;
			else {
				printf("Error: --image:premulti is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--ignore_blanks")==0)
		{
			i++;
			if (strcmp(argv[i], "yes")==0)
				ignore_blanks = true;
			else
			if (strcmp(argv[i], "no")==0)
				ignore_blanks = false;
			else {
				printf("Error: --ignore_blanks is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--trace")==0)
		{
			i++;
			if (strcmp(argv[i], "yes")==0)
				trace = true;
			else
			if (strcmp(argv[i], "no")==0)
				trace = false;
			else {
				printf("Error: --trace is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--warnings")==0)
		{
			i++;
			if (strcmp(argv[i], "yes")==0)
				print_warnings = true;
			else
			if (strcmp(argv[i], "no")==0)
				print_warnings = false;
			else {
				printf("Error: --warnings is invalid\n");
				return false;
			}
		}
		else
		if (strcmp(argv[i], "--silent")==0)
		{
			i++;
			if (strcmp(argv[i], "yes")==0)
				silent = true;
			else
			if (strcmp(argv[i], "no")==0)
				silent = false;
			else {
				printf("Error: --warnings is invalid\n");
				return false;
			}
		}
		else {
			printf("Error: unknown parameter %s\n", argv[i]);
			return false;
		}
	}

	if (mode=="") {
		printf("Error: --mode is not specified\n");
		return false;
	}

	return true;
}


int main(int argc, char *argv[]) {

	string mode, tafffile, extfile;
	bool ignore_blanks, trace, print_warnings, silent;
	TAFF_DESCRIPTION *tafftype;
	MMSTAFF_EXTERNAL_TYPE exttype;
	MMSTAFF_PF pf = MMSTAFF_PF_ARGB;
	int mirror_size = 0;
	bool premultiplied = true;

	if (!getparams(argc, argv, mode, tafffile, &tafftype, extfile, &exttype,
				   pf, mirror_size, premultiplied, ignore_blanks, trace, print_warnings, silent)) {
		help();
		return 1;
	}

	if (mode=="c2t") {
		// convert to taff

		if (tafffile=="") {
			printf("Error: taff file not specified\n");
			return 1;
		}
		if (extfile=="") {
			printf("Error: external file not specified\n");
			return 1;
		}
		printf("\nCreating TAFF from %s...\n", extfile.c_str());

		MMSTaffFile *tafff;
		if ((exttype == MMSTAFF_EXTERNAL_TYPE_IMAGE)
			&&((pf != MMSTAFF_PF_ARGB)
			|| (mirror_size != 0))) {
			// special settings for images
			tafff = new MMSTaffFile(tafffile, tafftype, "", exttype, ignore_blanks, trace, print_warnings, true);
			tafff->setExternal(extfile, MMSTAFF_EXTERNAL_TYPE_IMAGE);
			tafff->setDestinationPixelFormat(pf, premultiplied);
			tafff->setMirrorEffect(mirror_size);
			tafff->convertExternal2TAFF();
			tafff->readFile();
		}
		else {
			// normal conversion
			tafff = new MMSTaffFile(tafffile, tafftype, extfile, exttype, ignore_blanks, trace, print_warnings, true);
		}
		if (!tafff) {
			printf("Error: creating MMSTaffFile()\n");
			help();
			return 2;
		}
		if (!tafff->isLoaded()) {
			if(!silent) {
				unlink(tafffile.c_str());
				printf("Error: creating MMSTaffFile2()\n");
				help();
			}
			return 2;
		}

		printf("TAFF created.\n");

		delete tafff;
		return 0;
	}
	else
	if (mode=="tc2t") {
		// convert to taff - test mode

		printf("\nCreating TAFF - test mode...\n");
		if (extfile=="") {
			printf("Error: external file not specified\n");
			return 1;
		}

		MMSTaffFile *tafff = new MMSTaffFile("", tafftype, "", exttype, ignore_blanks, trace, print_warnings, true);
		if (!tafff) {
			printf("Error: creating MMSTaffFile()\n");
			help();
			return 2;
		}

		tafff->setExternal(extfile, exttype);
		if (!tafff->convertExternal2TAFF()) {
			printf("Error: conversion external to TAFF\n");
			return 2;
		}

		printf("Creation test was successful.\n");

		delete tafff;
		return 0;
	}
	else
	if (mode=="c2e") {
		// convert to external

		printf("\nCreating external file...\n");
		if (tafffile=="") {
			printf("Error: taff file not specified\n");
			return 1;
		}
		if (extfile=="") {
			printf("Error: external file not specified\n");
			return 1;
		}

		MMSTaffFile *tafff = new MMSTaffFile(tafffile, tafftype, "", exttype, ignore_blanks, trace, print_warnings, false);
		if (!tafff) {
			printf("Error: creating MMSTaffFile()\n");
			help();
			return 2;
		}
		if (!tafff->isLoaded()) {
			if (!tafff->checkVersion())
				printf("Error: taff file has wrong format/version\n");
			else
				printf("Error: cannot load taff file\n");
			help();
			return 2;
		}

		tafff->setExternal(extfile, exttype);
		if (!tafff->convertTAFF2External())
			return 2;

		printf("External file created.\n");

		delete tafff;
		return 0;
	}
	else
	if (mode=="tc2e") {
		// convert to external - test mode

		printf("\nCreating external file - test mode...\n");
		if (tafffile=="") {
			printf("Error: taff file not specified\n");
			return 1;
		}

		MMSTaffFile *tafff = new MMSTaffFile(tafffile, tafftype, "", exttype, ignore_blanks, trace, print_warnings, false);
		if (!tafff) {
			printf("Error: creating MMSTaffFile()\n");
			help();
			return 2;
		}
		if (!tafff->isLoaded()) {
			if (!tafff->checkVersion())
				printf("Error: taff file has wrong verion\n");
			else
				printf("Error: cannot load taff file\n");
			help();
			return 2;
		}

		if (!tafff->convertTAFF2External()) {
			printf("Error: conversion TAFF to external\n");
			return 2;
		}

		printf("Creation test was successful.\n");

		delete tafff;
		return 0;
	}


	return 0;
}


