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

#include "mmsgui/mmsinputwidget.h"
#include "mmsgui/mmsinputwidgetthread.h"
#include "mmsgui/mmstextbase.h"


MMSInputWidget::MMSInputWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    // initialize the callbacks
	this->onBeforeChange = new sigc::signal<bool, MMSWidget*, string, bool, MMSFBRectangle>::accumulated<bool_accumulator>;

    create(root, className, theme);
}

MMSInputWidget::~MMSInputWidget() {
    // delete the callbacks
    if (this->onBeforeChange)
    	delete this->onBeforeChange;

    if (this->iwt)
		delete this->iwt;
}

bool MMSInputWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_INPUT;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->inputWidgetClass = this->da->theme->getInputWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->inputWidgetClass.widgetClass);
    if (this->inputWidgetClass) this->da->widgetClass = &(this->inputWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    // clear
    initLanguage();
	this->fontpath = "";
	this->fontname = "";
	this->fontsize = 0;
    this->font = NULL;
    this->load_font = true;
    this->cursor_pos = 0;
	this->cursor_on = true;
	this->scroll_x = 0;

    // create thread
    this->iwt = new MMSInputWidgetThread(this);
	if (this->iwt)
		this->iwt->start();

	this->current_fgset = false;

    return MMSWidget::create(root, true, false, true, true, false, false, true);
}

MMSWidget *MMSInputWidget::copyWidget() {
    // create widget
    MMSInputWidget *newWidget = new MMSInputWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->inputWidgetClass = this->inputWidgetClass;
    newWidget->myInputWidgetClass = this->myInputWidgetClass;

    newWidget->lang = this->lang;

    newWidget->cursor_pos = this->cursor_pos;
    newWidget->cursor_on = this->cursor_on;
    newWidget->scroll_x = this->scroll_x;
    newWidget->cursor_rect = this->cursor_rect;

    newWidget->iwt = this->iwt;

    newWidget->current_fgset = this->current_fgset;
    newWidget->current_fgcolor = this->current_fgcolor;

    // copy base widget
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    // initialize the callbacks
	this->onBeforeChange = new sigc::signal<bool, MMSWidget*, string, bool, MMSFBRectangle>::accumulated<bool_accumulator>;

    // reload my font
    initLanguage(newWidget);
    newWidget->fontpath = "";
	newWidget->fontname = "";
	newWidget->fontsize = 0;
    newWidget->font = NULL;
    newWidget->load_font = true;
    if (this->rootwindow) {
    	// load font
        loadFont(newWidget);
    }

    return newWidget;
}

void MMSInputWidget::initLanguage(MMSInputWidget *widget) {
	if (!widget) widget = this;

	widget->lang = (!this->rootwindow)?MMSLANG_NONE:this->rootwindow->windowmanager->getTranslator()->getTargetLang();
}

void MMSInputWidget::loadFont(MMSInputWidget *widget) {
	if (!this->load_font) return;
	if (!widget) widget = this;

	if (this->rootwindow) {
		// get font parameter
		widget->lang = this->rootwindow->windowmanager->getTranslator()->getTargetLang();
    	string fontpath = widget->getFontPath();
    	string fontname = widget->getFontName(widget->lang);
    	unsigned int fontsize = widget->getFontSize();

    	if (fontpath != widget->fontpath || fontname != widget->fontname || fontsize != widget->fontsize || !widget->font) {
    		// font parameter changed, (re)load it
			if (widget->font)
				this->rootwindow->fm->releaseFont(widget->font);
			widget->fontpath = fontpath;
			widget->fontname = fontname;
			widget->fontsize = fontsize;
			widget->font = this->rootwindow->fm->getFont(widget->fontpath, widget->fontname, widget->fontsize);
			if (this->font) this->load_font = false;
    	}
    	else {
    		// font parameter not changed, so we do not reload it
            this->load_font = false;
    	}
    }
}

bool MMSInputWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

	// init language
    initLanguage();

    // load font
	loadFont();

    return true;
}

