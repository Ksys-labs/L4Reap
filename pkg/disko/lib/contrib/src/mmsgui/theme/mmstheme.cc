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

#include "mmsgui/theme/mmstheme.h"

// mmsInit() will initialize the theme object which stores the global theme
MMSTheme *globalTheme = NULL;


TAFF_ATTRDESC MMSGUI_MMSDIALOG_ATTR_I[]			= MMSGUI_MMSDIALOG_ATTR_INIT;
TAFF_ATTRDESC MMSGUI_BASE_ATTR_I[]				= MMSGUI_BASE_ATTR_INIT;
TAFF_ATTRDESC MMSGUI_NONE_ATTR_I[]              = {{ NULL, TAFF_ATTRTYPE_NONE }};


TAFF_TAGTABLE mmsgui_taff_tagtable[] = {
	{	"mmstheme",		NULL, 	NULL,			MMSGUI_MMSTHEME_ATTR_I			},
	{	"mmsdialog",	NULL, 	NULL,			MMSGUI_MMSDIALOG_ATTR_I			},
	{	"description",	NULL, 	NULL,			MMSGUI_DESCRIPTION_ATTR_I		},
	{	"vbox",			NULL, 	NULL,			MMSGUI_BASE_ATTR_I				},
	{	"hbox",			NULL, 	NULL,			MMSGUI_BASE_ATTR_I				},
	{	"template", 	NULL,	NULL,			MMSGUI_TEMPLATE_ATTR_I			},
	{	"class", 		"type",	"template",		MMSGUI_TEMPLATE_ATTR_I			},
	{	"mainwindow",	NULL, 	NULL,			MMSGUI_MAINWINDOW_ATTR_I		},
	{	"class", 		"type",	"mainwindow",	MMSGUI_MAINWINDOW_ATTR_I		},
	{	"childwindow",	NULL, 	NULL,			MMSGUI_CHILDWINDOW_ATTR_I		},
	{	"class", 		"type",	"childwindow",	MMSGUI_CHILDWINDOW_ATTR_I		},
	{	"popupwindow",	NULL, 	NULL,			MMSGUI_POPUPWINDOW_ATTR_I		},
	{	"class", 		"type",	"popupwindow", 	MMSGUI_POPUPWINDOW_ATTR_I		},
	{	"rootwindow",	NULL, 	NULL,			MMSGUI_ROOTWINDOW_ATTR_I		},
	{	"class", 		"type",	"rootwindow",	MMSGUI_ROOTWINDOW_ATTR_I		},
	{	"arrow", 		NULL, 	NULL,			MMSGUI_ARROWWIDGET_ATTR_I		},
	{	"class", 		"type",	"arrow",		MMSGUI_ARROWWIDGET_ATTR_I		},
	{	"button", 		NULL, 	NULL,			MMSGUI_BUTTONWIDGET_ATTR_I		},
	{	"class", 		"type",	"button",		MMSGUI_BUTTONWIDGET_ATTR_I		},
	{	"image", 		NULL, 	NULL,			MMSGUI_IMAGEWIDGET_ATTR_I		},
	{	"class", 		"type",	"image",		MMSGUI_IMAGEWIDGET_ATTR_I		},
	{	"label", 		NULL, 	NULL,			MMSGUI_LABELWIDGET_ATTR_I		},
	{	"class", 		"type",	"label",		MMSGUI_LABELWIDGET_ATTR_I		},
	{	"menu", 		NULL, 	NULL,			MMSGUI_MENUWIDGET_ATTR_I		},
	{	"class", 		"type",	"menu",			MMSGUI_MENUWIDGET_ATTR_I		},
	{	"menuitem",		NULL, 	NULL,			MMSGUI_BASE_ATTR_I				},
	{	"progressbar",	NULL, 	NULL,			MMSGUI_PROGRESSBARWIDGET_ATTR_I	},
	{	"class", 		"type",	"progressbar",	MMSGUI_PROGRESSBARWIDGET_ATTR_I	},
	{	"slider",		NULL, 	NULL,			MMSGUI_SLIDERWIDGET_ATTR_I		},
	{	"class", 		"type",	"slider",		MMSGUI_SLIDERWIDGET_ATTR_I		},
	{	"textbox",		NULL, 	NULL,			MMSGUI_TEXTBOXWIDGET_ATTR_I		},
	{	"class", 		"type",	"textbox",		MMSGUI_TEXTBOXWIDGET_ATTR_I		},
	{	"input", 		NULL, 	NULL,			MMSGUI_INPUTWIDGET_ATTR_I		},
	{	"class", 		"type",	"input",		MMSGUI_INPUTWIDGET_ATTR_I		},
	{	"checkbox",		NULL, 	NULL,			MMSGUI_CHECKBOXWIDGET_ATTR_I	},
	{	"class", 		"type",	"checkbox",		MMSGUI_CHECKBOXWIDGET_ATTR_I	},
	{	"gap",			NULL, 	NULL,			MMSGUI_BASE_ATTR_I				},
	{	NULL, 			NULL, 	NULL,			NULL							}
};

TAFF_DESCRIPTION mmsgui_taff_description = { "mmsgui", 30, mmsgui_taff_tagtable };





bool MMSTheme::addTemplateClass(MMSTemplateClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < templateClasses.size(); i++)
        if (templateClasses.at(i)->getClassName() == className)
            return false;
    templateClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addMainWindowClass(MMSMainWindowClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < mainWindowClasses.size(); i++)
        if (mainWindowClasses.at(i)->getClassName() == className)
            return false;
    mainWindowClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addPopupWindowClass(MMSPopupWindowClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < popupWindowClasses.size(); i++)
        if (popupWindowClasses.at(i)->getClassName() == className)
            return false;
    popupWindowClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addRootWindowClass(MMSRootWindowClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < rootWindowClasses.size(); i++)
        if (rootWindowClasses.at(i)->getClassName() == className)
            return false;
    rootWindowClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addChildWindowClass(MMSChildWindowClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < childWindowClasses.size(); i++)
        if (childWindowClasses.at(i)->getClassName() == className)
            return false;
    childWindowClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addLabelWidgetClass(MMSLabelWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < labelWidgetClasses.size(); i++)
        if (labelWidgetClasses.at(i)->getClassName() == className)
            return false;
    labelWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addImageWidgetClass(MMSImageWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < imageWidgetClasses.size(); i++)
        if (imageWidgetClasses.at(i)->getClassName() == className)
            return false;
    imageWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addButtonWidgetClass(MMSButtonWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < buttonWidgetClasses.size(); i++)
        if (buttonWidgetClasses.at(i)->getClassName() == className)
            return false;
    buttonWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addProgressBarWidgetClass(MMSProgressBarWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < progressBarWidgetClasses.size(); i++)
        if (progressBarWidgetClasses.at(i)->getClassName() == className)
            return false;
    progressBarWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addMenuWidgetClass(MMSMenuWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < menuWidgetClasses.size(); i++)
        if (menuWidgetClasses.at(i)->getClassName() == className)
            return false;
    menuWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addTextBoxWidgetClass(MMSTextBoxWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < textBoxWidgetClasses.size(); i++)
        if (textBoxWidgetClasses.at(i)->getClassName() == className)
            return false;
    textBoxWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addArrowWidgetClass(MMSArrowWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < arrowWidgetClasses.size(); i++)
        if (arrowWidgetClasses.at(i)->getClassName() == className)
            return false;
    arrowWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addSliderWidgetClass(MMSSliderWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < sliderWidgetClasses.size(); i++)
        if (sliderWidgetClasses.at(i)->getClassName() == className)
            return false;
    sliderWidgetClasses.push_back(themeClass);
    return true;
}

bool MMSTheme::addInputWidgetClass(MMSInputWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < inputWidgetClasses.size(); i++)
        if (inputWidgetClasses.at(i)->getClassName() == className)
            return false;
    inputWidgetClasses.push_back(themeClass);
    return true;
}


bool MMSTheme::addCheckBoxWidgetClass(MMSCheckBoxWidgetClass *themeClass) {
    string className = themeClass->getClassName();
    if (className == "") return false;
    for (unsigned int i = 0; i < checkBoxWidgetClasses.size(); i++)
        if (checkBoxWidgetClasses.at(i)->getClassName() == className)
            return false;
    checkBoxWidgetClasses.push_back(themeClass);
    return true;
}


#ifdef ssfsf
void MMSTheme::addSimpleHSliderClass(MMSSIMPLESLIDERH_THEME *simpleHSliderClass, const string name) {
    /* add to hash lookup table */
    pair<SIMPLEHSLIDERCLASSMAP::iterator, bool> p = simpleHSliderClassMap.insert(pair<const char*, MMSSIMPLESLIDERH_THEME*>(name.c_str(), (MMSSIMPLESLIDERH_THEME*)simpleHSliderClass));
    if(!p.second)
        throw MMSThemeError(1, "duplicate class name: " + name);

    simpleHSliderClasses.push_back(simpleHSliderClass);
}

