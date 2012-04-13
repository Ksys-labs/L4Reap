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

#include "mmsgui/mmsfbmanager.h"
#include "mmsgui/fb/mmsfbsurfacemanager.h"
#include <string.h>
#include <stdlib.h>

MMS_CREATEERROR(MMSFBManagerError);

/* initialize the mmsfbmanager object */
MMSFBManager mmsfbmanager;

/* exit handler routine */
void mmsfbmanager_onexit(int num, void *arg) {
    MMSFBManager *o=(MMSFBManager*)arg;
    o->release();
}

MMSFBManager::MMSFBManager() {
    // init me
    this->graphicslayer = NULL;
    this->videolayer = NULL;
    this->graphicslayerid = -1;
    this->videolayerid = -1;
    this->layercount = 0;
}

MMSFBManager::~MMSFBManager() {
}

bool MMSFBManager::init(int argc, char **argv, string appl_name, string appl_icon_name,
						bool virtual_console, bool flip_flush) {
	int myargc=argc;
	char *myargv[255];
	int i;

	// save virtual console state
	this->virtual_console = virtual_console;

	// per default we have one layer
    this->layercount = 1;

	for(i=0;i<argc;i++)
		myargv[i]=strdup(argv[i]);

    DEBUGMSG("MMSGUI", "init mmsfb");
    bool ea = config.getExtendedAccel();
#ifdef  __HAVE_DIRECTFB__
	if (config.getAllocMethod() == "DFB") {
		// use dfb even if extended accel
		ea = false;
	}
#endif

	if (this->config.getRotateScreen() == 180) {
		// set rotate by 180° flag
		MMSFBBase_rotate180 = true;
	}

	// get layer settings from config
	MMSConfigDataLayer videolayer_conf = this->config.getVideoLayer();
	MMSConfigDataLayer graphicslayer_conf = this->config.getGraphicsLayer();

#ifdef  __HAVE_DIRECTFB__
	if(videolayer_conf.outputtype == MMSFB_OT_X11) {
		myargv[myargc++] = strdup("--dfb:system=x11");
		char mode[24];
		snprintf(mode, 24, "--dfb:mode=%dx%d", graphicslayer_conf.rect.w, graphicslayer_conf.rect.h);
		myargv[myargc++] = strdup(mode);
	}
#endif

	// init the MMSFB class
    if (!mmsfb->init(myargc, myargv, config.getBackend(), graphicslayer_conf.rect,
					 ea, config.getFullScreen(), config.getPointer(), appl_name, appl_icon_name, config.getHideApplication())) {
	    DEBUGMSG("MMSGUI", "init mmsfb failed!");
        throw MMSFBManagerError(0, MMSFB_LastErrorString);
	}

    DEBUGMSG("MMSGUI", "get video layer");
    if (!mmsfb->getLayer(videolayer_conf.id, &this->videolayer, videolayer_conf.outputtype, this->virtual_console))
        throw MMSFBManagerError(0, MMSFB_LastErrorString);

    if (videolayer_conf.id == graphicslayer_conf.id) {
    	DEBUGMSG("MMSGUI", "video layer and graphics layer are the same");
        this->graphicslayer = this->videolayer;

        if (!flip_flush)
        	this->graphicslayer->setFlipFlags(MMSFB_FLIP_ONSYNC);
        else
        	this->graphicslayer->setFlipFlags(MMSFB_FLIP_ONSYNC | MMSFB_FLIP_FLUSH);
    }
    else {
        this->layercount++;
        DEBUGMSG("MMSGUI", "get graphics layer");
        if (!mmsfb->getLayer(graphicslayer_conf.id, &this->graphicslayer, graphicslayer_conf.outputtype, false))
            throw MMSFBManagerError(0, MMSFB_LastErrorString);

        if (!flip_flush)
        	this->graphicslayer->setFlipFlags(MMSFB_FLIP_ONSYNC);
        else
        	this->graphicslayer->setFlipFlags(MMSFB_FLIP_ONSYNC | MMSFB_FLIP_FLUSH);

    	if (videolayer_conf.outputtype == MMSFB_OT_MATROXFB)
        	this->videolayer->setFlipFlags(MMSFB_FLIP_WAITFORSYNC);
        else
        	this->videolayer->setFlipFlags(MMSFB_FLIP_ONSYNC);
    }

    if (!this->graphicslayer->getID(&this->graphicslayerid))
        throw MMSFBManagerError(0, MMSFB_LastErrorString);

    if (!this->videolayer->getID(&this->videolayerid))
        throw MMSFBManagerError(0, MMSFB_LastErrorString);

    /* set on exit handler */
    on_exit(mmsfbmanager_onexit, this);

    return true;
}

