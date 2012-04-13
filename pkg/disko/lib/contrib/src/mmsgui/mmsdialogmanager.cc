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

#include "mmsgui/mmsdialogmanager.h"
#include "mmsgui/mmswindows.h"
#include "mmsgui/mmswidgets.h"
#include "mmsgui/theme/mmsthememanager.h"
#include <string.h>
#include <algorithm>

// read widget attributes from TAFF which are not defined in theme classes
#define READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height) \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name: \
		name = attrval_str; \
		break; \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size: \
		size = attrval_str; \
		break; \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_min_width: \
		min_width = attrval_str; \
		break; \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_min_height: \
		min_height = attrval_str; \
		break; \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_max_width: \
		max_width = attrval_str; \
		break; \
	case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_max_height: \
		max_height = attrval_str; \
		break;


// set widget attributes which are not defined in theme classes
#define SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(widget, name, size, min_width, min_height, max_width, max_height) \
    if (!name.empty()) { \
		widget->setName(name); \
		insertNamedWidget(widget); \
	} \
    if (!size.empty()) { \
		if (!widget->setSizeHint(size)) \
            throw MMSDialogManagerError(1, "invalid widget size '" + size + "'"); \
    } \
	if (!min_width.empty()) { \
	    if (!widget->setMinWidth(min_width)) \
            throw MMSDialogManagerError(1, "invalid widget min_width '" + min_width + "'"); \
	} \
	if (!min_height.empty()) { \
	    if (!widget->setMinHeight(min_height)) \
            throw MMSDialogManagerError(1, "invalid widget min_height '" + min_height + "'"); \
	} \
	if (!max_width.empty()) { \
	    if (!widget->setMaxWidth(max_width)) \
            throw MMSDialogManagerError(1, "invalid widget max_width '" + max_width + "'"); \
	} \
	if (!max_height.empty()) { \
	    if (!widget->setMaxHeight(max_height)) \
            throw MMSDialogManagerError(1, "invalid widget max_height '" + max_height + "'"); \
	}


MMSDialogManager::MMSDialogManager(bool leave_window) :
	leave_window(leave_window),
	rootWindow(NULL),
	rootWindow_is_mine(true) {
}

MMSDialogManager::MMSDialogManager(MMSWindow *rootWindow) :
	leave_window(false),
	rootWindow(rootWindow),
	rootWindow_is_mine(!rootWindow) {
}

MMSDialogManager::~MMSDialogManager() {
	if (!this->leave_window) {
		// have to delete my objects
		if (this->rootWindow_is_mine) {
			// delete the rootWindow if it was initialized by me
			if (this->rootWindow)
				delete this->rootWindow;
		}
		else {
			// i should not delete the rootWindow because it was not initialized by me
			// so delete only the loaded child windows
			for(vector<MMSChildWindow*>::iterator i = this->childWins.begin(); i != this->childWins.end(); ++i) {
				delete *i;
			}
			this->childWins.clear();
		}
	}
}

bool MMSDialogManager::isLoaded() {
	return (this->rootWindow != NULL);
}

void MMSDialogManager::insertNamedWidget(MMSWidget *widget) {
    this->namedWidgets.push_back(widget);
}


MMSWidget* MMSDialogManager::findWidget(string name) {
    if (this->rootWindow)
        return this->rootWindow->findWidget(name);
    else
        return NULL;
}

MMSWidget* MMSDialogManager::operator[](string name) {
    MMSWidget *widget;

    if ((widget = findWidget(name)))
        return widget;
    throw MMSDialogManagerError(1, "widget " + name + " not found");
}