bool MMSInputWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    // release my font
    this->rootwindow->fm->releaseFont(this->font);
    this->fontpath = "";
    this->fontname = "";
    this->fontsize = 0;
    this->font = NULL;
    this->load_font = true;

    return true;
}

void MMSInputWidget::getForeground(MMSFBColor *color) {
	color->a = 0;

	if (isActivated()) {
		if (isSelected()) {
	        *color = getSelColor();
		}
		else {
	        *color = getColor();
		}
		if (isPressed()) {
			MMSFBColor mycol;
			if (isSelected()) {
		        mycol = getSelColor_p();
				if (mycol.a>0) *color=mycol;
			}
			else {
		        mycol = getColor_p();
				if (mycol.a>0) *color=mycol;
			}
		}
	}
	else {
		if (isSelected()) {
	        *color = getSelColor_i();
		}
		else {
	        *color = getColor_i();
		}
	}
}

bool MMSInputWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSInputWidget::checkRefreshStatus() {
	if (MMSWidget::checkRefreshStatus()) return true;

	if (this->current_fgset) {
		// current foreground initialized
		MMSFBColor color;
		getForeground(&color);

		if (color == this->current_fgcolor) {
			// foreground color not changed, so we do not enable refreshing
			return false;
		}
	}

	// (re-)enable refreshing
	enableRefresh();

	return true;
}


