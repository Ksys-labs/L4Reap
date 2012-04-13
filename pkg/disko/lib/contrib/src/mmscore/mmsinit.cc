/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#include <cstring>
extern "C" {
#include <libxml/parser.h>
}
#include "mmscore/mmsinit.h"
#include "mms.h"

#ifdef __ENABLE_ACTMON__
#include "mmscore/mmsperf.h"
#include "mmscore/mmsperfinterface.h"

static MMSPerf *mmsperf = NULL;
#endif

static MMSPluginManager             *pluginmanager      = NULL;
static MMSEventDispatcher           *eventdispatcher    = NULL;
static MMSEventSignupManager        *eventsignupmanager = NULL;
static MMSConfigData                *config             = NULL;
static MMSEvent                     *masterevent        = NULL;
static MMSEventSignup               *mastereventsignup  = NULL;
/* static MMSImportScheduler           *importscheduler    = NULL; */
static MMSInputManager              *inputs             = NULL;
static MMSThemeManager 				*themeManager		= NULL;
static MMSWindowManager             *windowmanager		= NULL;

void (*pluginRegisterCallback)(MMSPluginManager*) = NULL;

static void on_exit() {
	if(pluginmanager) delete pluginmanager;

	// free memory from libxml2
	xmlCleanupParser();
}


