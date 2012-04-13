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
 * mmsprocessmonitor.cpp
 *
 *  Created on: 05.11.2008
 *      Author: sxs
 */

#include "mmstools/mmsprocessmonitor.h"
#include "mmstools/tools.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

MMSProcessMonitor::MMSProcessMonitor(unsigned int interval) : monitoringInterval(interval) {
	this->shutdown = false;
}

MMSProcessMonitor::~MMSProcessMonitor() {

}

void MMSProcessMonitor::commenceShutdown() {
    DEBUGMSG("PROCESSMONITOR", "Processmonitor shutdown initiated.");
	this->shutdown = true;
}

void MMSProcessMonitor::addProcess(std::string process) {
	MMSPROCESS_TASK task;
	task.cmdline = process;
	processes.push_back(task);
}

void MMSProcessMonitor::addProcess(const char *process) {
	MMSPROCESS_TASK task;
	task.cmdline = process;
	processes.push_back(task);
}

bool MMSProcessMonitor::startprocess(MMSPROCESS_TASKLIST::iterator &it) {
	pid_t pid = fork();

	if(pid == -1) {
		return false;
	}
	if(pid == 0) {
		char *argv[255];
		argv[0]=(char *)it->cmdline.c_str();
		argv[1]=NULL;
		DEBUGMSG("PROCESSMONITOR", "Starting process %s", it->cmdline.c_str());
		execv(it->cmdline.c_str(),argv);
		DEBUGMSG("PROCESSMONITOR", "Starting of process %s failed. (ERRNO: %d)", it->cmdline.c_str(), errno);
		exit(1);
	}
	//Iam the caller...
	it->pid=pid;
	return true;
}

bool MMSProcessMonitor::checkprocess(MMSPROCESS_TASKLIST::iterator &it) {
	if(kill(it->pid,0)==0)
		return true;
	else
		return false;

}
bool MMSProcessMonitor::killprocess(MMSPROCESS_TASKLIST::iterator &it) {
    DEBUGMSG("PROCESSMONITOR", "Killing process %s (%d)", it->cmdline.c_str(), it->pid);
    if(kill(it->pid,SIGTERM)==0)
		return true;
	else
		return false;

}

void MMSProcessMonitor::threadMain() {
	for(MMSPROCESS_TASKLIST::iterator it = this->processes.begin();it != this->processes.end();it++) {
	    startprocess(it);
	}

	while(1) {
		if(shutdown) {
			for(MMSPROCESS_TASKLIST::iterator it = this->processes.begin();it != this->processes.end();it++) {
				killprocess(it);
			}
			return;
		}

		for(MMSPROCESS_TASKLIST::iterator it = this->processes.begin();it != this->processes.end();it++) {
			if(!checkprocess(it)) {
				startprocess(it);
			}
		}
		int status;
		while(waitpid(-1, &status, WNOHANG)>0);
		sleep(this->monitoringInterval);
	}

}

