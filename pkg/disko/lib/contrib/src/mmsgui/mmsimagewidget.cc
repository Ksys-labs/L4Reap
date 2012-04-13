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

#include "mmsgui/mmsimagewidget.h"
#include "mmsgui/mmsimagewidgetthread.h"


//#define __PUPTRACE__


MMSImageWidget::MMSImageWidget(MMSWindow *root, string className, MMSTheme *theme) : imageThread(NULL) {
	create(root, className, theme);
}

MMSImageWidget::~MMSImageWidget() {
    if (imageThread) {
        imageThread->stop();
        while(imageThread->isRunning()) {
        	usleep(1000);
        }

        delete imageThread;
        imageThread=NULL;
    }
}

bool MMSImageWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_IMAGE;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->imageWidgetClass = this->da->theme->getImageWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->imageWidgetClass.widgetClass);
    if (this->imageWidgetClass) this->da->widgetClass = &(this->imageWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    // clear
    this->imagepath_set = false;
    this->selimagepath_set = false;

    this->imagepath_p_set = false;
    this->selimagepath_p_set = false;

    this->imagepath_i_set = false;
    this->selimagepath_i_set = false;

    this->image = NULL;
    image_loaded = false;
    image_curr_index = 0;

    this->selimage = NULL;
    selimage_loaded = false;
    selimage_curr_index = 0;

    this->image_p = NULL;
    image_p_loaded = false;
    image_p_curr_index = 0;

    this->selimage_p = NULL;
    selimage_p_loaded = false;
    selimage_p_curr_index = 0;

    this->image_i = NULL;
    image_i_loaded = false;
    image_i_curr_index = 0;

    this->selimage_i = NULL;
    selimage_i_loaded = false;
    selimage_i_curr_index = 0;

    if(imageThread) {
    	imageThread->stop();
        while(imageThread->isRunning()) {
        	usleep(1000);
        }
    	delete imageThread;
    }
    imageThread = NULL;
	this->current_fgset = false;

    // create widget base
    return MMSWidget::create(root, true, false, false, true, true, true, true);
}

void MMSImageWidget::loadMyImage(string path, string filename, MMSFBSurface **surface, MMSIM_DESC_SUF **surfdesc,
								 unsigned int *index, unsigned int mirror_size, bool gen_taff) {
    /* pause the imageThread */
    if (this->imageThread)
        this->imageThread->pause(true);

    /* get image from imagemanager */
    *surface = this->rootwindow->im->getImage(path, filename, surfdesc, mirror_size, gen_taff);
    *index = 0;
    if (!*surface) {
        if (this->imageThread)
            this->imageThread->pause(false);
        return;
    }
    if (!*surfdesc) {
        if (this->imageThread)
            this->imageThread->pause(false);
        return;
    }

    /* check if I have more than one image for animation */
    if ((*surfdesc)[1].delaytime == MMSIM_DESC_SUF_END) {
        if (this->imageThread)
            this->imageThread->pause(false);
        return;
    }

    /* yes, I have something to animate */
    if (this->imageThread) {
        /* toggle pause off */
        this->imageThread->pause(false);
    }
    else {
        /* start a thread for it */
        this->imageThread = new MMSImageWidgetThread(this);
        this->imageThread->start();
    }
}

