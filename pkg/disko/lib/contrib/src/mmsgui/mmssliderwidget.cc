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

#include "mmsgui/mmssliderwidget.h"

MMSSliderWidget::MMSSliderWidget(MMSWindow *root, string className, MMSTheme *theme) {
    create(root, className, theme);
}

MMSSliderWidget::~MMSSliderWidget() {
	// delete the callbacks
    if (this->onSliderIncrement) delete this->onSliderIncrement;
    if (this->onSliderDecrement) delete this->onSliderDecrement;
}

bool MMSSliderWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_SLIDER;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->sliderWidgetClass = this->da->theme->getSliderWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->sliderWidgetClass.widgetClass);
    if (this->sliderWidgetClass) this->da->widgetClass = &(this->sliderWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    // clear
    this->imagepath_set = false;
    this->selimagepath_set = false;

    this->imagepath_p_set = false;
    this->selimagepath_p_set = false;

    this->imagepath_i_set = false;
    this->selimagepath_i_set = false;

    this->barimagepath_set = false;
    this->selbarimagepath_set = false;

    this->image = NULL;
    this->selimage = NULL;
    this->image_p = NULL;
    this->selimage_p = NULL;
    this->image_i = NULL;
    this->selimage_i = NULL;
    this->barimage = NULL;
    this->selbarimage = NULL;

    this->vertical = true;
	this->current_fgset = false;

    // initialize the callbacks
    this->onSliderIncrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;
    this->onSliderDecrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;

    // create widget base
    return MMSWidget::create(root, true, false, true, true, true, true, true);
}

MMSWidget *MMSSliderWidget::copyWidget() {
    /* create widget */
    MMSSliderWidget *newWidget = new MMSSliderWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->sliderWidgetClass = this->sliderWidgetClass;
    newWidget->mySliderWidgetClass = this->mySliderWidgetClass;

    newWidget->imagepath_set = this->imagepath_set;
    newWidget->selimagepath_set = this->selimagepath_set;

    newWidget->imagepath_p_set = this->imagepath_p_set;
    newWidget->selimagepath_p_set = this->selimagepath_p_set;

    newWidget->imagepath_i_set = this->imagepath_i_set;
    newWidget->selimagepath_i_set = this->selimagepath_i_set;

    newWidget->barimagepath_set = this->barimagepath_set;
    newWidget->selbarimagepath_set = this->selbarimagepath_set;

    newWidget->current_fgset = this->current_fgset;
    newWidget->current_fgimage = this->current_fgimage;
    newWidget->current_fgbarimage = this->current_fgbarimage;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    /* reload my images */
    newWidget->image = NULL;
    newWidget->selimage = NULL;
    newWidget->image_p = NULL;
    newWidget->selimage_p = NULL;
    newWidget->image_i = NULL;
    newWidget->selimage_i = NULL;
    newWidget->barimage = NULL;
    newWidget->selbarimage = NULL;
    this->vertical = true;
    if (this->rootwindow) {
        newWidget->image = this->rootwindow->im->getImage(newWidget->getImagePath(), newWidget->getImageName());
        newWidget->selimage = this->rootwindow->im->getImage(newWidget->getSelImagePath(), newWidget->getSelImageName());
        newWidget->image_p = this->rootwindow->im->getImage(newWidget->getImagePath_p(), newWidget->getImageName_p());
        newWidget->selimage_p = this->rootwindow->im->getImage(newWidget->getSelImagePath_p(), newWidget->getSelImageName_p());
        newWidget->image_i = this->rootwindow->im->getImage(newWidget->getImagePath_i(), newWidget->getImageName_i());
        newWidget->selimage_i = this->rootwindow->im->getImage(newWidget->getSelImagePath_i(), newWidget->getSelImageName_i());
        newWidget->barimage = this->rootwindow->im->getImage(newWidget->getBarImagePath(), newWidget->getBarImageName());
        newWidget->selbarimage = this->rootwindow->im->getImage(newWidget->getSelBarImagePath(), newWidget->getSelBarImageName());
    }

    return newWidget;
}

