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

IMMSWindowManager 				*MMSSwitcher::windowmanager;
MMSPluginManager  				*MMSSwitcher::pluginmanager;
MMSInputManager   				*MMSSwitcher::inputmanager;
vector<MMSInputSubscription*> 	MMSSwitcher::subscriptions;

MMSDialogManager          MMSSwitcher::dm;            /**< dialog manager for whole switcher window                      */
MMSMainWindow*            MMSSwitcher::window;        /**< whole switcher window                                         */
map<int, plugin_data_t *> MMSSwitcher::plugins;       /**< loaded plugins                                                */
int                       MMSSwitcher::curr_plugin;   /**< index to pluginSwitchers which points to the current plugin   */

MMSSwitcherThread       *MMSSwitcher::switcherThread;//my update thread

#define SWITCHER_MENUBAR			"switcher_menubar"
#define SWITCHER_MENU				"switcher_menu"
#define	SWITCHER_MENU_PLUGINNAME	"switcher_menu_pluginname"
#define	SWITCHER_MENU_PLUGINTITLE	"switcher_menu_plugintitle"
#define	SWITCHER_MENU_PLUGINICON	"switcher_menu_pluginicon"

#define SWITCHER_MENUBAR_STATIC		"switcher_menubar_static"
#define SWITCHER_MENU_STATIC		"switcher_menu_static"

/* public */
MMSSwitcher::MMSSwitcher(MMSPluginData *plugindata) :
    osdhandler(NULL),
    centralhandler(NULL) {
    /* switcher instantiated by plugin */
    if(plugindata) {
        /* get access to the plugin handler */
    	this->plugindata = plugindata;
        if (this->plugindata->getType()->getName()=="OSD_PLUGIN") {
            this->osdhandler = this->pluginmanager->getOSDPluginHandler(this->plugindata->getId());
            this->showPreviewThread = new MMSSwitcherThread(this);
        }
        else if (this->plugindata->getType()->getName()=="CENTRAL_PLUGIN") {
            this->centralhandler = this->pluginmanager->getCentralPluginHandler(this->plugindata->getId());
            this->showPreviewThread = new MMSSwitcherThread(this);
        }

        plugin_data_t *pd = new plugin_data_t;
        pd->plugindata = *plugindata;
        pd->switcher = this;
        plugins.insert(std::make_pair(plugindata->getId(), pd));

        return;
    }

    /* switcher start */
	DEBUGMSG("MMSSwitcher", "startup");

    this->windowmanager = NULL;
    this->pluginmanager = NULL;
    this->inputmanager  = NULL;
    this->curr_plugin = -1;
    this->window      = NULL;

    try {
        /* get the active osd and central plugins */
        DataSource source = DataSource(config.getConfigDBDBMS(),
                                       config.getConfigDBDatabase(),
                                       config.getConfigDBAddress(),
                                       config.getConfigDBPort(),
                                       config.getConfigDBUser(),
                                       config.getConfigDBPassword());

        /* load switcher dialog */
        this->window = (MMSMainWindow*)dm.loadDialog(config.getData() + "/themes/" + config.getTheme() + "/switcher.xml");
        if(!this->window) throw MMSError(0, "Error loading switchers root window");

        /* get access to the menu bar */
        this->menuBar = (MMSChildWindow *)this->window->findWindow(SWITCHER_MENUBAR);
        if(!this->menuBar) throw MMSError(0, "Error loading switchers menuBar childwindow");
        this->menu    = dynamic_cast<MMSMenuWidget*>(this->menuBar->findWidget(SWITCHER_MENU));
        if(!this->menu) throw MMSError(0, "Error loading switchers menu");

        /* get access to the static menu bar */
        this->menuBar_static = (MMSChildWindow *)this->window->findWindow(SWITCHER_MENUBAR_STATIC);
        if (this->menuBar_static)
        	this->menu_static = dynamic_cast<MMSMenuWidget*>(this->menuBar_static->findWidget(SWITCHER_MENU_STATIC));
        else
        	this->menu_static = NULL;

        /* fill the menu */
        MMSPluginService service(&source);
        addPluginsToMenu(service.getOSDPlugins());
        addPluginsToMenu(service.getCentralPlugins());

        /* show the menu bar */
        if (this->menuBar_static) {
        	this->menuBar_static->show();
        	this->menuBar_static->waitUntilShown();
        }
        this->menuBar->show();

        /* connect onBeforeScroll callback of the menu widget */
        menu->onBeforeScroll->connect(sigc::mem_fun(this,&MMSSwitcher::onBeforeScroll));

        /* connect onSelectItem callback of the menu widget */
        menu->onSelectItem->connect(sigc::mem_fun(this,&MMSSwitcher::onSelectItem));

        /* connect onReturn callback of the menu widget */
        menu->onReturn->connect(sigc::mem_fun(this,&MMSSwitcher::onReturn));

    	/* create inputs */
//        subscribeKey(MMSKEY_MENU);
//        subscribeKey(MMSKEY_BACKSPACE);

        /* start my update thread */
        this->switcherThread = new MMSSwitcherThread(this, NULL, NULL, NULL, NULL);
        this->switcherThread->start();

    } catch(MMSError &error) {
        DEBUGMSG("Switcher", "Abort due to: " + error.getMessage());
        throw error;
    }
}

