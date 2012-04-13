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

#ifndef MMSSWITCHER_H_
#define MMSSWITCHER_H_

#include "mmsbase/interfaces/immsswitcher.h"
#include "mmscore/mmspluginmanager.h"
#include "mmscore/mmsswitcherthread.h"
#include "mmsinput/mmsinput.h"
#include "mmsgui/mmsdialogmanager.h"
#include "mmsgui/mmsmainwindow.h"

typedef struct {
    MMSPluginData 				plugindata;
    vector<MMSChildWindow *> 	previewWins;
    MMSSwitcher					*switcher;
} plugin_data_t;


class MMSSwitcher : public IMMSSwitcher {
	private:
		const void addPluginsToMenu(const vector<MMSPluginData *> &plugins);

	protected:
        MMSConfigData                        config;

        static IMMSWindowManager             *windowmanager;
        static MMSPluginManager              *pluginmanager;
        static MMSInputManager               *inputmanager;
        static vector<MMSInputSubscription*> subscriptions;

        static MMSDialogManager              dm;                 /**< dialog manager for whole switcher window                      */
        static MMSMainWindow                 *window;            /**< whole switcher window                                         */

        static map<int, plugin_data_t *>     plugins;            /**< loaded plugins                                                */
        static int                           curr_plugin;        /**< index to pluginSwitchers which points to the current plugin   */

        MMSOSDPluginHandler                  *osdhandler;
        MMSCentralPluginHandler              *centralhandler;
        MMSSwitcherThread                    *showPreviewThread; /**< a separate thread for each plugin                             */
        static MMSSwitcherThread             *switcherThread;    /**< my update thread                                              */

        bool switchToPluginEx(int toplugin);

	protected:
        MMSPluginData                        *plugindata;        /**< for plugin owned switcher instances                           */

        MMSChildWindow                       *menuBar;           /**< shows the plugin menu                                         */
        MMSMenuWidget                        *menu;              /**< plugin menu                                                   */
        MMSChildWindow                       *menuBar_static;    /**< shows the static plugin menu                                  */
        MMSMenuWidget                        *menu_static;       /**< plugin static menu, switcher does not control it              */

        int  searchingForImage(string pluginpath, string imagename, string *path);

        virtual void setMenuItemValues(MMSWidget *item);
        virtual void onBeforeScroll(MMSWidget *widget);
        virtual void onSelectItem(MMSWidget *widget);
        virtual void onReturn(MMSWidget *widget);
        virtual bool onBeforeShowPreview(MMSWindow *win);

	public:
        /* initialization */
		MMSSwitcher(MMSPluginData *plugindata = NULL);
		~MMSSwitcher();
		void setWindowManager(IMMSWindowManager *wm);
		void setPluginManager(MMSPluginManager *pm);
		void setInputManager(MMSInputManager  *im);
		void onSubscription(MMSInputSubscription *subscription);
		void subscribeKey(MMSKeySymbol key);

        /* methods for IMMSSwitcher */
        virtual void show();
        virtual void hide();
        virtual MMSChildWindow* loadPreviewDialog(string filename, MMSTheme *theme = NULL, int id=-1);
        virtual MMSChildWindow* loadInfoBarDialog(string filename, MMSTheme *theme = NULL);
        virtual void setVolume(unsigned int volume, bool init = false);
        virtual IMMSSwitcher *newSwitcher(MMSPluginData *plugindata);
        virtual bool switchToPlugin();
        virtual bool revertToLastPlugin();
        virtual bool leavePlugin(bool show_switcher);
        virtual void* callback(void *data);
		virtual MMSChildWindow* loadChildWindow(string filename, MMSTheme *theme = NULL);
		virtual void refresh();
		virtual MMSWidget *getMyButton();
    friend class MMSSwitcherThread;
};

#endif /*MMSSWITCHER_H_*/