bool MMSSliderWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    // load images
    this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
    this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
    this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
    this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
    this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
    this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
    this->barimage = this->rootwindow->im->getImage(getBarImagePath(), getBarImageName());
    this->selbarimage = this->rootwindow->im->getImage(getSelBarImagePath(), getSelBarImageName());

    return true;
}

bool MMSSliderWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    // release my images
    this->rootwindow->im->releaseImage(this->image);
    this->image = NULL;
    this->rootwindow->im->releaseImage(this->selimage);
    this->selimage = NULL;
    this->rootwindow->im->releaseImage(this->image_p);
    this->image_p = NULL;
    this->rootwindow->im->releaseImage(this->selimage_p);
    this->selimage_p = NULL;
    this->rootwindow->im->releaseImage(this->image_i);
    this->image_i = NULL;
    this->rootwindow->im->releaseImage(this->selimage_i);
    this->selimage_i = NULL;
    this->rootwindow->im->releaseImage(this->barimage);
    this->barimage = NULL;
    this->rootwindow->im->releaseImage(this->selbarimage);
    this->selbarimage = NULL;

    return true;
}

void MMSSliderWidget::getImage(MMSFBSurface **suf) {
	// searching for the image
	*suf = NULL;

	if (isActivated()) {
		if (isSelected())
			*suf = this->selimage;
		else
			*suf = this->image;
		if (isPressed()) {
			if (isSelected()) {
				if (this->selimage_p)
					*suf = this->selimage_p;
			}
			else {
				if (this->image_p)
					*suf = this->image_p;
			}
		}
	}
	else {
		if (isSelected())
			*suf = this->selimage_i;
		else
			*suf = this->image_i;
	}
}

void MMSSliderWidget::getBarImage(MMSFBSurface **suf) {
	// searching for the image
	*suf = NULL;

	if (isSelected())
		*suf = this->selbarimage;
	else
		*suf = this->barimage;
}

void MMSSliderWidget::calcPos(MMSFBSurface *suf, MMSFBRectangle *surfaceGeom, bool *vertical,
							  MMSFBSurface *barsuf, MMSFBRectangle *src_barGeom, MMSFBRectangle *dst_barGeom) {
    // calculate position of the slider
	int w = 0, h = 0;
	if (suf) {
		// if surface is set we get width and height, else we use zero for width and height (no slider image)
		suf->getSize(&w, &h);
	}

	if (barsuf && src_barGeom) {
		// get the default source bar rectangle
		src_barGeom->x = 0;
		src_barGeom->y = 0;
		barsuf->getSize(&src_barGeom->w, &src_barGeom->h);
	}

	if (dst_barGeom) *dst_barGeom = *surfaceGeom;
	if (surfaceGeom->w < w) w = surfaceGeom->w;
	if (surfaceGeom->h < h) h = surfaceGeom->h;
	if (surfaceGeom->w - w < surfaceGeom->h - h) {
		// vertical slider
		surfaceGeom->y += ((surfaceGeom->h - h) * getPosition()) / 100;
		if (dst_barGeom) {
			dst_barGeom->y = surfaceGeom->y + h / 2;
			dst_barGeom->h = surfaceGeom->h - surfaceGeom->y - h / 2;
			if (src_barGeom) {
				// calculate bar source rectangle
				if (src_barGeom->h == surfaceGeom->h - h / 2) {
					// bar height fits into the full dst bar geom
					src_barGeom->y+= src_barGeom->h - dst_barGeom->h;
					src_barGeom->h = dst_barGeom->h;
				}
				else {
					// have to stretch the bar
					int	hh = ((src_barGeom->h * dst_barGeom->h * 100) / (surfaceGeom->h - h / 2) + 50) / 100;
					src_barGeom->y+= src_barGeom->h - hh;
					src_barGeom->h = hh;
				}
			}
		}
		surfaceGeom->h = h;
		*vertical = true;
	}
	else {
		// horizontal slider
		surfaceGeom->x += ((surfaceGeom->w - w) * getPosition()) / 100;
		if (dst_barGeom) {
			dst_barGeom->x = surfaceGeom->x + w / 2;
			dst_barGeom->w = surfaceGeom->w - surfaceGeom->x - w / 2;
			if (src_barGeom) {
				// calculate bar source rectangle
				if (src_barGeom->w == surfaceGeom->w - w / 2) {
					// bar width fits into the full dst bar geom
					src_barGeom->x+= src_barGeom->w - dst_barGeom->w;
					src_barGeom->w = dst_barGeom->w;
				}
				else {
					// have to stretch the bar
					int	ww = ((src_barGeom->w * dst_barGeom->w * 100) / (surfaceGeom->w - w / 2) + 50) / 100;
					src_barGeom->x+= src_barGeom->w - ww;
					src_barGeom->w = ww;
				}
			}
		}
		surfaceGeom->w = w;
		*vertical = false;
	}
}


