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

#ifndef MMSTHEME_H_
#define MMSTHEME_H_

#include "mmstools/mmserror.h"
#include "mmsgui/theme/mmsthemeclass.h"
#include "mmsgui/theme/mmsdescriptionclass.h"
#include "mmsgui/theme/mmstemplateclass.h"
#include "mmsgui/theme/mmsmainwindowclass.h"
#include "mmsgui/theme/mmspopupwindowclass.h"
#include "mmsgui/theme/mmsrootwindowclass.h"
#include "mmsgui/theme/mmschildwindowclass.h"
#include "mmsgui/theme/mmslabelwidgetclass.h"
#include "mmsgui/theme/mmsimagewidgetclass.h"
#include "mmsgui/theme/mmsbuttonwidgetclass.h"
#include "mmsgui/theme/mmsprogressbarwidgetclass.h"
#include "mmsgui/theme/mmsmenuwidgetclass.h"
#include "mmsgui/theme/mmstextboxwidgetclass.h"
#include "mmsgui/theme/mmsarrowwidgetclass.h"
#include "mmsgui/theme/mmssliderwidgetclass.h"
#include "mmsgui/theme/mmsinputwidgetclass.h"
#include "mmsgui/theme/mmscheckboxwidgetclass.h"


MMS_CREATEERROR(MMSThemeError);


extern TAFF_DESCRIPTION mmsgui_taff_description;

typedef enum {
	MMSGUI_TAGTABLE_TAG_MMSTHEME,
	MMSGUI_TAGTABLE_TAG_MMSDIALOG,
	MMSGUI_TAGTABLE_TAG_DESCRIPTION,
	MMSGUI_TAGTABLE_TAG_VBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_HBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_TEMPLATE,
	MMSGUI_TAGTABLE_TAG_CLASS_TEMPLATE,
	MMSGUI_TAGTABLE_TAG_MAINWINDOW,
	MMSGUI_TAGTABLE_TAG_CLASS_MAINWINDOW,
	MMSGUI_TAGTABLE_TAG_CHILDWINDOW,
	MMSGUI_TAGTABLE_TAG_CLASS_CHILDWINDOW,
	MMSGUI_TAGTABLE_TAG_POPUPWINDOW,
	MMSGUI_TAGTABLE_TAG_CLASS_POPUPWINDOW,
	MMSGUI_TAGTABLE_TAG_ROOTWINDOW,
	MMSGUI_TAGTABLE_TAG_CLASS_ROOTWINDOW,
	MMSGUI_TAGTABLE_TAG_ARROWWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_ARROWWIDGET,
	MMSGUI_TAGTABLE_TAG_BUTTONWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_BUTTONWIDGET,
	MMSGUI_TAGTABLE_TAG_IMAGEWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_IMAGEWIDGET,
	MMSGUI_TAGTABLE_TAG_LABELWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_LABELWIDGET,
	MMSGUI_TAGTABLE_TAG_MENUWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_MENUWIDGET,
	MMSGUI_TAGTABLE_TAG_MENUITEM,
	MMSGUI_TAGTABLE_TAG_PROGRESSBARWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_PROGRESSBARWIDGET,
	MMSGUI_TAGTABLE_TAG_SLIDERWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_SLIDERWIDGET,
	MMSGUI_TAGTABLE_TAG_TEXTBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_TEXTBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_INPUTWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_INPUTWIDGET,
	MMSGUI_TAGTABLE_TAG_CHECKBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_CLASS_CHECKBOXWIDGET,
	MMSGUI_TAGTABLE_TAG_GAPWIDGET
} MMSGUI_TAGTABLE_TAG;




class MMSTheme {
    private:
    	//! path to the theme
        string              path;

        //! name of the theme e.g. "default"
        string              themeName;

        //! path to theme files
        string              themePath;

        //! complete name of the theme file
        string              themeFile;

        //! attributes of the mmstheme tag
        MMSThemeClass 		theme_tag;

        //! description of the theme
        MMSDescriptionClass description;

