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

#ifndef MMSTHREAD_H_
#define MMSTHREAD_H_

#include "mmstools/mmsmutex.h"
#include "mmstools/mmslogger.h"
#include <sched.h>


//! This class is the base class for all threads.
/*!
This class includes the base functionality available for all threads within MMS/DISKO.
This class cannot be constructed. Only classes which are derived from this class can be constructed.
\author Jens Schneider
*/
class MMSThread {

	private:
		//! helper mutex to perform a safe start
		MMSMutex	startlock;

		//! starting thread is in progress
		bool		starting;

		//! thread is running
		bool		running;

		//! if thread is detached, its resources are automatically released when it terminates
		bool		detached;


		//! thread attributes
        pthread_attr_t	tattr;

        //! scheduling parameter
        sched_param		param;

        //! id of the thread, valid for the running state
		pthread_t		id;


		//! requested priority of the thread
		int			priority;

		//! should thread automatically detached?
		bool		autodetach;

		//! requested stack size
		size_t		stacksize;


		//! static helper routine to call this->run()
        static void *runThread(void *thiz);

        //! the code of this method runs in the new thread and calls the virtual threadMain()
        void run();

	public:
		//! identification string
        string	identity;


	public:

		//! Constructor
		/*!
        \param identity    identification string
        \param priority    requested priority of the thread, default is 0
        \param autodetach  automatically detach the thread after starting, default is true
		*/
		MMSThread(string identity = "MMSThread", int priority = 0, bool autodetach = true);


		//! Destructor
		virtual ~MMSThread();


        //! Virtual main method for the thread.
        /*!
        This virtual method is empty and have to be setup with code by a derived class.
        The MMSThread class is only the base class and cannot be constructed.
        */
        virtual void threadMain() = 0;


        //! Create and start a new thread.
        /*!
        \return true if thread is started
        \note The method returns false, if the thread is already running or cannot be started.
        \note If the method returns true, it is possible, that the new thread is already finished.
        \see isRunning()
        */
        virtual bool start();


        //! Check if the thread is running.
        /*!
        \return true if running
        \note This check is a combination between the "starting" and "running" states. This means
              that if the thread is currently starting this method also returns true.
        \see start()
        */
		virtual bool isRunning();


        //! Mark the thread as detached.
        /*!
        \note If a thread is detached, its resources are automatically released when it terminates.
        */
		void detach();


        //! Cancel execution of a thread.
        /*!
        \return true if successfully canceled
        \note The method returns false, if the thread is not running or cannot be canceled.
        */
		bool cancel();


        //! The caller of this method will wait of the termination of the thread.
        /*!
        \note This works only, if the thread is NOT detached.
        \see MMSThread(), detach()
        */
		void join();


        //! Set the size of the stack for the new thread.
        /*!
        The stack size determines the minimum size (in bytes) that will be allocated.
        \param stacksize  size of the stack in bytes
        \note The default stack size is 1000000 bytes.
        \note The stack size must be changed before the start() method will be called.
        */
		void setStacksize(size_t stacksize = 1000000);
};


void addGarbageHandler(void (*handlerfunc)(void *), void *data);

void callGarbageHandler();

void clearGarbageHandler();


#endif /*MMSTHREAD_H_*/
