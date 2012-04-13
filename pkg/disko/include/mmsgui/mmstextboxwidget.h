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

#ifndef MMSTEXTBOXWIDGET_H_
#define MMSTEXTBOXWIDGET_H_

#include "mmsgui/mmswidget.h"


//! With this class you can display text with more than one line.
/*!
The textbox is focusable. So the user can scroll in it.
Line breaks will be done with the normal line feed (\n (0x0a)). Specify &#10; as line feed within XML.
If you want to display only one line of static text, you should use the MMSLabel widget.
\author Jens Schneider
*/
class MMSTextBoxWidget : public MMSWidget {
    private:
        typedef struct {
            MMSFBRectangle geom;
            string       word;
            unsigned int line;
            unsigned int paragraph;
        } TEXTBOX_WORDGEOM;

        string          		className;
        MMSTextBoxWidgetClass 	*textBoxWidgetClass;
        MMSTextBoxWidgetClass 	myTextBoxWidgetClass;

        //! language in which the text is to be translated
        MMSLanguage	lang;

        //! loaded font
        MMSFBFont *font;

        //! path to the loaded font file
        string fontpath;

        //! name of the loaded font file
        string fontname;

        //! requested size of the font
        unsigned int fontsize;

        //! have to (re)load font?
        bool load_font;


        vector<TEXTBOX_WORDGEOM *> wordgeom;

        string  lasttext;
        bool    surfaceChanged;

        //! the translated text will be stored here, this is used in the draw() method
        string translated_text;

        //! if true the translated_text is valid
        bool translated;

        //! swap left-right alignment
        bool swap_left_right;

        //! used to load text from a file
        MMSFile *file;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground color
        MMSFBColor		current_fgcolor;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        void initLanguage(MMSTextBoxWidget *widget = NULL);
        void loadFont(MMSTextBoxWidget *widget = NULL);

        bool setSurfaceGeometry(unsigned int width = 0, unsigned int height = 0);

        bool calcWordGeom(string &text, unsigned int startWidth, unsigned int startHeight,
                          unsigned int *realWidth, unsigned int *realHeight,
                          unsigned int *scrollDX, unsigned int *scrollDY, unsigned int *lines, unsigned int *paragraphs,
                          bool wrap = true, bool splitwords = true, MMSALIGNMENT alignment = MMSALIGNMENT_CENTER,
                          unsigned int *minWidth = NULL, unsigned int *minHeight = NULL, bool force_recalc = false);

        bool init();
        bool release();

        bool prepareText(int *width, int *height, bool recalc = false);
        void calcContentSize();

        void getForeground(MMSFBColor *color);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);

		//! Internal method: Inform the widget, that the language has changed.
		void targetLangChanged(MMSLanguage lang);

		bool loadFile(bool refresh);

    public:
        MMSTextBoxWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSTextBoxWidget();

        MMSWidget *copyWidget();

    public:
		//! reload the file and display it in the textbox
		bool reloadFile();

        // theme access methods
        string getFontPath();
        string getFontName(MMSLanguage lang = MMSLANG_NONE);
        unsigned int getFontSize();
        MMSALIGNMENT getAlignment();
        bool getWrap();
        bool getSplitWords();
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        MMSFBColor getColor_p();
        MMSFBColor getSelColor_p();
        MMSFBColor getColor_i();
        MMSFBColor getSelColor_i();
        string getText();
        void getText(string &text);
        bool getTranslate();
        string getFilePath();
        string getFileName();
        MMSFBColor getShadowColor(MMSPOSITION position);
        MMSFBColor getSelShadowColor(MMSPOSITION position);

		void setFontPath(string fontpath, bool load = true, bool refresh = true);
        void setFontName(MMSLanguage lang, string fontname, bool load = true, bool refresh = true);
        void setFontName(string fontname, bool load = true, bool refresh = true);
        void setFontSize(unsigned int  fontsize, bool load = true, bool refresh = true);
        void setFont(MMSLanguage lang, string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setFont(string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true);
        void setWrap(bool wrap, bool refresh = true);
        void setSplitWords(bool splitwords, bool refresh = true);
        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setColor_p(MMSFBColor color_p, bool refresh = true);
        void setSelColor_p(MMSFBColor selcolor_p, bool refresh = true);
        void setColor_i(MMSFBColor color_i, bool refresh = true);
        void setSelColor_i(MMSFBColor selcolor_i, bool refresh = true);
        void setText(string *text, bool refresh = true);
        void setText(string text, bool refresh = true);
        void setTranslate(bool translate, bool refresh = true);
		void setFilePath(string filepath, bool load = true, bool refresh = true);
        void setFileName(string filename, bool load = true, bool refresh = true);
        void setShadowColor(MMSPOSITION position, MMSFBColor color, bool refresh = true);
        void setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor, bool refresh = true);

        void updateFromThemeClass(MMSTextBoxWidgetClass *themeClass);

	// friends
	friend class MMSWindow;
};

#endif /*MMSTEXTBOXWIDGET_H_*/
