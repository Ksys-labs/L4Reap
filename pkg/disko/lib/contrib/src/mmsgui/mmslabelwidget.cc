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

#include "mmsgui/mmslabelwidget.h"
#include "mmsgui/mmstextbase.h"


MMSLabelWidget::MMSLabelWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    create(root, className, theme);
}

MMSLabelWidget::~MMSLabelWidget() {
    if (labelThread) {
        labelThread->stop();
        labelThread=NULL;
    }
}

bool MMSLabelWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_LABEL;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->labelWidgetClass = this->da->theme->getLabelWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->labelWidgetClass.widgetClass);
    if (this->labelWidgetClass) this->da->widgetClass = &(this->labelWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    // clear
    initLanguage();
	this->fontpath = "";
	this->fontname = "";
	this->fontsize = 0;
    this->font = NULL;
    this->load_font = true;
    this->slide_width = 0;
    this->slide_offset = 0;
    this->frame_delay = 100;
    this->frame_delay_set = false;
    this->labelThread = NULL;
    this->translated = false;
    this->swap_left_right = false;
	this->current_fgset = false;

    return MMSWidget::create(root, true, false, false, true, false, false, false);
}

MMSWidget *MMSLabelWidget::copyWidget() {
    // create widget
    MMSLabelWidget *newWidget = new MMSLabelWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->labelWidgetClass = this->labelWidgetClass;
    newWidget->myLabelWidgetClass = this->myLabelWidgetClass;

    newWidget->lang = this->lang;
    newWidget->translated_text = this->translated_text;
    newWidget->current_fgcolor = this->current_fgcolor;

    // copy base widget
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    // reload my font
    initLanguage(newWidget);
    newWidget->fontpath = "";
	newWidget->fontname = "";
	newWidget->fontsize = 0;
    newWidget->font = NULL;
    newWidget->load_font = true;
    newWidget->slide_width = 0;
    newWidget->slide_offset = 0;
    newWidget->frame_delay = 100;
    newWidget->frame_delay_set = false;
    newWidget->labelThread = NULL;
    newWidget->translated = false;
    newWidget->swap_left_right = false;
    newWidget->current_fgset = false;
    if (this->rootwindow) {
    	// load font
        loadFont(newWidget);

        // first time the label thread has to be started
        if (newWidget->getSlidable()) {
			newWidget->setSlidable(true);
        }
    }

    return newWidget;
}


void MMSLabelWidget::initLanguage(MMSLabelWidget *widget) {
	if (!widget) widget = this;

	widget->lang = (!this->rootwindow)?MMSLANG_NONE:this->rootwindow->windowmanager->getTranslator()->getTargetLang();
}

void MMSLabelWidget::loadFont(MMSLabelWidget *widget) {
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

bool MMSLabelWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

	// init language
    initLanguage();

    // load font
	loadFont();

    // first time the label thread has to be started
    if (getSlidable()) {
    	setSlidable(true);
    }

    return true;
}