MMSWindow* MMSDialogManager::loadDialog(string filename, MMSTheme *theme) {

	if (this->leave_window) {
		// reset all values if a window is already loaded
		if (this->rootWindow) {
			childWins.clear();
			namedWidgets.clear();
			description.unsetAll();
			this->filename = "";
			if (this->rootWindow_is_mine)
				this->rootWindow = NULL;
		}
	}

	// get taff file name
    string tafffilename = filename + ".taff";

    //check for file
    if(!file_exist(filename))
        if(!file_exist(tafffilename))
        	throw MMSDialogManagerError(1, "dialog file (" + filename + ") not found");

    // open the taff file
	MMSTaffFile *tafff = new MMSTaffFile(tafffilename, &mmsgui_taff_description,
										 filename, MMSTAFF_EXTERNAL_TYPE_XML);

	if (!tafff)
        throw MMSDialogManagerError(1, "could not load dialog file " + filename);

	if (!tafff->isLoaded()) {
		delete tafff;
        throw MMSDialogManagerError(1, "could not load dialog file " + filename);
	}

	// get root tag
	int tagid = tafff->getFirstTag();
	if (tagid < 0) {
		delete tafff;
        throw MMSDialogManagerError(1, "invalid taff file " + tafffilename);
	}

	// check if the correct tag
	if (tagid != MMSGUI_TAGTABLE_TAG_MMSDIALOG) {
    	DEBUGMSG("MMSGUI", "no valid dialog file: %s", filename.c_str());
        return NULL;
    }

	// save the filename
	this->filename = filename;

	// through the doc
    this->throughDoc(tafff, NULL, this->rootWindow, theme);

    // free the document
	delete tafff;

	return rootWindow;
}

MMSChildWindow* MMSDialogManager::loadChildDialog(string filename, MMSTheme *theme) {

	unsigned int cw_size = childWins.size();

	// get taff file name
    string tafffilename = filename + ".taff";

    //check for file
    if(!file_exist(filename))
        if(!file_exist(tafffilename))
        	throw MMSDialogManagerError(1, "dialog file (" + filename + ") not found");

    // open the taff file
	MMSTaffFile *tafff = new MMSTaffFile(tafffilename, &mmsgui_taff_description,
										 filename, MMSTAFF_EXTERNAL_TYPE_XML);

	if (!tafff)
        throw MMSDialogManagerError(1, "could not load dialog file " + filename);

	if (!tafff->isLoaded()) {
		delete tafff;
        throw MMSDialogManagerError(1, "could not load dialog file " + filename);
	}

	// get root tag
	int tagid = tafff->getFirstTag();
	if (tagid < 0) {
		delete tafff;
        throw MMSDialogManagerError(1, "invalid taff file " + tafffilename);
	}

	// check if the correct tag
	if (tagid != MMSGUI_TAGTABLE_TAG_MMSDIALOG) {
    	DEBUGMSG("MMSGUI", "no valid dialog file: %s", filename.c_str());
        return NULL;
    }

	// through the doc
//printf("loadChildDialog(), root=%x, file=%s\n", this->rootWindow, filename.c_str());
    this->throughDoc(tafff, NULL, this->rootWindow, theme);

    // free the document
	delete tafff;

    if (cw_size < childWins.size())
        return childWins.at(cw_size);
    else
        return NULL;
}

MMSDescriptionClass MMSDialogManager::getDescription() {
    return this->description;
}


