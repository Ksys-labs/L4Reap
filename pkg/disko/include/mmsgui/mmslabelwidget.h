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

#ifndef MMSLABELWIDGET_H_
#define MMSLABELWIDGET_H_

#include "mmsgui/mmswidget.h"
#include "mmsgui/mmslabelwidgetthread.h"

//! With this class you can display one line text.
/*!
You can display one line of text. If you want to have more the one line, you should use the MMSTextBox widget.
The label widget cannot be focused.
\author Jens Schneider
*/
class MMSLabelWidget : public MMSWidget {

    private:
        string         		className;
        MMSLabelWidgetClass *labelWidgetClass;
        MMSLabelWidgetClass myLabelWidgetClass;

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


        int slide_width;
        int slide_offset;

        unsigned int frame_delay;
        unsigned int frame_delay_set;

        class MMSLabelWidgetThread  *labelThread;

        //! the translated text will be stored here, this is used in the draw() method
        string translated_text;

        //! if true the translated_text is valid
        bool translated;

        //! swap left-right alignment
        bool swap_left_right;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground color
        MMSFBColor		current_fgcolor;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        void initLanguage(MMSLabelWidget *widget = NULL);
        void loadFont(MMSLabelWidget *widget = NULL);
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

    public:
        MMSLabelWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSLabelWidget();

        MMSWidget *copyWidget();

    public:
        /* theme access methods */
        string getFontPath();
        string getFontName(MMSLanguage lang = MMSLANG_NONE);
        unsigned int getFontSize();
        MMSALIGNMENT getAlignment();
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        MMSFBColor getColor_p();
        MMSFBColor getSelColor_p();
        MMSFBColor getColor_i();
        MMSFBColor getSelColor_i();
        string getText();
        void getText(string &text);
        bool getSlidable();
        unsigned char getSlideSpeed();
        bool getTranslate();
        MMSFBColor getShadowColor(MMSPOSITION position);
        MMSFBColor getSelShadowColor(MMSPOSITION position);

        void setFontPath(string fontpath, bool load = true, bool refresh = true);
        void setFontName(MMSLanguage lang, string fontname, bool load = true, bool refresh = true);
        void setFontName(string fontname, bool load = true, bool refresh = true);
        void setFontSize(unsigned int  fontsize, bool load = true, bool refresh = true);
        void setFont(MMSLanguage lang, string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setFont(string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true);
        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setColor_p(MMSFBColor color_p, bool refresh = true);
        void setSelColor_p(MMSFBColor selcolor_p, bool refresh = true);
        void setColor_i(MMSFBColor color_i, bool refresh = true);
        void setSelColor_i(MMSFBColor selcolor_i, bool refresh = true);
        void setText(string text, bool refresh = true);
        void setSlidable(bool slidable);
        void setSlideSpeed(unsigned char slidespeed);
        void setTranslate(bool translate, bool refresh = true);
        void setShadowColor(MMSPOSITION position, MMSFBColor color, bool refresh = true);
        void setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor, bool refresh = true);

        void updateFromThemeClass(MMSLabelWidgetClass *themeClass);

	// friends
	friend class MMSWindow;
	friend class MMSLabelWidgetThread;
};

#endif /*MMSLABELWIDGET_H_*/