MMSSwitcher::~MMSSwitcher() {
	DEBUGMSG("Switcher", "deletion");
    this->plugins.clear();
	this->subscriptions.clear();
}

const void MMSSwitcher::addPluginsToMenu(const vector<MMSPluginData *> &plugins) {
	vector<MMSPluginData*>::const_iterator i;
	vector<MMSPluginData*>::const_iterator end = plugins.end();
	for(i = plugins.begin(); i != end; ++i) {
    	// new item
        MMSWidget *pluginItem = this->menu->newItem();
        if (!pluginItem) break;

        // set plugin data to the item
        DEBUGMSG("MMSSwitcher", (*i)->getName().c_str());
        pluginItem->setBinData((void*)(*i));

        // set values if widgets are defined
        setMenuItemValues(pluginItem);

    	// new static item
        if (this->menu_static) {
            pluginItem = this->menu_static->newItem();
            if (pluginItem) {
		        // set plugin data to the item
		        pluginItem->setBinData((void*)(*i));

		        // set values if widgets are defined
		        setMenuItemValues(pluginItem);
            }
        }
    }
}

void MMSSwitcher::setMenuItemValues(MMSWidget *item) {
	// get the plugin data
	if (!item) return;
	MMSPluginData *plugindata = (MMSPluginData *)item->getBinData();

	// set the plugin name
	MMSLabelWidget *pluginName = dynamic_cast<MMSLabelWidget*>(item->findWidget(SWITCHER_MENU_PLUGINNAME));
    if (pluginName) pluginName->setText(plugindata->getName());

    // set the plugin title
    MMSLabelWidget *pluginTitle = dynamic_cast<MMSLabelWidget*>(item->findWidget(SWITCHER_MENU_PLUGINTITLE));
    if (pluginTitle) pluginTitle->setText(plugindata->getTitle());

    // set the plugin icon
    MMSImageWidget *pluginIcon = dynamic_cast<MMSImageWidget*>(item->findWidget(SWITCHER_MENU_PLUGINICON));
    if (pluginIcon) {
        string path;
        string name;

        name = plugindata->getIcon();
		if (!searchingForImage(plugindata->getPath(), name, &path))
            if (!path.empty())
            	pluginIcon->setImage(path, name);
            else
				pluginIcon->setImageName(name);
		else
            pluginIcon->setImageName("plugin_icon.png");

        name = plugindata->getSelectedIcon();
		if (!searchingForImage(plugindata->getPath(), name, &path))
            if (!path.empty())
            	pluginIcon->setSelImage(path, name);
            else
				pluginIcon->setSelImageName(name);
		else
            pluginIcon->setSelImageName("plugin_icon_s.png");
    }
}

int MMSSwitcher::searchingForImage(string pluginpath, string imagename, string *path) {

    // searching for image
    MMSFile *myfile;
    int err;

    if (imagename.empty()) {
        *path = "";
        return 1;
    }

    // first: current plugin theme, try with ".taff" extension
    *path = pluginpath + "/themes/" + config.getTheme() + "/";
    myfile = new MMSFile(*path + imagename + ".taff");
    err = myfile->getLastError();
    delete myfile;
    if (err) {
        // try without ".taff" extension
        myfile = new MMSFile(*path + imagename);
        err = myfile->getLastError();
        delete myfile;
    }

    if (err) {
        // second: plugin default theme, try with ".taff" extension
        if (config.getTheme() != DEFAULT_THEME) {
            *path = pluginpath + "/themes/" + DEFAULT_THEME + "/";
            myfile = new MMSFile(*path + imagename + ".taff");
            err = myfile->getLastError();
            delete myfile;
            if (err) {
                // try without ".taff" extension
                myfile = new MMSFile(*path + imagename);
                err = myfile->getLastError();
                delete myfile;
            }
        }
    }
    if (err) {
        // third: current theme, try with ".taff" extension
        *path = "./themes/" + config.getTheme() + "/";
        myfile = new MMSFile(*path + imagename + ".taff");
        err = myfile->getLastError();
        delete myfile;
        if (err) {
        	// try without ".taff" extension
            myfile = new MMSFile(*path + imagename);
            err = myfile->getLastError();
            delete myfile;
        }
        if (!err) *path = "";
    }
    if (err) {
        // fourth: default theme, try with ".taff" extension
        if (config.getTheme() != DEFAULT_THEME) {
            *path = "./themes/";
            *path = *path + DEFAULT_THEME + "/";
            myfile = new MMSFile(*path + imagename + ".taff");
            err = myfile->getLastError();
            delete myfile;
            if (err) {
            	// try without ".taff" extension
                myfile = new MMSFile(*path + imagename);
                err = myfile->getLastError();
                delete myfile;
            }
            if (!err) *path = "";
        }
    }

    return err;
}



