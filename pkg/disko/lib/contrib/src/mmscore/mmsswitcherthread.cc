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

#include "mmscore/mmsswitcher.h"
#include "mmscore/mmsswitcherthread.h"

/**
 * Constructor of main thread
 */
MMSSwitcherThread::MMSSwitcherThread(MMSSwitcher *sw, MMSLabelWidget *date_s, MMSLabelWidget *time_s, MMSLabelWidget *date_p, MMSLabelWidget *time_p) :
	MMSThread("MMSSwitcherThread"),
	mode(0),
	curr_date(""),
	curr_time(""),
	sw(sw),
	date_s(date_s),
	time_s(time_s),
	date_p(date_p),
	time_p(time_p),
	sleepcnt(10),
	invoke_plugin(-1),
	plugin_invoked(0),
	my_spt(NULL),
	preview_shown(false) {
}

/**
 * Constructor for plugin specific access to switcher
 *
 * This instance is used for the show preview method
 * of the plugins.
 */
MMSSwitcherThread::MMSSwitcherThread(MMSSwitcher *sw) :
	mode(1),
	sw(sw) {
}

void MMSSwitcherThread::invokeShowPreview() {

    /* reset sleep cnt */
    this->sleepcnt = 10;

    /* lock me */
    this->lock.lock();

    /* save the plugin index */
    this->invoke_plugin = this->sw->curr_plugin;
    this->plugin_invoked = 0;
    this->my_spt = NULL;

    /* unlock me */
    this->lock.unlock();
}

void MMSSwitcherThread::previewShown() {
    this->preview_shown = true;
}

void MMSSwitcherThread::threadMain() {

    if (this->mode == 0) {
        unsigned int cnt = 0;

        while (1) {
            /* lock me */
            this->lock.lock();

            if (cnt % 10 == 0) {
                /* check and update date & time */
            	if ((date_s)||(date_p)||(time_s)||(time_p)) {
		            string datestr, timestr;
		            getCurrentTimeString(NULL, &datestr, &timestr);

		            if (datestr != this->curr_date) {
		                this->curr_date = datestr;
		                if (date_s) date_s->setText(this->curr_date);
		                if (date_p) date_p->setText(this->curr_date);
		            }

		            if (timestr.substr(0, 5) != this->curr_time) {
		                this->curr_time = timestr.substr(0, 5);
		                if (time_s) time_s->setText(this->curr_time);
		                if (time_p) time_p->setText(this->curr_time);
		            }
            	}
            }

            if (this->plugin_invoked) {
                if (cnt - this->plugin_invoked >= 3) {
/*                    if (this->sw->curr_previewWin < 0)
                        this->sw->waitForPreview->show();*/
                    this->plugin_invoked = 0;
                }
                else
                    if (this->preview_shown)
                        this->plugin_invoked = 0;
            }

            if (this->my_spt)
                if (!this->my_spt->isRunning()) {
                    /* if the last invoked show preview thread is not running anymore, check of waitForPreview */
/*                	if (!this->sw->waitForPreview->willHide())
	                	if (this->sw->waitForPreview->isShown()) {
	                        // the wait for preview window is not removed by the plugin thread
	                        // therefore i have to display nopreview window
	                        this->sw->noPreview->show();
	                    }*/
                    this->my_spt = NULL;
                }

            if (this->invoke_plugin >= 0) {
                /* start the showPreviewThread only if it does not running */
            	map<int, plugin_data_t *>::iterator i = this->sw->plugins.find(this->invoke_plugin);
                this->invoke_plugin = -1;
            	if (i != this->sw->plugins.end()) {
            		this->my_spt = i->second->switcher->showPreviewThread;
	                this->plugin_invoked = cnt;
	                this->preview_shown = false;

	                if (!this->my_spt->isRunning())
	                    this->my_spt->start();
            	}
            }

            /* unlock me */
            this->lock.unlock();

            /* sleep a little bit */
            while (this->sleepcnt > 0) {
                this->sleepcnt--;
                msleep(50);
            }
            this->sleepcnt = 10;
            cnt++;
        }
    }
    else
    if (this->mode == 1) {
        /* check if i have to call showpreview */
        if (this->sw->osdhandler)
            this->sw->osdhandler->invokeShowPreview(NULL);
        else
        if (this->sw->centralhandler)
            this->sw->centralhandler->invokeShowPreview(NULL);
    }
}