void MMSTheme::addSimpleVSliderClass(MMSSIMPLESLIDERV_THEME *simpleVSliderClass, const string name) {
    /* add to hash lookup table */
    pair<SIMPLEVSLIDERCLASSMAP::iterator, bool> p = simpleVSliderClassMap.insert(pair<const char*, MMSSIMPLESLIDERV_THEME*>(name.c_str(), (MMSSIMPLESLIDERV_THEME*)simpleVSliderClass));
    if(!p.second)
        throw MMSThemeError(1, "duplicate class name: " + name);

    simpleVSliderClasses.push_back(simpleVSliderClass);
}

void MMSTheme::addSimpleHMenuClass(MMSSIMPLEHMENU_THEME *simpleHMenuClass, const string name) {
    /* add to hash lookup table */
    pair<SIMPLEHMENUCLASSMAP::iterator, bool> p = simpleHMenuClassMap.insert(pair<const char*, MMSSIMPLEHMENU_THEME*>(name.c_str(), (MMSSIMPLEHMENU_THEME*)simpleHMenuClass));
    if(!p.second)
        throw MMSThemeError(1, "duplicate class name: " + name);

    simpleHMenuClasses.push_back(simpleHMenuClass);
}

void MMSTheme::addSimpleVMenuClass(MMSSIMPLEVMENU_THEME *simpleVMenuClass, const string name) {
    /* add to hash lookup table */
    pair<SIMPLEVMENUCLASSMAP::iterator, bool> p = simpleVMenuClassMap.insert(pair<const char*, MMSSIMPLEVMENU_THEME*>(name.c_str(), (MMSSIMPLEVMENU_THEME*)simpleVMenuClass));
    if(!p.second)
        throw MMSThemeError(1, "duplicate class name: " + name);

    simpleVMenuClasses.push_back(simpleVMenuClass);
}
#endif

#ifdef ssfsf
void MMSTheme::addTextboxClass(MMSTEXTBOX_THEME *textboxClass, const string name) {
    /* add to hash lookup table */
    pair<TEXTBOXCLASSMAP::iterator, bool> p = textboxClassMap.insert(pair<const char*, MMSTEXTBOX_THEME*>(name.c_str(), (MMSTEXTBOX_THEME*)textboxClass));
    if(!p.second)
        throw MMSThemeError(1, "duplicate class name: " + name);

    textboxClasses.push_back(textboxClass);
}
#endif

MMSTemplateClass* MMSTheme::getTemplateClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < templateClasses.size(); i++)
        if (templateClasses.at(i)->getClassName() == className)
            return templateClasses.at(i);
    return NULL;
}

MMSMainWindowClass* MMSTheme::getMainWindowClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < mainWindowClasses.size(); i++)
        if (mainWindowClasses.at(i)->getClassName() == className)
            return mainWindowClasses.at(i);
    return NULL;
}

MMSPopupWindowClass* MMSTheme::getPopupWindowClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < popupWindowClasses.size(); i++)
        if (popupWindowClasses.at(i)->getClassName() == className)
            return popupWindowClasses.at(i);
    return NULL;
}

MMSRootWindowClass* MMSTheme::getRootWindowClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < rootWindowClasses.size(); i++)
        if (rootWindowClasses.at(i)->getClassName() == className)
            return rootWindowClasses.at(i);
    return NULL;
}

MMSChildWindowClass* MMSTheme::getChildWindowClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < childWindowClasses.size(); i++)
        if (childWindowClasses.at(i)->getClassName() == className)
            return childWindowClasses.at(i);
    return NULL;
}

MMSLabelWidgetClass* MMSTheme::getLabelWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < labelWidgetClasses.size(); i++)
        if (labelWidgetClasses.at(i)->getClassName() == className)
            return labelWidgetClasses.at(i);
    return NULL;
}

MMSImageWidgetClass* MMSTheme::getImageWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < imageWidgetClasses.size(); i++)
        if (imageWidgetClasses.at(i)->getClassName() == className)
            return imageWidgetClasses.at(i);
    return NULL;
}

MMSButtonWidgetClass* MMSTheme::getButtonWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < buttonWidgetClasses.size(); i++)
        if (buttonWidgetClasses.at(i)->getClassName() == className)
            return buttonWidgetClasses.at(i);
    return NULL;
}

MMSProgressBarWidgetClass* MMSTheme::getProgressBarWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < progressBarWidgetClasses.size(); i++)
        if (progressBarWidgetClasses.at(i)->getClassName() == className)
            return progressBarWidgetClasses.at(i);
    return NULL;
}

MMSMenuWidgetClass* MMSTheme::getMenuWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < menuWidgetClasses.size(); i++)
        if (menuWidgetClasses.at(i)->getClassName() == className)
            return menuWidgetClasses.at(i);
    return NULL;
}

MMSTextBoxWidgetClass* MMSTheme::getTextBoxWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < textBoxWidgetClasses.size(); i++)
        if (textBoxWidgetClasses.at(i)->getClassName() == className)
            return textBoxWidgetClasses.at(i);
    return NULL;
}

MMSArrowWidgetClass* MMSTheme::getArrowWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < arrowWidgetClasses.size(); i++)
        if (arrowWidgetClasses.at(i)->getClassName() == className)
            return arrowWidgetClasses.at(i);
    return NULL;
}

MMSSliderWidgetClass* MMSTheme::getSliderWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < sliderWidgetClasses.size(); i++)
        if (sliderWidgetClasses.at(i)->getClassName() == className)
            return sliderWidgetClasses.at(i);
    return NULL;
}

MMSInputWidgetClass* MMSTheme::getInputWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < inputWidgetClasses.size(); i++)
        if (inputWidgetClasses.at(i)->getClassName() == className)
            return inputWidgetClasses.at(i);
    return NULL;
}


MMSCheckBoxWidgetClass* MMSTheme::getCheckBoxWidgetClass(string className) {
    if (className=="") return NULL;
    for (unsigned int i = 0; i < checkBoxWidgetClasses.size(); i++)
        if (checkBoxWidgetClasses.at(i)->getClassName() == className)
            return checkBoxWidgetClasses.at(i);
    return NULL;
}


