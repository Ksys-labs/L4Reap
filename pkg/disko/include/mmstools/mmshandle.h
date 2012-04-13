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

#ifndef MMSHANDLER_H_
#define MMSHANDLER_H_

#include "mmstools/mmsmutex.h"
#include <iostream>

/**
 * Handler class that implements reference counting.
 */
template<class X> class MMSHandle {
    private:
        X        *data;      /**< pointer to object that shall be handled                 */
        int      *refCount;  /**< reference counter                                       */
        MMSMutex *lock;      /**< needed to make increase and decrease of refCount atomic */

    public:
        /**
         * Constructor
         */
        explicit MMSHandle(X* orig) : data(orig), refCount(new int(1)), lock(new MMSMutex()) {
        }

        /**
         * Copy constructor
         */
        MMSHandle(const MMSHandle& orig) : data(orig.data), refCount(orig.refCount), lock(orig.lock) {
        	lock->lock();
            (*refCount)++;
            lock->unlock();
        }

        /**
         * Destructor
         */
        virtual ~MMSHandle() throw() {
        	lock->lock();
        	--(*refCount);
        	lock->unlock();
        	if(*refCount == 0) {
        		if(data) delete data;
        		delete refCount;
        		delete lock;
        	}
        }

        /**
         * Overloaded -> operator.
         * Just returns the data object.
         */
        X* operator->() throw() {
        	return data;
        }

        /**
         * Overloaded = operator.
         */
        void operator=(const MMSHandle& orig) throw() {
            /* do not copy if data is already the same */
            if(data != orig.data) {
	            /* free memory if reference counter is 0 */
	        	lock->lock();
	        	--(*refCount);
	        	lock->unlock();
	        	if(*refCount == 0) {
	        		if(data) delete data;
	        		delete refCount;
	        		delete lock;
	        	}

	            /* create a copy */
	            data     = orig.data;
	            refCount = orig.refCount;
	            lock     = orig.lock;

	            /* increase reference counter */
	            lock->lock();
	            refCount++;
	            lock->unlock();
            }
        }

        /**
         * Get data object.
         */
        X& operator*() const throw() {
            return *data;
        }

        /**
         * Get data object.
         */
        X* getObject() const throw() {
            return data;
        }
};

#endif /*MMSHANDLER_H_*/