void MMSDialogManager::throughDoc(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow,
							      MMSTheme *theme, bool only_first_child) {
    vector<string> widgetNames;
    string widgetName;
    bool loop = true;

    // iterate through childs
	while (loop) {
		if (only_first_child) loop = false;

		bool eof;
		int tid = tafff->getNextTag(eof);
		if (eof) break;
		if (tid < 0) break;

		switch (tid) {
		case MMSGUI_TAGTABLE_TAG_DESCRIPTION:
			getDescriptionValues(tafff, theme);
			// get close tag of description
			tafff->getNextTag(eof);
			break;
		case MMSGUI_TAGTABLE_TAG_MAINWINDOW:
            getMainWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_POPUPWINDOW:
            getPopupWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_ROOTWINDOW:
            getRootWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_CHILDWINDOW:
            getChildWindowValues(tafff, rootWindow, theme);
            break;
		default: {
                widgetName="";
        		switch (tid) {
        		case MMSGUI_TAGTABLE_TAG_TEMPLATE:
                    widgetName = getTemplateValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_VBOXWIDGET:
                    widgetName = getVBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_HBOXWIDGET:
        	        widgetName = getHBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_LABELWIDGET:
        	        widgetName = getLabelValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_BUTTONWIDGET:
        	        widgetName = getButtonValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_IMAGEWIDGET:
        	        widgetName = getImageValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_PROGRESSBARWIDGET:
        	        widgetName = getProgressBarValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_MENUWIDGET:
                    widgetName = getMenuValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_TEXTBOXWIDGET:
                    widgetName = getTextBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_ARROWWIDGET:
                    widgetName = getArrowValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_SLIDERWIDGET:
                    widgetName = getSliderValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_INPUTWIDGET:
        	        widgetName = getInputValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_CHECKBOXWIDGET:
                    widgetName = getCheckBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_GAPWIDGET:
                    widgetName = getGapValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		}

                if(!widgetName.empty()) {
                    // search for duplicate names for the same parent
                    if(find(widgetNames.begin(), widgetNames.end(), widgetName) != widgetNames.end()) {
                        throw MMSDialogManagerError(1, "duplicate widget name: " + widgetName);
                    }
                    widgetNames.push_back(widgetName);
                }

            }
			break;
		}
	}
}

void MMSDialogManager::getDescriptionValues(MMSTaffFile *tafff, MMSTheme *theme) {

    this->description.setAttributesFromTAFF(tafff);
}