bool mmsInit(MMSINIT_FLAGS flags, int argc, char *argv[], string configfile,
			 string appl_name, string appl_icon_name,
			 MMSConfigDataGlobal *global, MMSConfigDataDB *configdb, MMSConfigDataDB *datadb,
			 MMSConfigDataGraphics *graphics, MMSConfigDataLanguage *language) {

	try {
		// get special args from configfile string
		int args_pos;
		string args;
		if ((args_pos = (int)configfile.find("--disko:")) >= 0) {
			args = configfile.substr(args_pos);
			if (args_pos > 1)
				configfile = configfile.substr(0, args_pos-1);
			else
				configfile = "";
		}

        //check if config file is given per commandline
        for (int i = 1; i < argc; i++) {
        	if (memcmp(argv[i], "--disko:config=", 15)==0) {
        		// yes
        		configfile = &(argv[i][15]);
        	}
        }

        // initialize libxml2
        xmlInitParser();

        MMSRcParser 			rcparser;
        MMSConfigDataGlobal     *rcGlobal 	= NULL;
        MMSConfigDataDB         *rcConfigDB = NULL;
        MMSConfigDataDB			*rcDataDB 	= NULL;
        MMSConfigDataGraphics   *rcGraphics = NULL;
        MMSConfigDataLanguage	*rcLanguage = NULL;

        bool config_avail = false;

        if(!configfile.empty()) {
        	// config file given
        	DEBUGOUT("set configfile: %s\n", configfile.c_str());
		    try {
				rcparser.parseFile(configfile);
				rcparser.getMMSRc(&rcGlobal, &rcConfigDB, &rcDataDB, &rcGraphics, &rcLanguage);
				config_avail = true;
		    } catch (MMSRcParserError &ex) {
	        	// config file not found
	        	DEBUGMSG_OUTSTR("Core", "could not read config, try --disko:config=./etc/diskorc.xml");
		    }
        } else {
        	// searching for diskorc.xml
		    try {
		        rcparser.parseFile("./etc/diskorc.xml");
		        rcparser.getMMSRc(&rcGlobal, &rcConfigDB, &rcDataDB, &rcGraphics, &rcLanguage);
				config_avail = true;
		    } catch (MMSRcParserError &ex) {
		    	// next try
		        try {
					rcparser.parseFile("/etc/diskorc.xml");
					rcparser.getMMSRc(&rcGlobal, &rcConfigDB, &rcDataDB, &rcGraphics, &rcLanguage);
		        } catch (MMSRcParserError &ex) {
		        	// config file not found
		        	DEBUGMSG_OUTSTR("Core", "could not read config, try --disko:config=./etc/diskorc.xml");
		        }
		    }
        }

		if (!config_avail) {
			// failed to read diskorc
			return false;
		}

        // create first (static) MMSConfigData
        if (!rcGlobal) {
        	// config file not set, load defaults
            MMSRcParser rcparser;
			rcparser.getMMSRc(&rcGlobal, &rcConfigDB, &rcDataDB, &rcGraphics, &rcLanguage);
        	config = new MMSConfigData((global)?*global:*rcGlobal, (configdb)?*configdb:*rcConfigDB, (datadb)?*datadb:*rcDataDB,
									   (graphics)?*graphics:*rcGraphics, (language)?*language:*rcLanguage);
        } else {
        	config = new MMSConfigData(*rcGlobal, *rcConfigDB, *rcDataDB, *rcGraphics, *rcLanguage);
        }

        // overwrite config values from args and/or argv
        rcparser.updateConfig(config, args, argc, argv);

        if(!(flags & MMSINIT_SILENT)) {
			printf("\n");
			printf("****   *   ***   *  *   ***\n");
			printf(" *  *  *  *      * *   *   *\n");
			printf(" *  *  *   ***   **    *   *\n");
			printf(" *  *  *      *  * *   *   *\n");
			printf("****   *   ***   *  *   ***  V%s\n",DISKO_VERSION_STR);
			printf("----------------------------------------------------------------------\n");
			printf("The Linux application framework for embedded devices.\n");
			printf("\n");
			printf("   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,\n");
			printf("                           Matthias Hardt, Guido Madaus\n");
			printf("   Copyright (C) 2007-2008 BerLinux Solutions GbR\n");
			printf("                           Stefan Schwarzer & Guido Madaus\n");
			printf("   Copyright (C) 2009-2011 BerLinux Solutions GmbH\n");
			printf("----------------------------------------------------------------------\n");
#ifdef __L4_RE__
            printf("----------------------------------------------------------------------\n");
            printf("Porting Disko UI Framework to L4Re.\n");
            printf("\n");
            printf("   (C) 2011 Ivan Loskutov\n");
            printf("----------------------------------------------------------------------\n");
#endif

			int pcv = 1;
			if (*((char *)&pcv) == 1) {
				DEBUGMSG_OUTSTR("Core", "Platform type................: little-endian");
			} else {
				DEBUGMSG_OUTSTR("Core", "Platform type................: big-endian");
			}

			MMSConfigDataLayer videolayer = config->getVideoLayer();
			MMSConfigDataLayer graphicslayer = config->getGraphicsLayer();

			DEBUGMSG_OUTSTR("Core", "ConfigDB.....................: " + config->getConfigDBDatabase() + " (" + config->getConfigDBAddress() + ")");
			DEBUGMSG_OUTSTR("Core", "DataDB.......................: " + config->getDataDBDatabase() + " (" + config->getDataDBAddress() + ")");
			DEBUGMSG_OUTSTR("Core", "Logfile......................: " + config->getLogfile());
			DEBUGMSG_OUTSTR("Core", "First plugin.................: " + config->getFirstPlugin());
			DEBUGMSG_OUTSTR("Core", "Input map....................: " + config->getInputMap());
			DEBUGMSG_OUTSTR("Core", "Prefix.......................: " + config->getPrefix());
			DEBUGMSG_OUTSTR("Core", "Theme........................: " + config->getTheme());
			DEBUGMSG_OUTSTR("Core", "Backend......................: " + getMMSFBBackendString(config->getBackend()));
			DEBUGMSG_OUTSTR("Core", "Graphics layer id............: " + iToStr(graphicslayer.id));
			DEBUGMSG_OUTSTR("Core", "Output type..................: " + getMMSFBOutputTypeString(config->getGraphicsLayer().outputtype));
			DEBUGMSG_OUTSTR("Core", "Graphics layer resolution....: " + iToStr(graphicslayer.rect.w) + "x" + iToStr(graphicslayer.rect.h));
			DEBUGMSG_OUTSTR("Core", "Graphics layer position......: " + iToStr(graphicslayer.rect.x) + "," + iToStr(graphicslayer.rect.y));
			DEBUGMSG_OUTSTR("Core", "Graphics layer pixelformat...: " + getMMSFBPixelFormatString(graphicslayer.pixelformat));
			DEBUGMSG_OUTSTR("Core", "Graphics layer options.......: " + graphicslayer.options);
			DEBUGMSG_OUTSTR("Core", "Graphics layer buffermode....: " + graphicslayer.buffermode);
			DEBUGMSG_OUTSTR("Core", "Video layer id...............: " + iToStr(videolayer.id));
			DEBUGMSG_OUTSTR("Core", "Output type..................: " + getMMSFBOutputTypeString(config->getVideoLayer().outputtype));
			DEBUGMSG_OUTSTR("Core", "Video layer resolution.......: " + iToStr(videolayer.rect.w) + "x" + iToStr(videolayer.rect.h));
			DEBUGMSG_OUTSTR("Core", "Video layer position.........: " + iToStr(videolayer.rect.x) + "," + iToStr(videolayer.rect.y));
			DEBUGMSG_OUTSTR("Core", "Video layer pixelformat......: " + getMMSFBPixelFormatString(videolayer.pixelformat));
			DEBUGMSG_OUTSTR("Core", "Video layer options..........: " + videolayer.options);
			DEBUGMSG_OUTSTR("Core", "Video layer buffermode.......: " + videolayer.buffermode);
			DEBUGMSG_OUTSTR("Core", "Visible screen area..........: " + iToStr(config->getVRect().x) + "," + iToStr(config->getVRect().y) + "," + iToStr(config->getVRect().w) + "," + iToStr(config->getVRect().h));

			if (config->getStdout()) {
				DEBUGMSG_OUTSTR("Core", "Log to stdout................: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Log to stdout................: no");
			}

			DEBUGMSG_OUTSTR("Core", "Input Interval...............: " + iToStr(config->getInputInterval()) + " ms");
			DEBUGMSG_OUTSTR("Core", "Input Mode...................: " + config->getInputMode());

			if (config->getShutdown()) {
				DEBUGMSG_OUTSTR("Core", "Call shutdown command........: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Call shutdown command........: no");
			}

			DEBUGMSG_OUTSTR("Core", "Shutdown command.............: " + config->getShutdownCmd());

			DEBUGMSG_OUTSTR("Core", "Touch pad/screen area........: " + iToStr(config->getTouchRect().x) + "," + iToStr(config->getTouchRect().y) + "," + iToStr(config->getTouchRect().w) + "," + iToStr(config->getTouchRect().h));

			DEBUGMSG_OUTSTR("Core", "Show mouse pointer...........: " + getMMSFBPointerModeString(config->getPointer()));

			DEBUGMSG_OUTSTR("Core", "Graphics window pixelformat..: " + getMMSFBPixelFormatString(config->getGraphicsWindowPixelformat()));
			DEBUGMSG_OUTSTR("Core", "Graphics surface pixelformat.: " + getMMSFBPixelFormatString(config->getGraphicsSurfacePixelformat()));

			if (config->getExtendedAccel()) {
				DEBUGMSG_OUTSTR("Core", "Extended acceleration........: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Extended acceleration........: no");
			}

			DEBUGMSG_OUTSTR("Core", "Alloc Method.................: " + config->getAllocMethod());

			DEBUGMSG_OUTSTR("Core", "Fullscreen...................: " + getMMSFBFullScreenModeString(config->getFullScreen()));
			DEBUGMSG_OUTSTR("Core", "Rotate screen................: " + iToStr(config->getRotateScreen()) + "°");

			if (config->getHideApplication()) {
				DEBUGMSG_OUTSTR("Core", "Hide application.............: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Hide application.............: no");
			}

			if (config->getInitialLoad()) {
				DEBUGMSG_OUTSTR("Core", "Initial load.................: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Initial load.................: no");
			}

			if (config->getDebugFrames()) {
				DEBUGMSG_OUTSTR("Core", "Draw debug frames............: yes");
			} else {
				DEBUGMSG_OUTSTR("Core", "Draw debug frames............: no");
			}

			DEBUGMSG_OUTSTR("Core", "Sourcelanguage...............: " + getMMSLanguageString(config->getSourceLang()));
			DEBUGMSG_OUTSTR("Core", "Targetlanguage...............: " + getMMSLanguageString(config->getDefaultTargetLang()));
			DEBUGMSG_OUTSTR("Core", "Add missing translations.....: " + (config->getAddTranslations() ? string("yes") : string("no")));
			DEBUGMSG_OUTSTR("Core", "Language file directory......: " + config->getLanguagefileDir());

			DEBUGMSG_OUTSTR("Core", "Activity monitor address.....: " + config->getActMonAddress());
			DEBUGMSG_OUTSTR("Core", "Activity monitor port........: " + iToStr(config->getActMonPort()));

			printf("----------------------------------------------------------------------\n");

			if (!appl_name.empty()) {
				DEBUGMSG_OUTSTR("Core", "Starting " + appl_name + "...");
			}
        }

        if((flags & MMSINIT_WINDOWMANAGER)||(flags & MMSINIT_GRAPHICS)) {
            DEBUGMSG("Core", "initialize frame buffer");

#ifdef __ENABLE_ACTMON__
			// init mmsfb performance data collector
			mmsperf = new MMSPerf();

			// init mmsfb performance data interface
			MMSTCPServer *server = new MMSTCPServer(new MMSPerfInterface(mmsperf),
													config->getActMonAddress(), config->getActMonPort(),
													"MMSTCPServer4Perfmon");
			server->start();
#endif

            // set static MMSWidget inputmode
        	MMSWidget_inputmode = config->getInputMode();

        	// initialize the theme object which stores the global theme
        	globalTheme = new MMSTheme(config->getInitialLoad(), config->getDebugFrames());

			// init the fbmanager, check if virtual console should be opened
            mmsfbmanager.init(argc, argv, appl_name, appl_icon_name,
								(!(flags & MMSINIT_NO_CONSOLE)), (flags & MMSINIT_FLIP_FLUSH));
            mmsfbmanager.applySettings();

			if (flags & MMSINIT_THEMEMANAGER) {
				DEBUGMSG("Core", "starting theme manager");
				themeManager = new MMSThemeManager(config->getData(),config->getTheme());
            }

            if(flags & MMSINIT_WINDOWMANAGER) {
                DEBUGMSG("Core", "starting window manager");

                windowmanager = new MMSWindowManager(config->getVRect());
    	        if(!windowmanager) {
    	        	DEBUGMSG("Core", "couldn't create windowmanager.");
    	        	return false;
    	        }

    	        // creating the background window
    	        // note: regarding performance, this window must not have a alphachannel!
    	        //         --> using MMSW_VIDEO flag
    	        // note: additionally the window should never reside on the video layer
    	        //         --> using MMSW_USEGRAPHICSLAYER flag
    	        DEBUGMSG("Core", "creating background window");
    	        MMSRootWindow *rootwin = new MMSRootWindow("background_rootwindow","","",
    	        											MMSALIGNMENT_NOTSET,
    	        											(MMSWINDOW_FLAGS)(MMSW_VIDEO | MMSW_USEGRAPHICSLAYER));
    	        if(!rootwin) {
    	        	DEBUGMSG("Core", "couldn't create background window.");
    	        	return false;
    	        }

    	        DEBUGMSG("Core", "setting windowmanager for background window");
    	        rootwin->setWindowManager((IMMSWindowManager*)windowmanager);

    	        DEBUGMSG("Core", "setting window as background window");
    	        windowmanager->setBackgroundWindow(rootwin);

    	        DEBUGMSG("Core", "windowmanager initialization done");
            }
            if(flags  & MMSINIT_INPUTS) {
    			DEBUGMSG("Core", "creating input manager");
    			inputs = new MMSInputManager(config->getSysConfig() + "/inputmap.xml", config->getInputMap());
    			inputs->setWindowManager((IMMSWindowManager*)windowmanager);

    			//inputs
    			DEBUGMSG("Core", "add input device");
    			inputs->addDevice(MMS_INPUT_KEYBOARD, config->getInputInterval());

    			DEBUGMSG("Core", "creating master subscription");
    			MMSInputSubscription sub1(inputs);
            }
        }

        if(flags & MMSINIT_PLUGINMANAGER) {
        	if(!pluginmanager) {
				DEBUGMSG("Core", "creating PluginManager");
				pluginmanager = new MMSPluginManager();
        	}

			if(pluginRegisterCallback)
				pluginRegisterCallback(pluginmanager);

	        DEBUGMSG("Core", "loading Backend Plugins...");
	        pluginmanager->loadBackendPlugins();

	        DEBUGMSG("Core", "loading OSD Plugins...");
	        pluginmanager->loadOSDPlugins();

	        DEBUGMSG("Core", "loading Central Plugins...");
	        pluginmanager->loadCentralPlugins();

	        DEBUGMSG("Core", "loading Import Plugins...");
	        pluginmanager->loadImportPlugins();

	        DEBUGMSG("Core", "initialize Import Plugins...");
	        pluginmanager->initializeImportPlugins();
        }

        if(flags & MMSINIT_EVENTS) {

        	DEBUGMSG("Core", "creating EventSignupManager");
        	eventsignupmanager = new MMSEventSignupManager();

	        DEBUGMSG("Core", "creating EventDispatcher");
	        eventdispatcher = new MMSEventDispatcher(pluginmanager,eventsignupmanager);

	        masterevent = new MMSEvent();
	        masterevent->setDispatcher(eventdispatcher);

	        MMSPluginData data;
	        mastereventsignup = new MMSEventSignup(data);
	        mastereventsignup->setManager(eventsignupmanager);
        }

//        DEBUGMSG("Core", "starting ImportScheduler");
//        importscheduler = new MMSImportScheduler(pluginmanager);
//        importscheduler->start();

//        DEBUGMSG("Core", "starting music manager");
//        soundmanager = new MMSMusicManager();

		/* here must be a barrier implemented */
        if((flags & MMSINIT_INPUTS) && (flags & MMSINIT_GRAPHICS)) {
            DEBUGMSG("Core", "wait for inputs");
            inputs->startListen();

        }

#ifdef __L4_RE__
        // TODO:
#else
    	atexit(on_exit);
#endif

    } catch(MMSError &error) {
        DEBUGMSG("Core", "Abort due to: " + error.getMessage());
        fprintf(stderr, "Error initializing disko: %s\n", error.getMessage().c_str());
        return false;
    }

	return true;
}