MMSTheme::MMSTheme(bool initial_load, bool debug) {

    /* initialize the theme with default values */
    MMSFBColor color;
    color.a = 0;
    color.r = 0;
    color.g = 0;
    color.b = 0;

    /* MMSMainWindow */
    {
        /* base window settings */
        this->mainWindowClass.windowClass.setAlignment(MMSALIGNMENT_CENTER);
        this->mainWindowClass.windowClass.setDx("0px");
        this->mainWindowClass.windowClass.setDy("0px");
        this->mainWindowClass.windowClass.setWidth("100%");
        this->mainWindowClass.windowClass.setHeight("100%");
        this->mainWindowClass.windowClass.setBgColor(color);
        this->mainWindowClass.windowClass.setBgImagePath("");
        this->mainWindowClass.windowClass.setBgImageName("");
        this->mainWindowClass.windowClass.setOpacity(255);
        this->mainWindowClass.windowClass.setFadeIn(false);
        this->mainWindowClass.windowClass.setFadeOut(false);
        this->mainWindowClass.windowClass.setDebug(debug);
        this->mainWindowClass.windowClass.setMargin(0);
        this->mainWindowClass.windowClass.setUpArrow("");
        this->mainWindowClass.windowClass.setDownArrow("");
        this->mainWindowClass.windowClass.setLeftArrow("");
        this->mainWindowClass.windowClass.setRightArrow("");
        this->mainWindowClass.windowClass.setNavigateUp("");
        this->mainWindowClass.windowClass.setNavigateDown("");
        this->mainWindowClass.windowClass.setNavigateLeft("");
        this->mainWindowClass.windowClass.setNavigateRight("");
        this->mainWindowClass.windowClass.setOwnSurface(false);
        this->mainWindowClass.windowClass.setMoveIn(MMSDIRECTION_NOTSET);
        this->mainWindowClass.windowClass.setMoveOut(MMSDIRECTION_NOTSET);
        this->mainWindowClass.windowClass.setModal(false);
        this->mainWindowClass.windowClass.setStaticZOrder(false);
        this->mainWindowClass.windowClass.setAlwaysOnTop(false);
        this->mainWindowClass.windowClass.setFocusable(true);
        this->mainWindowClass.windowClass.setBackBuffer(false);
        this->mainWindowClass.windowClass.setInitialLoad(initial_load);

        /* base window border settings */
        this->mainWindowClass.windowClass.border.setColor(color);
        this->mainWindowClass.windowClass.border.setSelColor(color);
        this->mainWindowClass.windowClass.border.setImagePath("");
        this->mainWindowClass.windowClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->mainWindowClass.windowClass.border.setSelImagePath("");
        this->mainWindowClass.windowClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->mainWindowClass.windowClass.border.setThickness(0);
        this->mainWindowClass.windowClass.border.setMargin(0);
        this->mainWindowClass.windowClass.border.setRCorners(false);
    }

    /* MMSPopupWindow */
    {
        /* base window settings */
        this->popupWindowClass.windowClass.setAlignment(MMSALIGNMENT_CENTER);
        this->popupWindowClass.windowClass.setDx("0px");
        this->popupWindowClass.windowClass.setDy("0px");
        this->popupWindowClass.windowClass.setWidth("100%");
        this->popupWindowClass.windowClass.setHeight("100%");
        this->popupWindowClass.windowClass.setBgColor(color);
        this->popupWindowClass.windowClass.setBgImagePath("");
        this->popupWindowClass.windowClass.setBgImageName("");
        this->popupWindowClass.windowClass.setOpacity(255);
        this->popupWindowClass.windowClass.setFadeIn(false);
        this->popupWindowClass.windowClass.setFadeOut(false);
        this->popupWindowClass.windowClass.setDebug(debug);
        this->popupWindowClass.windowClass.setMargin(0);
        this->popupWindowClass.windowClass.setUpArrow("");
        this->popupWindowClass.windowClass.setDownArrow("");
        this->popupWindowClass.windowClass.setLeftArrow("");
        this->popupWindowClass.windowClass.setRightArrow("");
        this->popupWindowClass.windowClass.setNavigateUp("");
        this->popupWindowClass.windowClass.setNavigateDown("");
        this->popupWindowClass.windowClass.setNavigateLeft("");
        this->popupWindowClass.windowClass.setNavigateRight("");
        this->popupWindowClass.windowClass.setOwnSurface(true);
        this->popupWindowClass.windowClass.setMoveIn(MMSDIRECTION_NOTSET);
        this->popupWindowClass.windowClass.setMoveOut(MMSDIRECTION_NOTSET);
        this->popupWindowClass.windowClass.setModal(false);
        this->popupWindowClass.windowClass.setStaticZOrder(false);
        this->popupWindowClass.windowClass.setAlwaysOnTop(false);
        this->popupWindowClass.windowClass.setFocusable(false);
        this->popupWindowClass.windowClass.setBackBuffer(false);
        this->popupWindowClass.windowClass.setInitialLoad(initial_load);

        /* base window border settings */
        this->popupWindowClass.windowClass.border.setColor(color);
        this->popupWindowClass.windowClass.border.setSelColor(color);
        this->popupWindowClass.windowClass.border.setImagePath("");
        this->popupWindowClass.windowClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->popupWindowClass.windowClass.border.setSelImagePath("");
        this->popupWindowClass.windowClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->popupWindowClass.windowClass.border.setThickness(0);
        this->popupWindowClass.windowClass.border.setMargin(0);
        this->popupWindowClass.windowClass.border.setRCorners(false);

        /* popup window settings */
        this->popupWindowClass.setDuration(5);
    }

    /* MMSRootWindow */
    {
        /* base window settings */
        this->rootWindowClass.windowClass.setAlignment(MMSALIGNMENT_CENTER);
        this->rootWindowClass.windowClass.setDx("0px");
        this->rootWindowClass.windowClass.setDy("0px");
        this->rootWindowClass.windowClass.setWidth("100%");
        this->rootWindowClass.windowClass.setHeight("100%");
        this->rootWindowClass.windowClass.setBgColor(color);
        this->rootWindowClass.windowClass.setBgImagePath("");
        this->rootWindowClass.windowClass.setBgImageName("");
        this->rootWindowClass.windowClass.setOpacity(255);
        this->rootWindowClass.windowClass.setFadeIn(false);
        this->rootWindowClass.windowClass.setFadeOut(false);
        this->rootWindowClass.windowClass.setDebug(debug);
        this->rootWindowClass.windowClass.setMargin(0);
        this->rootWindowClass.windowClass.setUpArrow("");
        this->rootWindowClass.windowClass.setDownArrow("");
        this->rootWindowClass.windowClass.setLeftArrow("");
        this->rootWindowClass.windowClass.setRightArrow("");
        this->rootWindowClass.windowClass.setNavigateUp("");
        this->rootWindowClass.windowClass.setNavigateDown("");
        this->rootWindowClass.windowClass.setNavigateLeft("");
        this->rootWindowClass.windowClass.setNavigateRight("");
        this->rootWindowClass.windowClass.setOwnSurface(false);
        this->rootWindowClass.windowClass.setMoveIn(MMSDIRECTION_NOTSET);
        this->rootWindowClass.windowClass.setMoveOut(MMSDIRECTION_NOTSET);
        this->rootWindowClass.windowClass.setModal(false);
        this->rootWindowClass.windowClass.setStaticZOrder(false);
        this->rootWindowClass.windowClass.setAlwaysOnTop(false);
        this->rootWindowClass.windowClass.setFocusable(true);
        this->rootWindowClass.windowClass.setBackBuffer(false);
        this->rootWindowClass.windowClass.setInitialLoad(initial_load);

        /* base window border settings */
        this->rootWindowClass.windowClass.border.setColor(color);
        this->rootWindowClass.windowClass.border.setSelColor(color);
        this->rootWindowClass.windowClass.border.setImagePath("");
        this->rootWindowClass.windowClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->rootWindowClass.windowClass.border.setSelImagePath("");
        this->rootWindowClass.windowClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->rootWindowClass.windowClass.border.setThickness(0);
        this->rootWindowClass.windowClass.border.setMargin(0);
        this->rootWindowClass.windowClass.border.setRCorners(false);
    }

    /* MMSChildWindow */
    {
        /* base window settings */
        this->childWindowClass.windowClass.setAlignment(MMSALIGNMENT_CENTER);
        this->childWindowClass.windowClass.setDx("0px");
        this->childWindowClass.windowClass.setDy("0px");
        this->childWindowClass.windowClass.setWidth("100%");
        this->childWindowClass.windowClass.setHeight("100%");
        this->childWindowClass.windowClass.setBgColor(color);
        this->childWindowClass.windowClass.setBgImagePath("");
        this->childWindowClass.windowClass.setBgImageName("");
        this->childWindowClass.windowClass.setOpacity(255);
        this->childWindowClass.windowClass.setFadeIn(false);
        this->childWindowClass.windowClass.setFadeOut(false);
        this->childWindowClass.windowClass.setDebug(debug);
        this->childWindowClass.windowClass.setMargin(0);
        this->childWindowClass.windowClass.setUpArrow("");
        this->childWindowClass.windowClass.setDownArrow("");
        this->childWindowClass.windowClass.setLeftArrow("");
        this->childWindowClass.windowClass.setRightArrow("");
        this->childWindowClass.windowClass.setNavigateUp("");
        this->childWindowClass.windowClass.setNavigateDown("");
        this->childWindowClass.windowClass.setNavigateLeft("");
        this->childWindowClass.windowClass.setNavigateRight("");
        this->childWindowClass.windowClass.setOwnSurface(true);
        this->childWindowClass.windowClass.setMoveIn(MMSDIRECTION_NOTSET);
        this->childWindowClass.windowClass.setMoveOut(MMSDIRECTION_NOTSET);
        this->childWindowClass.windowClass.setModal(false);
        this->childWindowClass.windowClass.setStaticZOrder(false);
        this->childWindowClass.windowClass.setAlwaysOnTop(false);
        this->childWindowClass.windowClass.setFocusable(true);
        this->childWindowClass.windowClass.setBackBuffer(false);
        this->childWindowClass.windowClass.setInitialLoad(initial_load);

        /* base window border settings */
        this->childWindowClass.windowClass.border.setColor(color);
        this->childWindowClass.windowClass.border.setSelColor(color);
        this->childWindowClass.windowClass.border.setImagePath("");
        this->childWindowClass.windowClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->childWindowClass.windowClass.border.setSelImagePath("");
        this->childWindowClass.windowClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->childWindowClass.windowClass.border.setThickness(0);
        this->childWindowClass.windowClass.border.setMargin(0);
        this->childWindowClass.windowClass.border.setRCorners(false);
    }

    /* MMSLabelWidget */
    {
        /* base widget settings */
        this->labelWidgetClass.widgetClass.setBgColor(color);
        this->labelWidgetClass.widgetClass.setSelBgColor(color);
        this->labelWidgetClass.widgetClass.setBgColor_p(color);
        this->labelWidgetClass.widgetClass.setSelBgColor_p(color);
        this->labelWidgetClass.widgetClass.setBgColor_i(color);
        this->labelWidgetClass.widgetClass.setSelBgColor_i(color);
        this->labelWidgetClass.widgetClass.setBgImagePath("");
        this->labelWidgetClass.widgetClass.setBgImageName("");
        this->labelWidgetClass.widgetClass.setSelBgImagePath("");
        this->labelWidgetClass.widgetClass.setSelBgImageName("");
        this->labelWidgetClass.widgetClass.setBgImagePath_p("");
        this->labelWidgetClass.widgetClass.setBgImageName_p("");
        this->labelWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->labelWidgetClass.widgetClass.setSelBgImageName_p("");
        this->labelWidgetClass.widgetClass.setBgImagePath_i("");
        this->labelWidgetClass.widgetClass.setBgImageName_i("");
        this->labelWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->labelWidgetClass.widgetClass.setSelBgImageName_i("");
        this->labelWidgetClass.widgetClass.setMargin(0);
        this->labelWidgetClass.widgetClass.setFocusable(false);
        this->labelWidgetClass.widgetClass.setSelectable(true);
        this->labelWidgetClass.widgetClass.setUpArrow("");
        this->labelWidgetClass.widgetClass.setDownArrow("");
        this->labelWidgetClass.widgetClass.setLeftArrow("");
        this->labelWidgetClass.widgetClass.setRightArrow("");
        this->labelWidgetClass.widgetClass.setData("");
        this->labelWidgetClass.widgetClass.setNavigateUp("");
        this->labelWidgetClass.widgetClass.setNavigateDown("");
        this->labelWidgetClass.widgetClass.setNavigateLeft("");
        this->labelWidgetClass.widgetClass.setNavigateRight("");
        this->labelWidgetClass.widgetClass.setVSlider("");
        this->labelWidgetClass.widgetClass.setHSlider("");
        this->labelWidgetClass.widgetClass.setImagesOnDemand(false);
        this->labelWidgetClass.widgetClass.setBlend(0);
        this->labelWidgetClass.widgetClass.setBlendFactor(0);
        this->labelWidgetClass.widgetClass.setScrollOnFocus(false);
        this->labelWidgetClass.widgetClass.setClickable(false);
        this->labelWidgetClass.widgetClass.setReturnOnScroll(true);
        this->labelWidgetClass.widgetClass.setInputMode("");
        this->labelWidgetClass.widgetClass.setJoinedWidget("");
        this->labelWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->labelWidgetClass.widgetClass.border.setColor(color);
        this->labelWidgetClass.widgetClass.border.setSelColor(color);
        this->labelWidgetClass.widgetClass.border.setImagePath("");
        this->labelWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->labelWidgetClass.widgetClass.border.setSelImagePath("");
        this->labelWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->labelWidgetClass.widgetClass.border.setThickness(0);
        this->labelWidgetClass.widgetClass.border.setMargin(0);
        this->labelWidgetClass.widgetClass.border.setRCorners(false);

        /* label settings */
        this->labelWidgetClass.setFontPath("./themes/default");
        this->labelWidgetClass.setFontName("decker.ttf");
        this->labelWidgetClass.setFontSize(16);
        this->labelWidgetClass.setAlignment(MMSALIGNMENT_CENTER);
        this->labelWidgetClass.setColor(MMSFBColor(0xc0, 0xc0, 0xc0, 0xff));
        this->labelWidgetClass.setSelColor(MMSFBColor(0xff, 0xff, 0xff, 0xff));
        this->labelWidgetClass.setColor_p(MMSFBColor());
        this->labelWidgetClass.setSelColor_p(MMSFBColor());
        this->labelWidgetClass.setColor_i(MMSFBColor(0x80, 0x80, 0x80, 0xff));
        this->labelWidgetClass.setSelColor_i(MMSFBColor(0xbf, 0xbf, 0xbf, 0xff));
        this->labelWidgetClass.setText("");
        this->labelWidgetClass.setSlidable(false);
        this->labelWidgetClass.setSlideSpeed(50);
        this->labelWidgetClass.setTranslate(true);

        for (int position = 0; position < MMSPOSITION_SIZE; position++) {
			this->labelWidgetClass.setShadowColor((MMSPOSITION)position, MMSFBColor(0,0,0,0));
			this->labelWidgetClass.setSelShadowColor((MMSPOSITION)position, MMSFBColor(0,0,0,0));
        }
    }

    /* MMSImageWidget */
    {
        /* base widget settings */
        this->imageWidgetClass.widgetClass.setBgColor(color);
        this->imageWidgetClass.widgetClass.setSelBgColor(color);
        this->imageWidgetClass.widgetClass.setBgColor_p(color);
        this->imageWidgetClass.widgetClass.setSelBgColor_p(color);
        this->imageWidgetClass.widgetClass.setBgColor_i(color);
        this->imageWidgetClass.widgetClass.setSelBgColor_i(color);
        this->imageWidgetClass.widgetClass.setBgImagePath("");
        this->imageWidgetClass.widgetClass.setBgImageName("");
        this->imageWidgetClass.widgetClass.setSelBgImagePath("");
        this->imageWidgetClass.widgetClass.setSelBgImageName("");
        this->imageWidgetClass.widgetClass.setBgImagePath_p("");
        this->imageWidgetClass.widgetClass.setBgImageName_p("");
        this->imageWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->imageWidgetClass.widgetClass.setSelBgImageName_p("");
        this->imageWidgetClass.widgetClass.setBgImagePath_i("");
        this->imageWidgetClass.widgetClass.setBgImageName_i("");
        this->imageWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->imageWidgetClass.widgetClass.setSelBgImageName_i("");
        this->imageWidgetClass.widgetClass.setMargin(0);
        this->imageWidgetClass.widgetClass.setFocusable(false);
        this->imageWidgetClass.widgetClass.setSelectable(true);
        this->imageWidgetClass.widgetClass.setUpArrow("");
        this->imageWidgetClass.widgetClass.setDownArrow("");
        this->imageWidgetClass.widgetClass.setLeftArrow("");
        this->imageWidgetClass.widgetClass.setRightArrow("");
        this->imageWidgetClass.widgetClass.setData("");
        this->imageWidgetClass.widgetClass.setNavigateUp("");
        this->imageWidgetClass.widgetClass.setNavigateDown("");
        this->imageWidgetClass.widgetClass.setNavigateLeft("");
        this->imageWidgetClass.widgetClass.setNavigateRight("");
        this->imageWidgetClass.widgetClass.setVSlider("");
        this->imageWidgetClass.widgetClass.setHSlider("");
        this->imageWidgetClass.widgetClass.setImagesOnDemand(false);
        this->imageWidgetClass.widgetClass.setBlend(0);
        this->imageWidgetClass.widgetClass.setBlendFactor(0);
        this->imageWidgetClass.widgetClass.setScrollOnFocus(false);
        this->imageWidgetClass.widgetClass.setClickable(false);
        this->imageWidgetClass.widgetClass.setReturnOnScroll(true);
        this->imageWidgetClass.widgetClass.setInputMode("");
        this->imageWidgetClass.widgetClass.setJoinedWidget("");
        this->imageWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->imageWidgetClass.widgetClass.border.setColor(color);
        this->imageWidgetClass.widgetClass.border.setSelColor(color);
        this->imageWidgetClass.widgetClass.border.setImagePath("");
        this->imageWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->imageWidgetClass.widgetClass.border.setSelImagePath("");
        this->imageWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->imageWidgetClass.widgetClass.border.setThickness(0);
        this->imageWidgetClass.widgetClass.border.setMargin(0);
        this->imageWidgetClass.widgetClass.border.setRCorners(false);

        /* image settings */
        this->imageWidgetClass.setImagePath("");
        this->imageWidgetClass.setImageName("");
        this->imageWidgetClass.setSelImagePath("");
        this->imageWidgetClass.setSelImageName("");
        this->imageWidgetClass.setImagePath_p("");
        this->imageWidgetClass.setImageName_p("");
        this->imageWidgetClass.setSelImagePath_p("");
        this->imageWidgetClass.setSelImageName_p("");
        this->imageWidgetClass.setImagePath_i("");
        this->imageWidgetClass.setImageName_i("");
        this->imageWidgetClass.setSelImagePath_i("");
        this->imageWidgetClass.setSelImageName_i("");
        this->imageWidgetClass.setUseRatio(false);
        this->imageWidgetClass.setFitWidth(false);
        this->imageWidgetClass.setFitHeight(false);
        this->imageWidgetClass.setAlignment(MMSALIGNMENT_CENTER);
        this->imageWidgetClass.setMirrorSize(0);
        this->imageWidgetClass.setGenTaff(true);
    }

    /* MMSButtonWidget */
    {
        /* base widget settings */
        this->buttonWidgetClass.widgetClass.setBgColor(color);
        this->buttonWidgetClass.widgetClass.setSelBgColor(color);
        this->buttonWidgetClass.widgetClass.setBgColor_p(color);
        this->buttonWidgetClass.widgetClass.setSelBgColor_p(color);
        this->buttonWidgetClass.widgetClass.setBgColor_i(color);
        this->buttonWidgetClass.widgetClass.setSelBgColor_i(color);
        this->buttonWidgetClass.widgetClass.setBgImagePath("");
        this->buttonWidgetClass.widgetClass.setBgImageName("");
        this->buttonWidgetClass.widgetClass.setSelBgImagePath("");
        this->buttonWidgetClass.widgetClass.setSelBgImageName("");
        this->buttonWidgetClass.widgetClass.setBgImagePath_p("");
        this->buttonWidgetClass.widgetClass.setBgImageName_p("");
        this->buttonWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->buttonWidgetClass.widgetClass.setSelBgImageName_p("");
        this->buttonWidgetClass.widgetClass.setBgImagePath_i("");
        this->buttonWidgetClass.widgetClass.setBgImageName_i("");
        this->buttonWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->buttonWidgetClass.widgetClass.setSelBgImageName_i("");
        this->buttonWidgetClass.widgetClass.setMargin(0);
        this->buttonWidgetClass.widgetClass.setFocusable(true);
        this->buttonWidgetClass.widgetClass.setSelectable(true);
        this->buttonWidgetClass.widgetClass.setUpArrow("");
        this->buttonWidgetClass.widgetClass.setDownArrow("");
        this->buttonWidgetClass.widgetClass.setLeftArrow("");
        this->buttonWidgetClass.widgetClass.setRightArrow("");
        this->buttonWidgetClass.widgetClass.setData("");
        this->buttonWidgetClass.widgetClass.setNavigateUp("");
        this->buttonWidgetClass.widgetClass.setNavigateDown("");
        this->buttonWidgetClass.widgetClass.setNavigateLeft("");
        this->buttonWidgetClass.widgetClass.setNavigateRight("");
        this->buttonWidgetClass.widgetClass.setVSlider("");
        this->buttonWidgetClass.widgetClass.setHSlider("");
        this->buttonWidgetClass.widgetClass.setImagesOnDemand(false);
        this->buttonWidgetClass.widgetClass.setBlend(0);
        this->buttonWidgetClass.widgetClass.setBlendFactor(0);
        this->buttonWidgetClass.widgetClass.setScrollOnFocus(false);
        this->buttonWidgetClass.widgetClass.setClickable(true);
        this->buttonWidgetClass.widgetClass.setReturnOnScroll(true);
        this->buttonWidgetClass.widgetClass.setInputMode("");
        this->buttonWidgetClass.widgetClass.setJoinedWidget("");
        this->buttonWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->buttonWidgetClass.widgetClass.border.setColor(color);
        this->buttonWidgetClass.widgetClass.border.setSelColor(color);
        this->buttonWidgetClass.widgetClass.border.setImagePath("");
        this->buttonWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->buttonWidgetClass.widgetClass.border.setSelImagePath("");
        this->buttonWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->buttonWidgetClass.widgetClass.border.setThickness(0);
        this->buttonWidgetClass.widgetClass.border.setMargin(0);
        this->buttonWidgetClass.widgetClass.border.setRCorners(false);
    }

    /* MMSProgressBarWidget */
    {
        /* base widget settings */
        this->progressBarWidgetClass.widgetClass.setBgColor(color);
        this->progressBarWidgetClass.widgetClass.setSelBgColor(color);
        this->progressBarWidgetClass.widgetClass.setBgColor_p(color);
        this->progressBarWidgetClass.widgetClass.setSelBgColor_p(color);
        this->progressBarWidgetClass.widgetClass.setBgColor_i(color);
        this->progressBarWidgetClass.widgetClass.setSelBgColor_i(color);
        this->progressBarWidgetClass.widgetClass.setBgImagePath("");
        this->progressBarWidgetClass.widgetClass.setBgImageName("");
        this->progressBarWidgetClass.widgetClass.setSelBgImagePath("");
        this->progressBarWidgetClass.widgetClass.setSelBgImageName("");
        this->progressBarWidgetClass.widgetClass.setBgImagePath_p("");
        this->progressBarWidgetClass.widgetClass.setBgImageName_p("");
        this->progressBarWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->progressBarWidgetClass.widgetClass.setSelBgImageName_p("");
        this->progressBarWidgetClass.widgetClass.setBgImagePath_i("");
        this->progressBarWidgetClass.widgetClass.setBgImageName_i("");
        this->progressBarWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->progressBarWidgetClass.widgetClass.setSelBgImageName_i("");
        this->progressBarWidgetClass.widgetClass.setMargin(0);
        this->progressBarWidgetClass.widgetClass.setFocusable(false);
        this->progressBarWidgetClass.widgetClass.setSelectable(true);
        this->progressBarWidgetClass.widgetClass.setUpArrow("");
        this->progressBarWidgetClass.widgetClass.setDownArrow("");
        this->progressBarWidgetClass.widgetClass.setLeftArrow("");
        this->progressBarWidgetClass.widgetClass.setRightArrow("");
        this->progressBarWidgetClass.widgetClass.setData("");
        this->progressBarWidgetClass.widgetClass.setNavigateUp("");
        this->progressBarWidgetClass.widgetClass.setNavigateDown("");
        this->progressBarWidgetClass.widgetClass.setNavigateLeft("");
        this->progressBarWidgetClass.widgetClass.setNavigateRight("");
        this->progressBarWidgetClass.widgetClass.setVSlider("");
        this->progressBarWidgetClass.widgetClass.setHSlider("");
        this->progressBarWidgetClass.widgetClass.setImagesOnDemand(false);
        this->progressBarWidgetClass.widgetClass.setBlend(0);
        this->progressBarWidgetClass.widgetClass.setBlendFactor(0);
        this->progressBarWidgetClass.widgetClass.setScrollOnFocus(false);
        this->progressBarWidgetClass.widgetClass.setClickable(false);
        this->progressBarWidgetClass.widgetClass.setReturnOnScroll(true);
        this->progressBarWidgetClass.widgetClass.setInputMode("");
        this->progressBarWidgetClass.widgetClass.setJoinedWidget("");
        this->progressBarWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->progressBarWidgetClass.widgetClass.border.setColor(color);
        this->progressBarWidgetClass.widgetClass.border.setSelColor(color);
        this->progressBarWidgetClass.widgetClass.border.setImagePath("");
        this->progressBarWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->progressBarWidgetClass.widgetClass.border.setSelImagePath("");
        this->progressBarWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->progressBarWidgetClass.widgetClass.border.setThickness(0);
        this->progressBarWidgetClass.widgetClass.border.setMargin(0);
        this->progressBarWidgetClass.widgetClass.border.setRCorners(false);

        /* progressbar settings */
        MMSFBColor c;
        c.a = 0;
        c.r = 0;
        c.g = 0;
        c.b = 0;
        this->progressBarWidgetClass.setColor(c);
        c.a = 255;
        c.r = 255;
        c.g = 255;
        c.b = 255;
        this->progressBarWidgetClass.setSelColor(c);
        this->progressBarWidgetClass.setProgress(100);
    }

    /* MMSMenuWidget */
    {
        /* base widget settings */
        this->menuWidgetClass.widgetClass.setBgColor(color);
        this->menuWidgetClass.widgetClass.setSelBgColor(color);
        this->menuWidgetClass.widgetClass.setBgColor_p(color);
        this->menuWidgetClass.widgetClass.setSelBgColor_p(color);
        this->menuWidgetClass.widgetClass.setBgColor_i(color);
        this->menuWidgetClass.widgetClass.setSelBgColor_i(color);
        this->menuWidgetClass.widgetClass.setBgImagePath("");
        this->menuWidgetClass.widgetClass.setBgImageName("");
        this->menuWidgetClass.widgetClass.setSelBgImagePath("");
        this->menuWidgetClass.widgetClass.setSelBgImageName("");
        this->menuWidgetClass.widgetClass.setBgImagePath_p("");
        this->menuWidgetClass.widgetClass.setBgImageName_p("");
        this->menuWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->menuWidgetClass.widgetClass.setSelBgImageName_p("");
        this->menuWidgetClass.widgetClass.setBgImagePath_i("");
        this->menuWidgetClass.widgetClass.setBgImageName_i("");
        this->menuWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->menuWidgetClass.widgetClass.setSelBgImageName_i("");
        this->menuWidgetClass.widgetClass.setMargin(0);
        this->menuWidgetClass.widgetClass.setFocusable(true);
        this->menuWidgetClass.widgetClass.setSelectable(true);
        this->menuWidgetClass.widgetClass.setUpArrow("");
        this->menuWidgetClass.widgetClass.setDownArrow("");
        this->menuWidgetClass.widgetClass.setLeftArrow("");
        this->menuWidgetClass.widgetClass.setRightArrow("");
        this->menuWidgetClass.widgetClass.setData("");
        this->menuWidgetClass.widgetClass.setNavigateUp("");
        this->menuWidgetClass.widgetClass.setNavigateDown("");
        this->menuWidgetClass.widgetClass.setNavigateLeft("");
        this->menuWidgetClass.widgetClass.setNavigateRight("");
        this->menuWidgetClass.widgetClass.setVSlider("");
        this->menuWidgetClass.widgetClass.setHSlider("");
        this->menuWidgetClass.widgetClass.setImagesOnDemand(false);
        this->menuWidgetClass.widgetClass.setBlend(0);
        this->menuWidgetClass.widgetClass.setBlendFactor(0);
        this->menuWidgetClass.widgetClass.setScrollOnFocus(false);
        this->menuWidgetClass.widgetClass.setClickable(true);
        this->menuWidgetClass.widgetClass.setReturnOnScroll(true);
        this->menuWidgetClass.widgetClass.setInputMode("");
        this->menuWidgetClass.widgetClass.setJoinedWidget("");
        this->menuWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->menuWidgetClass.widgetClass.border.setColor(color);
        this->menuWidgetClass.widgetClass.border.setSelColor(color);
        this->menuWidgetClass.widgetClass.border.setImagePath("");
        this->menuWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->menuWidgetClass.widgetClass.border.setSelImagePath("");
        this->menuWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->menuWidgetClass.widgetClass.border.setThickness(0);
        this->menuWidgetClass.widgetClass.border.setMargin(0);
        this->menuWidgetClass.widgetClass.border.setRCorners(false);

        /* menu settings */
        this->menuWidgetClass.setItemWidth("");
        this->menuWidgetClass.setItemHeight("");
        this->menuWidgetClass.setItemHMargin(0);
        this->menuWidgetClass.setItemVMargin(0);
        this->menuWidgetClass.setCols(0);
        this->menuWidgetClass.setDimItems(0);
        this->menuWidgetClass.setFixedPos(-1);
        this->menuWidgetClass.setHLoop(false);
        this->menuWidgetClass.setVLoop(false);
        this->menuWidgetClass.setTransItems(0);
        this->menuWidgetClass.setDimTop(0);
        this->menuWidgetClass.setDimBottom(0);
        this->menuWidgetClass.setDimLeft(0);
        this->menuWidgetClass.setDimRight(0);
        this->menuWidgetClass.setTransTop(0);
        this->menuWidgetClass.setTransBottom(0);
        this->menuWidgetClass.setTransLeft(0);
        this->menuWidgetClass.setTransRight(0);
        this->menuWidgetClass.setZoomSelWidth("");
        this->menuWidgetClass.setZoomSelHeight("");
        this->menuWidgetClass.setZoomSelShiftX("");
        this->menuWidgetClass.setZoomSelShiftY("");
        this->menuWidgetClass.setSmoothScrolling(MMSSEQUENCEMODE_NONE);
        this->menuWidgetClass.setParentWindow("");
        this->menuWidgetClass.setSelImagePath("");
        this->menuWidgetClass.setSelImageName("");
        this->menuWidgetClass.setSmoothSelection(MMSSEQUENCEMODE_NONE);
        this->menuWidgetClass.setSmoothDelay(0);
    }

    /* MMSTextBoxWidget */
    {
        /* base widget settings */
        this->textBoxWidgetClass.widgetClass.setBgColor(color);
        this->textBoxWidgetClass.widgetClass.setSelBgColor(color);
        this->textBoxWidgetClass.widgetClass.setBgColor_p(color);
        this->textBoxWidgetClass.widgetClass.setSelBgColor_p(color);
        this->textBoxWidgetClass.widgetClass.setBgColor_i(color);
        this->textBoxWidgetClass.widgetClass.setSelBgColor_i(color);
        this->textBoxWidgetClass.widgetClass.setBgImagePath("");
        this->textBoxWidgetClass.widgetClass.setBgImageName("");
        this->textBoxWidgetClass.widgetClass.setSelBgImagePath("");
        this->textBoxWidgetClass.widgetClass.setSelBgImageName("");
        this->textBoxWidgetClass.widgetClass.setBgImagePath_p("");
        this->textBoxWidgetClass.widgetClass.setBgImageName_p("");
        this->textBoxWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->textBoxWidgetClass.widgetClass.setSelBgImageName_p("");
        this->textBoxWidgetClass.widgetClass.setBgImagePath_i("");
        this->textBoxWidgetClass.widgetClass.setBgImageName_i("");
        this->textBoxWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->textBoxWidgetClass.widgetClass.setSelBgImageName_i("");
        this->textBoxWidgetClass.widgetClass.setMargin(0);
        this->textBoxWidgetClass.widgetClass.setFocusable(true);
        this->textBoxWidgetClass.widgetClass.setSelectable(true);
        this->textBoxWidgetClass.widgetClass.setUpArrow("");
        this->textBoxWidgetClass.widgetClass.setDownArrow("");
        this->textBoxWidgetClass.widgetClass.setLeftArrow("");
        this->textBoxWidgetClass.widgetClass.setRightArrow("");
        this->textBoxWidgetClass.widgetClass.setData("");
        this->textBoxWidgetClass.widgetClass.setNavigateUp("");
        this->textBoxWidgetClass.widgetClass.setNavigateDown("");
        this->textBoxWidgetClass.widgetClass.setNavigateLeft("");
        this->textBoxWidgetClass.widgetClass.setNavigateRight("");
        this->textBoxWidgetClass.widgetClass.setVSlider("");
        this->textBoxWidgetClass.widgetClass.setHSlider("");
        this->textBoxWidgetClass.widgetClass.setImagesOnDemand(false);
        this->textBoxWidgetClass.widgetClass.setBlend(0);
        this->textBoxWidgetClass.widgetClass.setBlendFactor(0);
        this->textBoxWidgetClass.widgetClass.setScrollOnFocus(false);
        this->textBoxWidgetClass.widgetClass.setClickable(true);
        this->textBoxWidgetClass.widgetClass.setReturnOnScroll(true);
        this->textBoxWidgetClass.widgetClass.setInputMode("");
        this->textBoxWidgetClass.widgetClass.setJoinedWidget("");
        this->textBoxWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->textBoxWidgetClass.widgetClass.border.setColor(color);
        this->textBoxWidgetClass.widgetClass.border.setSelColor(color);
        this->textBoxWidgetClass.widgetClass.border.setImagePath("");
        this->textBoxWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->textBoxWidgetClass.widgetClass.border.setSelImagePath("");
        this->textBoxWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->textBoxWidgetClass.widgetClass.border.setThickness(0);
        this->textBoxWidgetClass.widgetClass.border.setMargin(0);
        this->textBoxWidgetClass.widgetClass.border.setRCorners(false);

        /* textbox settings */
        this->textBoxWidgetClass.setFontPath("./themes/default");
        this->textBoxWidgetClass.setFontName("decker.ttf");
        this->textBoxWidgetClass.setFontSize(16);
        this->textBoxWidgetClass.setAlignment(MMSALIGNMENT_CENTER);
        this->textBoxWidgetClass.setWrap(true);
        this->textBoxWidgetClass.setSplitWords(true);
        this->textBoxWidgetClass.setColor(MMSFBColor(0xc0, 0xc0, 0xc0, 0xff));
        this->textBoxWidgetClass.setSelColor(MMSFBColor(0xff, 0xff, 0xff, 0xff));
        this->textBoxWidgetClass.setColor_p(MMSFBColor());
        this->textBoxWidgetClass.setSelColor_p(MMSFBColor());
        this->textBoxWidgetClass.setColor_i(MMSFBColor(0x80, 0x80, 0x80, 0xff));
        this->textBoxWidgetClass.setSelColor_i(MMSFBColor(0xbf, 0xbf, 0xbf, 0xff));
        this->textBoxWidgetClass.setText("");
        this->textBoxWidgetClass.setTranslate(true);
        this->textBoxWidgetClass.setFilePath("");
        this->textBoxWidgetClass.setFileName("");
    }

    /* MMSArrowWidget */
    {
        /* base widget settings */
        this->arrowWidgetClass.widgetClass.setBgColor(color);
        this->arrowWidgetClass.widgetClass.setSelBgColor(color);
        this->arrowWidgetClass.widgetClass.setBgColor_p(color);
        this->arrowWidgetClass.widgetClass.setSelBgColor_p(color);
        this->arrowWidgetClass.widgetClass.setBgColor_i(color);
        this->arrowWidgetClass.widgetClass.setSelBgColor_i(color);
        this->arrowWidgetClass.widgetClass.setBgImagePath("");
        this->arrowWidgetClass.widgetClass.setBgImageName("");
        this->arrowWidgetClass.widgetClass.setSelBgImagePath("");
        this->arrowWidgetClass.widgetClass.setSelBgImageName("");
        this->arrowWidgetClass.widgetClass.setBgImagePath_p("");
        this->arrowWidgetClass.widgetClass.setBgImageName_p("");
        this->arrowWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->arrowWidgetClass.widgetClass.setSelBgImageName_p("");
        this->arrowWidgetClass.widgetClass.setBgImagePath_i("");
        this->arrowWidgetClass.widgetClass.setBgImageName_i("");
        this->arrowWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->arrowWidgetClass.widgetClass.setSelBgImageName_i("");
        this->arrowWidgetClass.widgetClass.setMargin(0);
        this->arrowWidgetClass.widgetClass.setFocusable(false);
        this->arrowWidgetClass.widgetClass.setSelectable(true);
        this->arrowWidgetClass.widgetClass.setUpArrow("");
        this->arrowWidgetClass.widgetClass.setDownArrow("");
        this->arrowWidgetClass.widgetClass.setLeftArrow("");
        this->arrowWidgetClass.widgetClass.setRightArrow("");
        this->arrowWidgetClass.widgetClass.setData("");
        this->arrowWidgetClass.widgetClass.setNavigateUp("");
        this->arrowWidgetClass.widgetClass.setNavigateDown("");
        this->arrowWidgetClass.widgetClass.setNavigateLeft("");
        this->arrowWidgetClass.widgetClass.setNavigateRight("");
        this->arrowWidgetClass.widgetClass.setVSlider("");
        this->arrowWidgetClass.widgetClass.setHSlider("");
        this->arrowWidgetClass.widgetClass.setImagesOnDemand(false);
        this->arrowWidgetClass.widgetClass.setBlend(0);
        this->arrowWidgetClass.widgetClass.setBlendFactor(0);
        this->arrowWidgetClass.widgetClass.setScrollOnFocus(false);
        this->arrowWidgetClass.widgetClass.setClickable(true);
        this->arrowWidgetClass.widgetClass.setReturnOnScroll(true);
        this->arrowWidgetClass.widgetClass.setInputMode("");
        this->arrowWidgetClass.widgetClass.setJoinedWidget("");
        this->arrowWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->arrowWidgetClass.widgetClass.border.setColor(color);
        this->arrowWidgetClass.widgetClass.border.setSelColor(color);
        this->arrowWidgetClass.widgetClass.border.setImagePath("");
        this->arrowWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->arrowWidgetClass.widgetClass.border.setSelImagePath("");
        this->arrowWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->arrowWidgetClass.widgetClass.border.setThickness(0);
        this->arrowWidgetClass.widgetClass.border.setMargin(0);
        this->arrowWidgetClass.widgetClass.border.setRCorners(false);

        /* arrow settings */
        MMSFBColor c;
        c.a = 255;
        c.r = 192;
        c.g = 192;
        c.b = 192;
        this->arrowWidgetClass.setColor(c);
        c.a = 255;
        c.r = 255;
        c.g = 255;
        c.b = 255;
        this->arrowWidgetClass.setSelColor(c);
        this->arrowWidgetClass.setDirection(MMSDIRECTION_LEFT);
        this->arrowWidgetClass.setCheckSelected(false);
    }

    /* MMSSliderWidget */
    {
        /* base widget settings */
        this->sliderWidgetClass.widgetClass.setBgColor(color);
        this->sliderWidgetClass.widgetClass.setSelBgColor(color);
        this->sliderWidgetClass.widgetClass.setBgColor_p(color);
        this->sliderWidgetClass.widgetClass.setSelBgColor_p(color);
        this->sliderWidgetClass.widgetClass.setBgColor_i(color);
        this->sliderWidgetClass.widgetClass.setSelBgColor_i(color);
        this->sliderWidgetClass.widgetClass.setBgImagePath("");
        this->sliderWidgetClass.widgetClass.setBgImageName("");
        this->sliderWidgetClass.widgetClass.setSelBgImagePath("");
        this->sliderWidgetClass.widgetClass.setSelBgImageName("");
        this->sliderWidgetClass.widgetClass.setBgImagePath_p("");
        this->sliderWidgetClass.widgetClass.setBgImageName_p("");
        this->sliderWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->sliderWidgetClass.widgetClass.setSelBgImageName_p("");
        this->sliderWidgetClass.widgetClass.setBgImagePath_i("");
        this->sliderWidgetClass.widgetClass.setBgImageName_i("");
        this->sliderWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->sliderWidgetClass.widgetClass.setSelBgImageName_i("");
        this->sliderWidgetClass.widgetClass.setMargin(0);
        this->sliderWidgetClass.widgetClass.setFocusable(false);
        this->sliderWidgetClass.widgetClass.setSelectable(true);
        this->sliderWidgetClass.widgetClass.setUpArrow("");
        this->sliderWidgetClass.widgetClass.setDownArrow("");
        this->sliderWidgetClass.widgetClass.setLeftArrow("");
        this->sliderWidgetClass.widgetClass.setRightArrow("");
        this->sliderWidgetClass.widgetClass.setData("");
        this->sliderWidgetClass.widgetClass.setNavigateUp("");
        this->sliderWidgetClass.widgetClass.setNavigateDown("");
        this->sliderWidgetClass.widgetClass.setNavigateLeft("");
        this->sliderWidgetClass.widgetClass.setNavigateRight("");
        this->sliderWidgetClass.widgetClass.setVSlider("");
        this->sliderWidgetClass.widgetClass.setHSlider("");
        this->sliderWidgetClass.widgetClass.setImagesOnDemand(false);
        this->sliderWidgetClass.widgetClass.setBlend(0);
        this->sliderWidgetClass.widgetClass.setBlendFactor(0);
        this->sliderWidgetClass.widgetClass.setScrollOnFocus(false);
        this->sliderWidgetClass.widgetClass.setClickable(true);
        this->sliderWidgetClass.widgetClass.setReturnOnScroll(true);
        this->sliderWidgetClass.widgetClass.setInputMode("");
        this->sliderWidgetClass.widgetClass.setJoinedWidget("");
        this->sliderWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->sliderWidgetClass.widgetClass.border.setColor(color);
        this->sliderWidgetClass.widgetClass.border.setSelColor(color);
        this->sliderWidgetClass.widgetClass.border.setImagePath("");
        this->sliderWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->sliderWidgetClass.widgetClass.border.setSelImagePath("");
        this->sliderWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->sliderWidgetClass.widgetClass.border.setThickness(0);
        this->sliderWidgetClass.widgetClass.border.setMargin(0);
        this->sliderWidgetClass.widgetClass.border.setRCorners(false);

        /* slider settings */
        this->sliderWidgetClass.setImagePath("");
        this->sliderWidgetClass.setImageName("");
        this->sliderWidgetClass.setSelImagePath("");
        this->sliderWidgetClass.setSelImageName("");
        this->sliderWidgetClass.setImagePath_p("");
        this->sliderWidgetClass.setImageName_p("");
        this->sliderWidgetClass.setSelImagePath_p("");
        this->sliderWidgetClass.setSelImageName_p("");
        this->sliderWidgetClass.setImagePath_i("");
        this->sliderWidgetClass.setImageName_i("");
        this->sliderWidgetClass.setSelImagePath_i("");
        this->sliderWidgetClass.setSelImageName_i("");
        this->sliderWidgetClass.setPosition(0);
    }

    /* MMSInputWidget */
    {
        /* base widget settings */
        this->inputWidgetClass.widgetClass.setBgColor(color);
        this->inputWidgetClass.widgetClass.setSelBgColor(color);
        this->inputWidgetClass.widgetClass.setBgColor_p(color);
        this->inputWidgetClass.widgetClass.setSelBgColor_p(color);
        this->inputWidgetClass.widgetClass.setBgColor_i(color);
        this->inputWidgetClass.widgetClass.setSelBgColor_i(color);
        this->inputWidgetClass.widgetClass.setBgImagePath("");
        this->inputWidgetClass.widgetClass.setBgImageName("");
        this->inputWidgetClass.widgetClass.setSelBgImagePath("");
        this->inputWidgetClass.widgetClass.setSelBgImageName("");
        this->inputWidgetClass.widgetClass.setBgImagePath_p("");
        this->inputWidgetClass.widgetClass.setBgImageName_p("");
        this->inputWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->inputWidgetClass.widgetClass.setSelBgImageName_p("");
        this->inputWidgetClass.widgetClass.setBgImagePath_i("");
        this->inputWidgetClass.widgetClass.setBgImageName_i("");
        this->inputWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->inputWidgetClass.widgetClass.setSelBgImageName_i("");
        this->inputWidgetClass.widgetClass.setMargin(0);
        this->inputWidgetClass.widgetClass.setFocusable(true);
        this->inputWidgetClass.widgetClass.setSelectable(true);
        this->inputWidgetClass.widgetClass.setUpArrow("");
        this->inputWidgetClass.widgetClass.setDownArrow("");
        this->inputWidgetClass.widgetClass.setLeftArrow("");
        this->inputWidgetClass.widgetClass.setRightArrow("");
        this->inputWidgetClass.widgetClass.setData("");
        this->inputWidgetClass.widgetClass.setNavigateUp("");
        this->inputWidgetClass.widgetClass.setNavigateDown("");
        this->inputWidgetClass.widgetClass.setNavigateLeft("");
        this->inputWidgetClass.widgetClass.setNavigateRight("");
        this->inputWidgetClass.widgetClass.setVSlider("");
        this->inputWidgetClass.widgetClass.setHSlider("");
        this->inputWidgetClass.widgetClass.setImagesOnDemand(false);
        this->inputWidgetClass.widgetClass.setBlend(0);
        this->inputWidgetClass.widgetClass.setBlendFactor(0);
        this->inputWidgetClass.widgetClass.setScrollOnFocus(false);
        this->inputWidgetClass.widgetClass.setClickable(true);
        this->inputWidgetClass.widgetClass.setReturnOnScroll(true);
        this->inputWidgetClass.widgetClass.setInputMode("");
        this->inputWidgetClass.widgetClass.setJoinedWidget("");
        this->inputWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->inputWidgetClass.widgetClass.border.setColor(color);
        this->inputWidgetClass.widgetClass.border.setSelColor(color);
        this->inputWidgetClass.widgetClass.border.setImagePath("");
        this->inputWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->inputWidgetClass.widgetClass.border.setSelImagePath("");
        this->inputWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->inputWidgetClass.widgetClass.border.setThickness(0);
        this->inputWidgetClass.widgetClass.border.setMargin(0);
        this->inputWidgetClass.widgetClass.border.setRCorners(false);

        /* input settings */
        this->inputWidgetClass.setFontPath("./themes/default");
        this->inputWidgetClass.setFontName("decker.ttf");
        this->inputWidgetClass.setFontSize(16);
        this->inputWidgetClass.setAlignment(MMSALIGNMENT_CENTER);
        this->inputWidgetClass.setColor(MMSFBColor(0xc0, 0xc0, 0xc0, 0xff));
        this->inputWidgetClass.setSelColor(MMSFBColor(0xff, 0xff, 0xff, 0xff));
        this->inputWidgetClass.setColor_p(MMSFBColor());
        this->inputWidgetClass.setSelColor_p(MMSFBColor());
        this->inputWidgetClass.setColor_i(MMSFBColor(0x80, 0x80, 0x80, 0xff));
        this->inputWidgetClass.setSelColor_i(MMSFBColor(0xbf, 0xbf, 0xbf, 0xff));
        this->inputWidgetClass.setText("");
        this->inputWidgetClass.setCursorState(MMSSTATE_AUTO);
    }


    /* MMSCheckBoxWidget */
    {
        /* base widget settings */
        this->checkBoxWidgetClass.widgetClass.setBgColor(color);
        this->checkBoxWidgetClass.widgetClass.setSelBgColor(color);
        this->checkBoxWidgetClass.widgetClass.setBgColor_p(color);
        this->checkBoxWidgetClass.widgetClass.setSelBgColor_p(color);
        this->checkBoxWidgetClass.widgetClass.setBgColor_i(color);
        this->checkBoxWidgetClass.widgetClass.setSelBgColor_i(color);
        this->checkBoxWidgetClass.widgetClass.setBgImagePath("");
        this->checkBoxWidgetClass.widgetClass.setBgImageName("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImagePath("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImageName("");
        this->checkBoxWidgetClass.widgetClass.setBgImagePath_p("");
        this->checkBoxWidgetClass.widgetClass.setBgImageName_p("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImagePath_p("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImageName_p("");
        this->checkBoxWidgetClass.widgetClass.setBgImagePath_i("");
        this->checkBoxWidgetClass.widgetClass.setBgImageName_i("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImagePath_i("");
        this->checkBoxWidgetClass.widgetClass.setSelBgImageName_i("");
        this->checkBoxWidgetClass.widgetClass.setMargin(0);
        this->checkBoxWidgetClass.widgetClass.setFocusable(true);
        this->checkBoxWidgetClass.widgetClass.setSelectable(true);
        this->checkBoxWidgetClass.widgetClass.setUpArrow("");
        this->checkBoxWidgetClass.widgetClass.setDownArrow("");
        this->checkBoxWidgetClass.widgetClass.setLeftArrow("");
        this->checkBoxWidgetClass.widgetClass.setRightArrow("");
        this->checkBoxWidgetClass.widgetClass.setData("");
        this->checkBoxWidgetClass.widgetClass.setNavigateUp("");
        this->checkBoxWidgetClass.widgetClass.setNavigateDown("");
        this->checkBoxWidgetClass.widgetClass.setNavigateLeft("");
        this->checkBoxWidgetClass.widgetClass.setNavigateRight("");
        this->checkBoxWidgetClass.widgetClass.setVSlider("");
        this->checkBoxWidgetClass.widgetClass.setHSlider("");
        this->checkBoxWidgetClass.widgetClass.setImagesOnDemand(false);
        this->checkBoxWidgetClass.widgetClass.setBlend(0);
        this->checkBoxWidgetClass.widgetClass.setBlendFactor(0);
        this->checkBoxWidgetClass.widgetClass.setScrollOnFocus(false);
        this->checkBoxWidgetClass.widgetClass.setClickable(true);
        this->checkBoxWidgetClass.widgetClass.setReturnOnScroll(true);
        this->checkBoxWidgetClass.widgetClass.setInputMode("");
        this->checkBoxWidgetClass.widgetClass.setJoinedWidget("");
        this->checkBoxWidgetClass.widgetClass.setActivated(true);

        /* base widget border settings */
        this->checkBoxWidgetClass.widgetClass.border.setColor(color);
        this->checkBoxWidgetClass.widgetClass.border.setSelColor(color);
        this->checkBoxWidgetClass.widgetClass.border.setImagePath("");
        this->checkBoxWidgetClass.widgetClass.border.setImageNames("", "", "", "", "", "", "", "");
        this->checkBoxWidgetClass.widgetClass.border.setSelImagePath("");
        this->checkBoxWidgetClass.widgetClass.border.setSelImageNames("", "", "", "", "", "", "", "");
        this->checkBoxWidgetClass.widgetClass.border.setThickness(0);
        this->checkBoxWidgetClass.widgetClass.border.setMargin(0);
        this->checkBoxWidgetClass.widgetClass.border.setRCorners(false);

        /* checkbox settings */
        this->checkBoxWidgetClass.setCheckedBgColor(color);
        this->checkBoxWidgetClass.setCheckedSelBgColor(color);
        this->checkBoxWidgetClass.setCheckedBgColor_p(color);
        this->checkBoxWidgetClass.setCheckedSelBgColor_p(color);
        this->checkBoxWidgetClass.setCheckedBgColor_i(color);
        this->checkBoxWidgetClass.setCheckedSelBgColor_i(color);
        this->checkBoxWidgetClass.setCheckedBgImagePath("");
        this->checkBoxWidgetClass.setCheckedBgImageName("");
        this->checkBoxWidgetClass.setCheckedSelBgImagePath("");
        this->checkBoxWidgetClass.setCheckedSelBgImageName("");
        this->checkBoxWidgetClass.setCheckedBgImagePath_p("");
        this->checkBoxWidgetClass.setCheckedBgImageName_p("");
        this->checkBoxWidgetClass.setCheckedSelBgImagePath_p("");
        this->checkBoxWidgetClass.setCheckedSelBgImageName_p("");
        this->checkBoxWidgetClass.setCheckedBgImagePath_i("");
        this->checkBoxWidgetClass.setCheckedBgImageName_i("");
        this->checkBoxWidgetClass.setCheckedSelBgImagePath_i("");
        this->checkBoxWidgetClass.setCheckedSelBgImageName_i("");
        this->checkBoxWidgetClass.setChecked(false);
    }


}

MMSTheme::~MMSTheme() {
}

void MMSTheme::setTheme(string path, string themeName) {
    this->path      = path;
    this->themeName = themeName;
#ifndef __L4_RE__
    if (path == "")
        this->themePath = "./themes/" + themeName;
    else
        this->themePath = path + "/themes/" + themeName;

    this->themeFile = this->themePath + "/theme.xml";
#else
    this->themePath = "";
    this->themeFile = "theme.xml";
#endif
}

string MMSTheme::getPath() {
    return this->path;
}

string MMSTheme::getThemeName() {
    return this->themeName;
}

string MMSTheme::getThemePath() {
    return this->themePath;
}

string MMSTheme::getThemeFile() {
    return this->themeFile;
}