bool MMSInputWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    // lock
    this->surface->lock();

    // draw widget basics
    if (MMSWidget::draw(backgroundFilled)) {
        int width, height, x, y = 0;
        int cursor_x=0, cursor_w = 4;

   		// check if we have to (re)load the font
   	    loadFont();

   	    // draw my things
        if (this->font) {
            MMSFBRectangle surfaceGeom = getSurfaceGeometry();

            this->surface->setFont(this->font);

            string text = getText();

            // get width and height of the string to be drawn
            this->font->getStringWidth(text, -1, &width);
            this->font->getHeight(&height);

            // calc cursor position
            if (text.size() < (unsigned int)this->cursor_pos)
            	cursor_x = width;
            else
            	this->font->getStringWidth(text.substr(0,this->cursor_pos), -1, &cursor_x);

            MMSALIGNMENT alignment = getAlignment();
            switch (alignment) {
                case MMSALIGNMENT_LEFT:
                case MMSALIGNMENT_TOP_LEFT:
                case MMSALIGNMENT_BOTTOM_LEFT:
                default:
                    if (cursor_x + cursor_w + scroll_x > surfaceGeom.w)
                    	// move text to the left
                    	scroll_x-= cursor_x + cursor_w + scroll_x - surfaceGeom.w;
                    else
                    if (scroll_x < 0) {
                    	// move text to the right
                    	scroll_x+= surfaceGeom.w - (cursor_x + cursor_w + scroll_x);
                    	if (scroll_x > 0)
                    		scroll_x = 0;
                    }
                    x = surfaceGeom.x + scroll_x;
                    cursor_x+=scroll_x;
                	switch (alignment) {
                		case MMSALIGNMENT_LEFT:
                		default:
                			y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                			break;
                        case MMSALIGNMENT_TOP_LEFT:
                            y = surfaceGeom.y;
                            break;
                        case MMSALIGNMENT_BOTTOM_LEFT:
                            y = surfaceGeom.y + surfaceGeom.h - height;
                            break;
                	}
                    break;
                case MMSALIGNMENT_RIGHT:
                case MMSALIGNMENT_TOP_RIGHT:
                case MMSALIGNMENT_BOTTOM_RIGHT:
                    if (cursor_x + scroll_x > surfaceGeom.w)
                    	// move text to the left
                    	scroll_x-= cursor_x + scroll_x - surfaceGeom.w;
                    else
                    if (scroll_x < 0) {
                    	// move text to the right
                    	scroll_x+= surfaceGeom.w - (cursor_x + cursor_w + scroll_x);
                    	if (scroll_x > 0)
                    		scroll_x = 0;
                    }
                    x = surfaceGeom.x + surfaceGeom.w - width - cursor_w;
                    cursor_x+=surfaceGeom.w - width - cursor_w;
                    if (cursor_x < 0) {
                    	x-=cursor_x;
                    	cursor_x = 0;
                    }
                	switch (alignment) {
                		case MMSALIGNMENT_RIGHT:
                			y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                			break;
                        case MMSALIGNMENT_TOP_RIGHT:
                            y = surfaceGeom.y;
                            break;
                        case MMSALIGNMENT_BOTTOM_RIGHT:
                            y = surfaceGeom.y + surfaceGeom.h - height;
                            break;
                        default:
                        	break;
                	}
                    break;
                case MMSALIGNMENT_CENTER:
                case MMSALIGNMENT_TOP_CENTER:
                case MMSALIGNMENT_BOTTOM_CENTER:
                    x = surfaceGeom.x + ((surfaceGeom.w - width) / 2) - cursor_w;
                    cursor_x+=((surfaceGeom.w - width) / 2) - cursor_w;
                    if (cursor_x < 0) {
                    	x-=cursor_x;
                    	cursor_x = 0;
                    }
                	switch (alignment) {
                		case MMSALIGNMENT_CENTER:
                			y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                			break;
                        case MMSALIGNMENT_TOP_CENTER:
                            y = surfaceGeom.y;
                            break;
                        case MMSALIGNMENT_BOTTOM_CENTER:
                            y = surfaceGeom.y + surfaceGeom.h - height;
                            break;
                        default:
                        	break;
                	}
                    break;
            }


            // get color
            MMSFBColor color;
            getForeground(&color);
            this->current_fgcolor   = color;
            this->current_fgset     = true;

            if (color.a) {
                // prepare for drawing
                this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(
									color,
									(isSelected())?getSelShadowColor(MMSPOSITION_TOP):getShadowColor(MMSPOSITION_TOP),
									(isSelected())?getSelShadowColor(MMSPOSITION_BOTTOM):getShadowColor(MMSPOSITION_BOTTOM),
									(isSelected())?getSelShadowColor(MMSPOSITION_LEFT):getShadowColor(MMSPOSITION_LEFT),
									(isSelected())?getSelShadowColor(MMSPOSITION_RIGHT):getShadowColor(MMSPOSITION_RIGHT),
									(isSelected())?getSelShadowColor(MMSPOSITION_TOP_LEFT):getShadowColor(MMSPOSITION_TOP_LEFT),
									(isSelected())?getSelShadowColor(MMSPOSITION_TOP_RIGHT):getShadowColor(MMSPOSITION_TOP_RIGHT),
									(isSelected())?getSelShadowColor(MMSPOSITION_BOTTOM_LEFT):getShadowColor(MMSPOSITION_BOTTOM_LEFT),
									(isSelected())?getSelShadowColor(MMSPOSITION_BOTTOM_RIGHT):getShadowColor(MMSPOSITION_BOTTOM_RIGHT),
									getBrightness(), getOpacity());

                // draw the text
                this->surface->drawString(text, -1, x, y);
            }

            // get cursor rect
            this->cursor_rect.x = cursor_x;
            this->cursor_rect.y = y;
            this->cursor_rect.w = cursor_w;
            this->cursor_rect.h = height;

        	if (this->cursor_on) {
                // draw the cursor
        		MMSSTATE cursor_state = getCursorState();
        		if ((cursor_state == MMSSTATE_TRUE)||((cursor_state == MMSSTATE_AUTO) && isFocused())) {
					this->surface->drawRectangle(cursor_x, y, cursor_w, height);
        		}
        	}
        }

        // update window surface with an area of surface
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    // unlock
    this->surface->unlock();

    // draw widgets debug frame
    return MMSWidget::drawDebug();
}

void MMSInputWidget::drawCursor(bool cursor_on) {
	// show/hide the cursor
	this->cursor_on = cursor_on;
	MMSSTATE cursor_state = getCursorState();
	if ((cursor_state == MMSSTATE_TRUE)||((cursor_state == MMSSTATE_AUTO) && isFocused())) {
	    // refresh is required
	    enableRefresh();

	    // refresh only, if cursor should be shown
		refresh();
	}
}


