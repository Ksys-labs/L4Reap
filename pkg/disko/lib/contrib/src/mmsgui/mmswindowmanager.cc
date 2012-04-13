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

#include "mmsgui/mmswindowmanager.h"
#include "mmsgui/fb/mmsfbwindowmanager.h"


MMSWindowManager::MMSWindowManager(MMSFBRectangle vrect) {
	// init me
    this->vrect = vrect;
    this->toplevel = NULL;
    this->anim_saved_screen = NULL;

    // add language changed callback
    this->onTargetLangChanged_connection = this->translator.onTargetLangChanged.connect(sigc::mem_fun(this, &MMSWindowManager::onTargetLangChanged));

    // add theme changed callback
    this->onThemeChanged_connection = this->themeManager.onThemeChanged.connect(sigc::mem_fun(this, &MMSWindowManager::onThemeChanged));

    // add animation callbacks
    this->onAnimation_connection = this->pulser.onAnimation.connect(sigc::mem_fun(this, &MMSWindowManager::onAnimation));
    this->onAfterAnimation_connection = this->pulser.onAfterAnimation.connect(sigc::mem_fun(this, &MMSWindowManager::onAfterAnimation));
}

MMSWindowManager::~MMSWindowManager() {
	// disconnect my callbacks
	this->onTargetLangChanged_connection.disconnect();
	this->onThemeChanged_connection.disconnect();
	this->onAnimation_connection.disconnect();
	this->onAfterAnimation_connection.disconnect();
}

void MMSWindowManager::reset() {
	mmsfbwindowmanager->reset();
}

MMSFBRectangle MMSWindowManager::getVRect() {
    return this->vrect;
}

void MMSWindowManager::addWindow(MMSWindow *window) {
    // add window to list
    this->windows.push_back(window);
}

void MMSWindowManager::removeWindow(MMSWindow *window){
    // search for the window and erase it
    for(unsigned int i = 0; i < windows.size(); i++) {
        if(window != windows.at(i))
            continue;
        windows.erase(windows.begin()+i);
        if (window == this->toplevel)
            this->toplevel = NULL;
        return;
    }
}

bool MMSWindowManager::lowerToBottom(MMSWindow *window) {
	return window->lowerToBottom();
}

bool MMSWindowManager::raiseToTop(MMSWindow *window) {
    // searching for popup windows and count it
	int cnt = 0;
    for(unsigned int i = 0; i < windows.size(); i++) {
        if (windows.at(i)->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
            if (windows.at(i)->isShown()) cnt++;
        }
    }

    // raise window to top, cnt number of windows are over it
    return window->raiseToTop(cnt);
}

bool MMSWindowManager::hideAllMainWindows(bool goback) {
    bool ret = false;

    // searching for main windows
    for(unsigned int i = 0; i < windows.size(); i++) {
        if (windows.at(i)->getType() == MMSWINDOWTYPE_MAINWINDOW) {
            if (windows.at(i)->isShown()) {
                if (this->toplevel == windows.at(i)) {
                    removeWindowFromToplevel(windows.at(i));
                    windows.at(i)->hide(goback, true);
                }
                else
                    windows.at(i)->hide(false, true);
                ret = true;
            }
        }
    }

    // return true if at least one main window was found
    return ret;
}

bool MMSWindowManager::hideAllPopupWindows(bool except_modal) {
    bool ret = false;

    // searching for popup windows
    for(unsigned int i = 0; i < windows.size(); i++) {
        if (windows.at(i)->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
            if (windows.at(i)->isShown()) {
            	if (except_modal) {
					// hide only non-modal popups
					bool modal;
					if (windows.at(i)->getModal(modal)) {
						if (modal)
							continue;
					}
            	}
                windows.at(i)->hide(false, true);
                ret = true;
            }
        }
    }

    // return true if at least one popup window was found
    return ret;
}

bool MMSWindowManager::hideAllRootWindows(bool willshown) {
    bool ret = false;

    // search for root windows
    for(unsigned int i = 0; i < windows.size(); i++)
        if (windows.at(i)->getType() == MMSWINDOWTYPE_ROOTWINDOW)
            if (windows.at(i)->isShown()) {
                if (this->toplevel == windows.at(i)) {
                    removeWindowFromToplevel(windows.at(i));
                    windows.at(i)->hide(false, true);
                }
                else
                    windows.at(i)->hide(false, true);
                ret = true;
            }

    // if at least one root window was hidden and no other will shown, show the default root window
    if ((ret)&&(!willshown)) {
    	showBackgroundWindow();
    }

    // return true if at least one root window was found
    return ret;
}

