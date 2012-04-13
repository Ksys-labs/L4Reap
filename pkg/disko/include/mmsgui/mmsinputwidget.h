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

#ifndef MMSINPUTWIDGET_H_
#define MMSINPUTWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can display and edit one line text.
/*!
You can display and edit one line of text.
\author Jens Schneider
*/
class MMSInputWidget : public MMSWidget {

    private:
        string         		className;
        MMSInputWidgetClass *inputWidgetClass;
        MMSInputWidgetClass myInputWidgetClass;

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

        int				cursor_pos;
        bool			cursor_on;
        int 			scroll_x;
        MMSFBRectangle	cursor_rect;

        class MMSInputWidgetThread	*iwt;

        //! current foreground values set?
        bool			current_fgset;

        //! current foreground color
        MMSFBColor		current_fgcolor;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        void initLanguage(MMSInputWidget *widget = NULL);
        void loadFont(MMSInputWidget *widget = NULL);

        void handleInput(MMSInputEvent *inputevent);

        bool init();
        bool release();

        void getForeground(MMSFBColor *color);
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();

        bool draw(bool *backgroundFilled = NULL);
        void drawCursor(bool cursor_on);

    public:
        MMSInputWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSInputWidget();

        MMSWidget *copyWidget();

        void setCursorPos(int cursor_pos, bool refresh = true);
        bool addTextAfterCursorPos(string text, bool refresh = true);
        bool removeTextBeforeCursorPos(int textlen, bool refresh = true);

        //! Set one or more callbacks for the onBeforeChange event.
        /*!
        The connected callbacks will be called during handleInput() if user changes the text.
        If at least one of the callbacks returns false, the user input will be ignored
        and the text will not be changed.

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSWidget *widget, string text, bool add, MMSFBRectangle rect);

        	Parameters:

        		widget -> is the pointer to the input widget which is to be changed
        		text   -> string to be added or removed
        		add    -> true: string is to be added, false: string is to be removed
        		rect   -> affected rectangle within the widget

        	Returns:

        		true if the user input is accepted, else false if the input is to be ignored

        To connect your callback to onBeforeChange do this:

            sigc::connection connection;
            connection = mywindow->onBeforeChange->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onBeforeChange BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSWidget*, string, bool, MMSFBRectangle>::accumulated<bool_accumulator> *onBeforeChange;

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
        MMSSTATE getCursorState();
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
        void setText(string text, bool refresh = true, bool reset_cursor = true);
        void setCursorState(MMSSTATE cursor_state, bool refresh = true);
        void setShadowColor(MMSPOSITION position, MMSFBColor color, bool refresh = true);
        void setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor, bool refresh = true);

        void updateFromThemeClass(MMSInputWidgetClass *themeClass);

    friend class MMSInputWidgetThread;
};

#endif /*MMSINPUTWIDGET_H_*/