void MMSSliderWidget::getForeground(MMSFBSurface **image, MMSFBSurface **barimage) {
	getImage(image);
	getBarImage(barimage);

}

bool MMSSliderWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSSliderWidget::checkRefreshStatus() {
	if (MMSWidget::checkRefreshStatus()) return true;

	if (this->current_fgset) {
		// current foreground initialized
		MMSFBSurface *image, *barimage;
		getForeground(&image, &barimage);

		if (image == this->current_fgimage && barimage == this->current_fgbarimage) {
			// foreground images not changed, so we do not enable refreshing
			return false;
		}
	}

	// (re-)enable refreshing
	enableRefresh();

	return true;
}


bool MMSSliderWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    // draw widget basics
    if (MMSWidget::draw(backgroundFilled)) {

        // lock
        this->surface->lock();

        // draw my things
        MMSFBRectangle surfaceGeom = getSurfaceGeometry();

        // get images
        MMSFBSurface *suf, *barsuf;
        getForeground(&suf, &barsuf);
        this->current_fgimage   = suf;
        this->current_fgbarimage= barsuf;
        this->current_fgset     = true;

        // calculate position of the slider
        MMSFBRectangle src_barGeom;
        MMSFBRectangle dst_barGeom;
		calcPos(suf, &surfaceGeom, &this->vertical, barsuf, &src_barGeom, &dst_barGeom);

		if (barsuf) {
			// blit the bar image, prepare for blitting
			this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, 255, opacity);

			// blit
			this->surface->stretchBlit(barsuf, &src_barGeom, &dst_barGeom);
		}

        if (suf) {
        	// blit the slider image, prepare for blitting
            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, 255, opacity);

            // blit
            this->surface->stretchBlit(suf, NULL, &surfaceGeom);
        }

        // unlock
        this->surface->unlock();

        // update window surface with an area of surface
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    // draw widgets debug frame
    return MMSWidget::drawDebug();
}


void MMSSliderWidget::switchArrowWidgets() {
    // connect arrow widgets
    loadArrowWidgets();

    // get current pos
	int pos = (int)getPosition();

	if (this->vertical) {
		// vertical slider
	    if (this->da->upArrowWidget) {
			if (pos <= 0)
				this->da->upArrowWidget->setSelected(false);
			else
				this->da->upArrowWidget->setSelected(true);
	    }

	    if (this->da->downArrowWidget) {
			if (pos >= 100)
				this->da->downArrowWidget->setSelected(false);
			else
				this->da->downArrowWidget->setSelected(true);
	    }
	}
	else {
		// horizontal slider
	    if (this->da->leftArrowWidget) {
			if (pos <= 0)
				this->da->leftArrowWidget->setSelected(false);
			else
				this->da->leftArrowWidget->setSelected(true);
	    }

	    if (this->da->rightArrowWidget) {
			if (pos >= 100)
				this->da->rightArrowWidget->setSelected(false);
			else
				this->da->rightArrowWidget->setSelected(true);
	    }
	}
}