bool MMSWindowManager::setToplevelWindow(MMSWindow *window) {
    if (window->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
        // set popup window as toplevel if it is in modal mode
		bool modal;
		if (window->getModal(modal)) {
			if (modal) {
            	this->toplevel = window;
                return true;
			}
		}

		// popup window is not set as toplevel
		return false;
    }

    if (window->getType() != MMSWINDOWTYPE_MAINWINDOW && window->getType() != MMSWINDOWTYPE_ROOTWINDOW) {
    	// this type cannot be set as toplevel
    	return false;
    }

    if (this->toplevel) {
		// check current toplevel window
		if (this->toplevel->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
			if (this->toplevel->isShown()&&(!this->toplevel->willHide())) {
				// current toplevel window is already shown and keep the toplevel status
				return false;
			}
		}
	}

    if (window->getType() == MMSWINDOWTYPE_MAINWINDOW) {
        // searching for active popup window
        for(unsigned int i = 0; i < windows.size(); i++) {
            if (windows.at(i)->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
                if (windows.at(i)->isShown()&&(!windows.at(i)->willHide())) {
                    // set active popup window as toplevel
					bool modal;
					if (windows.at(i)->getModal(modal)) {
						if (modal) {
		                	this->toplevel = windows.at(i);
		                    return false;
						}
					}
                }
            }
        }
    }

    if (window->getType() == MMSWINDOWTYPE_ROOTWINDOW) {
        // searching for active main window
        for(unsigned int i = 0; i < windows.size(); i++) {
            if (windows.at(i)->getType() == MMSWINDOWTYPE_MAINWINDOW) {
                if (windows.at(i)->isShown()&&(!windows.at(i)->willHide())) {
                    // set active main window as toplevel
                    this->toplevel = windows.at(i);
                    return false;
                }
            }
        }
    }

    // set new toplevel window
    this->toplevel = window;
    return true;
}

MMSWindow *MMSWindowManager::getToplevelWindow() {
    return this->toplevel;
}

void MMSWindowManager::removeWindowFromToplevel(MMSWindow *window) {
	// toplevel window?
    if (this->toplevel != window)
    	return;

	if (window->getType() == MMSWINDOWTYPE_POPUPWINDOW) {
		MMSWindow *firstMainWindow = NULL;
		// a popup window will be hidden, so try to find an active
		// popup or main window to get the toplevel status
		for(vector<MMSWindow*>::iterator i = this->windows.begin(); i != this->windows.end(); ++i) {
			bool focusable;
			MMSWINDOWTYPE type = (*i)->getType();
			if(
				((*i) != window) &&
				((*i)->isShown()) &&
				(*i)->getFocusable(focusable) &&
				focusable
			) {
				if(type == MMSWINDOWTYPE_POPUPWINDOW) {
					this->toplevel = *i;
					return;
				} else if(!firstMainWindow && (type == MMSWINDOWTYPE_MAINWINDOW)) {
					firstMainWindow = *i;
				}
			}
		}
		if(firstMainWindow) {
			this->toplevel = firstMainWindow;
			return;
		}
	}

	if ((window->getType() == MMSWINDOWTYPE_MAINWINDOW) || (window->getType() == MMSWINDOWTYPE_POPUPWINDOW)) {
		// a main or popup window will be hidden, so try to find an active
		// root window to get the toplevel status
		for(vector<MMSWindow*>::iterator i = this->windows.begin(); i != this->windows.end(); ++i) {
			if(
				((*i)->getType() == MMSWINDOWTYPE_ROOTWINDOW) &&
				((*i)->isShown())
			) {
					this->toplevel = *i;
					return;
			}
		}
	}

	// no window found, no toplevel :)
	this->toplevel = NULL;
}

void MMSWindowManager::setBackgroundWindow(MMSWindow *window) {
    if (window) {
        this->backgroundwindow = window;
    }
    showBackgroundWindow();
}

MMSWindow *MMSWindowManager::getBackgroundWindow() {
    return this->backgroundwindow;
}

