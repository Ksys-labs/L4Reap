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

#include "mmstools/mmstcpclient.h"
#include "mmstools/tools.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <cerrno>

MMSTCPClient::MMSTCPClient(string host, unsigned int port) {
    this->host = host;
    this->port = port;
    this->s = -1;
}

bool MMSTCPClient::isConnected() {
	return (this->s>=0);
}

bool MMSTCPClient::connectToServer() {
	struct hostent  	*he;
	struct in_addr  	ia;
	struct sockaddr_in	sa;

	WRITE_MSG("MMSTCPClient", "connect to %s:%u",this->host.c_str(), this->port);

	if (this->s>=0) {
		WRITE_MSG("MMSTCPClient", "already connected");
		return true;
	}

	// get host ip in network byte order
	he = gethostbyname(this->host.c_str());
	WRITE_MSG("MMSTCPClient", "hostname: %s", he->h_name);

	// get host ip in numbers-and-dots
	ia.s_addr = *((unsigned long int*)*(he->h_addr_list));
	this->hostip = inet_ntoa(ia);

	// get a socket
	if ((this->s = socket(AF_INET, SOCK_STREAM, 0))<=0) {
		WRITE_ERR("MMSTCPClient", "socket() failed");
		return false;
	}

	// connect to hostip
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(this->port); //this->port / 0x100 + (this->port % 0x100) * 0x100;
	sa.sin_addr.s_addr = inet_addr(this->host.c_str());
	if (connect(this->s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in))!=0) {
		WRITE_ERR("MMSTCPClient", "connect to %s:%d failed: %s",this->host.c_str(), this->port, strerror(errno));
		disconnectFromServer();
		return false;
	}

	// connection established
	return true;
}

bool MMSTCPClient::disconnectFromServer() {
	if (this->s<0) return true;
	close(this->s);
	this->s = -1;
	return true;
}

bool MMSTCPClient::sendString(string rbuf) {
	char 	mybuf[128000+1];
	int		len, from;

	if (!isConnected()) {
		WRITE_ERR("MMSTCPClient", "in send not connected");
		return false;
	}

	// send request
	from = 0;
	do {
		strcpy(mybuf, (rbuf.substr(from, sizeof(mybuf)-1)).c_str());
		if (!*mybuf) break;
		if ((len = send(this->s, mybuf, strlen(mybuf), 0))<0) return false;
		from+=len;
	} while (len>0);
	send(this->s, "\0", 1, 0);
	WRITE_MSG("MMSTCPClient", "sent %d bytes", from + 1);
	return true;
}

bool MMSTCPClient::receiveString(string *abuf) {
	char 	mybuf[128000+1];
	int		len;

	if (!isConnected()) return false;

	// receive answer
	*abuf = "";
	do {
		if ((len = recv(this->s, mybuf, sizeof(mybuf)-1, 0))<0) return false;
		if (len>0) {
			mybuf[len]=0;
			(*abuf)+= mybuf;
		}
	} while ((len>0)&&(mybuf[len-1]!=0));

	return true;
}

bool MMSTCPClient::receiveString(string *abuf, int buflen) {
	//char 	mybuf[128000+1];
	char 	*mybuf;
	ssize_t		len;
	ssize_t received=0;

	if (!isConnected()) return false;

	mybuf = new char[buflen+1];

	memset(mybuf,0,buflen+1);

	// receive answer
	*abuf = "";
	do {
		if ((len = recv(this->s, &mybuf[received], buflen-received, MSG_WAITALL))<0) return false;

		received+=len;
		if (len>0) {
			mybuf[len]=0;
		}
	} while(received < buflen);

	*abuf= mybuf;
	delete mybuf;
	return true;
}

bool MMSTCPClient::peekString(string *abuf, int buflen) {
	char 	mybuf[128000+1];
	int		len;
	int 	received=0;

	if (!isConnected()) return false;
	memset(mybuf,0,128000+1);

	// receive answer
	*abuf = "";
	do {
		if ((len = recv(this->s, &mybuf[received], buflen-received, MSG_PEEK))<0) return false;

		received+=len;
		if (len>0) {
			mybuf[len]=0;
		}
	} while(received < buflen);

	(*abuf) = mybuf;
	return true;
}

bool MMSTCPClient::sendAndReceive(string rbuf, string *abuf) {
	bool	retcode = false;

	if (!connectToServer()) {
		return false;
	}

	WRITE_MSG("MMSTCPClient", "send string");
	if (sendString(rbuf)) {
		WRITE_MSG("MMSTCPClient", "receive string");
		if (receiveString(abuf)) {
			WRITE_MSG("MMSTCPClient", "receive string");
			retcode = true;
		}
	}
	disconnectFromServer();

	return retcode;
}


void MMSTCPClient::setAddress(string &host, unsigned int port)  {
	this->host = host;
	this->port = port;
}

void MMSTCPClient::setAddress(const char *host, unsigned int port) {
	this->host = host;
	this->port = port;
}
