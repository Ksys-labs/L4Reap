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

#include "mmsgui/theme/mmsthememanager.h"
#include "mmsinfo/mmsinfo.h"
#include "mmsconfig/mmsconfigdata.h"

#define INITCHECK if (!this->initialized) throw MMSThemeManagerError(1, "MMSThemeManager is not initialized!");

// static variables
bool								MMSThemeManager::initialized = false;
string								MMSThemeManager::themepath;
vector<MMSTheme*>					MMSThemeManager::localThemes;
sigc::signal<void, string, bool>	MMSThemeManager::onThemeChanged;

MMSThemeManager::MMSThemeManager(string themepath, string globalThemeName) {
	if (!this->initialized) {
		// init the first time, save the path to the global theme and load it
		this->themepath = themepath;
		loadGlobalTheme(globalThemeName);
		this->initialized = true;
	}
}

MMSThemeManager::MMSThemeManager() {
    // check if initialized
	INITCHECK;
}

MMSThemeManager::~MMSThemeManager() {
}

void MMSThemeManager::loadTheme(string path, string themeName, MMSTheme *theme) {
    if (themeName == "")
        return;

    if(path == "")
    	path = this->themepath;

    theme->setTheme(path, themeName);

    /* get files */
    string themefile 	 = theme->getThemeFile();
    string themetafffile = themefile + ".taff";

    //check for file
    if(!file_exist(themefile))
        if(!file_exist(themetafffile))
        	throw MMSThemeManagerError(1, "theme file (" + themefile + ") not found");

    /* open the taff file */
	MMSTaffFile *tafff = new MMSTaffFile(themetafffile, &mmsgui_taff_description,
										 themefile, MMSTAFF_EXTERNAL_TYPE_XML);

	if (!tafff)
        throw MMSThemeManagerError(1, "could not load theme file " + themefile);

	if (!tafff->isLoaded()) {
		delete tafff;
        throw MMSThemeManagerError(1, "could not load theme file " + themefile);
	}

	/* get root tag */
	int tagid = tafff->getFirstTag();
	if (tagid < 0) {
		delete tafff;
        throw MMSThemeManagerError(1, "invalid taff file " + themetafffile);
	}

	/* through the file */
    this->throughFile(tafff, theme);

    /* free the document */
	delete tafff;
}

void MMSThemeManager::loadGlobalTheme(string themeName) {
#ifndef __L4_RE__
    try {
        // load global default theme delivered with the disko framework
    	loadTheme((string)getPrefix() + "/share/disko", DEFAULT_THEME, globalTheme);

    	if (themeName != DEFAULT_THEME) {
            // overload with special theme delivered with the disko framework
            loadTheme((string)getPrefix() + "/share/disko", themeName, globalTheme);
        }
    } catch(MMSError &error) {}

    // load global default theme
    loadTheme("", DEFAULT_THEME, globalTheme);

    if (themeName != DEFAULT_THEME) {
        // overload global default theme with global theme
        loadTheme("", themeName, globalTheme);
    }
#else
    // load global default theme
    loadTheme("", DEFAULT_THEME, globalTheme);
#endif
}


void MMSThemeManager::loadLocalTheme(MMSTheme *theme, string path, string themeName) {

	// check if initialized
	INITCHECK;

    if (themeName == "") {
        // use the name from already loaded global theme
        themeName = globalTheme->getThemeName();
    }

    try {
        // load global default theme delivered with the disko framework
    	loadTheme((string)getPrefix() + "/share/disko", DEFAULT_THEME, theme);

    	if (themeName != DEFAULT_THEME) {
            // overload with special theme delivered with the disko framework
            loadTheme((string)getPrefix() + "/share/disko", themeName, theme);
        }
    } catch(MMSError &error) {}

    // load global default theme
    loadTheme("", DEFAULT_THEME, theme);

    if (themeName != DEFAULT_THEME) {
        // overload global default theme with global theme
        loadTheme("", themeName, theme);
    }

    // overload global theme with local default theme
    loadTheme(path, DEFAULT_THEME, theme);

    if (themeName != DEFAULT_THEME) {
        // overload global theme with local theme
        loadTheme(path, themeName, theme);
    }
}


MMSTheme *MMSThemeManager::loadLocalTheme(string path, string themeName) {

	// check if initialized
	INITCHECK;

	// check if theme is already loaded
	for(vector<MMSTheme*>::const_iterator i = this->localThemes.begin(); i != this->localThemes.end(); ++i) {
        if(((*i)->getPath() == path) && ((*i)->getThemeName() == themeName)) {
            // already loaded
        	return *i;
        }
    }

	// load new theme
	MMSConfigData config;
	MMSTheme *theme = new MMSTheme(config.getInitialLoad(), config.getDebugFrames());
	loadLocalTheme(theme, path, themeName);

	// add theme to list
	this->localThemes.push_back(theme);

    return theme;
}