bool MMSSliderWidget::scrollDown(unsigned int count, bool refresh, bool test, bool leave_selection) {
	// check for vertical slider
	if (!this->vertical)
		return false;

	// check for 100%
	int pos = (int)getPosition();
	if (pos >= 100)
		return false;

	// check for test mode
	if (test)
		return true;

	// increase position
	setPosition(pos+1);

	return true;
}


bool MMSSliderWidget::scrollUp(unsigned int count, bool refresh, bool test, bool leave_selection) {
	// check for vertical slider
	if (!this->vertical)
		return false;

	// check for 0%
	int pos = (int)getPosition();
	if (pos <= 0)
		return false;

	// check for test mode
	if (test)
		return true;

	// increase position
	setPosition(pos-1);

	return true;
}


bool MMSSliderWidget::scrollRight(unsigned int count, bool refresh, bool test, bool leave_selection) {
	// check for horizontal slider
	if (this->vertical)
		return false;

	// check for 100%
	int pos = (int)getPosition();
	if (pos >= 100)
		return false;

	// check for test mode
	if (test)
		return true;

	// increase position
	setPosition(pos+1);

	return true;
}


bool MMSSliderWidget::scrollLeft(unsigned int count, bool refresh, bool test, bool leave_selection) {
	// check for horizontal slider
	if (this->vertical)
		return false;

	// check for 0%
	int pos = (int)getPosition();
	if (pos <= 0)
		return false;

	// check for test mode
	if (test)
		return true;

	// increase position
	setPosition(pos-1);

	return true;
}


