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

#include "mmsgui/mmsimagewidgetthread.h"

MMSImageWidgetThread::MMSImageWidgetThread(MMSImageWidget *image) {
    this->image = image;
    this->inWait = false;
    this->stopThread = false;
    this->pauseThread = false;
}

void MMSImageWidgetThread::wait(unsigned int delaytime) {
    this->inWait = true;
    usleep(delaytime);
    while (this->pauseThread)
        usleep(1000);
    this->inWait = false;
}

void MMSImageWidgetThread::stop() {
    while (!this->inWait)
        usleep(1000);
    this->stopThread = true;
}

void MMSImageWidgetThread::pause(bool pauseThread) {
    if (pauseThread) {
        while (!this->inWait)
            usleep(1000);
        this->pauseThread = true;
    }
    else {
        this->pauseThread = false;
    }
}

void MMSImageWidgetThread::doIt() {

    /* init */
    this->inWait = false;
    this->stopThread = false;
    this->pauseThread = false;

    while (1) {
        /* first check if the window is shown */
        MMSWindow *win = this->image->getRootWindow();
        if (!win) {
            wait(1000000);
            if (this->stopThread)
                return;
            else
                continue;
        }
        if (!win->isShown()) {
            wait(1000000);
            if (this->stopThread)
                return;
            else
                continue;
        }

        /* get the next sleep time */
        unsigned int delaytime = 1000;
        bool changed = false;

        if (image->isActivated()) {
            if (image->isSelected()) {
                if ((image->selimage)&&(image->selimage_suf)) {
                    if (image->selimage_suf[image->selimage_curr_index].delaytime > 0) {
                        /* save current index */
                        unsigned int index = image->selimage_curr_index;

                        /* wait of image */
                        int s = 10*1000;
                        while (image->selimage_suf[image->selimage_curr_index+1].delaytime == MMSIM_DESC_SUF_LOADING) {
                            wait(s);
                            if (this->stopThread)
                                return;
                            if (s < 100*1000)
                                s+= 10*1000;
                            if (!image->selimage_suf)
                                /* image was deleted!!! */
                                break;
                        }

                        if (image->selimage_suf) {
                            /* get next index */
                            if (image->selimage_suf[image->selimage_curr_index+1].delaytime == MMSIM_DESC_SUF_END)
                                /* go back to the beginning */
                                image->selimage_curr_index = 0;
                            else
                                /* next image */
                                image->selimage_curr_index++;

                            /* check if changed */
                            changed = (index != image->selimage_curr_index);

                            /* get the delaytime */
                            delaytime = image->selimage_suf[image->selimage_curr_index].delaytime;
                        }
                    }
                }
            }
            else {
                if ((image->image)&&(image->image_suf)) {
                    if (image->image_suf[image->image_curr_index].delaytime > 0) {
                        /* save current index */
                        unsigned int index = image->image_curr_index;

                        /* wait of image */
                        int s = 10*1000;
                        while (image->image_suf[image->image_curr_index+1].delaytime == MMSIM_DESC_SUF_LOADING) {
                            wait(s);
                            if (this->stopThread)
                                return;
                            if (s < 100*1000)
                                s+= 10*1000;
                            if (!image->image_suf)
                                /* image was deleted!!! */
                                break;
                        }

                        if (image->image_suf) {
                            /* get next index */
                            if (image->image_suf[image->image_curr_index+1].delaytime == MMSIM_DESC_SUF_END)
                                /* go back to the beginning */
                                image->image_curr_index = 0;
                            else
                                /* next image */
                                image->image_curr_index++;

                            /* check if changed */
                            changed = (index != image->image_curr_index);

                            /* get the delaytime */
                            delaytime = image->image_suf[image->image_curr_index].delaytime;
                        }
                    }
                }
            }
        }
        else {
            if (image->isSelected()) {
                if ((image->selimage_i)&&(image->selimage_i_suf)) {
                    if (image->selimage_i_suf[image->selimage_i_curr_index].delaytime > 0) {
                        /* save current index */
                        unsigned int index = image->selimage_i_curr_index;

                        /* wait of image */
                        int s = 10*1000;
                        while (image->selimage_i_suf[image->selimage_i_curr_index+1].delaytime == MMSIM_DESC_SUF_LOADING) {
                            wait(s);
                            if (this->stopThread)
                                return;
                            if (s < 100*1000)
                                s+= 10*1000;
                            if (!image->selimage_i_suf)
                                /* image was deleted!!! */
                                break;
                        }

                        if (image->selimage_i_suf) {
                            /* get next index */
                            if (image->selimage_i_suf[image->selimage_i_curr_index+1].delaytime == MMSIM_DESC_SUF_END)
                                /* go back to the beginning */
                                image->selimage_i_curr_index = 0;
                            else
                                /* next image */
                                image->selimage_i_curr_index++;

                            /* check if changed */
                            changed = (index != image->selimage_i_curr_index);

                            /* get the delaytime */
                            delaytime = image->selimage_i_suf[image->selimage_i_curr_index].delaytime;
                        }
                    }
                }
            }
            else {
                if ((image->image_i)&&(image->image_i_suf)) {
                    if (image->image_i_suf[image->image_i_curr_index].delaytime > 0) {
                        /* save current index */
                        unsigned int index = image->image_i_curr_index;

                        /* wait of image */
                        int s = 10*1000;
                        while (image->image_i_suf[image->image_i_curr_index+1].delaytime == MMSIM_DESC_SUF_LOADING) {
                            wait(s);
                            if (this->stopThread)
                                return;
                            if (s < 100*1000)
                                s+= 10*1000;
                            if (!image->image_i_suf)
                                /* image was deleted!!! */
                                break;
                        }

                        if (image->image_i_suf) {
                            /* get next index */
                            if (image->image_i_suf[image->image_i_curr_index+1].delaytime == MMSIM_DESC_SUF_END)
                                /* go back to the beginning */
                                image->image_i_curr_index = 0;
                            else
                                /* next image */
                                image->image_i_curr_index++;

                            /* check if changed */
                            changed = (index != image->image_i_curr_index);

                            /* get the delaytime */
                            delaytime = image->image_i_suf[image->image_i_curr_index].delaytime;
                        }
                    }
                }
            }
        }

        // refresh the widget
        if (changed) {
            // refresh is required
        	this->image->enableRefresh();

            this->image->refresh();
        }

        // sleep
        if (delaytime > 0)
            wait(delaytime*1000);
        else
            wait(1000000);
        if (this->stopThread)
            return;
    }
}

void MMSImageWidgetThread::threadMain() {
    doIt();
    delete this;
}