void MMSFBManager::release() {
	DEBUGMSG("MMSGUI", "release mmsfb");
    if (this->videolayer)
        delete this->videolayer;
    mmsfb->release();
}

void MMSFBManager::applySettings() {
	DEBUGMSG("MMSGUI", "configure graphics layer");

	// get layer settings from config
	MMSConfigDataLayer videolayer_conf = this->config.getVideoLayer();
	MMSConfigDataLayer graphicslayer_conf = this->config.getGraphicsLayer();

	// get the window pixelformat
	MMSFBSurfacePixelFormat window_pixelformat = config.getGraphicsWindowPixelformat();
	switch (window_pixelformat) {
	case MMSFB_PF_ARGB:
	case MMSFB_PF_AiRGB:
	case MMSFB_PF_AYUV:
	case MMSFB_PF_ARGB4444:
	case MMSFB_PF_RGB16:
		break;
	default:
		// window pixelformat not set or unsupported, use the layer pixelformat
		window_pixelformat = graphicslayer_conf.pixelformat;
		if (!isAlphaPixelFormat(window_pixelformat)) {
			// the gui internally needs surfaces with alpha channel
			// now we have to decide if we are working in RGB or YUV color space
			if (!isRGBPixelFormat(window_pixelformat))
				// so switch all non-alpha pixelformats to AYUV
				window_pixelformat = MMSFB_PF_AYUV;
			else
				// so switch all non-alpha pixelformats to ARGB
				window_pixelformat = MMSFB_PF_ARGB;
		}
		else
		if (isIndexedPixelFormat(window_pixelformat)) {
			// the gui internally needs non-indexed surfaces
			// so switch all indexed pixelformats to ARGB
			window_pixelformat = MMSFB_PF_ARGB;
		}
		break;
	}

	// get the surface pixelformat
	MMSFBSurfacePixelFormat surface_pixelformat = config.getGraphicsSurfacePixelformat();
	switch (surface_pixelformat) {
	case MMSFB_PF_ARGB:
	case MMSFB_PF_AiRGB:
	case MMSFB_PF_AYUV:
	case MMSFB_PF_ARGB4444:
	case MMSFB_PF_RGB16:
		break;
	default:
		// surface pixelformat not set or unsupported, use the layer pixelformat
		surface_pixelformat = graphicslayer_conf.pixelformat;
		if (!isAlphaPixelFormat(surface_pixelformat)) {
			// the gui internally needs surfaces with alpha channel
			// now we have to decide if we are working in RGB or YUV color space
			if (!isRGBPixelFormat(surface_pixelformat))
				// so switch all non-alpha pixelformats to AYUV
				surface_pixelformat = MMSFB_PF_AYUV;
			else
				// so switch all non-alpha pixelformats to ARGB
				surface_pixelformat = MMSFB_PF_ARGB;
		}
		else
		if (isIndexedPixelFormat(surface_pixelformat)) {
			// the gui internally needs non-indexed surfaces
			// so switch all indexed pixelformats to ARGB
			surface_pixelformat = MMSFB_PF_ARGB;
		}
		break;
	}

	// set exclusive access to the graphics layer
	DEBUGMSG("MMSGUI", "set exclusive access");
	if (!this->graphicslayer->setExclusiveAccess())
        throw MMSFBManagerError(0, MMSFB_LastErrorString);

	DEBUGMSG("MMSGUI", "set configuration");
    if(!this->graphicslayer->setConfiguration(graphicslayer_conf.rect.w, graphicslayer_conf.rect.h,
											   graphicslayer_conf.pixelformat,
											   graphicslayer_conf.buffermode,
											   graphicslayer_conf.options,
                                               window_pixelformat,
                                               surface_pixelformat))
        throw MMSFBManagerError(0, MMSFB_LastErrorString);

    if (this->videolayerid != this->graphicslayerid) {
#ifdef  __HAVE_DIRECTFB__
    	if (config.getBackend() == MMSFB_BE_X11) {
			//give a little time to window routines
			usleep(300000);
        }
#endif

        // use both layers
        DEBUGMSG("MMSGUI", "configure video layer");

        DEBUGMSG("MMSGUI", "set exclusive access");
        // set exclusive access to the video layer
        if (!this->videolayer->setExclusiveAccess())
            throw MMSFBManagerError(0, MMSFB_LastErrorString);

    	DEBUGMSG("MMSGUI", "set configuration");
        // set video layer's config
        if (!this->videolayer->setConfiguration(videolayer_conf.rect.w, videolayer_conf.rect.h,
												videolayer_conf.pixelformat,
												videolayer_conf.buffermode,
												videolayer_conf.options))
            throw MMSFBManagerError(0, MMSFB_LastErrorString);
		//this->videolayer->dfblayer->SetFieldParity(this->videolayer->dfblayer,0);

        // set the full opacity of the graphics layer
        this->graphicslayer->setOpacity(0);

        if (graphicslayer_conf.outputtype == MMSFB_OT_VIAFB) {
            // set the video layer behind the graphics layer
        	DEBUGMSG("MMSGUI", "set the video layer behind the graphics layer");
            this->videolayer->setLevel(-1);
        }
        else
        if (graphicslayer_conf.outputtype == MMSFB_OT_XSHM) {
        	DEBUGMSG("MMSGUI", "set the video layer behind the graphics layer");
            this->graphicslayer->setLevel(+1);
        }
    }

    // set global surface attributes
    string buffermode = graphicslayer_conf.buffermode;
    MMSFBSurface *gls;
    if (this->graphicslayer->getSurface(&gls, this->virtual_console)) {
    	// set the static extended accel flag
		gls->setExtendedAcceleration(config.getExtendedAccel());

		// set the global alloc method (default is malloc)
		if (mmsfb->getBackend() == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
			string am = config.getAllocMethod();
			if (am == "MALLOC") {
				if (!config.getExtendedAccel())
					gls->setAllocMethod(MMSFBSurfaceAllocMethod_dfb);
			}
			else
				gls->setAllocMethod(MMSFBSurfaceAllocMethod_dfb);
#endif
		}
		else
		if (graphicslayer_conf.outputtype == MMSFB_OT_OGL) {
			gls->setAllocMethod(MMSFBSurfaceAllocMethod_ogl);
		}
    }

    // init the mmsfbwindowmanager
	mmsfbwindowmanager->init(this->graphicslayer, (config.getPointer()==MMSFB_PM_TRUE));

    DEBUGMSG("MMSGUI", "creating temporary surface: %dx%d, %s", graphicslayer_conf.rect.w, graphicslayer_conf.rect.h, getMMSFBPixelFormatString(surface_pixelformat).c_str());
    mmsfbsurfacemanager->createTemporarySurface(graphicslayer_conf.rect.w, graphicslayer_conf.rect.h, surface_pixelformat, (buffermode == MMSFB_BM_BACKSYSTEM));
}


MMSFBLayer *MMSFBManager::getVideoLayer() {
    return this->videolayer;
}

MMSFBLayer *MMSFBManager::getGraphicsLayer() {
    return this->graphicslayer;
}

int MMSFBManager::getLayerCount() {
    return this->layercount;
}