        vector<MMSTemplateClass*>       	templateClasses;
        vector<MMSMainWindowClass*>     	mainWindowClasses;
        vector<MMSPopupWindowClass*>    	popupWindowClasses;
        vector<MMSRootWindowClass*>     	rootWindowClasses;
        vector<MMSChildWindowClass*>    	childWindowClasses;
        vector<MMSLabelWidgetClass*>    	labelWidgetClasses;
        vector<MMSImageWidgetClass*>    	imageWidgetClasses;
        vector<MMSButtonWidgetClass*>   	buttonWidgetClasses;
        vector<MMSProgressBarWidgetClass*>  progressBarWidgetClasses;
        vector<MMSMenuWidgetClass*>         menuWidgetClasses;
        vector<MMSTextBoxWidgetClass*>      textBoxWidgetClasses;
        vector<MMSArrowWidgetClass*>        arrowWidgetClasses;
        vector<MMSSliderWidgetClass*>       sliderWidgetClasses;
        vector<MMSInputWidgetClass*>        inputWidgetClasses;
        vector<MMSCheckBoxWidgetClass*>     checkBoxWidgetClasses;

        bool addTemplateClass(MMSTemplateClass *themeClass);
        bool addMainWindowClass(MMSMainWindowClass *themeClass);
        bool addPopupWindowClass(MMSPopupWindowClass *themeClass);
        bool addRootWindowClass(MMSRootWindowClass *themeClass);
        bool addChildWindowClass(MMSChildWindowClass *themeClass);
        bool addLabelWidgetClass(MMSLabelWidgetClass *themeClass);
        bool addImageWidgetClass(MMSImageWidgetClass *themeClass);
        bool addButtonWidgetClass(MMSButtonWidgetClass *themeClass);
        bool addProgressBarWidgetClass(MMSProgressBarWidgetClass *themeClass);
        bool addMenuWidgetClass(MMSMenuWidgetClass *themeClass);
        bool addTextBoxWidgetClass(MMSTextBoxWidgetClass *themeClass);
        bool addArrowWidgetClass(MMSArrowWidgetClass *themeClass);
        bool addSliderWidgetClass(MMSSliderWidgetClass *themeClass);
        bool addInputWidgetClass(MMSInputWidgetClass *themeClass);
        bool addCheckBoxWidgetClass(MMSCheckBoxWidgetClass *themeClass);

    public:

        MMSMainWindowClass  		mainWindowClass;
        MMSPopupWindowClass 		popupWindowClass;
        MMSRootWindowClass  		rootWindowClass;
        MMSChildWindowClass 		childWindowClass;
        MMSLabelWidgetClass       	labelWidgetClass;
        MMSImageWidgetClass       	imageWidgetClass;
        MMSButtonWidgetClass      	buttonWidgetClass;
        MMSProgressBarWidgetClass	progressBarWidgetClass;
        MMSMenuWidgetClass       	menuWidgetClass;
        MMSTextBoxWidgetClass     	textBoxWidgetClass;
        MMSArrowWidgetClass       	arrowWidgetClass;
        MMSSliderWidgetClass      	sliderWidgetClass;
        MMSInputWidgetClass       	inputWidgetClass;
        MMSCheckBoxWidgetClass      checkBoxWidgetClass;

        MMSTemplateClass*       	getTemplateClass(string className = "");
        MMSMainWindowClass*     	getMainWindowClass(string className = "");
        MMSPopupWindowClass*    	getPopupWindowClass(string className = "");
        MMSRootWindowClass*     	getRootWindowClass(string className = "");
        MMSChildWindowClass*    	getChildWindowClass(string className = "");
        MMSLabelWidgetClass*      	getLabelWidgetClass(string className = "");
        MMSImageWidgetClass*        getImageWidgetClass(string className = "");
        MMSButtonWidgetClass*       getButtonWidgetClass(string className = "");
        MMSProgressBarWidgetClass*	getProgressBarWidgetClass(string className = "");
        MMSMenuWidgetClass*         getMenuWidgetClass(string className = "");
        MMSTextBoxWidgetClass*      getTextBoxWidgetClass(string className = "");
        MMSArrowWidgetClass*        getArrowWidgetClass(string className = "");
        MMSSliderWidgetClass*       getSliderWidgetClass(string className = "");
        MMSInputWidgetClass*        getInputWidgetClass(string className = "");
        MMSCheckBoxWidgetClass*     getCheckBoxWidgetClass(string className = "");

        MMSTheme(bool initial_load, bool debug);
        ~MMSTheme();

        void setTheme(string path, string themeName);
        string getPath();
        string getThemeName();
        string getThemePath();
        string getThemeFile();

    /* friends */
    friend class MMSThemeManager;
};


/* access to global theme */
extern MMSTheme *globalTheme;


#endif /*MMSTHEME_H_*/



