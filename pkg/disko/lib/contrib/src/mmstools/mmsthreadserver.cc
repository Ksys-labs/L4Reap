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

#include "mmstools/mmsthreadserver.h"
#include <cstdlib>
#include <cstdio>
#include <string.h>

MMSThreadServer::MMSThreadServer(int queue_size, string identity, bool blocking) : MMSThread(identity) {
	// allocate and setup the queue
	if (queue_size < 100)
		this->queue_size = 100;
	else
		this->queue_size = queue_size;
	this->queue = (MMSTS_QUEUE_ITEM**)malloc(this->queue_size * sizeof(MMSTS_QUEUE_ITEM*));
	memset(this->queue, 0, this->queue_size * sizeof(MMSTS_QUEUE_ITEM*));
	this->queue_rp = 0;
	this->queue_wp = 0;
	this->buffer_full = false;

	// in non-blocking mode the caller of trigger() will get control directly after triggering
	// and do not wait until server has finished processData()
	this->blocking = blocking;

	// init the mutex and cond variable for the server thread
	pthread_mutex_init(&this->mutex, NULL);
	pthread_cond_init(&this->cond, NULL);
}

MMSThreadServer::~MMSThreadServer() {
	pthread_mutex_unlock(&this->mutex);
	pthread_cond_destroy(&this->cond);
	pthread_mutex_destroy(&this->mutex);
	free(this->queue);
}

bool MMSThreadServer::start() {
	// lock the server mutex
	pthread_mutex_lock(&this->mutex);

	// start the thread
	return MMSThread::start();
}

void MMSThreadServer::threadMain() {
	// server loop
	while (1) {
		if (pthread_cond_wait(&this->cond, &this->mutex) == 0) {
			// signal from a client thread received
			while (this->queue_rp != this->queue_wp) {
				// processing next item in the queue
				MMSTS_QUEUE_ITEM *item = this->queue[this->queue_rp];
				if (item) {
					if (this->blocking) {
						// blocking mode, now calling the worker routine for data processing
						processData(item->in_data, item->in_data_len, item->out_data, item->out_data_len);

						// handshake with the client thread
						pthread_mutex_lock(&item->mutex);
						pthread_cond_signal(&item->cond);
						pthread_mutex_unlock(&item->mutex);
					}
					else {
						// non-blocking mode, processing data in parallel to the caller thread of trigger()
						void *in_data		= item->in_data;
						int in_data_len		= item->in_data_len;
						void **out_data		= item->out_data;
						int *out_data_len	= item->out_data_len;

						// handshake with the client thread
						pthread_mutex_lock(&item->mutex);
						pthread_cond_signal(&item->cond);
						pthread_mutex_unlock(&item->mutex);

						// now calling the worker routine for data processing
						processData(in_data, in_data_len, out_data, out_data_len);
					}
				}

				// remove item from queue
				this->queue[this->queue_rp] = NULL;
				if (this->queue_rp + 1 < this->queue_size)
					this->queue_rp++;
				else
					this->queue_rp = 0;
				this->buffer_full = false;
			}
		}
	}
}

void MMSThreadServer::processData(void *in_data, int in_data_len, void **out_data, int *out_data_len) {
	this->onProcessData.emit(in_data, in_data_len, out_data, out_data_len);
}

bool MMSThreadServer::trigger(void *in_data, int in_data_len, void **out_data, int *out_data_len) {
	// create new queue item and put data to it
	MMSTS_QUEUE_ITEM item;
	item.in_data		= in_data;
	item.in_data_len	= in_data_len;
	item.out_data 		= out_data;
	item.out_data_len	= out_data_len;

	// init queue item's conditional variables
	pthread_mutex_init(&item.mutex, NULL);
	pthread_cond_init(&item.cond, NULL);
	pthread_mutex_lock(&item.mutex);

	// lock the server mutex and push the new item at the end of the queue
	pthread_mutex_lock(&this->mutex);
	this->queue[this->queue_wp] = &item;
	this->queue_wp++;
	if (this->queue_wp >= this->queue_size)
		this->queue_wp = 0;

	// check if the queue is full now
	if (this->queue_rp == this->queue_wp) {
		// yes, the ring buffer is full, no next request can be inserted
		this->buffer_full = true;
		printf("%s - ring buffer is full!\n", this->identity.c_str());
		while (this->buffer_full) usleep(10000);
	}

	// send signal to the server and unlock the server mutex
	pthread_cond_signal(&this->cond);
	pthread_mutex_unlock(&this->mutex);

	// waiting for the answer from the server
	pthread_cond_wait(&item.cond, &item.mutex);

	// freeing temporary conditional variables
	pthread_mutex_unlock(&item.mutex);
	pthread_cond_destroy(&item.cond);
	pthread_mutex_destroy(&item.mutex);

	return true;
}