void MMSInputWidget::setCursorPos(int cursor_pos, bool refresh) {
	if (cursor_pos < 0) {
		// out of range
		this->cursor_pos = 0;
		return;
	}

	string text = getText();
	if (text.size() >= (unsigned int)cursor_pos)
		// okay, set it
		this->cursor_pos = cursor_pos;
	else
		// out of range
		this->cursor_pos = text.size();

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

bool MMSInputWidget::addTextAfterCursorPos(string text, bool refresh) {
	// something to add?
	if (text=="") return false;
	int textlen = text.size();

	// check if the change is accepted
	if (!this->onBeforeChange->emit(this, text, true, cursor_rect))
		return false;

	string oldtext;
	getText(oldtext);
	if (oldtext.size() < (unsigned int)this->cursor_pos)
		this->cursor_pos = oldtext.size();

	// insert the text and change the cursor position
	this->cursor_pos+=textlen;
	setText(oldtext.substr(0,this->cursor_pos-textlen) + text + oldtext.substr(this->cursor_pos-textlen), refresh, false);

	return true;
}

bool MMSInputWidget::removeTextBeforeCursorPos(int textlen, bool refresh) {
	// something to remove?
	if (textlen<=0) return false;
	if (this->cursor_pos<=0) return false;

	// check pos
	string text;
	getText(text);
	if (text.size() < (unsigned int)this->cursor_pos)
		this->cursor_pos = text.size();

	if (this->cursor_pos <= textlen)
		textlen = this->cursor_pos;

	// check if the change is accepted
	string rmtext = text.substr(this->cursor_pos - textlen, textlen);
	if (!this->onBeforeChange->emit(this, rmtext, false, cursor_rect))
		return false;

	//remove a part of the text
	this->cursor_pos-=textlen;
	setText(text.substr(0,this->cursor_pos) + text.substr(this->cursor_pos+textlen), refresh, false);

	return true;
}

void MMSInputWidget::handleInput(MMSInputEvent *inputevent) {
	bool processed = false;

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		// keyboard inputs

		// save last inputevent
		this->da->last_inputevent = *inputevent;

		processed = true;
		switch (inputevent->key) {
			case MMSKEY_CURSOR_RIGHT:
				setCursorPos(this->cursor_pos+1);
		        break;
			case MMSKEY_CURSOR_LEFT:
				setCursorPos(this->cursor_pos-1);
				break;
			case MMSKEY_BACKSPACE:
				removeTextBeforeCursorPos(1);
				break;
			case MMSKEY_HOME:
		    	setCursorPos(0);
				break;
			case MMSKEY_END:
		    	setCursorPos(0xffff);
				break;
			case MMSKEY_SPACE:
				addTextAfterCursorPos(" ");
				break;
			case MMSKEY_PLUS_SIGN:
				addTextAfterCursorPos("+");
				break;
			case MMSKEY_MINUS_SIGN:
				addTextAfterCursorPos("-");
				break;
			case MMSKEY_PERIOD:
				addTextAfterCursorPos(".");
				break;
			case MMSKEY_SLASH:
				addTextAfterCursorPos("/");
				break;
			case MMSKEY_UNDERSCORE:
				addTextAfterCursorPos("_");
				break;
			case MMSKEY_0:
				addTextAfterCursorPos("0");
				break;
			case MMSKEY_1:
				addTextAfterCursorPos("1");
				break;
			case MMSKEY_2:
				addTextAfterCursorPos("2");
				break;
			case MMSKEY_3:
				addTextAfterCursorPos("3");
				break;
			case MMSKEY_4:
				addTextAfterCursorPos("4");
				break;
			case MMSKEY_5:
				addTextAfterCursorPos("5");
				break;
			case MMSKEY_6:
				addTextAfterCursorPos("6");
				break;
			case MMSKEY_7:
				addTextAfterCursorPos("7");
				break;
			case MMSKEY_8:
				addTextAfterCursorPos("8");
				break;
			case MMSKEY_9:
				addTextAfterCursorPos("9");
				break;
			case MMSKEY_CAPITAL_A:
				addTextAfterCursorPos("A");
				break;
			case MMSKEY_CAPITAL_B:
				addTextAfterCursorPos("B");
				break;
			case MMSKEY_CAPITAL_C:
				addTextAfterCursorPos("C");
				break;
			case MMSKEY_CAPITAL_D:
				addTextAfterCursorPos("D");
				break;
			case MMSKEY_CAPITAL_E:
				addTextAfterCursorPos("E");
				break;
			case MMSKEY_CAPITAL_F:
				addTextAfterCursorPos("F");
				break;
			case MMSKEY_CAPITAL_G:
				addTextAfterCursorPos("G");
				break;
			case MMSKEY_CAPITAL_H:
				addTextAfterCursorPos("H");
				break;
			case MMSKEY_CAPITAL_I:
				addTextAfterCursorPos("I");
				break;
			case MMSKEY_CAPITAL_J:
				addTextAfterCursorPos("J");
				break;
			case MMSKEY_CAPITAL_K:
				addTextAfterCursorPos("K");
				break;
			case MMSKEY_CAPITAL_L:
				addTextAfterCursorPos("L");
				break;
			case MMSKEY_CAPITAL_M:
				addTextAfterCursorPos("M");
				break;
			case MMSKEY_CAPITAL_N:
				addTextAfterCursorPos("N");
				break;
			case MMSKEY_CAPITAL_O:
				addTextAfterCursorPos("O");
				break;
			case MMSKEY_CAPITAL_P:
				addTextAfterCursorPos("P");
				break;
			case MMSKEY_CAPITAL_Q:
				addTextAfterCursorPos("Q");
				break;
			case MMSKEY_CAPITAL_R:
				addTextAfterCursorPos("R");
				break;
			case MMSKEY_CAPITAL_S:
				addTextAfterCursorPos("S");
				break;
			case MMSKEY_CAPITAL_T:
				addTextAfterCursorPos("T");
				break;
			case MMSKEY_CAPITAL_U:
				addTextAfterCursorPos("U");
				break;
			case MMSKEY_CAPITAL_V:
				addTextAfterCursorPos("V");
				break;
			case MMSKEY_CAPITAL_W:
				addTextAfterCursorPos("W");
				break;
			case MMSKEY_CAPITAL_X:
				addTextAfterCursorPos("X");
				break;
			case MMSKEY_CAPITAL_Y:
				addTextAfterCursorPos("Y");
				break;
			case MMSKEY_CAPITAL_Z:
				addTextAfterCursorPos("Z");
				break;
			case MMSKEY_SMALL_A:
				addTextAfterCursorPos("a");
				break;
			case MMSKEY_SMALL_B:
				addTextAfterCursorPos("b");
				break;
			case MMSKEY_SMALL_C:
				addTextAfterCursorPos("c");
				break;
			case MMSKEY_SMALL_D:
				addTextAfterCursorPos("d");
				break;
			case MMSKEY_SMALL_E:
				addTextAfterCursorPos("e");
				break;
			case MMSKEY_SMALL_F:
				addTextAfterCursorPos("f");
				break;
			case MMSKEY_SMALL_G:
				addTextAfterCursorPos("g");
				break;
			case MMSKEY_SMALL_H:
				addTextAfterCursorPos("h");
				break;
			case MMSKEY_SMALL_I:
				addTextAfterCursorPos("i");
				break;
			case MMSKEY_SMALL_J:
				addTextAfterCursorPos("j");
				break;
			case MMSKEY_SMALL_K:
				addTextAfterCursorPos("k");
				break;
			case MMSKEY_SMALL_L:
				addTextAfterCursorPos("l");
				break;
			case MMSKEY_SMALL_M:
				addTextAfterCursorPos("m");
				break;
			case MMSKEY_SMALL_N:
				addTextAfterCursorPos("n");
				break;
			case MMSKEY_SMALL_O:
				addTextAfterCursorPos("o");
				break;
			case MMSKEY_SMALL_P:
				addTextAfterCursorPos("p");
				break;
			case MMSKEY_SMALL_Q:
				addTextAfterCursorPos("q");
				break;
			case MMSKEY_SMALL_R:
				addTextAfterCursorPos("r");
				break;
			case MMSKEY_SMALL_S:
				addTextAfterCursorPos("s");
				break;
			case MMSKEY_SMALL_T:
				addTextAfterCursorPos("t");
				break;
			case MMSKEY_SMALL_U:
				addTextAfterCursorPos("u");
				break;
			case MMSKEY_SMALL_V:
				addTextAfterCursorPos("v");
				break;
			case MMSKEY_SMALL_W:
				addTextAfterCursorPos("w");
				break;
			case MMSKEY_SMALL_X:
				addTextAfterCursorPos("x");
				break;
			case MMSKEY_SMALL_Y:
				addTextAfterCursorPos("y");
				break;
			case MMSKEY_SMALL_Z:
				addTextAfterCursorPos("z");
				break;

			default:
				processed = false;
		}
	}

	// not processed, call base widget
	if (!processed)
		MMSWidget::handleInput(inputevent);
}