bool MMSLabelWidget::release() {
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


bool MMSLabelWidget::prepareText(int *width, int *height, bool recalc) {
	// check if we have to (re)load the font
	loadFont();

    if (!this->font)
    	return false;

	// font available, use it for this surface
	this->surface->setFont(this->font);

	if (!this->translated) {
		// text changed and have to be translated
		if ((this->rootwindow)&&(this->rootwindow->windowmanager)&&(getTranslate())) {
			// translate text
			string source;
			getText(source);
			this->rootwindow->windowmanager->getTranslator()->translate(source, this->translated_text);
		}
		else {
			// text can not or should not translated
			getText(this->translated_text);
		}

		// reset swap flag
		this->swap_left_right = false;

		// language specific conversions
		MMSLanguage targetlang = this->rootwindow->windowmanager->getTranslator()->getTargetLang();
		if (targetlang == MMSLANG_IL) {
			if (convBidiString(this->translated_text, this->translated_text)) {
				// bidirectional conversion successful, swap alignment horizontal
				this->swap_left_right = true;
			}
		}

		// mark as translated
		this->translated = true;
	}

	// get width and height of the string to be drawn
	int realWidth, realHeight;
	this->font->getStringWidth(this->translated_text, -1, &realWidth);
	this->font->getHeight(&realHeight);

	if (!this->minmax_set) {
		if (width)  *width = realWidth;
		if (height) *height = realHeight;
	}
	else {
		if (recalc) {
			// calculate dynamic label size

			// get maximum width and height of the label
			int maxWidth = getMaxWidthPix();
			if (maxWidth <= 0) maxWidth = getInnerGeometry().w;
			int maxHeight = getMaxHeightPix();
			if (maxHeight <= 0) maxHeight = getInnerGeometry().h;

			// get minimum width and height of the label
			int minWidth = getMinWidthPix();
			int minHeight = getMinHeightPix();

			if (width)  {
				if (realWidth < minWidth)
					*width = minWidth;
				else
				if (realWidth > maxWidth)
					*width = maxWidth;
				else
					*width = realWidth;

				if (*width <= 0) *width = 1;
			}

			if (height) {
				if (realHeight < minHeight)
					*height = minHeight;
				else
				if (realHeight > maxHeight)
					*height = maxHeight;
				else
					*height = realHeight;

				if (*height <= 0) *height = 1;
			}
		}
		else {
			if (width)  *width = realWidth;
			if (height) *height = realHeight;
		}
	}

	return true;
}


void MMSLabelWidget::calcContentSize() {
	int width, height;

	if (prepareText(&width, &height, true)) {
    	// text is translated and font is set
        setContentSize(width, height);
	}
}


void MMSLabelWidget::getForeground(MMSFBColor *color) {
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

bool MMSLabelWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSLabelWidget::checkRefreshStatus() {
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


bool MMSLabelWidget::draw(bool *backgroundFilled) {
    int width, height, x, y;
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
    	// draw my things
    	if (prepareText(&width, &height)) {
    		// text is translated and font is set
            MMSFBRectangle surfaceGeom = getSurfaceGeometry();

            // save the width of the text
            this->slide_width = width;

            switch ((!this->swap_left_right) ? getAlignment() : swapAlignmentHorizontal(getAlignment())) {
                case MMSALIGNMENT_LEFT:
                    x = surfaceGeom.x;
                    y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                    break;
                case MMSALIGNMENT_RIGHT:
                    x = surfaceGeom.x + surfaceGeom.w - width;
                    y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                    break;
                case MMSALIGNMENT_CENTER:
                    x = ((surfaceGeom.w - width) / 2) + surfaceGeom.x;
                    y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
                    break;
                case MMSALIGNMENT_TOP_CENTER:
                    x = ((surfaceGeom.w - width) / 2) + surfaceGeom.x;
                    y = surfaceGeom.y;
                    break;
                case MMSALIGNMENT_TOP_LEFT:
                    x = surfaceGeom.x;
                    y = surfaceGeom.y;
                    break;
                case MMSALIGNMENT_TOP_RIGHT:
                    x = surfaceGeom.x + surfaceGeom.w - width;
                    y = surfaceGeom.y;
                    break;
                case MMSALIGNMENT_BOTTOM_CENTER:
                    x = ((surfaceGeom.w - width) / 2) + surfaceGeom.x;
                    y = surfaceGeom.y + surfaceGeom.h - height;
                    break;
                case MMSALIGNMENT_BOTTOM_LEFT:
                    x = surfaceGeom.x;
                    y = surfaceGeom.y + surfaceGeom.h - height;
                    break;
                case MMSALIGNMENT_BOTTOM_RIGHT:
                    x = surfaceGeom.x + surfaceGeom.w - width;
                    y = surfaceGeom.y + surfaceGeom.h - height;
                    break;
                default:
                    x = ((surfaceGeom.w - width) / 2) + surfaceGeom.x;
                    y = ((surfaceGeom.h - height) / 2) + surfaceGeom.y;
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
                this->surface->drawString(this->translated_text, -1, x - this->slide_offset, y);
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

void MMSLabelWidget::targetLangChanged(MMSLanguage lang) {
    this->translated = false;
    this->load_font = true;

    // recalculate content size for dynamic widgets, because new language can result in new widget size
    // note: DO NOT REFRESH at this point
    recalcContentSize(false);
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETLABEL(x) \
    if (this->myLabelWidgetClass.is##x()) return myLabelWidgetClass.get##x(); \
    else if ((labelWidgetClass)&&(labelWidgetClass->is##x())) return labelWidgetClass->get##x(); \
    else return this->da->theme->labelWidgetClass.get##x();

#define GETLABEL2(x,y) \
    if (this->myLabelWidgetClass.is##x()) y=myLabelWidgetClass.get##x(); \
    else if ((labelWidgetClass)&&(labelWidgetClass->is##x())) y=labelWidgetClass->get##x(); \
    else y=this->da->theme->labelWidgetClass.get##x();

#define GETLABELFONT(lang) \
    if (this->myLabelWidgetClass.isFontName(lang)) return myLabelWidgetClass.getFontName(lang); \
    else if (this->myLabelWidgetClass.isFontName(MMSLANG_NONE)) return myLabelWidgetClass.getFontName(MMSLANG_NONE); \
    else if ((labelWidgetClass)&&(labelWidgetClass->isFontName(lang))) return labelWidgetClass->getFontName(lang); \
    else if ((labelWidgetClass)&&(labelWidgetClass->isFontName(MMSLANG_NONE))) return labelWidgetClass->getFontName(MMSLANG_NONE); \
    else return this->da->theme->labelWidgetClass.getFontName();

#define GETLABELSHADOW(x) \
    if (this->myLabelWidgetClass.isShadowColor(x)) return myLabelWidgetClass.getShadowColor(x); \
    else if ((labelWidgetClass)&&(labelWidgetClass->isShadowColor(x))) return labelWidgetClass->getShadowColor(x); \
    else return this->da->theme->labelWidgetClass.getShadowColor(x);

#define GETLABELSHADOWSEL(x) \
    if (this->myLabelWidgetClass.isSelShadowColor(x)) return myLabelWidgetClass.getSelShadowColor(x); \
    else if ((labelWidgetClass)&&(labelWidgetClass->isSelShadowColor(x))) return labelWidgetClass->getSelShadowColor(x); \
    else return this->da->theme->labelWidgetClass.getSelShadowColor(x);


string MMSLabelWidget::getFontPath() {
    GETLABEL(FontPath);
}

string MMSLabelWidget::getFontName(MMSLanguage lang) {
    GETLABELFONT(lang);
}

unsigned int MMSLabelWidget::getFontSize() {
    GETLABEL(FontSize);
}

MMSALIGNMENT MMSLabelWidget::getAlignment() {
    GETLABEL(Alignment);
}

MMSFBColor MMSLabelWidget::getColor() {
    GETLABEL(Color);
}

MMSFBColor MMSLabelWidget::getSelColor() {
    GETLABEL(SelColor);
}

MMSFBColor MMSLabelWidget::getColor_p() {
    GETLABEL(Color_p);
}

MMSFBColor MMSLabelWidget::getSelColor_p() {
    GETLABEL(SelColor_p);
}

MMSFBColor MMSLabelWidget::getColor_i() {
    GETLABEL(Color_i);
}

MMSFBColor MMSLabelWidget::getSelColor_i() {
    GETLABEL(SelColor_i);
}

string MMSLabelWidget::getText() {
    GETLABEL(Text);
}

void MMSLabelWidget::getText(string &text) {
    GETLABEL2(Text, text);
}

bool MMSLabelWidget::getSlidable() {
    GETLABEL(Slidable);
}

unsigned char MMSLabelWidget::getSlideSpeed() {
    GETLABEL(SlideSpeed);
}

bool MMSLabelWidget::getTranslate() {
    GETLABEL(Translate);
}


MMSFBColor MMSLabelWidget::getShadowColor(MMSPOSITION position) {
	GETLABELSHADOW(position);
}

MMSFBColor MMSLabelWidget::getSelShadowColor(MMSPOSITION position) {
	GETLABELSHADOWSEL(position);
}


/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSLabelWidget::setFontPath(string fontpath, bool load, bool refresh) {
    myLabelWidgetClass.setFontPath(fontpath);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setFontName(MMSLanguage lang, string fontname, bool load, bool refresh) {
    myLabelWidgetClass.setFontName(fontname, lang);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setFontName(string fontname, bool load, bool refresh) {
	setFontName(MMSLANG_NONE, fontname, load, refresh);
}

void MMSLabelWidget::setFontSize(unsigned int fontsize, bool load, bool refresh) {
    myLabelWidgetClass.setFontSize(fontsize);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setFont(MMSLanguage lang, string fontpath, string fontname, unsigned int fontsize, bool load, bool refresh) {
    myLabelWidgetClass.setFontPath(fontpath);
    myLabelWidgetClass.setFontName(fontname, lang);
    myLabelWidgetClass.setFontSize(fontsize);
    if (load) {
        this->load_font = true;
    	loadFont();
    }

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setFont(string fontpath, string fontname, unsigned int fontsize, bool load, bool refresh) {
	setFont(MMSLANG_NONE, fontpath, fontname, fontsize, load, refresh);
}

void MMSLabelWidget::setAlignment(MMSALIGNMENT alignment, bool refresh) {
    myLabelWidgetClass.setAlignment(alignment);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setColor(MMSFBColor color, bool refresh) {
    myLabelWidgetClass.setColor(color);

	// refresh required?
	enableRefresh((color != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSLabelWidget::setSelColor(MMSFBColor selcolor, bool refresh) {
    myLabelWidgetClass.setSelColor(selcolor);

	// refresh required?
	enableRefresh((selcolor != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSLabelWidget::setColor_p(MMSFBColor color_p, bool refresh) {
    myLabelWidgetClass.setColor_p(color_p);

	// refresh required?
	enableRefresh((color_p != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSLabelWidget::setSelColor_p(MMSFBColor selcolor_p, bool refresh) {
    myLabelWidgetClass.setSelColor_p(selcolor_p);

	// refresh required?
	enableRefresh((selcolor_p != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSLabelWidget::setColor_i(MMSFBColor color_i, bool refresh) {
    myLabelWidgetClass.setColor_i(color_i);

	// refresh required?
	enableRefresh((color_i != this->current_fgcolor));

	this->refresh(refresh);
}

void MMSLabelWidget::setSelColor_i(MMSFBColor selcolor_i, bool refresh) {
    myLabelWidgetClass.setSelColor_i(selcolor_i);

	// refresh required?
	enableRefresh((selcolor_i != this->current_fgcolor));

	this->refresh(refresh);
}


void MMSLabelWidget::setText(string text, bool refresh) {
    myLabelWidgetClass.setText(text);
    this->translated = false;

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setSlidable(bool slidable) {
    myLabelWidgetClass.setSlidable(slidable);
    if (slidable) {
    	// text should slide
    	this->slide_offset = 0;

        if (this->labelThread) {
            // toggle pause off
            this->labelThread->pause(false);
        }
        else {
            // start a thread
            this->labelThread = new MMSLabelWidgetThread(this);
            this->labelThread->start();
        }
    }
    else {
    	// static text
        if (labelThread)
            labelThread->stop();
    	this->slide_offset = 0;

        // refresh is required
        enableRefresh();

        this->refresh();
    }
}

void MMSLabelWidget::setSlideSpeed(unsigned char slidespeed) {
    myLabelWidgetClass.setSlideSpeed(slidespeed);
    this->frame_delay = 100;
    this->frame_delay_set = false;
}

void MMSLabelWidget::setTranslate(bool translate, bool refresh) {
    myLabelWidgetClass.setTranslate(translate);
    this->translated = false;

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}



void MMSLabelWidget::setShadowColor(MMSPOSITION position, MMSFBColor color, bool refresh) {
    myLabelWidgetClass.setShadowColor(position, color);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSLabelWidget::setSelShadowColor(MMSPOSITION position, MMSFBColor selcolor, bool refresh) {
    myLabelWidgetClass.setSelShadowColor(position, selcolor);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}



void MMSLabelWidget::updateFromThemeClass(MMSLabelWidgetClass *themeClass) {

	// update widget-specific settings
	if (themeClass->isSlidable())
        setSlidable(themeClass->getSlidable());
    if (themeClass->isSlideSpeed())
        setSlideSpeed(themeClass->getSlideSpeed());
    if (themeClass->isTranslate())
        setTranslate(themeClass->getTranslate());

    // update base text-specific settings
	MMSTEXTBASE_UPDATE_FROM_THEME_CLASS(this, themeClass);

	// update general widget settings
    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
