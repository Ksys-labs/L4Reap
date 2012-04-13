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

#ifndef IMMSCENTRALPLUGIN_H_
#define IMMSCENTRALPLUGIN_H_

#include "mmsconfig/mmsplugindata.h"
#include "mmsbase/interfaces/immsevent.h"
#include "mmsbase/interfaces/immsswitcher.h"

class IMMSCentralPlugin {
    public:
        virtual bool initialize(MMSPluginData data, IMMSSwitcher *switcher) = 0;
        virtual bool onEvent(IMMSEvent event) = 0;
        virtual bool shutdown() = 0;
        virtual bool showPreview(void *data) = 0;
        virtual bool show(void *data) = 0;
        virtual ~IMMSCentralPlugin() {};
};

#define MMS_EXPORT_CENTRAL_PLUGIN(classname) extern "C" { IMMSCentralPlugin *newCentralPlugin() { return new classname; }}

extern "C"  IMMSCentralPlugin *newCentralPlugin();
typedef IMMSCentralPlugin *(*NEWCENTRALPLUGIN_PROC)();

#endif /*IMMSCENTRALPLUGIN_H_*/
