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

#include "mmstools/mmstcpserver.h"
#include "mmstools/tools.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <cerrno>

MMSTCPServer::MMSTCPServer(vector<MMSServerInterface *> interfaces,
					 	   string host, unsigned int port, string identity) : MMSThread(identity) {

	// create threads and link a interface to each
    this->st_size = interfaces.size();
	for (size_t i = 0; i < this->st_size; i++)
		this->sthreads.push_back(new MMSTCPServerThread(interfaces.at(i)));
    this->st_cnt = 0;

	// connection data
    this->host = host;
    this->port = port;
    this->s = -1;
}

MMSTCPServer::MMSTCPServer(MMSServerInterface *interface,
					 	   string host, unsigned int port, string identity) : MMSThread(identity) {

	// create threads and link a interface to each
    this->st_size = 1;
	this->sthreads.push_back(new MMSTCPServerThread(interface));
    this->st_cnt = 0;

	// connection data
    this->host = host;
    this->port = port;
    this->s = -1;
}

void MMSTCPServer::threadMain() {
	struct hostent  	*he;
	struct in_addr  	ia;
	struct sockaddr_in	sa;
	struct sockaddr_in	sac;
	fd_set				readfds;
	fd_set				writefds;
	fd_set				errorfds;
	struct timeval		timeout;
	int					new_s;
	socklen_t			saclen = sizeof(struct sockaddr_in);

	// get host ip in network byte order
	he = gethostbyname(this->host.c_str());

	// get host ip in numbers-and-dots
	ia.s_addr = *((unsigned long int*)*(he->h_addr_list));
	this->hostip = inet_ntoa(ia);
	WRITE_MSGI("ip: %s", this->hostip.c_str());

	// get a socket
	if ((this->s = socket(AF_INET, SOCK_STREAM, 0))<=0) return;

	// bind to hostip
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(this->port);
	sa.sin_addr.s_addr = inet_addr(this->hostip.c_str());
	WRITE_MSGI("bind at %d(%d)",this->port, sa.sin_port);

	// set socket options
	int optbuf = 1;
	if(setsockopt(this->s, SOL_SOCKET, SO_REUSEADDR, &optbuf, sizeof(optbuf)) < 0) {
		WRITE_ERRI("socket error: cannot set socket option");
	}

	if(bind(this->s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in))!=0) {
		WRITE_ERRI("Error while binding at %s:%d: %s", this->hostip.c_str(), this->port, strerror(errno));
		return;
	}

	if (listen(this->s,SOMAXCONN)!=0) {
		WRITE_ERRI("Error while listening at %s:%d: %s", this->hostip.c_str(), this->port, strerror(errno));
		return;
	}

	// listen/select/accept loop
	while (1) {
		// set filedescriptor
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&errorfds);
		FD_SET(this->s, &readfds);
		FD_SET(this->s, &writefds);
		FD_SET(this->s, &errorfds);

		// set timeout
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		// check socket
		if (select(this->s+1, &readfds, &writefds, &errorfds, &timeout)<0) {
			WRITE_ERRI("select failed");
			return;
		}
		if (FD_ISSET(this->s, &readfds)) {
			// going to accept the new connection
			if ((new_s = accept(this->s, (struct sockaddr *)&sac, &saclen))<0) {
				WRITE_ERRI("accept failed");
				continue;
			}

			WRITE_MSGI("check st_size");
			// call next server thread
			if (this->st_size<=0) {
				shutdown(new_s, 2);
				close(new_s);
				continue;
			}
			WRITE_MSGI("set and start thread");
			if (this->st_cnt >= this->st_size) this->st_cnt=0;
			while(this->sthreads.at(this->st_cnt)->isRunning()) {
				//fprintf(stderr, "still running \n");
				usleep(50);
			}
			this->sthreads.at(this->st_cnt)->setSocket(new_s);
			this->sthreads.at(this->st_cnt)->start();
			this->st_cnt++;
		}
		else
		if (FD_ISSET(this->s, &writefds)) {
			//DEBUGOUT ("WRITEFDS\n");
			return;
		}
		else
		if (FD_ISSET(this->s, &errorfds)) {
			//DEBUGOUT ("ERRORFDS\n");
			return;
		}
		else {
			//DEBUGMSG("MMSTCPServer", "select timeout");
		}
	}
}