void MMSWindowManager::showBackgroundWindow() {
	if (this->backgroundwindow) {
		unsigned int opacity;
		if (this->backgroundwindow->getOpacity(opacity))
			if (opacity) {
				this->backgroundwindow->show();
				this->backgroundwindow->waitUntilShown();
			}
	}
}

void MMSWindowManager::setPointerPosition(int pointer_posx, int pointer_posy, bool pressed) {
	mmsfbwindowmanager->setPointerPosition(pointer_posx, pointer_posy, pressed);
}

MMSTranslator *MMSWindowManager::getTranslator() {
	return &this->translator;
}

void MMSWindowManager::onTargetLangChanged(MMSLanguage lang) {
	// the language has changed, inform all windows
    for (unsigned int i = 0; i < this->windows.size(); i++) {
        this->windows.at(i)->targetLangChanged(lang);
    }
}

MMSThemeManager *MMSWindowManager::getThemeManager() {
	return &this->themeManager;
}

bool MMSWindowManager::onAnimation(MMSPulser *pulser) {
	// get new opacity
	int opacity = 255 - pulser->getOffset();
	// animation finished?
	if (opacity <= 0) {
		// yes
		return false;
	}

	// set new opacity
	this->anim_saved_screen->setOpacity(opacity);

	return true;
}

void MMSWindowManager::onAfterAnimation(MMSPulser *pulser) {
	// animation finished
	if (this->anim_saved_screen) {
		// delete the temporary window
		this->anim_saved_screen->hide();
		delete this->anim_saved_screen;
		this->anim_saved_screen = NULL;
	}
}

void MMSWindowManager::onThemeChanged(string themeName, bool fade_in) {
	// get access to the layer
	MMSFBLayer *layer = mmsfbmanager.getGraphicsLayer();
	this->anim_saved_screen = NULL;

	if (fade_in) {
		// create a temporary window to save the screen
		// so we can have a nice animation while switching the theme
		if (layer) {
			MMSFBSurfacePixelFormat pixelformat;
			layer->getPixelFormat(&pixelformat);
			int w, h;
			layer->getResolution(&w, &h);
			layer->createWindow(&this->anim_saved_screen, 0, 0, w, h,
								pixelformat, isAlphaPixelFormat(pixelformat), 0);
		}
	}

	if (this->anim_saved_screen) {
		// get a screenshot
		this->anim_saved_screen->getScreenshot();

		// show the saved screen
		this->anim_saved_screen->raiseToTop();
		this->anim_saved_screen->setOpacity(255);
		this->anim_saved_screen->show();
	}

	// the theme has changed, inform all windows
    for (unsigned int i = 0; i < this->windows.size(); i++) {
        this->windows.at(i)->themeChanged(themeName);
    }

    if (this->anim_saved_screen) {
    	// do the animation
    	this->pulser.setMaxOffset(255,MMSPULSER_SEQ_LINEAR);
    	this->pulser.setStepsPerSecond(255);
    	this->pulser.start(false);
    }
}

unsigned int MMSWindowManager::printStack(char *buffer) {
	if (!buffer) {
		char buffer[50*1024];
		memset((void*)buffer, ' ', sizeof(buffer));
		char *ptr = buffer;

		// print header
		int cnt;
		cnt = sprintf(ptr, "NAME                             THIS     STATE         OPACITY OWN_SURFACE\n");
		ptr[cnt] = ' ';
		ptr+= cnt;
		cnt = sprintf(ptr, "---------------------------------------------------------------------------\n");
		ptr[cnt] = ' ';
		ptr+= cnt;

		for (unsigned int i = 0; i < this->windows.size(); i++) {
			ptr += this->windows.at(i)->printStack(ptr);
		}

		*ptr = 0;
		printf(buffer);
		return (unsigned int)(ptr - buffer);
	}
	else {
		char *ptr = buffer;

		// print header
		int cnt;
		cnt = sprintf(ptr, "NAME                             THIS     STATE         OPACITY OWN_SURFACE\n");
		ptr[cnt] = ' ';
		ptr+= cnt;
		cnt = sprintf(ptr, "---------------------------------------------------------------------------\n");
		ptr[cnt] = ' ';
		ptr+= cnt;

		for (unsigned int i = 0; i < this->windows.size(); i++) {
			ptr += this->windows.at(i)->printStack(ptr);
		}

		*ptr = 0;
		return (unsigned int)(ptr - buffer);
	}
}

