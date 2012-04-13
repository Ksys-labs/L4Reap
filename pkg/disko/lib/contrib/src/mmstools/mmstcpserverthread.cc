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

#include "mmstools/mmstcpserverthread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <mmstools/tools.h>

MMSTCPServerThread::MMSTCPServerThread(MMSServerInterface *interface, int s, string identity) : MMSThread(identity) {
	this->interface = interface;
    this->s = s;
    this->request_buffer = "";
    this->answer_buffer = "";
}

bool MMSTCPServerThread::setSocket(int s) {
    this->s = s;
    return true;
}

void MMSTCPServerThread::threadMain() {
	char 	mybuf[4096+1];
	int		len, from;

	DEBUGMSG("MMSTCPServerThread", "process TCP Request");
	
	/* check somthing */
	if (!this->s) return;
	if (!this->interface) {
		close(this->s);
		this->s=-1;
		return;
	}

	/* receive request */
	this->request_buffer = "";
	do {
		if ((len = recv(this->s, mybuf, sizeof(mybuf)-1, 0))<0) {
			close(this->s);
			this->s=-1;
			return;
		}
		if (len>0) {
			mybuf[len]=0;
			this->request_buffer+= mybuf;
		}
	} while ((len>0)&&(mybuf[len-1]!=0));

	/* process request */
	this->answer_buffer = "";
	if (!this->interface->processRequest(&this->request_buffer, &this->answer_buffer)) {
		close(this->s);
		this->s=-1;
		return;
	}

	/* send answer */
	from = 0;
	do {
		strcpy(mybuf, (this->answer_buffer.substr(from, sizeof(mybuf)-1)).c_str());
		if (!*mybuf) break;
		if ((len = send(this->s, mybuf, strlen(mybuf), MSG_NOSIGNAL))<0) {
			close(this->s);
			this->s=-1;
			return;
		}
		from+=len;
	} while (len>0);
	send(this->s, "\0", 1, 0);

	close(this->s);
	this->s=-1;
}