void MMSThemeManager::deleteLocalTheme(string path, string themeName) {
	for(vector<MMSTheme*>::iterator i = this->localThemes.begin(); i != this->localThemes.end(); ++i) {
        if(((*i)->getPath() == path) && ((*i)->getThemeName() == themeName)) {
            delete *i;
            this->localThemes.erase(i);
            break;
        }
    }
}

void MMSThemeManager::deleteLocalTheme(MMSTheme **theme) {
	for(vector<MMSTheme*>::iterator i = this->localThemes.begin(); i != this->localThemes.end(); ++i) {
        if (*i == *theme) {
            delete *theme;
            this->localThemes.erase(i);
            *theme = NULL;
            break;
        }
    }
}


void MMSThemeManager::setTheme(string themeName) {
    // check if initialized
	INITCHECK;

	// check if requested theme is equal to current theme
	if (themeName == globalTheme->getThemeName())
		return;

	// reset the global theme_tag (<mmstheme/> attributes)
	globalTheme->theme_tag.unsetAll();

	// load new global theme
	loadGlobalTheme(themeName);

	// reload the local themes
	for (vector<MMSTheme*>::iterator i = this->localThemes.begin(); i != this->localThemes.end(); ++i) {
		loadLocalTheme((*i), (*i)->getPath());
    }

	// have to fade?
	bool fadein = false;
	if (globalTheme->theme_tag.isFadeIn())
		fadein = globalTheme->theme_tag.getFadeIn();

	// inform attached callbacks
	this->onThemeChanged.emit(themeName, fadein);
}


void MMSThemeManager::setTheme(string themeName, bool fadein) {
    // check if initialized
	INITCHECK;

	// check if requested theme is equal to current theme
	if (themeName == globalTheme->getThemeName())
		return;

	// reset the global theme_tag (<mmstheme/> attributes)
	globalTheme->theme_tag.unsetAll();

	// load new theme
	loadGlobalTheme(themeName);

	// reload the local themes
	for (vector<MMSTheme*>::iterator i = this->localThemes.begin(); i != this->localThemes.end(); ++i) {
		loadLocalTheme((*i), (*i)->getPath());
    }

	// inform attached callbacks
	this->onThemeChanged.emit(themeName, fadein);
}


#define GET_THEME_CLASS(method) \
class_name = tafff->getAttributeString(MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name); \
if (class_name)	method(tafff, theme, class_name); \
else DEBUGMSG("MMSGUI", "name of class not specified (ignoring)");