bool MMSSliderWidget::scrollTo(int posx, int posy, bool refresh, bool *changed) {
	if (changed)
		*changed = false;

	// searching for the image
    MMSFBSurface *suf = NULL;
    getImage(&suf);
    if (suf) {
        // calculate position of the slider
        MMSFBRectangle sgeom = getGeometry();
        calcPos(suf, &sgeom, &this->vertical);

        if (this->vertical) {
        	// vertical slider
            if (posy < sgeom.y) {
            	// slider decrement
            	if (!this->onSliderDecrement->emit(this)) {
            		// here we can dec the slider

            	}
            	if (changed) *changed = true;
           		return true;
            }
            else
            if (posy >= sgeom.y + sgeom.h) {
            	// slider increment
            	if (!this->onSliderIncrement->emit(this)) {
            		// here we can inc the slider

            	}
           		if (changed) *changed = true;
           		return true;
            }
        }
        else {
        	// horizontal slider
            if (posx < sgeom.x) {
            	// slider decrement
            	if (!this->onSliderDecrement->emit(this)) {
            		// here we can dec the slider

            	}
            	if (changed) *changed = true;
           		return true;
            }
            else
            if (posx >= sgeom.x + sgeom.w) {
            	// slider increment
            	if (!this->onSliderIncrement->emit(this)) {
            		// here we can inc the slider

            	}
           		if (changed) *changed = true;
           		return true;
            }
        }
    }

    return false;
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETSLIDER(x) \
    if (this->mySliderWidgetClass.is##x()) return mySliderWidgetClass.get##x(); \
    else if ((sliderWidgetClass)&&(sliderWidgetClass->is##x())) return sliderWidgetClass->get##x(); \
    else return this->da->theme->sliderWidgetClass.get##x();

string MMSSliderWidget::getImagePath() {
    GETSLIDER(ImagePath);
}

string MMSSliderWidget::getImageName() {
    GETSLIDER(ImageName);
}

string MMSSliderWidget::getSelImagePath() {
    GETSLIDER(SelImagePath);
}

string MMSSliderWidget::getSelImageName() {
    GETSLIDER(SelImageName);
}

string MMSSliderWidget::getImagePath_p() {
    GETSLIDER(ImagePath_p);
}

string MMSSliderWidget::getImageName_p() {
    GETSLIDER(ImageName_p);
}

string MMSSliderWidget::getSelImagePath_p() {
    GETSLIDER(SelImagePath_p);
}

string MMSSliderWidget::getSelImageName_p() {
    GETSLIDER(SelImageName_p);
}

string MMSSliderWidget::getImagePath_i() {
    GETSLIDER(ImagePath_i);
}

string MMSSliderWidget::getImageName_i() {
    GETSLIDER(ImageName_i);
}

string MMSSliderWidget::getSelImagePath_i() {
    GETSLIDER(SelImagePath_i);
}

string MMSSliderWidget::getSelImageName_i() {
    GETSLIDER(SelImageName_i);
}

unsigned int MMSSliderWidget::getPosition() {
    GETSLIDER(Position);
}

string MMSSliderWidget::getBarImagePath() {
    GETSLIDER(BarImagePath);
}

string MMSSliderWidget::getBarImageName() {
    GETSLIDER(BarImageName);
}

string MMSSliderWidget::getSelBarImagePath() {
    GETSLIDER(SelBarImagePath);
}

string MMSSliderWidget::getSelBarImageName() {
    GETSLIDER(SelBarImageName);
}


/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSSliderWidget::setImagePath(string imagepath, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath(imagepath);
    this->imagepath_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage));

			this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImageName(string imagename, bool load, bool refresh) {
	if (!this->imagepath_set) mySliderWidgetClass.unsetImagePath();
    mySliderWidgetClass.setImageName(imagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImage(string imagepath, string imagename, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath(imagepath);
    mySliderWidgetClass.setImageName(imagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImagePath(string selimagepath, bool load, bool refresh) {
	if (!this->selimagepath_set) mySliderWidgetClass.unsetSelImagePath();
    mySliderWidgetClass.setSelImagePath(selimagepath);
    this->selimagepath_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImageName(string selimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName(selimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImage(string selimagepath, string selimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath(selimagepath);
    mySliderWidgetClass.setSelImageName(selimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImagePath_p(string imagepath_p, bool load, bool refresh) {
	if (!this->imagepath_p_set) mySliderWidgetClass.unsetImagePath_p();
    mySliderWidgetClass.setImagePath_p(imagepath_p);
    this->imagepath_p_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImageName_p(string imagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setImageName_p(imagename_p);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImage_p(string imagepath_p, string imagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_p(imagepath_p);
    mySliderWidgetClass.setImageName_p(imagename_p);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImagePath_p(string selimagepath_p, bool load, bool refresh) {
	if (!this->selimagepath_p_set) mySliderWidgetClass.unsetSelImagePath_p();
    mySliderWidgetClass.setSelImagePath_p(selimagepath_p);
    this->selimagepath_p_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImageName_p(string selimagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName_p(selimagename_p);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImage_p(string selimagepath_p, string selimagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_p(selimagepath_p);
    mySliderWidgetClass.setSelImageName_p(selimagename_p);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImagePath_i(string imagepath_i, bool load, bool refresh) {
	if (!this->imagepath_i_set) mySliderWidgetClass.unsetImagePath_i();
    mySliderWidgetClass.setImagePath_i(imagepath_i);
    this->imagepath_i_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImageName_i(string imagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setImageName_i(imagename_i);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setImage_i(string imagepath_i, string imagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_i(imagepath_i);
    mySliderWidgetClass.setImageName_i(imagename_i);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImagePath_i(string selimagepath_i, bool load, bool refresh) {
	if (!this->selimagepath_i_set) mySliderWidgetClass.unsetSelImagePath_i();
    mySliderWidgetClass.setSelImagePath_i(selimagepath_i);
    this->selimagepath_i_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImageName_i(string selimagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName_i(selimagename_i);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelImage_i(string selimagepath_i, string selimagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_i(selimagepath_i);
    mySliderWidgetClass.setSelImageName_i(selimagename_i);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage));

            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setPosition(unsigned int pos, bool refresh) {
	// check range
	unsigned int cpos = getPosition();
	if (pos == cpos)
		return;
	if (pos > 100) {
		if (cpos == 100)
			return;
		pos = 100;
	}

	// update position
    mySliderWidgetClass.setPosition(pos);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}



void MMSSliderWidget::setBarImagePath(string barimagepath, bool load, bool refresh) {
    mySliderWidgetClass.setBarImagePath(barimagepath);
    this->barimagepath_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->barimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->barimage);
            this->barimage = this->rootwindow->im->getImage(getBarImagePath(), getBarImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setBarImageName(string barimagename, bool load, bool refresh) {
	if (!this->barimagepath_set) mySliderWidgetClass.unsetBarImagePath();
    mySliderWidgetClass.setBarImageName(barimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->barimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->barimage);
            this->barimage = this->rootwindow->im->getImage(getBarImagePath(), getBarImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setBarImage(string barimagepath, string barimagename, bool load, bool refresh) {
    mySliderWidgetClass.setBarImagePath(barimagepath);
    mySliderWidgetClass.setBarImageName(barimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->barimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->barimage);
            this->barimage = this->rootwindow->im->getImage(getBarImagePath(), getBarImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelBarImagePath(string selbarimagepath, bool load, bool refresh) {
	if (!this->selbarimagepath_set) mySliderWidgetClass.unsetSelBarImagePath();
    mySliderWidgetClass.setSelBarImagePath(selbarimagepath);
    this->selbarimagepath_set = true;
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selbarimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->selbarimage);
            this->selbarimage = this->rootwindow->im->getImage(getSelBarImagePath(), getSelBarImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelBarImageName(string selbarimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelBarImageName(selbarimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selbarimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->selbarimage);
            this->selbarimage = this->rootwindow->im->getImage(getSelBarImagePath(), getSelBarImageName());
        }
    }

	this->refresh(refresh);
}

void MMSSliderWidget::setSelBarImage(string selbarimagepath, string selbarimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelBarImagePath(selbarimagepath);
    mySliderWidgetClass.setSelBarImageName(selbarimagename);
    if (load) {
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selbarimage == this->current_fgbarimage));

            this->rootwindow->im->releaseImage(this->selbarimage);
            this->selbarimage = this->rootwindow->im->getImage(getSelBarImagePath(), getSelBarImageName());
        }
    }

	this->refresh(refresh);
}




void MMSSliderWidget::updateFromThemeClass(MMSSliderWidgetClass *themeClass) {
    if (themeClass->isImagePath())
        setImagePath(themeClass->getImagePath());
    if (themeClass->isImageName())
        setImageName(themeClass->getImageName());
    if (themeClass->isSelImagePath())
        setSelImagePath(themeClass->getSelImagePath());
    if (themeClass->isSelImageName())
        setSelImageName(themeClass->getSelImageName());
    if (themeClass->isImagePath_p())
        setImagePath_p(themeClass->getImagePath_p());
    if (themeClass->isImageName_p())
        setImageName_p(themeClass->getImageName_p());
    if (themeClass->isSelImagePath_p())
        setSelImagePath_p(themeClass->getSelImagePath_p());
    if (themeClass->isSelImageName_p())
        setSelImageName_p(themeClass->getSelImageName_p());
    if (themeClass->isImagePath_i())
        setImagePath_i(themeClass->getImagePath_i());
    if (themeClass->isImageName_i())
        setImageName_i(themeClass->getImageName_i());
    if (themeClass->isSelImagePath_i())
        setSelImagePath_i(themeClass->getSelImagePath_i());
    if (themeClass->isSelImageName_i())
        setSelImageName_i(themeClass->getSelImageName_i());
    if (themeClass->isPosition())
        setPosition(themeClass->getPosition());
    if (themeClass->isBarImagePath())
        setBarImagePath(themeClass->getBarImagePath());
    if (themeClass->isBarImageName())
        setBarImageName(themeClass->getBarImageName());
    if (themeClass->isSelBarImagePath())
        setSelBarImagePath(themeClass->getSelBarImagePath());
    if (themeClass->isSelBarImageName())
        setSelBarImageName(themeClass->getSelBarImageName());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
