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

#ifndef MMSTEXTBOXWIDGETCLASS_H_
#define MMSTEXTBOXWIDGETCLASS_H_

#include "mmsgui/theme/mmstextbaseclass.h"

//! describe attributes for MMSTextBoxWidget which are additional to the MMSWidgetClass
namespace MMSGUI_TEXTBOXWIDGET_ATTR {

	#define MMSGUI_TEXTBOXWIDGET_ATTR_ATTRDESC \
		{ "wrap", TAFF_ATTRTYPE_BOOL }, \
		{ "splitwords", TAFF_ATTRTYPE_BOOL }, \
		{ "translate", TAFF_ATTRTYPE_BOOL }, \
		{ "file.path", TAFF_ATTRTYPE_STRING }, \
		{ "file.name", TAFF_ATTRTYPE_STRING }

	#define MMSGUI_TEXTBOXWIDGET_ATTR_IDS \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_wrap, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_splitwords, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_translate, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_file_path, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_file_name

	#define MMSGUI_TEXTBOXWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_FONT_ATTR_ATTRDESC, \
		MMSGUI_SHADOW_ATTR_ATTRDESC, \
		MMSGUI_TEXTINFO_ATTR_ATTRDESC, \
		MMSGUI_TEXTBOXWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_FONT_ATTR_IDS,
		MMSGUI_SHADOW_ATTR_IDS,
		MMSGUI_TEXTINFO_ATTR_IDS,
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_TEXTBOXWIDGET_ATTR_I[];


//! A data access class for the textbox widget.
/*!
This class is the base for the MMSTextBoxWidget class and is derived from
MMSTextBaseClass which is the base for all widgets with text output.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSTextBoxWidget.
\author Jens Schneider
*/
class MMSTextBoxWidgetClass : public MMSTextBaseClass {
    private:
    	//! name of the theme class
        string          className;

        //! is wrap flag set?
        bool            iswrap;

        //! wrap (true/false) the text
        bool            wrap;

        //! is splitwords flag set?
        bool            issplitwords;

        //! splitwords (true/false)
        bool            splitwords;

        //! is translate set?
        bool            istranslate;

        //! if true the text will be translated before displayed
        bool          	translate;

        //! is filepath set?
        bool            isfilepath;

        //! path to the file
        string          filepath;

        //! is filename set?
        bool            isfilename;

        //! name of the file
        string          filename;

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   		pointer to the TAFF buffer
        \param prefix  		optional, prefix to all attribute names (<prefix><attrname>=<attrvalue>)
        \param path    		optional, path needed for empty path values from the TAFF buffer
        \param reset_paths  optional, should reset all path attributes?
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix = NULL, string *path = NULL,
								   bool reset_paths = false);

    public:
    	//! stores base widget attributes
        MMSWidgetClass widgetClass;

        //! Constructor of class MMSTextBoxWidgetClass.
        MMSTextBoxWidgetClass();

        //! Mark all attributes as not set.
        void unsetAll();

        //! Set the name of the theme class.
        /*!
        \param classname  name of the class
        */
        void setClassName(string className);

        //! Get the name of the theme class.
        /*!
        \return name of the class
        */
        string getClassName();

        //! Check if the wrap flag is set.
        bool isWrap();

        //! Set the wrap flag.
        /*!
        \param wrap  wrap the text if set to true
        */
        void setWrap(bool wrap);

        //! Mark the wrap flag as not set.
        void unsetWrap();

        //! Get the wrap flag.
        /*!
        \return wrap flag
        */
        bool getWrap();

        //! Check if splitwords is set.
        bool isSplitWords();

        //! Set the splitwords flag.
        /*!
        \param splitwords  split words at the end of a line
        \note This works only if the wrap flag is set.
        */
        void setSplitWords(bool splitwords);

        //! Mark splitwords as not set.
        void unsetSplitWords();

        //! Get the splitwords flag.
        /*!
        \return splitwords flag
        */
        bool getSplitWords();

        //! Check if the translate flag is set.
        bool isTranslate();

        //! Set the translate flag.
        /*!
        \param translate  true/false
        */
        void setTranslate(bool translate);

        //! Mark the translate flag as not set.
        void unsetTranslate();

        //! Get the translate flag.
        /*!
        \return true/false
        */
        bool getTranslate();

        //! Check if the filepath is set.
        bool isFilePath();

        //! Set the filepath which is used to load the text.
        /*!
        \param filepath  path to the file
        */
        void setFilePath(string filepath);

        //! Mark the filepath as not set.
        void unsetFilePath();

        //! Get the filepath which is used to load the text.
        /*!
        \return path to the file
        */
        string getFilePath();

        //! Check if the filename is set.
        bool isFileName();

        //! Set the filename which is used to load the text.
        /*!
        \param filename  name of the file
        */
        void setFileName(string filename);

        //! Mark the filename as not set.
        void unsetFileName();

        //! Get the filename which is used to load the text.
        /*!
        \return name of the file
        */
        string getFileName();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSTEXTBOXWIDGETCLASS_H_*/