void MMSThemeManager::throughFile(MMSTaffFile *tafff, MMSTheme *theme) {

	/* check if the correct tag */
	int currTag = tafff->getCurrentTag();
	if (currTag == MMSGUI_TAGTABLE_TAG_MMSTHEME)
        getThemeValues(tafff, theme);
    else {
    	DEBUGMSG("MMSGUI", "no valid theme file: %s", theme->getThemeFile().c_str());
        return;
    }

    /* iterate through childs */
    // bool returntag = true;
    int depth = 0;
	while (1) {
		char *class_name;
		bool eof;
		int tid = tafff->getNextTag(eof);
		if (eof) break;
		if (tid < 0) {
			if (depth==0) break;
			depth--;
			continue;
		}
		else {
			depth++;
			if (depth>1) continue;
		}

		switch (tid) {
		case MMSGUI_TAGTABLE_TAG_DESCRIPTION:
			getDescriptionValues(tafff, theme);
			break;
		case MMSGUI_TAGTABLE_TAG_MAINWINDOW:
		    getMainWindowValues(tafff, &(theme->mainWindowClass), theme);
		    break;
		case MMSGUI_TAGTABLE_TAG_CHILDWINDOW:
			getChildWindowValues(tafff, &(theme->childWindowClass), theme);
			break;
		case MMSGUI_TAGTABLE_TAG_POPUPWINDOW:
		    getPopupWindowValues(tafff, &(theme->popupWindowClass), theme);
		    break;
		case MMSGUI_TAGTABLE_TAG_ROOTWINDOW:
	        getRootWindowValues(tafff, &(theme->rootWindowClass), theme);
	        break;
		case MMSGUI_TAGTABLE_TAG_LABELWIDGET:
            getLabelWidgetValues(tafff, &(theme->labelWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_IMAGEWIDGET:
			getImageWidgetValues(tafff, &(theme->imageWidgetClass), theme);
			break;
		case MMSGUI_TAGTABLE_TAG_BUTTONWIDGET:
			getButtonWidgetValues(tafff, &(theme->buttonWidgetClass), theme);
			break;
		case MMSGUI_TAGTABLE_TAG_PROGRESSBARWIDGET:
            getProgressBarWidgetValues(tafff, &(theme->progressBarWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_SLIDERWIDGET:
            getSliderWidgetValues(tafff, &(theme->sliderWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_MENUWIDGET:
            getMenuWidgetValues(tafff, &(theme->menuWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_TEXTBOXWIDGET:
            getTextBoxWidgetValues(tafff, &(theme->textBoxWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_ARROWWIDGET:
            getArrowWidgetValues(tafff, &(theme->arrowWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_INPUTWIDGET:
            getInputWidgetValues(tafff, &(theme->inputWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_CHECKBOXWIDGET:
            getCheckBoxWidgetValues(tafff, &(theme->checkBoxWidgetClass), theme);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_TEMPLATE:
			GET_THEME_CLASS(getTemplateClassValues);
		    break;
		case MMSGUI_TAGTABLE_TAG_CLASS_MAINWINDOW:
			GET_THEME_CLASS(getMainWindowClassValues);
		    break;
		case MMSGUI_TAGTABLE_TAG_CLASS_CHILDWINDOW:
			GET_THEME_CLASS(getChildWindowClassValues);
			break;
		case MMSGUI_TAGTABLE_TAG_CLASS_POPUPWINDOW:
			GET_THEME_CLASS(getPopupWindowClassValues);
		    break;
		case MMSGUI_TAGTABLE_TAG_CLASS_ROOTWINDOW:
			GET_THEME_CLASS(getRootWindowClassValues);
	        break;
		case MMSGUI_TAGTABLE_TAG_CLASS_LABELWIDGET:
			GET_THEME_CLASS(getLabelWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_IMAGEWIDGET:
			GET_THEME_CLASS(getImageWidgetClassValues);
			break;
		case MMSGUI_TAGTABLE_TAG_CLASS_BUTTONWIDGET:
			GET_THEME_CLASS(getButtonWidgetClassValues);
			break;
		case MMSGUI_TAGTABLE_TAG_CLASS_PROGRESSBARWIDGET:
			GET_THEME_CLASS(getProgressBarWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_SLIDERWIDGET:
			GET_THEME_CLASS(getSliderWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_MENUWIDGET:
			GET_THEME_CLASS(getMenuWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_TEXTBOXWIDGET:
			GET_THEME_CLASS(getTextBoxWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_ARROWWIDGET:
			GET_THEME_CLASS(getArrowWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_INPUTWIDGET:
			GET_THEME_CLASS(getInputWidgetClassValues);
            break;
		case MMSGUI_TAGTABLE_TAG_CLASS_CHECKBOXWIDGET:
			GET_THEME_CLASS(getCheckBoxWidgetClassValues);
            break;
		}
	}
}

void MMSThemeManager::getThemeValues(MMSTaffFile *tafff, MMSTheme *theme) {

	theme->theme_tag.setAttributesFromTAFF(tafff);

	if (theme->theme_tag.getName() != "") {
		if (theme->themeName != theme->theme_tag.getName()) {
			printf("Warning: Inconsistent Theme File '%s'\n>Theme name is set to '%s', but <mmstheme name=\"%s\"/> is specified!\n",
					theme->themeFile.c_str(),
					theme->themeName.c_str(),
					theme->theme_tag.getName().c_str());
		}
	}
}



void MMSThemeManager::getDescriptionValues(MMSTaffFile *tafff, MMSTheme *theme) {

    theme->description.setAttributesFromTAFF(tafff);
}


void  MMSThemeManager::getTemplateValues(MMSTaffFile *tafff, MMSTemplateClass *themeClass) {

    themeClass->setAttributesFromTAFF(tafff);

    themeClass->duplicateTAFF(tafff);
}

void MMSThemeManager::getMainWindowValues(MMSTaffFile *tafff, MMSMainWindowClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->windowClass.setAttributesFromTAFF(tafff, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, &themePath, true);
}

void MMSThemeManager::getPopupWindowValues(MMSTaffFile *tafff, MMSPopupWindowClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->windowClass.setAttributesFromTAFF(tafff, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, &themePath, true);
}

void MMSThemeManager::getRootWindowValues(MMSTaffFile *tafff, MMSRootWindowClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->windowClass.setAttributesFromTAFF(tafff, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, &themePath, true);
}


void MMSThemeManager::getChildWindowValues(MMSTaffFile *tafff, MMSChildWindowClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->windowClass.setAttributesFromTAFF(tafff, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, &themePath, true);
}


void MMSThemeManager::getLabelWidgetValues(MMSTaffFile *tafff, MMSLabelWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void  MMSThemeManager::getImageWidgetValues(MMSTaffFile *tafff, MMSImageWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}


void  MMSThemeManager::getButtonWidgetValues(MMSTaffFile *tafff, MMSButtonWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void  MMSThemeManager::getProgressBarWidgetValues(MMSTaffFile *tafff, MMSProgressBarWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void  MMSThemeManager::getSliderWidgetValues(MMSTaffFile *tafff, MMSSliderWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void  MMSThemeManager::getMenuWidgetValues(MMSTaffFile *tafff, MMSMenuWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->duplicateTAFF(tafff);
}

void  MMSThemeManager::getTextBoxWidgetValues(MMSTaffFile *tafff, MMSTextBoxWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void  MMSThemeManager::getArrowWidgetValues(MMSTaffFile *tafff, MMSArrowWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void MMSThemeManager::getInputWidgetValues(MMSTaffFile *tafff, MMSInputWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}

void MMSThemeManager::getCheckBoxWidgetValues(MMSTaffFile *tafff, MMSCheckBoxWidgetClass *themeClass, MMSTheme *theme) {

    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();

    themeClass->widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath, true);

    themeClass->setAttributesFromTAFF(tafff, NULL, &themePath, true);
}


void MMSThemeManager::getTemplateClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSTemplateClass *themeClass = theme->getTemplateClass(className);

    if (!themeClass) {
        themeClass = new MMSTemplateClass;
        getTemplateValues(tafff, themeClass);
        themeClass->setClassName(className);
        if (!theme->addTemplateClass(themeClass))
            delete themeClass;
    }
    else {
        getTemplateValues(tafff, themeClass);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getMainWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSMainWindowClass *themeClass = theme->getMainWindowClass(className);

    if (!themeClass) {
        themeClass = new MMSMainWindowClass;
        getMainWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addMainWindowClass(themeClass))
            delete themeClass;
    }
    else {
        getMainWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getPopupWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSPopupWindowClass *themeClass = theme->getPopupWindowClass(className);

    if (!themeClass) {
        themeClass = new MMSPopupWindowClass;
        getPopupWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addPopupWindowClass(themeClass))
            delete themeClass;
    }
    else {
        getPopupWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getRootWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSRootWindowClass *themeClass = theme->getRootWindowClass(className);

    if (!themeClass) {
        themeClass = new MMSRootWindowClass;
        getRootWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addRootWindowClass(themeClass))
            delete themeClass;
    }
    else {
        getRootWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getChildWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSChildWindowClass *themeClass = theme->getChildWindowClass(className);

    if (!themeClass) {
        themeClass = new MMSChildWindowClass;
        getChildWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addChildWindowClass(themeClass))
            delete themeClass;
    }
    else {
        getChildWindowValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getLabelWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSLabelWidgetClass *themeClass = theme->getLabelWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSLabelWidgetClass;
        getLabelWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addLabelWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getLabelWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getImageWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSImageWidgetClass *themeClass = theme->getImageWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSImageWidgetClass;
        getImageWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addImageWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getImageWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getButtonWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSButtonWidgetClass *themeClass = theme->getButtonWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSButtonWidgetClass;
        getButtonWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addButtonWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getButtonWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getProgressBarWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSProgressBarWidgetClass *themeClass = theme->getProgressBarWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSProgressBarWidgetClass;
        getProgressBarWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addProgressBarWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getProgressBarWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getSliderWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSSliderWidgetClass *themeClass = theme->getSliderWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSSliderWidgetClass;
        getSliderWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addSliderWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getSliderWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getMenuWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSMenuWidgetClass *themeClass = theme->getMenuWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSMenuWidgetClass;
        getMenuWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addMenuWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getMenuWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getTextBoxWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSTextBoxWidgetClass *themeClass = theme->getTextBoxWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSTextBoxWidgetClass;
        getTextBoxWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addTextBoxWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getTextBoxWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getArrowWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSArrowWidgetClass *themeClass = theme->getArrowWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSArrowWidgetClass;
        getArrowWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addArrowWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getArrowWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getInputWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSInputWidgetClass *themeClass = theme->getInputWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSInputWidgetClass;
        getInputWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addInputWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getInputWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}

void MMSThemeManager::getCheckBoxWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className) {
    MMSCheckBoxWidgetClass *themeClass = theme->getCheckBoxWidgetClass(className);

    if (!themeClass) {
        themeClass = new MMSCheckBoxWidgetClass;
        getCheckBoxWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
        if (!theme->addCheckBoxWidgetClass(themeClass))
            delete themeClass;
    }
    else {
        getCheckBoxWidgetValues(tafff, themeClass, theme);
        themeClass->setClassName(className);
    }
}