MMSWidget *MMSImageWidget::copyWidget() {
    /* create widget */
    MMSImageWidget *newWidget = new MMSImageWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->imageWidgetClass = this->imageWidgetClass;
    newWidget->myImageWidgetClass = this->myImageWidgetClass;

    newWidget->imagepath_set = this->imagepath_set;
    newWidget->selimagepath_set = this->selimagepath_set;

    newWidget->imagepath_p_set = this->imagepath_p_set;
    newWidget->selimagepath_p_set = this->selimagepath_p_set;

    newWidget->imagepath_i_set = this->imagepath_i_set;
    newWidget->selimagepath_i_set = this->selimagepath_i_set;

    newWidget->image_suf = this->image_suf;
    newWidget->image_curr_index = this->image_curr_index;
    newWidget->selimage_suf = this->selimage_suf;
    newWidget->selimage_curr_index = this->selimage_curr_index;

    newWidget->image_p_suf = this->image_p_suf;
    newWidget->image_p_curr_index = this->image_p_curr_index;
    newWidget->selimage_p_suf = this->selimage_p_suf;
    newWidget->selimage_p_curr_index = this->selimage_p_curr_index;

    newWidget->image_i_suf = this->image_i_suf;
    newWidget->image_i_curr_index = this->image_i_curr_index;
    newWidget->selimage_i_suf = this->selimage_i_suf;
    newWidget->selimage_i_curr_index = this->selimage_i_curr_index;

    newWidget->image_loaded = this->image_loaded;
    newWidget->image_p_loaded = this->image_p_loaded;
    newWidget->image_i_loaded = this->image_i_loaded;
    newWidget->selimage_loaded = this->selimage_loaded;
    newWidget->selimage_p_loaded = this->selimage_p_loaded;
    newWidget->selimage_i_loaded = this->selimage_i_loaded;

    newWidget->imageThread = this->imageThread;

    newWidget->current_fgset = this->current_fgset;
    newWidget->current_fgimage = this->current_fgimage;
    newWidget->current_fgimage2 = this->current_fgimage2;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    /* reload my images */
    newWidget->image = NULL;
    newWidget->selimage = NULL;
    newWidget->image_p = NULL;
    newWidget->selimage_p = NULL;
    newWidget->image_i = NULL;
    newWidget->selimage_i = NULL;
    if (this->rootwindow) {
    	bool b;
    	if (!getImagesOnDemand(b))
    		b = false;
        if ((!b)||(newWidget->isVisible())) {
            loadMyImage(newWidget->getImagePath(), newWidget->getImageName(),
                        &newWidget->image, &(newWidget->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
            image_loaded = true;
            loadMyImage(newWidget->getSelImagePath(), newWidget->getSelImageName(),
                        &newWidget->selimage, &(newWidget->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
            selimage_loaded = true;

            loadMyImage(newWidget->getImagePath_p(), newWidget->getImageName_p(),
                        &newWidget->image_p, &(newWidget->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
            image_p_loaded = true;
            loadMyImage(newWidget->getSelImagePath_p(), newWidget->getSelImageName_p(),
                        &newWidget->selimage_p, &(newWidget->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
            selimage_p_loaded = true;

            loadMyImage(newWidget->getImagePath_i(), newWidget->getImageName_i(),
                        &newWidget->image_i, &(newWidget->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
            image_i_loaded = true;
            loadMyImage(newWidget->getSelImagePath_i(), newWidget->getSelImageName_i(),
                        &newWidget->selimage_i, &(newWidget->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
            selimage_i_loaded = true;
        }
    }

    return newWidget;
}

bool MMSImageWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    bool b;
    if (!getImagesOnDemand(b))
    	b = false;

    if ((!b)||(this->isVisible())) {
        // load images
        if (!image_loaded) {
            loadMyImage(getImagePath(), getImageName(), &this->image, &(this->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
            image_loaded = true;
        }
        if (!selimage_loaded) {
            loadMyImage(getSelImagePath(), getSelImageName(), &this->selimage, &(this->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
            selimage_loaded = true;
        }

        if (!image_p_loaded) {
            loadMyImage(getImagePath_p(), getImageName_p(), &this->image_p, &(this->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
            image_p_loaded = true;
        }
        if (!selimage_p_loaded) {
            loadMyImage(getSelImagePath_p(), getSelImageName_p(), &this->selimage_p, &(this->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
            selimage_p_loaded = true;
        }

        if (!image_i_loaded) {
            loadMyImage(getImagePath_i(), getImageName_i(), &this->image_i, &(this->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
            image_i_loaded = true;
        }
        if (!selimage_i_loaded) {
            loadMyImage(getSelImagePath_i(), getSelImageName_i(), &this->selimage_i, &(this->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
            selimage_i_loaded = true;
        }
    }

    return true;
}

bool MMSImageWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    // release my images
    this->rootwindow->im->releaseImage(this->image);
    this->image = NULL;
    this->image_loaded = false;
    this->rootwindow->im->releaseImage(this->selimage);
    this->selimage = NULL;
    this->selimage_loaded = false;
    this->rootwindow->im->releaseImage(this->image_p);
    this->image_p = NULL;
    this->image_p_loaded = false;
    this->rootwindow->im->releaseImage(this->selimage_p);
    this->selimage_p = NULL;
    this->selimage_p_loaded = false;
    this->rootwindow->im->releaseImage(this->image_i);
    this->image_i = NULL;
    this->image_i_loaded = false;
    this->rootwindow->im->releaseImage(this->selimage_i);
    this->selimage_i = NULL;
    this->selimage_i_loaded = false;

    return true;
}

void MMSImageWidget::workWithRatio(MMSFBSurface *suf, MMSFBRectangle *surfaceGeom) {
    int w, h, dw, dh, ratio;

    if (getUseRatio()) {
	    /* use ratio from image */
	    suf->getSize(&w, &h);

	    dw = w - surfaceGeom->w;
	    dh = h - surfaceGeom->h;

	    if (dw || dh) {
	        ratio = (10000 * w) / h;
	        bool fw = false, fh = false;

	        if (getFitWidth())
	        	fw = true;
	        else
	        if (getFitHeight())
	        	fh = true;

	        if (((dw > dh)&&(!fh))||(fw)) {
	        	/* change image height */
	            int t = (10000 * surfaceGeom->w + 5000) / ratio;

	            /* work with alignment */
	            MMSALIGNMENT alignment = getAlignment();
	            if (alignment == MMSALIGNMENT_NOTSET) alignment = MMSALIGNMENT_CENTER;
	            switch (alignment) {
	                case MMSALIGNMENT_CENTER:
	                	surfaceGeom->y = (surfaceGeom->h - t) / 2;
	                    break;
	                case MMSALIGNMENT_LEFT:
	                	surfaceGeom->y = (surfaceGeom->h - t) / 2;
	                    break;
	                case MMSALIGNMENT_RIGHT:
	                	surfaceGeom->y = (surfaceGeom->h - t) / 2;
	                    break;
	                case MMSALIGNMENT_TOP_CENTER:
	                	surfaceGeom->y = 0;
	                    break;
	                case MMSALIGNMENT_TOP_LEFT:
	                	surfaceGeom->y = 0;
	                    break;
	                case MMSALIGNMENT_TOP_RIGHT:
	                	surfaceGeom->y = 0;
	                    break;
	                case MMSALIGNMENT_BOTTOM_CENTER:
	                	surfaceGeom->y = surfaceGeom->h - t;
	                    break;
	                case MMSALIGNMENT_BOTTOM_LEFT:
	                	surfaceGeom->y = surfaceGeom->h - t;
	                    break;
	                case MMSALIGNMENT_BOTTOM_RIGHT:
	                	surfaceGeom->y = surfaceGeom->h - t;
	                    break;
	                default:
	                	surfaceGeom->y = 0;
	                    break;
	            }

	            surfaceGeom->h = t;
	        }
	        else {
	        	/* change image width */
	            int t = (surfaceGeom->h * ratio + 5000) / 10000;

	            /* work with alignment */
	            MMSALIGNMENT alignment = getAlignment();
	            if (alignment == MMSALIGNMENT_NOTSET) alignment = MMSALIGNMENT_CENTER;
	            switch (alignment) {
	                case MMSALIGNMENT_CENTER:
	    	            surfaceGeom->x = (surfaceGeom->w - t) / 2;
	                    break;
	                case MMSALIGNMENT_LEFT:
	                	surfaceGeom->x = 0;
	                    break;
	                case MMSALIGNMENT_RIGHT:
	    	            surfaceGeom->x = surfaceGeom->w - t;
	                    break;
	                case MMSALIGNMENT_TOP_CENTER:
	    	            surfaceGeom->x = (surfaceGeom->w - t) / 2;
	                    break;
	                case MMSALIGNMENT_TOP_LEFT:
	                	surfaceGeom->x = 0;
	                    break;
	                case MMSALIGNMENT_TOP_RIGHT:
	    	            surfaceGeom->x = surfaceGeom->w - t;
	                    break;
	                case MMSALIGNMENT_BOTTOM_CENTER:
	    	            surfaceGeom->x = (surfaceGeom->w - t) / 2;
	                    break;
	                case MMSALIGNMENT_BOTTOM_LEFT:
	                	surfaceGeom->x = 0;
	                    break;
	                case MMSALIGNMENT_BOTTOM_RIGHT:
	    	            surfaceGeom->x = surfaceGeom->w - t;
	                    break;
	                default:
	                	surfaceGeom->x = 0;
	                    break;
	            }

	            surfaceGeom->w = t;
	        }
	    }
    }
}

void MMSImageWidget::getForeground(MMSFBSurface **image, MMSFBSurface **image2) {
	*image = NULL;
	*image2= NULL;

    if (isActivated()) {

        if (isSelected()) {
        	*image = (this->selimage)?this->selimage_suf[selimage_curr_index].surface:NULL;
        	*image2= (this->image)?this->image_suf[image_curr_index].surface:NULL;
        }
        else {
        	*image = (this->image)?this->image_suf[image_curr_index].surface:NULL;
        	*image2= (this->selimage)?this->selimage_suf[selimage_curr_index].surface:NULL;
        }
        if (isPressed()) {
            if (isSelected()) {
            	if (this->selimage_p)
            		*image = this->selimage_p_suf[selimage_p_curr_index].surface;
            	if (this->image_p)
            		*image2= this->image_p_suf[image_p_curr_index].surface;
            }
            else {
            	if (this->image_p)
            		*image = this->image_p_suf[image_p_curr_index].surface;
            	if (this->selimage_p)
            		*image2= this->selimage_p_suf[selimage_p_curr_index].surface;
            }
        }
    }
    else {
        if (isSelected()) {
        	*image = (this->selimage_i)?this->selimage_i_suf[selimage_i_curr_index].surface:NULL;
        	*image2= (this->image_i)?this->image_i_suf[image_i_curr_index].surface:NULL;
        }
        else {
        	*image = (this->image_i)?this->image_i_suf[image_i_curr_index].surface:NULL;
        	*image2= (this->selimage_i)?this->selimage_i_suf[selimage_i_curr_index].surface:NULL;
        }
    }
}

bool MMSImageWidget::enableRefresh(bool enable) {
	if (!MMSWidget::enableRefresh(enable)) return false;

	// mark foreground as not set
	this->current_fgset = false;

	return true;
}

bool MMSImageWidget::checkRefreshStatus() {
	if (MMSWidget::checkRefreshStatus()) return true;

	if (this->current_fgset) {
		// current foreground initialized
		MMSFBSurface *image, *image2;
		getForeground(&image, &image2);

		if (image == this->current_fgimage && image2 == this->current_fgimage2) {
			// foreground images not changed, so we do not enable refreshing
			return false;
		}
	}

	// (re-)enable refreshing
	enableRefresh();

	return true;
}


bool MMSImageWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if(!surface)
    	return false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    /* lock */
    this->surface->lock();

    /* draw widget basics */
    if (MMSWidget::draw(backgroundFilled)) {
        /* draw my things */
        MMSFBRectangle surfaceGeom;

        /* get the blend value */
        unsigned int blend;
        getBlend(blend);

        // get images
        MMSFBSurface *suf, *suf2;
        getForeground(&suf, &suf2);
        this->current_fgimage   = suf;
        this->current_fgimage2  = suf2;
        this->current_fgset     = true;

		if (!blend) {
			/* blend not set */
	        if (suf) {
	            /* prepare for blitting */
	            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, 255, opacity);

	            /* get surface geometry */
                surfaceGeom = getSurfaceGeometry();

                /* work with aspect ratio */
	            workWithRatio(suf, &surfaceGeom);

	            /* normal stretchblit */
	            this->surface->stretchBlit(suf, NULL, &surfaceGeom);
	        }
		}
		else {
			/* do blend between suf and suf2 */
			if (blend > 255)
				blend=255;

			/* background image */
	        if (suf) {
	            /* prepare for blitting */
	        	double blendfactor;
	        	getBlendFactor(blendfactor);
	        	unsigned int a = (unsigned int)(255 - (blendfactor * (double)blend));
	        	if (a > 255) a = 255;
	            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, a, opacity);

	            /* get surface geometry */
                surfaceGeom = getSurfaceGeometry();

                /* work with aspect ratio */
                workWithRatio(suf, &surfaceGeom);

	            /* normal stretchblit */
	            this->surface->stretchBlit(suf, NULL, &surfaceGeom);
	        }

	        /* foreground image which will blended */
	        if (suf2) {
	            /* prepare for blitting */
	            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, blend, opacity);

	            /* get surface geometry */
                surfaceGeom = getSurfaceGeometry();

                /* work with aspect ratio */
	            workWithRatio(suf2, &surfaceGeom);

	            /* normal stretchblit */
	            this->surface->stretchBlit(suf2, NULL, &surfaceGeom);
	        }
		}

        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* unlock */
    this->surface->unlock();

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}



void MMSImageWidget::setVisible(bool visible, bool refresh) {
	bool b;

    if (getImagesOnDemand(b))
    	if (b) {
	        /* load/unload on demand */
	        if (visible) {
	            /* load image on demand */
	            if (!this->isVisible()) {
	                if (!image_loaded) {
	                    loadMyImage(getImagePath(), getImageName(), &this->image, &(this->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
	                    image_loaded = true;
	                }
	                if (!selimage_loaded) {
	                    loadMyImage(getSelImagePath(), getSelImageName(), &this->selimage, &(this->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
	                    selimage_loaded = true;
	                }
	                if (!image_p_loaded) {
	                    loadMyImage(getImagePath_p(), getImageName_p(), &this->image_p, &(this->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
	                    image_p_loaded = true;
	                }
	                if (!selimage_p_loaded) {
	                    loadMyImage(getSelImagePath_p(), getSelImageName_p(), &this->selimage_p, &(this->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
	                    selimage_p_loaded = true;
	                }
	                if (!image_i_loaded) {
	                    loadMyImage(getImagePath_i(), getImageName_i(), &this->image_i, &(this->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
	                    image_i_loaded = true;
	                }
	                if (!selimage_i_loaded) {
	                    loadMyImage(getSelImagePath_i(), getSelImageName_i(), &this->selimage_i, &(this->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
	                    selimage_i_loaded = true;
	                }
	            }
	        }
	        else {
	            /* unload image on demand */
	            if (this->isVisible()) {
	                if (image_loaded) {
	                    this->rootwindow->im->releaseImage(this->image);
	                    this->image = NULL;
	                    image_loaded = false;
	                }
	                if (selimage_loaded) {
	                    this->rootwindow->im->releaseImage(this->selimage);
	                    this->selimage = NULL;
	                    selimage_loaded = false;
	                }
	                if (image_p_loaded) {
	                    this->rootwindow->im->releaseImage(this->image_p);
	                    this->image_p = NULL;
	                    image_p_loaded = false;
	                }
	                if (selimage_p_loaded) {
	                    this->rootwindow->im->releaseImage(this->selimage_p);
	                    this->selimage_p = NULL;
	                    selimage_p_loaded = false;
	                }
	                if (image_i_loaded) {
	                    this->rootwindow->im->releaseImage(this->image_i);
	                    this->image_i = NULL;
	                    image_i_loaded = false;
	                }
	                if (selimage_i_loaded) {
	                    this->rootwindow->im->releaseImage(this->selimage_i);
	                    this->selimage_i = NULL;
	                    selimage_i_loaded = false;
	                }
	            }
	        }
    	}

    /* do widget basics */
    MMSWidget::setVisible(visible, refresh);
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETIMAGE(x) \
    if (this->myImageWidgetClass.is##x()) return myImageWidgetClass.get##x(); \
    else if ((imageWidgetClass)&&(imageWidgetClass->is##x())) return imageWidgetClass->get##x(); \
    else return this->da->theme->imageWidgetClass.get##x();

string MMSImageWidget::getImagePath() {
    GETIMAGE(ImagePath);
}

string MMSImageWidget::getImageName() {
    GETIMAGE(ImageName);
}

string MMSImageWidget::getSelImagePath() {
    GETIMAGE(SelImagePath);
}

string MMSImageWidget::getSelImageName() {
    GETIMAGE(SelImageName);
}

string MMSImageWidget::getImagePath_p() {
    GETIMAGE(ImagePath_p);
}

string MMSImageWidget::getImageName_p() {
    GETIMAGE(ImageName_p);
}

string MMSImageWidget::getSelImagePath_p() {
    GETIMAGE(SelImagePath_p);
}

string MMSImageWidget::getSelImageName_p() {
    GETIMAGE(SelImageName_p);
}

string MMSImageWidget::getImagePath_i() {
    GETIMAGE(ImagePath_i);
}

string MMSImageWidget::getImageName_i() {
    GETIMAGE(ImageName_i);
}

string MMSImageWidget::getSelImagePath_i() {
    GETIMAGE(SelImagePath_i);
}

string MMSImageWidget::getSelImageName_i() {
    GETIMAGE(SelImageName_i);
}

bool MMSImageWidget::getUseRatio() {
    GETIMAGE(UseRatio);
}

bool MMSImageWidget::getFitWidth() {
    GETIMAGE(FitWidth);
}

bool MMSImageWidget::getFitHeight() {
    GETIMAGE(FitHeight);
}

MMSALIGNMENT MMSImageWidget::getAlignment() {
    GETIMAGE(Alignment);
}

unsigned int MMSImageWidget::getMirrorSize() {
    GETIMAGE(MirrorSize);
}

bool MMSImageWidget::getGenTaff() {
    GETIMAGE(GenTaff);
}


/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSImageWidget::setImagePath(string imagepath, bool load, bool refresh) {
    myImageWidgetClass.setImagePath(imagepath);
    this->imagepath_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage || this->image == this->current_fgimage2));

            this->rootwindow->im->releaseImage(this->image);
            this->image = NULL;
            image_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath(), getImageName(), &this->image, &(this->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
                image_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImageName(string imagename, bool load, bool refresh) {
	if (!this->imagepath_set) myImageWidgetClass.unsetImagePath();
    myImageWidgetClass.setImageName(imagename);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage || this->image == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image);
            this->image = NULL;
            image_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath(), getImageName(), &this->image, &(this->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
                image_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImage(string imagepath, string imagename, bool load, bool refresh) {
    myImageWidgetClass.setImagePath(imagepath);
    myImageWidgetClass.setImageName(imagename);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image == this->current_fgimage || this->image == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image);
            this->image = NULL;
            image_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath(), getImageName(), &this->image, &(this->image_suf), &image_curr_index, getMirrorSize(), getGenTaff());
                image_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImagePath(string selimagepath, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath(selimagepath);
    this->selimagepath_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage || this->selimage == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = NULL;
            selimage_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath(), getSelImageName(), &this->selimage, &(this->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
                selimage_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImageName(string selimagename, bool load, bool refresh) {
	if (!this->selimagepath_set) myImageWidgetClass.unsetSelImagePath();
    myImageWidgetClass.setSelImageName(selimagename);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage || this->selimage == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = NULL;
            selimage_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath(), getSelImageName(), &this->selimage, &(this->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
                selimage_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImage(string selimagepath, string selimagename, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath(selimagepath);
    myImageWidgetClass.setSelImageName(selimagename);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage == this->current_fgimage || this->selimage == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = NULL;
            selimage_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath(), getSelImageName(), &this->selimage, &(this->selimage_suf), &selimage_curr_index, getMirrorSize(), getGenTaff());
                selimage_loaded = true;
            }
        }

	this->refresh(refresh);
}



void MMSImageWidget::setImagePath_p(string imagepath_p, bool load, bool refresh) {
    myImageWidgetClass.setImagePath_p(imagepath_p);
    this->imagepath_p_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage || this->image_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = NULL;
            image_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_p(), getImageName_p(), &this->image_p, &(this->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
                image_p_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImageName_p(string imagename_p, bool load, bool refresh) {
	if (!this->imagepath_p_set) myImageWidgetClass.unsetImagePath_p();
    myImageWidgetClass.setImageName_p(imagename_p);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage || this->image_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = NULL;
            image_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_p(), getImageName_p(), &this->image_p, &(this->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
                image_p_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImage_p(string imagepath_p, string imagename_p, bool load, bool refresh) {
    myImageWidgetClass.setImagePath_p(imagepath_p);
    myImageWidgetClass.setImageName_p(imagename_p);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_p == this->current_fgimage || this->image_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = NULL;
            image_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_p(), getImageName_p(), &this->image_p, &(this->image_p_suf), &image_p_curr_index, getMirrorSize(), getGenTaff());
                image_p_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImagePath_p(string selimagepath_p, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath_p(selimagepath_p);
    this->selimagepath_p_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage || this->selimage_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = NULL;
            selimage_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_p(), getSelImageName_p(), &this->selimage_p, &(this->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
                selimage_p_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImageName_p(string selimagename_p, bool load, bool refresh) {
	if (!this->selimagepath_p_set) myImageWidgetClass.unsetSelImagePath_p();
    myImageWidgetClass.setSelImageName_p(selimagename_p);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage || this->selimage_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = NULL;
            selimage_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_p(), getSelImageName_p(), &this->selimage_p, &(this->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
                selimage_p_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImage_p(string selimagepath_p, string selimagename_p, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath_p(selimagepath_p);
    myImageWidgetClass.setSelImageName_p(selimagename_p);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_p == this->current_fgimage || this->selimage_p == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = NULL;
            selimage_p_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_p(), getSelImageName_p(), &this->selimage_p, &(this->selimage_p_suf), &selimage_p_curr_index, getMirrorSize(), getGenTaff());
                selimage_p_loaded = true;
            }
        }

	this->refresh(refresh);
}



void MMSImageWidget::setImagePath_i(string imagepath_i, bool load, bool refresh) {
    myImageWidgetClass.setImagePath_i(imagepath_i);
    this->imagepath_i_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage || this->image_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = NULL;
            image_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_i(), getImageName_i(), &this->image_i, &(this->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
                image_i_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImageName_i(string imagename_i, bool load, bool refresh) {
	if (!this->imagepath_i_set) myImageWidgetClass.unsetImagePath_i();
    myImageWidgetClass.setImageName_i(imagename_i);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage || this->image_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = NULL;
            image_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_i(), getImageName_i(), &this->image_i, &(this->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
                image_i_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setImage_i(string imagepath_i, string imagename_i, bool load, bool refresh) {
    myImageWidgetClass.setImagePath_i(imagepath_i);
    myImageWidgetClass.setImageName_i(imagename_i);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->image_i == this->current_fgimage || this->image_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = NULL;
            image_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getImagePath_i(), getImageName_i(), &this->image_i, &(this->image_i_suf), &image_i_curr_index, getMirrorSize(), getGenTaff());
                image_i_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImagePath_i(string selimagepath_i, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath_i(selimagepath_i);
    this->selimagepath_i_set = true;
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage || this->selimage_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = NULL;
            selimage_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_i(), getSelImageName_i(), &this->selimage_i, &(this->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
                selimage_i_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImageName_i(string selimagename_i, bool load, bool refresh) {
	if (!this->selimagepath_i_set) myImageWidgetClass.unsetSelImagePath_i();
    myImageWidgetClass.setSelImageName_i(selimagename_i);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage || this->selimage_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = NULL;
            selimage_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_i(), getSelImageName_i(), &this->selimage_i, &(this->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
                selimage_i_loaded = true;
            }
        }

	this->refresh(refresh);
}

void MMSImageWidget::setSelImage_i(string selimagepath_i, string selimagename_i, bool load, bool refresh) {
    myImageWidgetClass.setSelImagePath_i(selimagepath_i);
    myImageWidgetClass.setSelImageName_i(selimagename_i);
    if (load)
        if (this->rootwindow) {
			// refresh required?
			enableRefresh((this->selimage_i == this->current_fgimage || this->selimage_i == this->current_fgimage2));

			this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = NULL;
            selimage_i_loaded = false;
            bool b;
            if (!getImagesOnDemand(b))
            	b = false;
            if ((!b)||(this->isVisible())) {
                loadMyImage(getSelImagePath_i(), getSelImageName_i(), &this->selimage_i, &(this->selimage_i_suf), &selimage_i_curr_index, getMirrorSize(), getGenTaff());
                selimage_i_loaded = true;
            }
        }

    this->refresh(refresh);
}

void MMSImageWidget::setUseRatio(bool useratio, bool refresh) {
    myImageWidgetClass.setUseRatio(useratio);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::setFitWidth(bool fitwidth, bool refresh) {
    myImageWidgetClass.setFitWidth(fitwidth);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::setFitHeight(bool fitheight, bool refresh) {
    myImageWidgetClass.setFitHeight(fitheight);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::setAlignment(MMSALIGNMENT alignment, bool refresh) {
    myImageWidgetClass.setAlignment(alignment);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::setMirrorSize(unsigned int mirrorsize, bool refresh) {
    myImageWidgetClass.setMirrorSize(mirrorsize);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::setGenTaff(bool gentaff, bool refresh) {
    myImageWidgetClass.setGenTaff(gentaff);

    // refresh is required
    enableRefresh();

	this->refresh(refresh);
}

void MMSImageWidget::updateFromThemeClass(MMSImageWidgetClass *themeClass) {
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
    if (themeClass->isUseRatio())
        setUseRatio(themeClass->getUseRatio());
    if (themeClass->isFitWidth())
        setFitWidth(themeClass->getFitWidth());
    if (themeClass->isFitHeight())
        setFitHeight(themeClass->getFitHeight());
    if (themeClass->isAlignment())
        setAlignment(themeClass->getAlignment());
    if (themeClass->isMirrorSize())
        setMirrorSize(themeClass->getMirrorSize());
    if (themeClass->isGenTaff())
        setGenTaff(themeClass->getGenTaff());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