/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETINPUT(x) \
    if (this->myInputWidgetClass.is##x()) return myInputWidgetClass.get##x(); \
    else if ((inputWidgetClass)&&(inputWidgetClass->is##x())) return inputWidgetClass->get##x(); \
    else return this->da->theme->inputWidgetClass.get##x();

#define GETINPUT2(x, y) \
    if (this->myInputWidgetClass.is##x()) y=myInputWidgetClass.get##x(); \
    else if ((inputWidgetClass)&&(inputWidgetClass->is##x())) y=inputWidgetClass->get##x(); \
    else y=this->da->theme->inputWidgetClass.get##x();

#define GETINPUTFONT(lang) \
    if (this->myInputWidgetClass.isFontName(lang)) return myInputWidgetClass.getFontName(lang); \
    else if (this->myInputWidgetClass.isFontName(MMSLANG_NONE)) return myInputWidgetClass.getFontName(MMSLANG_NONE); \
    else if ((inputWidgetClass)&&(inputWidgetClass->isFontName(lang))) return inputWidgetClass->getFontName(lang); \
    else if ((inputWidgetClass)&&(inputWidgetClass->isFontName(MMSLANG_NONE))) return inputWidgetClass->getFontName(MMSLANG_NONE); \
    else return this->da->theme->inputWidgetClass.getFontName();

#define GETINPUTSHADOW(x) \
    if (this->myInputWidgetClass.isShadowColor(x)) return myInputWidgetClass.getShadowColor(x); \
    else if ((inputWidgetClass)&&(inputWidgetClass->isShadowColor(x))) return inputWidgetClass->getShadowColor(x); \
    else return this->da->theme->inputWidgetClass.getShadowColor(x);

#define GETINPUTSHADOWSEL(x) \
    if (this->myInputWidgetClass.isSelShadowColor(x)) return myInputWidgetClass.getSelShadowColor(x); \
    else if ((inputWidgetClass)&&(inputWidgetClass->isSelShadowColor(x))) return inputWidgetClass->getSelShadowColor(x); \
    else return this->da->theme->inputWidgetClass.getSelShadowColor(x);

string MMSInputWidget::getFontPath() {
    GETINPUT(FontPath);
}

string MMSInputWidget::getFontName(MMSLanguage lang) {
	GETINPUTFONT(lang);
}

unsigned int MMSInputWidget::getFontSize() {
    GETINPUT(FontSize);
}

MMSALIGNMENT MMSInputWidget::getAlignment() {
    GETINPUT(Alignment);
}

MMSFBColor MMSInputWidget::getColor() {
    GETINPUT(Color);
}

MMSFBColor MMSInputWidget::getSelColor() {
    GETINPUT(SelColor);
}

MMSFBColor MMSInputWidget::getColor_p() {
    GETINPUT(Color_p);
}

MMSFBColor MMSInputWidget::getSelColor_p() {
    GETINPUT(SelColor_p);
}

MMSFBColor MMSInputWidget::getColor_i() {
    GETINPUT(Color_i);
}

MMSFBColor MMSInputWidget::getSelColor_i() {
    GETINPUT(SelColor_i);
}

string MMSInputWidget::getText() {
    GETINPUT(Text);
}

void MMSInputWidget::getText(string &text) {
    GETINPUT2(Text, text);
}

MMSSTATE MMSInputWidget::getCursorState() {
    GETINPUT(CursorState);
}

MMSFBColor MMSInputWidget::getShadowColor(MMSPOSITION position) {
	GETINPUTSHADOW(position);
}

MMSFBColor MMSInputWidget::getSelShadowColor(MMSPOSITION position) {
	GETINPUTSHADOWSEL(position);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSInputWidget::setFontPath(string fontpath, bool load, bool refresh) {
    myInputWidgetClass.setFontPath(fontpath);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setFontName(MMSLanguage lang, string fontname, bool load, bool refresh) {
    myInputWidgetClass.setFontName(fontname, lang);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setFontName(string fontname, bool load, bool refresh) {
	setFontName(MMSLANG_NONE, fontname, load, refresh);
}

void MMSInputWidget::setFontSize(unsigned int fontsize, bool load, bool refresh) {
    myInputWidgetClass.setFontSize(fontsize);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setFont(MMSLanguage lang, string fontpath, string fontname, unsigned int fontsize, bool load, bool refresh) {
    myInputWidgetClass.setFontPath(fontpath);
    myInputWidgetClass.setFontName(fontname, lang);
    myInputWidgetClass.setFontSize(fontsize);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setFont(string fontpath, string fontname, unsigned int fontsize, bool load, bool refresh) {
	setFont(MMSLANG_NONE, fontpath, fontname, fontsize, load, refresh);
}

void MMSInputWidget::setAlignment(MMSALIGNMENT alignment, bool refresh) {
    myInputWidgetClass.setAlignment(alignment);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setColor(MMSFBColor color, bool refresh) {
    myInputWidgetClass.setColor(color);

	// refresh required?
	enableRefresh((color != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setSelColor(MMSFBColor selcolor, bool refresh) {
    myInputWidgetClass.setSelColor(selcolor);

	// refresh required?
	enableRefresh((selcolor != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setColor_p(MMSFBColor color_p, bool refresh) {
    myInputWidgetClass.setColor_p(color_p);

	// refresh required?
	enableRefresh((color_p != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setSelColor_p(MMSFBColor selcolor_p, bool refresh) {
    myInputWidgetClass.setSelColor_p(selcolor_p);

	// refresh required?
	enableRefresh((selcolor_p != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setColor_i(MMSFBColor color_i, bool refresh) {
    myInputWidgetClass.setColor_i(color_i);

	// refresh required?
	enableRefresh((color_i != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setSelColor_i(MMSFBColor selcolor_i, bool refresh) {
    myInputWidgetClass.setSelColor_i(selcolor_i);

	// refresh required?
	enableRefresh((selcolor_i != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSInputWidget::setText(string text, bool refresh, bool reset_cursor) {
    myInputWidgetClass.setText(text);

    if (reset_cursor)
    	// set cursor to the end
    	setCursorPos(0xffff, refresh);
    else
    	// try to leave cursor at its position
    	setCursorPos(this->cursor_pos, refresh);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setCursorState(MMSSTATE cursor_state, bool refresh) {
    myInputWidgetClass.setCursorState(cursor_state);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setShadowColor(MMSPOSITION position, MMSFBColor color, bool refresh) {
    myInputWidgetClass.setShadowColor(position, color);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor, bool refresh) {
    myInputWidgetClass.setSelShadowColor(position, selcolor);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSInputWidget::updateFromThemeClass(MMSInputWidgetClass *themeClass) {

	// update widget-specific settings
    if (themeClass->isCursorState())
        setCursorState(themeClass->getCursorState());

    // update base text-specific settings
	MMSTEXTBASE_UPDATE_FROM_THEME_CLASS(this, themeClass);

	// update general widget settings
    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