void MMSSwitcher::setWindowManager(IMMSWindowManager *wm) {
	this->windowmanager = wm;
}

void MMSSwitcher::setPluginManager(MMSPluginManager *pm){
	this->pluginmanager = pm;
}

void MMSSwitcher::setInputManager(MMSInputManager  *im) {
	this->inputmanager = im;
}

void MMSSwitcher::onSubscription(MMSInputSubscription *subscription){
    MMSKeySymbol key;

    subscription->getKey(key);

    if (key == MMSKEY_BACKSPACE) {
        /* hide all shown popups */
        /* hide all main windows only if no popups to be hide */
        if(this->windowmanager->hideAllPopupWindows()==false)
            if(this->windowmanager->hideAllMainWindows(true)==false) {
                /* hide all windows different from default root */
                MMSWindow *w = this->windowmanager->getBackgroundWindow();
                if(w && !(w->isShown()))
                    this->windowmanager->hideAllRootWindows();
            }
    }
    else {
    	if(this->window->isShown()) {
            /* hide all main and popup windows */
            this->windowmanager->hideAllPopupWindows();
            this->windowmanager->hideAllMainWindows();
        }
        else {
			DEBUGMSG("Switcher", "try to show");
            /* show the switcher */
    		show();
        }
    }
}

void MMSSwitcher::subscribeKey(MMSKeySymbol key){
	MMSInputSubscription *subscription = new MMSInputSubscription(key);

	this->subscriptions.push_back(subscription);
	subscription->callback.connect(sigc::mem_fun(this, &MMSSwitcher::onSubscription));
	subscription->registerMe();
}


void MMSSwitcher::onBeforeScroll(MMSWidget *widget) {
    // no plugin set
    this->curr_plugin = -1;

    // tell the switcher thread to invoke show preview
    this->switcherThread->invokeShowPreview();

    // hide all previews
    for (map<int, plugin_data_t *>::iterator i = this->plugins.begin(); i != this->plugins.end(); i++) {
    	vector<MMSChildWindow *> *wins = &(i->second->previewWins);
    	for (unsigned int j = 0; j < wins->size(); j++) {
    		MMSChildWindow *cw = wins->at(j);
    		cw->hide();
            cw->waitUntilHidden();
    	}
    }
}

void MMSSwitcher::onSelectItem(MMSWidget *widget) {
	if (!widget)
        widget = this->menu->getSelectedItem();

    // no menu item given
    if (!widget)
        return;

    // set the static menu
    if (this->menu_static)
    	this->menu_static->setSelected(menu->getSelected());

    MMSPluginData *data = (MMSPluginData*)widget->getBinData();

    // return if current plugin is selected plugin
    if(this->curr_plugin == data->getId())
        return;


    // hide all previews
    /*for (map<int, plugin_data_t *>::iterator i = this->plugins.begin(); i != this->plugins.end(); i++) {
    	vector<MMSChildWindow *> *wins = &(i->second->previewWins);
    	for (unsigned int j = 0; j < wins->size(); j++) {
    		MMSChildWindow *cw = wins->at(j);
    		cw->hide();
            cw->waitUntilHidden();
    	}
    }
    */

    // set current plugin
    this->curr_plugin = data->getId();

    // tell the switcher thread to invoke show preview
    this->switcherThread->invokeShowPreview();
}

void MMSSwitcher::onReturn(MMSWidget *widget) {
    try {
        // get the selected item
        widget = this->menu->getSelectedItem();

        // get access to the plugin data
        MMSPluginData *data = (MMSPluginData*)widget->getBinData();

        // invoke show
        if(data->getType()->getName() == "OSD_PLUGIN") {
            MMSOSDPluginHandler *handler = this->pluginmanager->getOSDPluginHandler(data->getId());
            handler->invokeShow(NULL);
        }
        else if(data->getType()->getName() == "CENTRAL_PLUGIN") {
            MMSCentralPluginHandler *handler = this->pluginmanager->getCentralPluginHandler(data->getId());
            handler->invokeShow(NULL);
        }

    } catch(MMSError &error) {
        DEBUGMSG("Switcher", "Abort due to: " + error.getMessage());
    }
}