void MMSDialogManager::getMainWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSMainWindowClass themeClass;
    string             name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw MMSDialogManagerError(1, "found nested windows, new mainwindow rejected");

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;
    bool bb;
    bool *bbp = NULL;
    if (themeClass.windowClass.getBackBuffer(bb))
    	bbp = &bb;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSMainWindow(themeClass.getClassName(),
                                             dx,
                                             dy,
                                             width,
                                             height,
                                             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp,
                                             bbp);
    else
        this->rootWindow = new MMSMainWindow(themeClass.getClassName(),
                                             width,
                                             height,
                                             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp,
                                             bbp);

    this->rootWindow->setName(name);
    ((MMSMainWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}


void MMSDialogManager::getPopupWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSPopupWindowClass themeClass;
    string              name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw MMSDialogManagerError(1, "found nested windows, new popupwindow rejected");

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;
    bool bb;
    bool *bbp = NULL;
    if (themeClass.windowClass.getBackBuffer(bb))
    	bbp = &bb;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSPopupWindow(themeClass.getClassName(),
							                  dx,
								              dy,
								              width,
								              height,
								              alignment,
                                              MMSW_NONE,
                                              theme,
                                              osp,
                                              bbp,
                                              0);
    else
        this->rootWindow = new MMSPopupWindow(themeClass.getClassName(),
								              width,
								              height,
								              alignment,
                                              MMSW_NONE,
                                              theme,
                                              osp,
                                              bbp,
                                              0);

    this->rootWindow->setName(name);
    ((MMSPopupWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}

void MMSDialogManager::getRootWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSRootWindowClass themeClass;
    string             name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw MMSDialogManagerError(1, "found nested windows, new rootwindow rejected");

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;
    bool bb;
    bool *bbp = NULL;
    if (themeClass.windowClass.getBackBuffer(bb))
    	bbp = &bb;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSRootWindow(themeClass.getClassName(),
								             dx,
								             dy,
								             width,
								             height,
								             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp,
                                             bbp);
    else
        this->rootWindow = new MMSRootWindow(themeClass.getClassName(),
								             width,
								             height,
								             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp,
                                             bbp);

    this->rootWindow->setName(name);
    ((MMSRootWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}

void MMSDialogManager::getChildWindowValues(MMSTaffFile *tafff, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSChildWindowClass themeClass;
    MMSChildWindow      *childwin;
    string              name = "", dx = "", dy = "", width = "", height = "";
    bool                show = false;

    if(!rootWindow)
        throw MMSDialogManagerError(1, "no parent window found, new childwindow rejected");

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;
    bool bb;
    bool *bbp = NULL;
    if (themeClass.windowClass.getBackBuffer(bb))
    	bbp = &bb;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_show:
            show = (attrval_int) ? true : false;
			break;
	    }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        childwin = new MMSChildWindow(themeClass.getClassName(),
                                      rootWindow,
                                      dx,
                                      dy,
                                      width,
                                      height,
                                      alignment,
                                      MMSW_NONE,
                                      theme,
                                      osp,
                                      bbp);
    else
        childwin = new MMSChildWindow(themeClass.getClassName(),
                                      rootWindow,
                                      width,
                                      height,
                                      alignment,
                                      MMSW_NONE,
                                      theme,
                                      osp,
                                      bbp);

    // store only the 'root' child window in my list
    if (this->rootWindow == rootWindow)
    	childWins.push_back(childwin);

    childwin->setName(name);
    childwin->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, childwin, theme);

    if (show) childwin->show();
}

string MMSDialogManager::getTemplateValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSTemplateClass    themeClass, *templateClass = NULL;
    string              name, size, min_width, min_height, max_width, max_height;
    bool				show = false;
    MMSTaffFile        	*tf;
    vector<string>      widgetNames;

    // read settings from dialog
    themeClass.setAttributesFromTAFF(tafff);

    // is a class name given?
    if (!themeClass.getClassName().empty()) {
		// can load templateClass?
		if (theme) {
			templateClass = theme->getTemplateClass(themeClass.getClassName());
		}
		else {
			templateClass = globalTheme->getTemplateClass(themeClass.getClassName());
		}
    }

    if (templateClass) {
		// are there any childs stored in the templateClass?
		if (!(tf = templateClass->getTAFF()))
			templateClass = NULL;
    }

    if (!templateClass) {
        // parse the children from dialog's template tag
    	throughDoc(tafff, currentWidget, rootWindow, theme);

    	return "";
    }

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);

        case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_show:
	        show = (attrval_int)?true:false;
			break;
        }
    }
    endTAFFScan
    startTAFFScan_WITHOUT_ID
    {
        if (memcmp(attrname, "widget.", 7)==0) {
            // search for attributes which are to be set for templates child widgets
            string widgetName = &attrname[7];
            int pos = (int)widgetName.find(".");
            if (pos > 0) {
                widgetName = widgetName.substr(0, pos);

                // store the requested widget name here
                if(find(widgetNames.begin(), widgetNames.end(), widgetName) == widgetNames.end()) {
					widgetNames.push_back(widgetName);
                }
            }
        }
    }
    endTAFFScan_WITHOUT_ID

	// parse the children from templateClass
	throughDoc(tf, currentWidget, rootWindow, theme);

	// get the last window of root window
	MMSWindow *newWindow = (!currentWidget)?rootWindow->getLastWindow():NULL;

    if (!newWindow) {
		// get the last widget of currentWidget
		MMSWidget *newWidget = (!currentWidget)?((*rootWindow)[""]):currentWidget->getLastWidget();

		if (newWidget) {
			// set widget attributes which are not defined in theme classes
			SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(newWidget, name, size, min_width, min_height, max_width, max_height);

			// for each child widget which is named by attribute
			vector<string>::iterator i;
			vector<string>::iterator end = widgetNames.end();
			for (i = widgetNames.begin(); i != end; ++i) {
				updateTAFFAttributes(tafff, newWidget->findWidget(*i), *i);
			}
		}
		else {
			throw MMSDialogManagerError(1, "template error, no widget");
		}
	}
	else {
	    if (!name.empty()) {
	    	// set name of window
	    	newWindow->setName(name);
	    }

    	if (show) {
	    	// show window
    		newWindow->show();
	    }

		// for each child widget which is named by attribute
		vector<string>::iterator i;
		vector<string>::iterator end = widgetNames.end();
		for (i = widgetNames.begin(); i != end; ++i) {
			updateTAFFAttributes(tafff, newWindow->findWidget(*i), *i);
		}
	}

	// parse the children from dialog's template tag
	throughDoc(tafff, currentWidget, rootWindow, theme);

	// return the name of the widget
    return name;
}


