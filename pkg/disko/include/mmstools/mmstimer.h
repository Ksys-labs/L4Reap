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

#ifndef MMSTIMER_H_
#define MMSTIMER_H_

#include <sigc++/sigc++.h>
#include <csignal>
extern "C"
{
#include "sys/types.h"
#include "unistd.h"
}
#include "mmstools/mmsthread.h"

/*!
 * \brief The MMSTimer provides timers.
 *
 * You can use MMSTimer for repetitive or single shot timers
 * \see MMSTimer(bool singleShot). After expiration of an interval
 * (\see start(unsigned int milliSeconds)) a signal \see timeOut emits.
 * <code>
 * 	void fn1() {printf("fn1 called\n);}
 * 	MMSTimer *repetitiveTimer = new MMSTimer();
 * 	repetitiveTimer->timeOut->connect(sigc::ptr_fun(&fn1));
 * 	// calls every 1.5 seconds fn1()
 * 	repetitiveTimer->start(1500);
 *
 * 	void fn2({printf("fn2 called\n")});
 * 	MMSTimer *singleShotTimer = new MMSTimer(true);
 * 	singleShotTimer->timeOut->connect(sigc::ptr_fun(&fn2));
 * 	// calls after 30 seconds fn2()
 * 	singleShotTimer->start(30000);
 * </code>
 */

class MMSTimer: public MMSThread
{
	public:
		/*!
		 * \brief Creates a new MMSTimer.
		 *
		 * \param singleShot    If this is set to true, the timer will
		 *                      emit only once, else it will do it
		 *                      repetitive.
		 *
		 * \note If singleShot is set to true, internally the thread will be stopped after the signal is emitted.
		 */
		MMSTimer(bool singleShot = false);

		/*!
		 * \brief Destroys a MMSTimer.
		 */
		~MMSTimer();

		/*!
		 * \brief Starts timer.
		 *
		 * \param milliSeconds    the timer interval in milli seconds
		 * \param firsttime_ms    the first timeOut will be emitted after firsttime_ms milliseconds
		 *                        default 0 means that the first timeOut will also be emitted after milliSeconds
		 *
		 * \note If the timer is stopped (see stop()), it will be restarted using restart() but with new settings.
		 * \see stop()
		 */
		bool start(unsigned int milliSeconds, unsigned int firsttime_ms = 0);

		/*!
		 * \brief Restarts timer.
		 *
		 * A running timer stops first and no signal emits. After this
		 * a new timer starts with the full interval (milliSeconds) set with start().
		 *
		 * \see start()
		 * \see stop()
		 */
		bool restart();

		/*!
		 * \brief Stops timer.
		 *
		 * A running timer stops and no signal emits.
		 *
		 * \note No signal will be emitted anymore, but internally the thread will be hold with pthread_cond_wait().
		 * \see restart()
		 */
		bool stop();

		/*!
		 * \brief This signal emits if the timer expires.
		 *
		 * Connect a function to this signal.
		 */
		sigc::signal<void> timeOut;

	private:
		bool 			singleShot;
		enum {
			START,
			RESTART,
			STOP,
			QUIT
		} 				action;

		bool			firsttime;
		__time_t 		secs;
		long int 		nSecs;
		__time_t 		ft_secs;
		long int 		ft_nSecs;

		pthread_cond_t 	cond;
		pthread_mutex_t	mutex;

		void threadMain();
};

#endif /* MMSTIMER_H_ */