void MMSSwitcher::show() {
	MMSConfigData config;
	int firstplugin = atoi(config.getFirstPlugin().c_str());
	if(firstplugin > 0) {
		this->switchToPluginEx(firstplugin);
	} else {
		this->window->show();
	}
}

void MMSSwitcher::hide() {
    this->window->hide();
}

MMSChildWindow* MMSSwitcher::loadPreviewDialog(string filename, MMSTheme *theme, int id) {
    MMSChildWindow *win;

	win = this->dm.loadChildDialog(filename, theme);

    if(win) {
    	// save the window pointer
        map<int, plugin_data_t *>::iterator i = this->plugins.find(id);
        if(i != this->plugins.end())
        	i->second->previewWins.push_back(win);

        // connect the callbacks, so I can handle show/hide of these windows
        win->onBeforeShow->connect(sigc::mem_fun(this,&MMSSwitcher::onBeforeShowPreview));
    }

    return win;
}

bool MMSSwitcher::onBeforeShowPreview(MMSWindow *win) {
	// plugin selected?
	if (this->curr_plugin < 0)
		return false;

	// searching the plugin
    map<int, plugin_data_t *>::iterator i = this->plugins.find(this->curr_plugin);
    if(i == this->plugins.end())
    	return false;
    vector<MMSChildWindow *> *wins = &(i->second->previewWins);

	// search for the window which will shown
    int pW = -1;
    for (unsigned int i = 0; i < wins->size(); i++) {
        if (win == wins->at(i)) {
            /* found */
            pW = i;
            break;
        }
    }

    if (pW < 0) {
        // window not found, stop the show process
        return false;
    }

    // hide all previews
    for (map<int, plugin_data_t *>::iterator i = this->plugins.begin(); i != this->plugins.end(); i++) {
    	vector<MMSChildWindow *> *wins = &(i->second->previewWins);
    	for (unsigned int j = 0; j < wins->size(); j++) {
    		MMSChildWindow *cw = wins->at(j);
    		if (cw!=win) {
	    		cw->hide();
	            cw->waitUntilHidden();
    		}
    	}
    }

    // set current preview window
    this->switcherThread->previewShown();

    return true;
}

MMSChildWindow* MMSSwitcher::loadInfoBarDialog(string filename, MMSTheme *theme) {
    return NULL;
}

void MMSSwitcher::setVolume(unsigned int volume, bool init) {
}

IMMSSwitcher *MMSSwitcher::newSwitcher(MMSPluginData *plugindata) {
	return new MMSSwitcher(plugindata);
}

bool MMSSwitcher::switchToPluginEx(int toplugin) {
    if (toplugin >= 0) {
    	try {
            MMSPluginData *data = &this->plugins[toplugin]->plugindata;
            if(!data) {
            	DEBUGMSG("Switcher", "Plugin with ID = %d not found", toplugin);
            	return false;
            }

            if(data->getType()->getName() == "OSD_PLUGIN") {
                MMSOSDPluginHandler *handler = this->pluginmanager->getOSDPluginHandler(data->getId());
                handler->invokeShow(NULL);
            }
            else if(data->getType()->getName() == "CENTRAL_PLUGIN") {
                MMSCentralPluginHandler *handler = this->pluginmanager->getCentralPluginHandler(data->getId());
                handler->invokeShow(NULL);
            }

            return true;
	    } catch(MMSError &error) {
	      	DEBUGMSG("Switcher", "Abort due to: " + error.getMessage());
	    }
    }

    return false;
}

bool MMSSwitcher::switchToPlugin() {
    return switchToPluginEx(this->plugindata->getId());
}

bool MMSSwitcher::leavePlugin(bool show_switcher) {
	bool b = false;

	if (!this->window->isShown())
		b = this->windowmanager->hideAllMainWindows(true);

	if (b==false) {
	    /* hide all windows different from default root */
	    MMSWindow *w = this->windowmanager->getBackgroundWindow();
	    if(w && !w->isShown())
	        this->windowmanager->hideAllRootWindows();
	}

	if (!this->window->isShown())
		if (show_switcher)
			show();

	return true;
}

/**
 * Generic callback for plugin->switcher communication.
 *
 * If you implement your own switcher, you can call this
 * method in your plugins to let the switcher handle
 * whatever you want.
 *
 * @param	data	application specific data
 *
 * @return	application specific data
 */
void* MMSSwitcher::callback(void *data) {
	return NULL;
}

bool MMSSwitcher::revertToLastPlugin() {
	return false;
}

MMSChildWindow* MMSSwitcher::loadChildWindow(string filename, MMSTheme *theme) {
	return this->dm.loadChildDialog(filename, theme);
}

void MMSSwitcher::refresh()
{
	if(window != NULL && window->isShown()) {
		window->refresh();
	}
}


MMSWidget *MMSSwitcher::getMyButton() {
	return NULL;
}
