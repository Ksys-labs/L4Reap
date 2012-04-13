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

#include "console.h"
#include <string.h>
#include <stdlib.h>

Console::Console(bool richformat) {
	this->richformat = richformat;
}

Console::~Console() {

}

void Console::formatText(char *input) {
	unsigned int i, len;

	len = strlen(input);

	for(i=0;i<len;i++) {
		if((input[i]=='@')&&(input[i+1]=='['))
			input[i]=27;
	}
}

void Console::printExample(const char *example) {
	char myexample[2048];

	strcpy(myexample,example);
	formatText(myexample);

	printf("\t\t\t%c[1mexample:%c[0m\n",27,27);

	printf("\t\t\t\t%s\n\n",myexample);
}

void Console::printCommand(const char *command, const char *desc) {
	char mydesc[2048]={0};
	char mycmd[2048]={0};
	char *worker,*tmpworker;
	strcpy(mydesc,desc);
	strcpy(mycmd,command);
	worker=mydesc+56;

	formatText(mydesc);
	while(*worker!=' ')
		worker--;

	*worker=0;
	worker++;
	printf("\t\t%c[1m%s%c[0m\t%s\n",27,command,27,mydesc);

	while(1) {
		tmpworker=worker;
		worker = worker + 56;
		if(worker>mydesc+strlen(desc)) {
			printf("\t\t\t%s\n",tmpworker);
			break;
		}
		while(*worker!=' ')
			worker--;

		*worker=0;
		worker++;
		printf("\t\t\t%s\n",tmpworker);

	}
		printf("\n");

}

void Console::printText(const char *text) {
	char mytext[2048]={0};
	char *worker,*tmpworker;
	strcpy(mytext,text);

	worker=mytext+65;

	formatText(mytext);
	while(*worker!=' ')
		worker--;

	*worker=0;
	worker++;
	printf("\t\t%s\n",mytext);

	while(1) {
		tmpworker=worker;
		worker = worker + 65;
		if(worker>mytext+strlen(text)) {
			printf("\t\t%s\n",tmpworker);
			break;
		}
		while(*worker!=' ')
			worker--;

		*worker=0;
		worker++;
		printf("\t\t%s\n",tmpworker);

	}
		printf("\n");

}

void Console::printError(const char *text) {
	char mytext[2048]={0};
	char *worker,*tmpworker;
	strcpy(mytext,text);

	worker=mytext+65;

	formatText(mytext);
	while(*worker!=' ')
		worker--;

	*worker=0;
	worker++;
	printf("\t\t%s\n",mytext);

	while(1) {
		tmpworker=worker;
		worker = worker + 65;
		if(worker>mytext+strlen(text)) {
			printf("\t\t%s\n",tmpworker);
			break;
		}
		while(*worker!=' ')
			worker--;

		*worker=0;
		worker++;
		printf("\t\t%s\n",tmpworker);

	}
		printf("\n");

}

void Console::linefeed() {
	printf("\n");
}

void Console::setRichformat(bool richformat) {
	this->richformat = richformat;
}