bool mmsRelease() {
	//TODO
	return true;
}

bool registerSwitcher(IMMSSwitcher *switcher) {
    DEBUGMSG("Core", "registering switcher");
    switcher->setInputManager(inputs);
    switcher->setWindowManager((IMMSWindowManager*)windowmanager);
    if(pluginmanager) {
        switcher->setPluginManager(pluginmanager);
        pluginmanager->setSwitcher(switcher);

        DEBUGMSG("Core", "initialize Backend Plugins...");
        pluginmanager->initializeBackendPlugins();

        DEBUGMSG("Core", "initialize OSD Plugins...");
        pluginmanager->initializeOSDPlugins();

        DEBUGMSG("Core", "initialize Central Plugins...");
        pluginmanager->initializeCentralPlugins();
    }

    /* send event that everything is initialized */
    if(masterevent) {
		MMSEvent *initializedEvent = new MMSEvent("MMSINIT.initialized");
		initializedEvent->send();
	}

    return true;
}

void setPluginRegisterCallback(void(*cb)(MMSPluginManager*)) {
	pluginRegisterCallback = cb;
}

IMMSWindowManager *getWindowManager() {
	return windowmanager;
}

void setPluginManager(MMSPluginManager *pm) {
	if(pluginmanager) {
		DEBUGMSG("CORE", "Error: You cannot set a pluginmanager after calling mmsinit()");
		return;
	}
	pluginmanager = pm;
}

MMSPluginManager *getPluginManager() {
	return pluginmanager;
}

MMSFBLayer *getVideoLayer() {
	return mmsfbmanager.getVideoLayer();
}

MMSFBLayer *getGraphicsLayer() {
	return mmsfbmanager.getGraphicsLayer();
}

void showBackgroundWindow() {
	// show the background window if it is hidden
	IMMSWindowManager *wm = getWindowManager();
	if (wm) {
		MMSWindow *win = wm->getBackgroundWindow();
		if (win) {
			win->show();
			win->waitUntilShown();
		}
	}
}
