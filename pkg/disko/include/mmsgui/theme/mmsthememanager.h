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

#ifndef MMSTHEMEMANAGER_H_
#define MMSTHEMEMANAGER_H_

#include "mmsgui/theme/mmstheme.h"

#define DEFAULT_THEME   "default"


//! The MMSThemeManager will be used to load theme definitions from the theme file.
/*!
During mmsInit() the first instance will be created which loads the global theme.
You can instantiate a separate object to interact with the theme manager.
\author Jens Schneider
*/
class MMSThemeManager {
    private:
    	//! first static object initialized?
    	static bool					initialized;

    	//! path to the theme
        static string				themepath;

        //! additional application (e.g. plugin) specific themes
        static vector<MMSTheme*>	localThemes;

        void throughFile(MMSTaffFile *tafff, MMSTheme *theme);

        void getThemeValues(MMSTaffFile *tafff, MMSTheme *theme);
        void getDescriptionValues(MMSTaffFile *tafff, MMSTheme *theme);

        void getTemplateValues(MMSTaffFile *tafff, MMSTemplateClass *themeClass);
        void getMainWindowValues(MMSTaffFile *tafff, MMSMainWindowClass *themeClass, MMSTheme *theme);
        void getPopupWindowValues(MMSTaffFile *tafff, MMSPopupWindowClass *themeClass, MMSTheme *theme);
        void getRootWindowValues(MMSTaffFile *tafff, MMSRootWindowClass *themeClass, MMSTheme *theme);
        void getChildWindowValues(MMSTaffFile *tafff, MMSChildWindowClass *themeClass, MMSTheme *theme);
        void getLabelWidgetValues(MMSTaffFile *tafff, MMSLabelWidgetClass *themeClass, MMSTheme *theme);
        void getImageWidgetValues(MMSTaffFile *tafff, MMSImageWidgetClass *themeClass, MMSTheme *theme);
        void getButtonWidgetValues(MMSTaffFile *tafff, MMSButtonWidgetClass *themeClass, MMSTheme *theme);
        void getProgressBarWidgetValues(MMSTaffFile *tafff, MMSProgressBarWidgetClass *themeClass, MMSTheme *theme);
        void getSliderWidgetValues(MMSTaffFile *tafff, MMSSliderWidgetClass *themeClass, MMSTheme *theme);
        void getMenuWidgetValues(MMSTaffFile *tafff, MMSMenuWidgetClass *themeClass, MMSTheme *theme);
        void getTextBoxWidgetValues(MMSTaffFile *tafff, MMSTextBoxWidgetClass *themeClass, MMSTheme *theme);
        void getArrowWidgetValues(MMSTaffFile *tafff, MMSArrowWidgetClass *themeClass, MMSTheme *theme);
        void getInputWidgetValues(MMSTaffFile *tafff, MMSInputWidgetClass *themeClass, MMSTheme *theme);
        void getCheckBoxWidgetValues(MMSTaffFile *tafff, MMSCheckBoxWidgetClass *themeClass, MMSTheme *theme);

        void getTemplateClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getMainWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getPopupWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getRootWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getChildWindowClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getLabelWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getImageWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getButtonWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getProgressBarWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getSliderWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getMenuWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getTextBoxWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getArrowWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getInputWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);
        void getCheckBoxWidgetClassValues(MMSTaffFile *tafff, MMSTheme *theme, string className);


        void loadTheme(string path, string themeName, MMSTheme *theme);
        void loadGlobalTheme(string themeName);
        void loadLocalTheme(MMSTheme *theme, string path, string themeName = "");

    public:
        MMSThemeManager(string themepath, string globalThemeName = DEFAULT_THEME);
        MMSThemeManager();
        ~MMSThemeManager();

        MMSTheme *loadLocalTheme(string path, string themeName = "");
        void deleteLocalTheme(string path, string themeName);
        void deleteLocalTheme(MMSTheme **theme);

        //! Change the theme.
        /*!
        The fadein effect switcher will be used from the theme.xml definition.
        For that, you can set the attribute "fadein" for the tag <mmstheme/> to "true" or "false".
        The default is "false".
        \param themeName	name of the new theme to be activated
        \note The attribute "fadein" have to set for the new theme which is to be activated.
        \note If fails, an MMSError exception will be throw.
        */
        void setTheme(string themeName);

        //! Change the theme.
        /*!
        You can switch on/off the fading animation during the theme switch.
        \param themeName	name of the new theme to be activated
        \param fadein		the new theme should fade in?
        \note If fails, an MMSError exception will be throw.
        */
        void setTheme(string themeName, bool fadein);


        //! Set one or more callbacks for the onThemeChanged event.
        /*!
        The connected callbacks will be called during setTheme().

        A callback method must be defined like this:

			void myclass::mycallbackmethod(string themeName, bool fadein);

			\param themeName	name of the new theme
			\param fadein		the new theme should fade in?

        To connect your callback to onThemeChanged do this:

            sigc::connection connection;
            connection = mywindow->onThemeChanged->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onThemeChanged BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        static sigc::signal<void, string, bool> onThemeChanged;
};

MMS_CREATEERROR(MMSThemeManagerError);

#endif /*MMSTHEMEMANAGER_H_*/
