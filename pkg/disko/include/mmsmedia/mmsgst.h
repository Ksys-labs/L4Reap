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

#ifndef __MMSGST_H__
#define __MMSGST_H__

#ifdef __HAVE_GSTREAMER__

/**
 * @brief   GStreamer input handling.
 *
 * @ingroup     mmsmedia
 *
 * @author      Stefan Schwarzer (stefan.schwarzer@diskohq.org)
 * @author      Matthias Hardt (matthias.hardt@diskohq.org)
 * @author      Jens Schneider (pupeider@gmx.de)
 * @author      Guido Madaus (guido.madaus@diskohq.org)
 * @author      Patrick Helterhoff (patrick.helterhoff@diskohq.org)
 * @author		René Bählkow (rene.baehlkow@diskohq.org)
 */

#include <stdlib.h>
#include <gst/gst.h>
#include <unistd.h>

#include "mmsgui/mmsgui.h"

//! freeing any allocated gstreamer memory
void mmsGstFree();

//! create a pipeline
GstElement *mmsGstLaunch(const char *pipeline_description);

//! init a gstreamer pipeline with an uri and connect it to an surface
GstElement *mmsGstInit(const string uri, MMSFBSurface *surface);

//! init a gstreamer pipeline with an uri and connect it to an window
GstElement *mmsGstInit(const string uri, MMSWindow *window);

//! start playback of a pipeline
bool mmsGstPlay(GstElement *pipelineX);

//! send key press event to a pipeline
bool mmsGstSendKeyPress(GstElement *pipeline, MMSKeySymbol key);

//! send key release event to a pipeline
bool mmsGstSendKeyRelease(GstElement *pipeline, MMSKeySymbol key);

//! send button press event to a pipeline
bool mmsGstSendButtonPress(GstElement *pipeline, int posx, int posy);

//! send button release event to a pipeline
bool mmsGstSendButtonRelease(GstElement *pipeline, int posx, int posy);

//! send axis motion event to a pipeline
bool mmsGstSendAxisMotion(GstElement *pipeline, int posx, int posy);

#endif

#endif /* __MMSGST_H__ */