string MMSDialogManager::getVBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSVBoxWidget *vbox;
    string name, size, min_width, min_height, max_width, max_height;

    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	vbox = new MMSVBoxWidget(rootWindow);

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(vbox, name, size, min_width, min_height, max_width, max_height);

	if (currentWidget)
        currentWidget->add(vbox);
    else
        rootWindow->add(vbox);

	throughDoc(tafff, vbox, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getHBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSHBoxWidget *hbox;
    string name, size, min_width, min_height, max_width, max_height;

    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	hbox = new MMSHBoxWidget(rootWindow);

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(hbox, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(hbox);
    else
        rootWindow->add(hbox);

	throughDoc(tafff, hbox, rootWindow, theme);

    // return the name of the widget
    return name;
}


string MMSDialogManager::getLabelValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSLabelWidgetClass   themeClass;
    MMSLabelWidget  *label;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new label from theme class
    label = new MMSLabelWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    label->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(label, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(label);
    else
        rootWindow->add(label);

    throughDoc(tafff, label, rootWindow, theme);

    // return the name of the widget
    return name;
}



string MMSDialogManager::getButtonValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSButtonWidgetClass  themeClass;
    MMSButtonWidget *button;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff,  NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new button from theme class
    button = new MMSButtonWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    button->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(button, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(button);
    else
        rootWindow->add(button);

    throughDoc(tafff, button, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getImageValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSImageWidgetClass   themeClass;
    MMSImageWidget  *image;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new image from theme class
    image = new MMSImageWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    image->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(image, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(image);
    else
        rootWindow->add(image);

    throughDoc(tafff, image, rootWindow, theme);

    // return the name of the widget
    return name;
}


string MMSDialogManager::getProgressBarValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSProgressBarWidgetClass 	themeClass;
    MMSProgressBarWidget	*pBar;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new progressbar from theme class
    pBar = new MMSProgressBarWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    pBar->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(pBar, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(pBar);
    else
        rootWindow->add(pBar);

    throughDoc(tafff, pBar, rootWindow, theme);

    // return the name of the widget
    return name;
}


string MMSDialogManager::getMenuValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSMenuWidgetClass    themeClass;
    MMSMenuWidget   *menu;
    string name, size, min_width, min_height, max_width, max_height;
    MMSTaffFile    	*tf;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new menu from theme class
    menu = new MMSMenuWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    menu->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(menu, name, size, min_width, min_height, max_width, max_height);

	if (currentWidget)
        currentWidget->add(menu);
    else
        rootWindow->add(menu);

    // set the item layout, we need a temporary parent widget
    MMSHBoxWidget *tmpWidget = new MMSHBoxWidget(NULL);

    // are there any childs stored in the theme?
    if ((tf = menu->getTAFF())) {
        // yes, parse the childs from theme
        throughDoc(tf, tmpWidget, NULL, theme);
    }
    else {
        // no, parse the childs from dialog file
    	throughDoc(tafff, tmpWidget, NULL, theme, true);
    }

    bool haveItemTemplate = false;
    MMSWidget *itemTemplate = tmpWidget->disconnectChild();
    if (itemTemplate) {
        menu->setItemTemplate(itemTemplate);
        haveItemTemplate = true;
    }
    else {
        if (tf) {
            // try with theme failed, retry with childs from dialog file
            throughDoc(tafff, tmpWidget, NULL, theme);
            MMSWidget *itemTemplate = tmpWidget->disconnectChild();
            if (itemTemplate) {
                menu->setItemTemplate(itemTemplate);
                haveItemTemplate = true;
            }
        }
    }

    delete tmpWidget;

    if (haveItemTemplate) {
        // have a template - search for menu items which are set in the dialog file
        bool haveItems = false;
        bool returntag = true;

        // iterate through childs
    	while (1) {
    		bool eof;
    		int tid = tafff->getNextTag(eof);
    		if (eof) break;
    		if (tid < 0) {
    			if (returntag) break;
    			returntag = true;
    			continue;
    		}
    		else
    			returntag = false;

    		// check if a <menuitem/> is given
            if (tid == MMSGUI_TAGTABLE_TAG_MENUITEM)
            {
				// new menu item
            	MMSWidget *item = NULL;
	            haveItems = true;

				if (tafff->hasAttributes()) {
					// menu item has attributes, so we assume that the new item should be created with item template style
					// create new menu item
					item = menu->newItem();

					// here we must loop for n widgets
					vector<string> wgs;
					bool wg_break = false;
					while (!wg_break) {
						wg_break = true;
						startTAFFScan_WITHOUT_ID
						{
							if (memcmp(attrname, "widget.", 7)==0) {
								// search for attributes which are to be set for menu items child widgets
								string widgetName = &attrname[7];
								int pos = (int)widgetName.find(".");
								if (pos > 0) {
									// widget name found
									widgetName = widgetName.substr(0, pos);

									// check if i have already processed this widget
									if(find(wgs.begin(), wgs.end(), widgetName) != wgs.end()) {
										widgetName = "";
										continue;
									}
									wg_break = false;
									wgs.push_back(widgetName);

									// okay, searching for the widget within the new item
									MMSWidget *widget;
									if (item->getName() == widgetName) {
										widget = item;
									}else {
										widget = item->findWidget(widgetName);
									}

									updateTAFFAttributes(tafff, widget, widgetName);
								}
							}
						}
						endTAFFScan_WITHOUT_ID
					}

					startTAFFScan_WITHOUT_ID
					{
						if (memcmp(attrname, "childwindow", 11)==0) {
							// there is a child window given which represents a sub menu
							menu->setSubMenuName(menu->getSize()-1, attrval_str);
						}
						else
						if (memcmp(attrname, "goback", 6)==0) {
							// if true, this item should be the go-back-item
							//! if the user enters this item, the parent menu (if does exist) will be shown
							if (memcmp(attrval_str, "true", 4)==0)
								menu->setBackItem(menu->getSize()-1);
						}
					}
					endTAFFScan_WITHOUT_ID

					startTAFFScan
					{
						switch (attrid) {
						case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
							if (*attrval_str)
								item->setName(attrval_str);
							break;
						}
					}
					endTAFFScan
				}
				else {
					// menu item has NO attributes, so we try to read a specific item style from the <menuitem/> children
					// checking for tags within <menuitem/>
					MMSHBoxWidget *tmpWidget = new MMSHBoxWidget(NULL);

					// parse the childs from dialog file
					throughDoc(tafff, tmpWidget, NULL, theme);
					returntag = true;

					// here we create a new menu item based on disconnected child (if != NULL) or item template style
					item = menu->newItem(-1, tmpWidget->disconnectChild());

					// delete the temporary container
					delete tmpWidget;
				}
            }
            else {
            	// any other widgets given in the menu
            	printf("Warning: Tag <%s/> is not supported within <menu/>.\n", tafff->getCurrentTagName());

            	// we need a temporary widget
                MMSHBoxWidget *tmpWidget = new MMSHBoxWidget(NULL);

                // parse the childs from dialog file
            	throughDoc(tafff, tmpWidget, NULL, theme);
            	returntag = true;

            	// delete the widget, we cannot use it
                delete tmpWidget;
            }
    	}

        if (haveItems)
            menu->setFocus(false, false);
    }

    // return the name of the widget
    return name;
}


string MMSDialogManager::getTextBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSTextBoxWidgetClass 	themeClass;
    MMSTextBoxWidget	*textbox;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new textbox from theme class
    textbox = new MMSTextBoxWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    textbox->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(textbox, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(textbox);
    else
        rootWindow->add(textbox);

    throughDoc(tafff, textbox, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getArrowValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSArrowWidgetClass   themeClass;
    MMSArrowWidget  *arrow;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new arrow from theme class
    arrow = new MMSArrowWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    arrow->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(arrow, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(arrow);
    else
        rootWindow->add(arrow);

    throughDoc(tafff, arrow, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getSliderValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSSliderWidgetClass  themeClass;
    MMSSliderWidget *slider;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new slider from theme class
    slider = new MMSSliderWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    slider->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(slider, name, size, min_width, min_height, max_width, max_height);

    if (currentWidget)
        currentWidget->add(slider);
    else
        rootWindow->add(slider);

    throughDoc(tafff, slider, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getInputValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSInputWidgetClass   themeClass;
    MMSInputWidget  *input;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new label from theme class
    input = new MMSInputWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    input->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(input, name, size, min_width, min_height, max_width, max_height);

	if (currentWidget)
        currentWidget->add(input);
    else
        rootWindow->add(input);

    throughDoc(tafff, input, rootWindow, theme);

    // return the name of the widget
    return name;
}


string MMSDialogManager::getCheckBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSCheckBoxWidgetClass 	themeClass;
    MMSCheckBoxWidget		*checkbox;
    string name, size, min_width, min_height, max_width, max_height;

    // get themepath
    string themePath = (theme ? theme->getThemePath() : globalTheme->getThemePath());

    // read settings from dialog
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    // create new checkbox from theme class
    checkbox = new MMSCheckBoxWidget(rootWindow, themeClass.getClassName(), theme);

    // apply settings from dialog
    checkbox->updateFromThemeClass(&themeClass);

    // search for attributes which are only supported within dialog
    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(checkbox, name, size, min_width, min_height, max_width, max_height);

	if (currentWidget)
        currentWidget->add(checkbox);
    else
        rootWindow->add(checkbox);

    throughDoc(tafff, checkbox, rootWindow, theme);

    // return the name of the widget
    return name;
}

string MMSDialogManager::getGapValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSGapWidget *gap;
    string name, size, min_width, min_height, max_width, max_height;

    startTAFFScan
    {
        switch (attrid) {
        // read widget attributes from TAFF which are not defined in theme classes
        READ_DM_SPECIFIC_WIDGET_ATTRIBUTES(name, size, min_width, min_height, max_width, max_height);
	    }
    }
    endTAFFScan

	gap = new MMSGapWidget(rootWindow);

	// set widget attributes which are not defined in theme classes
	SET_DM_SPECIFIC_WIDGET_ATTRIBUTES(gap, name, size, min_width, min_height, max_width, max_height);

	if (currentWidget)
        currentWidget->add(gap);
    else
        rootWindow->add(gap);

    throughDoc(tafff, gap, rootWindow, theme);

    // return the name of the widget
    return name;
}


MMSWindow *MMSDialogManager::getWindow() {
	return this->rootWindow;
}


void MMSDialogManager::updateTAFFAttributes(MMSTaffFile *tafff, MMSWidget *widget, string &widgetName) {
	if(!widget) {
		return;
	}

    string prefix = "widget." + widgetName + ".";

    switch (widget->getType()) {
        case MMSWIDGETTYPE_HBOX:
        case MMSWIDGETTYPE_VBOX:
            break;
        case MMSWIDGETTYPE_BUTTON:
            {
                // read attributes from node
                MMSButtonWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSButtonWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_IMAGE:
            {
                // read attributes from node
                MMSImageWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSImageWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_LABEL:
            {
                // read attributes from node
                MMSLabelWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSLabelWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_MENU:
            break;
        case MMSWIDGETTYPE_PROGRESSBAR:
            {
                // read attributes from node
                MMSProgressBarWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSProgressBarWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_TEXTBOX:
            {
                // read attributes from node
                MMSTextBoxWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSTextBoxWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_ARROW:
            {
                // read attributes from node
                MMSArrowWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSArrowWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_SLIDER:
            {
                // read attributes from node
                MMSSliderWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSSliderWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_INPUT:
            break;
        case MMSWIDGETTYPE_CHECKBOX:
            {
                // read attributes from node
                MMSCheckBoxWidgetClass themeCls;
                themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                themeCls.setAttributesFromTAFF(tafff, &prefix);
                // apply settings from node
                ((MMSCheckBoxWidget*)widget)->updateFromThemeClass(&themeCls);
            }
            break;
        case MMSWIDGETTYPE_GAP:
            break;
    }
}
