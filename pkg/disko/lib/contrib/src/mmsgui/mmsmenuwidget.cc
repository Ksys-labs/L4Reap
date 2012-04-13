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

#include "mmsgui/mmsmenuwidget.h"
#include "mmsgui/mmssliderwidget.h"
#include <string.h>

#define MMSMENUWIDGET_ANIM_MAX_OFFSET 		20
#define MMSMENUWIDGET_ANIM_STEPS_PER_SECOND	(getSmoothDelay()>=100)?MMSMENUWIDGET_ANIM_MAX_OFFSET * 1000 / getSmoothDelay():MMSMENUWIDGET_ANIM_MAX_OFFSET * 5

MMSMenuWidget::MMSMenuWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget::MMSWidget() {
    // initialize the callbacks
    onSelectItem = new sigc::signal<void, MMSWidget*>;
    onBeforeScroll = new sigc::signal<void, MMSWidget*>;

    // initialize the animation callbacks
    this->onBeforeAnimation_connection	= this->pulser.onBeforeAnimation.connect(sigc::mem_fun(this, &MMSMenuWidget::onBeforeAnimation));
    this->onAnimation_connection		= this->pulser.onAnimation.connect(sigc::mem_fun(this, &MMSMenuWidget::onAnimation));
    this->onAfterAnimation_connection	= this->pulser.onAfterAnimation.connect(sigc::mem_fun(this, &MMSMenuWidget::onAfterAnimation));

    create(root, className, theme);
}

MMSMenuWidget::~MMSMenuWidget() {
    // delete the callbacks
    if (onSelectItem) delete onSelectItem;
    if (onBeforeScroll) delete onBeforeScroll;

    // disconnect callbacks from pulser
    this->onBeforeAnimation_connection.disconnect();
    this->onAnimation_connection.disconnect();
    this->onAfterAnimation_connection.disconnect();

    if (this->itemTemplate)
        delete this->itemTemplate;
}

bool MMSMenuWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_MENU;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->menuWidgetClass = this->da->theme->getMenuWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->menuWidgetClass.widgetClass);
    if (this->menuWidgetClass) this->da->widgetClass = &(this->menuWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    this->selimage = NULL;
    this->itemTemplate = NULL;

    this->item_w = 0;
    this->item_h = 0;
    this->x = 0;
    this->y = 0;
    this->px = 0;
    this->py = 0;
    this->v_items = 0;
    this->h_items = 0;

    this->firstFocus = false;
    this->firstSelection = false;

    this->zoomselwidth = 0;
    this->zoomselheight = 0;
    this->zoomselshiftx = 0;
    this->zoomselshifty = 0;

    this->smooth_scrolling = getSmoothScrolling();
    this->scrolling_offset = 0;

    this->smooth_selection = getSmoothSelection();
    this->selection_offset_x = 0;
    this->selection_offset_y = 0;

    this->frame_delay = 0;
    this->frame_delay_set = false;

    this->parent_window = NULL;
    this->curr_submenu = -1;
    this->parent_menu = NULL;
    this->back_item = -1;

    return MMSWidget::create(root, true, false, true, true, true, false, true);
}

MMSWidget *MMSMenuWidget::copyWidget() {
    /* create widget */
    MMSMenuWidget *newWidget = new MMSMenuWidget(this->rootwindow, className);

    newWidget->className = this->className;
    newWidget->menuWidgetClass = this->menuWidgetClass;
    newWidget->myMenuWidgetClass = this->myMenuWidgetClass;

    newWidget->selimage = this->selimage;
    newWidget->itemTemplate = this->itemTemplate;

    newWidget->item_w = this->item_w;     /* width of an item */
    newWidget->item_h = this->item_h;     /* height of an item */
    newWidget->v_items = this->v_items;    /* number of visible vertical items */
    newWidget->h_items = this->h_items;    /* number of visible horizontal items */

    //! x position of the selected item
    newWidget->x = this->x;
    //! y position of the selected item
    newWidget->y = this->y;
    //! scroll x-offset
    newWidget->px = this->px;
    //! scroll y-offset
    newWidget->py = this->py;

    newWidget->firstFocus = this->firstFocus;
    newWidget->firstSelection = this->firstSelection;

    newWidget->zoomsel = this->zoomsel;		/* should the selected item zoomed? */
    newWidget->zoomselwidth = this->zoomselwidth;	/* this value will be added to item_w for the selected item */
    newWidget->zoomselheight = this->zoomselheight;	/* this value will be added to item_h for the selected item */
    newWidget->zoomselshiftx = this->zoomselshiftx;	/* x-move the unselected items around the selected item */
    newWidget->zoomselshifty = this->zoomselshifty;	/* y-move the unselected items around the selected item */
    newWidget->smooth_scrolling = this->smooth_scrolling;
    newWidget->scrolling_offset = this->scrolling_offset;
    newWidget->smooth_selection = this->smooth_selection;
    newWidget->selection_offset_x = this->selection_offset_x;
    newWidget->selection_offset_y = this->selection_offset_y;

    newWidget->frame_delay = this->frame_delay;
    newWidget->frame_delay_set = this->frame_delay_set;

    newWidget->pulser_mode = this->pulser_mode;
    newWidget->anim_offset = this->anim_offset;
    newWidget->anim_jumpover = this->anim_jumpover;
    newWidget->anim_factor = this->anim_factor;

    newWidget->virtualGeom = this->virtualGeom;

    newWidget->parent_window = this->parent_window;
    newWidget->iteminfos = this->iteminfos;
    newWidget->curr_submenu = this->curr_submenu;
    newWidget->parent_menu = this->parent_menu;
    newWidget->back_item = this->back_item;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    /* reload my images */
    newWidget->selimage = NULL;

    if (drawable) {
        if (this->rootwindow) {
            string path, name;

            if (!newWidget->getSelImagePath(path)) path = "";
            if (!newWidget->getSelImageName(name)) name = "";
            newWidget->selimage = this->rootwindow->im->getImage(path, name);
        }
    }

    return newWidget;
}

bool MMSMenuWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    // load my images
    string path, name;
    if (!getSelImagePath(path)) path = "";
    if (!getSelImageName(name)) name = "";
    this->selimage = this->rootwindow->im->getImage(path, name);

    return true;
}

bool MMSMenuWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    // release my images
	this->rootwindow->im->releaseImage(this->selimage);
	this->selimage = NULL;

    return true;
}


void MMSMenuWidget::lock() {
	if (this->surface)
		this->surface->lock();
}

void MMSMenuWidget::unlock() {
	if (this->surface)
		this->surface->unlock();
}



bool MMSMenuWidget::draw(bool *backgroundFilled) {

    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    // lock
    lock();

    // draw widget basics
    if (MMSWidget::draw(backgroundFilled)) {
        // update window surface with an area of surface
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    // unlock
    unlock();

    // draw widgets debug frame
    return MMSWidget::drawDebug();
}


void MMSMenuWidget::add(MMSWidget *widget) {
    // no widget to be added here, see newItem()
}



void MMSMenuWidget::adjustVirtualRect() {
    /* we use a virtual rectangle to support smooth move of items outside widgets inner geometry */
    /* per default the rectangles are the same */
    this->virtualGeom = this->innerGeom;

    /* if smooth scrolling we need to adjust the virtual rect */
	if (this->smooth_scrolling) {
		if (getFixedPos() >= 0) {
			/* menu with fixed selection */
			if (getCols() == 1) {
				/* fixed vertical menu */
				int rih = item_h + getItemVMargin() * 2;
				int used_h = this->h_items * rih + this->zoomselwidth;
				int min_h = this->virtualGeom.h + rih;
				while (used_h < min_h) {
					used_h+=rih;
					this->v_items++;
				}
				this->virtualGeom.y-= (used_h - this->virtualGeom.h)/2;
				this->virtualGeom.h = used_h;
			}
			else {
				/* fixed horizontal menu */
				int riw = item_w + getItemHMargin() * 2;
				int used_w = this->h_items * riw + this->zoomselwidth;
				int min_w = this->virtualGeom.w + riw;
				while (used_w < min_w) {
					used_w+=riw;
					this->h_items++;
				}
				this->virtualGeom.x-= (used_w - this->virtualGeom.w)/2;
				this->virtualGeom.w = used_w;
			}
		}
	}
}

bool MMSMenuWidget::getConfig(bool *firstTime) {
    unsigned int rest;

    if (!isGeomSet()) {
        /* i must have my geometry */
        MMSWindow *root = getRootWindow();
        if (root) {
            root->recalculateChildren();
        }
        else
            return false;
    }

    /* check if config already set */
    if (this->item_w) {
        if (firstTime) *firstTime = false;

        /* we need to adjust the virtual rect */
        adjustVirtualRect();

        return true;
    }
    else
        if (firstTime) *firstTime = true;

    /* not set, fill my variables */
    if (getItemWidth() != "")
        getPixelFromSizeHint(&this->item_w, getItemWidth(), this->innerGeom.w, 0);
    else
        this->item_w = this->innerGeom.w;

    if (getItemHeight() != "")
        getPixelFromSizeHint(&this->item_h, getItemHeight(), this->innerGeom.h, 0);
    else
        this->item_h = this->innerGeom.h;

    if (this->item_w <= 0) {
        /* it seems that width should be a factor of height */
        getPixelFromSizeHint(&this->item_w, getItemWidth(), this->innerGeom.w, this->item_h);
    }
    else {
        /* it seems that height should be a factor of width */
        getPixelFromSizeHint(&this->item_h, getItemHeight(), this->innerGeom.h, this->item_w);
    }

    if (getZoomSelWidth() != "")
        getPixelFromSizeHint((int*)&this->zoomselwidth, getZoomSelWidth(), this->item_w, 0);
    else
        this->zoomselwidth = 0;

    if (getZoomSelHeight() != "")
        getPixelFromSizeHint((int*)&this->zoomselheight, getZoomSelHeight(), this->item_h, 0);
    else
        this->zoomselheight = 0;

    this->zoomsel = ((this->zoomselwidth)||(this->zoomselheight));

   	if (this->zoomsel) {
	    if (getZoomSelShiftX() != "")
	        getPixelFromSizeHint((int*)&this->zoomselshiftx, getZoomSelShiftX(), this->zoomselwidth, 0);
	    else
	        this->zoomselshiftx = 0;

	    if (getZoomSelShiftY() != "")
	        getPixelFromSizeHint((int*)&this->zoomselshifty, getZoomSelShiftY(), this->zoomselheight, 0);
	    else
	        this->zoomselshifty = 0;
    }


	/* calculate visible horizontal items */
	this->h_items = (this->innerGeom.w - this->zoomselwidth) / this->item_w;
	rest = (this->innerGeom.w - this->zoomselwidth) % this->item_w;
	do {
	    if (this->h_items * getItemHMargin() * 2 <= rest)
	        break;
	    this->h_items--;
	    rest+=item_w;
	} while (this->h_items > 0);
	if (this->h_items == 0) {
	    this->h_items = 1;
	    this->item_w-=getItemHMargin() * 2;
	}

	/* calculate visible vertical items */
    this->v_items = (this->innerGeom.h - this->zoomselheight) / this->item_h;
    rest = (this->innerGeom.h - this->zoomselheight) % this->item_h;
    do {
        if (this->v_items * getItemVMargin() * 2 <= rest)
            break;
        this->v_items--;
        rest+=item_h;
    } while (this->v_items > 0);
    if (this->v_items == 0) {
        this->v_items = 1;
        this->item_h-=getItemVMargin() * 2;
    }

    /* columns */
    if (getCols()==0)
        setCols(this->h_items);

    /* we need to adjust the virtual rect */
    adjustVirtualRect();

    return true;
}



void MMSMenuWidget::drawchildren(bool toRedrawOnly, bool *backgroundFilled, MMSFBRectangle *rect2update) {

    if ((toRedrawOnly) && (this->toRedraw==false) && (this->redrawChildren==false))
        return;

    if (!this->visible)
        return;

    // lock me
    lock();

    if (this->selimage) {
		// draw the selection image behind the items
		MMSWidget *w = getSelectedItem();
		if (w) {
			MMSFBRectangle wgeom = w->getGeometry();
			wgeom.x+=selection_offset_x-innerGeom.x;
			wgeom.y+=selection_offset_y-innerGeom.y;
            if (getCols()==1) {
                if (smooth_scrolling)
                	wgeom.y-=scrolling_offset;
            }
            else {
                if (smooth_scrolling)
                	wgeom.x-=scrolling_offset;
            }
			this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, 255, opacity);
			this->surface->stretchBlit(selimage, NULL, &wgeom);
		}
	}

	// draw the items
	MMSWidget::drawchildren(toRedrawOnly, backgroundFilled, rect2update);

    // unlock me
    unlock();
}

void MMSMenuWidget::recalculateChildren() {
    MMSFBRectangle rect;
    bool         firstTime;
    int 		 item_hmargin;
    int 		 item_vmargin;
    unsigned int cols;
    int          fixedpos, realpos;

    /* check something first */
    if(this->children.empty())
        return;

    if(!isGeomSet())
        return;

    if (!getConfig(&firstTime))
        return;

    // lock me
    lock();

    /* get values */
    item_hmargin = getItemHMargin();
    item_vmargin = getItemVMargin();
    cols = getCols();
    fixedpos = getFixedPos();

    /* normal menu or fixed selection? */
    if (fixedpos < 0) {
        /* normal menu */
        /* item pos with margin */
        int item_xx = this->virtualGeom.x + item_hmargin;
        int item_yy = this->virtualGeom.y + item_vmargin;

        /* item width/height with margin */
        int item_ww = item_w + item_hmargin * 2;
        int item_hh = item_h + item_vmargin * 2;

        /* menu pos */
        int menu_xx = this->virtualGeom.x + this->virtualGeom.w - item_w - item_hmargin;
        int menu_yy = this->virtualGeom.y + this->virtualGeom.h - item_h - item_vmargin;

        /* calc some help values if the selected item should zoomed in */
        int selected_item = 0, rows_before = 0, rows_after = 0, selected_col = 0;
       	if (this->zoomsel) {
			selected_item = getSelected();
			rows_before = (selected_item / cols) * cols;
			rows_after = rows_before + cols;
			selected_col = selected_item % cols;
       	}

       	/* through all items */
        for(int i = 0; i < (int)this->children.size(); i++) {
            rect.x = item_xx + (i % (int)cols - (int)px) * item_ww;
            rect.y = item_yy + (i / (int)cols - (int)py) * item_hh;
            rect.w = item_w;
            rect.h = item_h;

            if (cols==1) {
                if (smooth_scrolling)
                	rect.y+=scrolling_offset;
            }
            else {
                if (smooth_scrolling)
                	rect.x+=scrolling_offset;
            }

            bool visibleBefore = this->children.at(i)->isVisible();
            bool visibleAfter = (!((rect.x < item_xx) || (rect.y < item_yy) || (rect.x > menu_xx) || (rect.y > menu_yy)));

            if (!visibleAfter) {
            	// checking for smooth scrolling
                if (smooth_scrolling) {
                    if (cols==1) {
                    	// draw parts of items before and after
                        visibleAfter = (!((rect.x < item_xx) || (rect.y <= item_yy - item_hh) || (rect.x > menu_xx) || (rect.y >= menu_yy + item_hh)));
                    }
                    else {
                    	// draw parts of items before and after
                        visibleAfter = (!((rect.x <= item_xx - item_ww) || (rect.y < item_yy) || (rect.x >= menu_xx + item_ww) || (rect.y > menu_yy)));
                    }
                }
            }

            if (visibleBefore || visibleAfter) {
                if (visibleAfter) {
                    /* the item is visible, set geometry for it */
                   	if (this->zoomsel) {
                   		/* the selected item should zoomed in, so i have to align the items around it */
						if (cols == 1) {
						    /* menu with one column (vertical) */
							if (i > selected_item)
								rect.y+=this->zoomselheight;

							if (i == selected_item) {
								rect.w+=this->zoomselwidth;
								rect.h+=this->zoomselheight;
							}
							else {
								int xxx = this->zoomselwidth / 2;
								rect.x+=xxx;
								if (this->zoomselshiftx > 0) {
									xxx+=this->zoomselwidth % 2;
									if (this->zoomselshiftx < xxx)
										xxx=this->zoomselshiftx;
								}
								else {
									xxx*=-1;
									if (this->zoomselshiftx > xxx)
										xxx=this->zoomselshiftx;
								}
								rect.x+=xxx;
							}
						}
						else {
						    /* menu with columns and rows */
						    if (i < rows_before) {
								/* the rows before */
								if (i > selected_col)
									rect.x+=this->zoomselwidth;

								if (i == selected_col) {
									int xxx = this->zoomselwidth / 2;
									rect.x+=xxx;
									if (this->zoomselshiftx > 0) {
										xxx+=this->zoomselwidth % 2;
										if (this->zoomselshiftx < xxx)
											xxx=this->zoomselshiftx;
									}
									else {
										xxx*=-1;
										if (this->zoomselshiftx > xxx)
											xxx=this->zoomselshiftx;
									}
									rect.x+=xxx;

								}
						    }
						    else
						    if (i < rows_after) {
						    	/* current row */
						    	if (i > selected_item)
						    		rect.x+=this->zoomselwidth;

						    	if (i == selected_item) {
									rect.w+=this->zoomselwidth;
									rect.h+=this->zoomselheight;
								}
								else {
									int yyy = this->zoomselheight / 2;
									rect.y+=yyy;
									if (this->zoomselshifty > 0) {
										yyy+=this->zoomselheight % 2;
										if (this->zoomselshifty < yyy)
											yyy=this->zoomselshifty;
									}
									else {
										yyy*=-1;
										if (this->zoomselshifty > yyy)
											yyy=this->zoomselshifty;
									}
									rect.y+=yyy;
								}
							}
							else {
								/* the rows after */
								rect.y+=this->zoomselheight;

								if (i > selected_col)
									rect.x+=this->zoomselwidth;

								if (i == selected_col) {
									int xxx = this->zoomselwidth / 2;
									rect.x+=xxx;
									if (this->zoomselshiftx > 0) {
										xxx+=this->zoomselwidth % 2;
										if (this->zoomselshiftx < xxx)
											xxx=this->zoomselshiftx;
									}
									else {
										xxx*=-1;
										if (this->zoomselshiftx > xxx)
											xxx=this->zoomselshiftx;
									}
									rect.x+=xxx;

								}
							}
						}
                   	}


//DEBUGOUT("rect=%s,%d,%d,%d,%d\n", this->children.at(i)->getName().c_str(),rect.x,rect.y,rect.w,rect.h);
//DEBUGOUT("rectx=%d,%d\n", this->innerGeom.x,this->innerGeom.y);
//DEBUGOUT("rectx=%d,%d\n", this->virtualGeom.x,this->virtualGeom.y);

//					rect.x+=this->innerGeom.x;
//					rect.y+=this->innerGeom.y;

                	this->children.at(i)->setGeometry(rect);

                }

                /* switch the visibility */
                this->children.at(i)->setVisible(visibleAfter, false);
            }


    		if ((i+1)%cols==0)
    			selected_col+=cols;

        }
    }
    else {
        /* menu with fixed selection */
        if (cols == 1) {
            /* menu with one column (vertical) */
            if (fixedpos >= this->v_items)
                fixedpos = (this->v_items - 1) / 2;

            /* item pos with margin */
            int item_xx = this->virtualGeom.x + item_hmargin;
            int item_yy = this->virtualGeom.y + item_vmargin;

            /* item height with margin */
            int item_hh = item_h + item_vmargin * 2;

            /* menu pos */
            int menu_yy = this->virtualGeom.y + this->virtualGeom.h - item_h - item_vmargin;

            /* check if the number of children is less than visible vertical items */
            int lessItems = (int)this->v_items - (int)this->children.size();
            if (lessItems > 0) {
                /* yes, I have to recalculate the temporary menu area around the fixedpos */
                int items_above = (lessItems * (100 * fixedpos) / (this->v_items - 1) + 50) / 100;
                item_yy+= items_above * item_hh;
                fixedpos-= items_above;
                menu_yy-= (lessItems - items_above) * item_hh;
            }

            /* through all items */
            for(int i = 0; i < (int)this->children.size(); i++) {
                rect.x = item_xx;
                rect.y = item_yy + (i + fixedpos - (int)py) * item_hh;
                rect.w = item_w;
                rect.h = item_h;

                bool visibleBefore = this->children.at(i)->isVisible();
                bool visibleAfter = (!((rect.y < item_yy) || (rect.y > menu_yy)));
                if (visibleAfter)
                    realpos = (rect.y - item_yy) / item_hh;
                else
                    realpos = -1;

                if (!visibleAfter) {
                    /* the item is will not visible, but fixedpos is set, try with it */
                    if (rect.y < item_yy) {
                        /* items after, added at bottom of the menu */
                        rect.y = item_yy + (i + (int)this->children.size() + fixedpos - (int)py) * item_hh;
                        visibleAfter = (!((rect.y < item_yy) || (rect.y > menu_yy)));
                        if (visibleAfter)
                            realpos = (rect.y - item_yy) / item_hh;
                    }
                    else {
                        /* items before, added at top of the menu */
                        int ic = (int)this->children.size() - fixedpos + (int)py;
                        if (i >= ic) {
                            /* try if it can be visible from top of menu */
                            rect.y = item_yy + (i - ic) * item_hh;
                            visibleAfter = (!((rect.y < item_yy) || (rect.y > menu_yy)));
                            if (visibleAfter)
                                realpos = (rect.y - item_yy) / item_hh;
                        }
                    }
                }

                if (visibleBefore || visibleAfter) {
                    if (visibleAfter) {
                        /* the item is visible, set geometry for it */
                       	if (this->zoomsel) {
                       		/* the selected item should zoomed in, so i have to align the items around it */
							if (realpos > fixedpos)
								rect.y+=this->zoomselheight;

							if (realpos == fixedpos) {
								rect.w+=this->zoomselwidth;
								rect.h+=this->zoomselheight;
							}
							else {
								int xxx = this->zoomselwidth / 2;
								rect.x+=xxx;
								if (this->zoomselshiftx > 0) {
									xxx+=this->zoomselwidth % 2;
									if (this->zoomselshiftx < xxx)
										xxx=this->zoomselshiftx;
								}
								else {
									xxx*=-1;
									if (this->zoomselshiftx > xxx)
										xxx=this->zoomselshiftx;
								}
								rect.x+=xxx;
							}
                       	}

                    	this->children.at(i)->setGeometry(rect);

                        /* dim down and/or increase transparency */
                        int bn = 255;
                        int op = 255;
                        if (realpos < fixedpos) {
                            /* pos before selected item */
                            int dim_top = getDimTop();
                            if (dim_top > 0)
                                if (realpos+1 < fixedpos) {
                                    bn = 255 - dim_top + ((realpos * dim_top * 10) / (fixedpos - 1) + 5) / 10;
                                }
                            int trans_top = getTransTop();
                            if (trans_top > 0)
                                if (realpos+1 < fixedpos) {
                                    op = 255 - trans_top + ((realpos * trans_top * 10) / (fixedpos - 1) + 5) / 10;
                                }
                        }
                        else
                        if (realpos > fixedpos) {
                            /* pos after selected item */
                            int dim_bottom = getDimBottom();
                            if (dim_bottom > 0)
                                if (realpos-1 > fixedpos) {
                                    bn = 255 - (((realpos - fixedpos - 1) * dim_bottom * 10) / (this->v_items - fixedpos - 2) + 5) / 10;
                                }
                            int trans_bottom = getTransBottom();
                            if (trans_bottom > 0)
                                if (realpos-1 > fixedpos) {
                                    op = 255 - (((realpos - fixedpos - 1) * trans_bottom * 10) / (this->v_items - fixedpos - 2) + 5) / 10;
                                }
                        }

                        /* check if focused */
                        if ((!isFocused())&&(this->firstFocus)) {
                            bn-= getDimItems();
                            op-= getTransItems();
                        }

                        /* set brightness */
                        if (bn < 0) bn = 0;
                        if (bn > 255) bn = 255;
                        this->children.at(i)->setBrightness(bn, false);

                        /* set opacity */
                        if (op < 0) op = 0;
                        if (op > 255) op = 255;
                        this->children.at(i)->setOpacity(op, false);
                    }

                    /* switch the visibility */
                    this->children.at(i)->setVisible(visibleAfter, false);
                }
            }
        }
        else {
            /* menu with one row (horizontal) */
            if (fixedpos >= this->h_items)
                fixedpos = (this->h_items - 1) / 2;

            /* item pos with margin */
            int item_xx = this->virtualGeom.x + item_hmargin;
            int item_yy = this->virtualGeom.y + item_vmargin;

            /* item width with margin */
            int item_ww = item_w + item_hmargin * 2;

            /* menu pos */
            int menu_xx = this->virtualGeom.x + this->virtualGeom.w - item_w - item_hmargin;

            /* check if the number of children is less than visible horizontal items */
            int lessItems;
            if (cols < this->children.size())
                lessItems = (int)this->h_items - (int)cols;
            else
                lessItems = (int)this->h_items - (int)this->children.size();
            if (lessItems > 0) {
                /* yes, I have to recalculate the temporary menu area around the fixedpos */
                int items_left = (lessItems * (100 * fixedpos) / (this->h_items - 1) + 50) / 100;
                item_xx+= items_left * item_ww;
                fixedpos-= items_left;
                menu_xx-= (lessItems - items_left) * item_ww;
            }

            /* through all items */
            for(int i = 0; i < (int)this->children.size(); i++) {

                if (i >= (int)cols) {
                    /* out of first row */
                    this->children.at(i)->setVisible(false, false);
                    continue;
                }

                rect.x = item_xx + (i + fixedpos - (int)px) * item_ww;

                if (smooth_scrolling)
                	rect.x+=scrolling_offset;

				rect.y = item_yy;
                rect.w = item_w;
                rect.h = item_h;

                bool visibleBefore = this->children.at(i)->isVisible();
                bool visibleAfter = (!((rect.x < item_xx) || (rect.x > menu_xx)));

                if (visibleAfter)
                    realpos = (rect.x - item_xx) / item_ww;
                else
                    realpos = -1;

                if (!visibleAfter) {
                    /* the item is will not visible, but fixedpos is set, try with it */
                    if (rect.x < item_xx) {
                        /* items after, added at right of the menu */
                        if (cols < this->children.size())
                            rect.x = item_xx + (i + (int)cols + fixedpos - (int)px) * item_ww;
                        else
                            rect.x = item_xx + (i + (int)this->children.size() + fixedpos - (int)px) * item_ww;

                        if (!smooth_scrolling)
                        	visibleAfter = (!((rect.x < item_xx) || (rect.x > menu_xx)));
                        else {
                        	rect.x+=scrolling_offset;
                        	visibleAfter = (!((rect.x+rect.w+(int)item_hmargin < item_xx) || (rect.x-rect.w-(int)item_hmargin > menu_xx)));
                        }

                        if (visibleAfter)
                            realpos = (rect.x - item_xx) / item_ww;
                    }
                    else {
                        /* items before, added at left of the menu */
                        int ic;
                        if (cols < this->children.size())
                            ic = (int)cols - fixedpos + (int)px;
                        else
                            ic = (int)this->children.size() - fixedpos + (int)px;

                        if (i >= ic) {
                            /* try if it can be visible from left of menu */
                            rect.x = item_xx + (i - ic) * item_ww;

                            if (!smooth_scrolling)
                            	visibleAfter = (!((rect.x < item_xx) || (rect.x > menu_xx)));
                            else {
                            	rect.x+=scrolling_offset;
                            	visibleAfter = (!((rect.x+rect.w+(int)item_hmargin < item_xx) || (rect.x-rect.w-(int)item_hmargin > menu_xx)));
                            }

                            if (visibleAfter)
                                realpos = (rect.x - item_xx) / item_ww;
                        }
                    }
                }

                if (visibleBefore || visibleAfter) {
                    if (visibleAfter) {
                        /* the item is visible, set geometry for it */
                       	if (this->zoomsel) {
                       		/* the selected item should zoomed in, so i have to align the items around it */
			                if (!smooth_scrolling) {
			                	/* smooth scrolling not desired */
								if (realpos > fixedpos) {
									rect.x+=(int)this->zoomselwidth;
								}

								if (realpos == fixedpos) {
									rect.w+=this->zoomselwidth;
									rect.h+=this->zoomselheight;
								}
								else {
									int yyy = this->zoomselheight / 2;
									rect.y+=yyy;
									if (this->zoomselshifty > 0) {
										yyy+=this->zoomselheight % 2;
										if (this->zoomselshifty < yyy)
											yyy=this->zoomselshifty;
									}
									else {
										yyy*=-1;
										if (this->zoomselshifty > yyy)
											yyy=this->zoomselshifty;
									}
									rect.y+=yyy;
								}
			                }
			                else {
			                	/* smooth scrolling */
			                	// DEBUGOUT("scroll smooth\n");
			                	/* get the percent of scrolling */
				                int d = (10000*scrolling_offset) / (this->item_w + item_hmargin*2);

								if (realpos > fixedpos) {
									rect.x+=(int)this->zoomselwidth;
								}

								if (d>=0)
								{
									if (realpos == fixedpos) {
										/* set blend value */
										this->children.at(i)->setBlend((d*255 + 5000) / 10000, false);

										/* calculate item's rectangle */
										int dd = ((int)this->zoomselwidth * d + 5000) / 10000;
										rect.x+=dd;
										rect.y+=((((int)this->zoomselheight/2)+this->zoomselshifty) * d + 5000) / 10000;
										rect.w+=(int)this->zoomselwidth - dd;
										rect.h+=(int)this->zoomselheight - ((int)this->zoomselheight * d + 5000) / 10000;
									}
									else {
										int yyy = this->zoomselheight / 2;
										rect.y+=yyy;
										if (this->zoomselshifty > 0) {
											yyy+=this->zoomselheight % 2;
											if (this->zoomselshifty < yyy)
												yyy=this->zoomselshifty;
										}
										else {
											yyy*=-1;
											if (this->zoomselshifty > yyy)
												yyy=this->zoomselshifty;
										}
										rect.y+=yyy;

										if ((d>0)&&(realpos+1 == fixedpos)) {
											/* set blend value */
											this->children.at(i)->setBlend((d*255 + 5000) / 10000, false);

											/* calculate item's rectangle */
											rect.y-=((((int)this->zoomselheight/2)+this->zoomselshifty) * d + 5000) / 10000;
											rect.w+=((int)this->zoomselwidth * d + 5000) / 10000;
											rect.h+=((int)this->zoomselheight * d + 5000) / 10000;
										}
										else
											/* reset blend value */
											this->children.at(i)->setBlend(0, false);
									}
								}
								else
								{
									if (realpos+1 == fixedpos) {
										/* set blend value */
										this->children.at(i)->setBlend((d*-255 + 5000) / 10000, false);

										/* calculate item's rectangle */
										rect.y-=((((int)this->zoomselheight/2)+this->zoomselshifty) * d - 5000) / 10000;
										rect.w+=(int)this->zoomselwidth + ((int)this->zoomselwidth * d - 5000) / 10000;
										rect.h+=(int)this->zoomselheight + ((int)this->zoomselheight * d - 5000) / 10000;
									}
									else {
										int yyy = this->zoomselheight / 2;
										rect.y+=yyy;
										if (this->zoomselshifty > 0) {
											yyy+=this->zoomselheight % 2;
											if (this->zoomselshifty < yyy)
												yyy=this->zoomselshifty;
										}
										else {
											yyy*=-1;
											if (this->zoomselshifty > yyy)
												yyy=this->zoomselshifty;
										}
										rect.y+=yyy;

										if (realpos == fixedpos) {
											/* set blend value */
											this->children.at(i)->setBlend((d*-255 + 5000) / 10000, false);

											/* calculate item's rectangle */
											rect.x+=this->zoomselwidth+((int)this->zoomselwidth * d - 5000) / 10000;
											rect.y+=((((int)this->zoomselheight/2)+this->zoomselshifty) * d - 5000) / 10000;
											rect.w-=((int)this->zoomselwidth * d - 5000) / 10000;
											rect.h-=((int)this->zoomselheight * d - 5000) / 10000;
										}
										else
											/* reset blend value */
											this->children.at(i)->setBlend(0, false);
									}
	                       		}

			                }
                       	}

                    	this->children.at(i)->setGeometry(rect);

                        /* dim down and/or increase transparency */
                        int bn = 255;
                        int op = 255;
                        if (realpos < fixedpos) {
                            /* pos before selected item */
                            int dim_left = getDimLeft();
                            if (dim_left > 0)
                                if (realpos+1 < fixedpos) {
                                    bn = 255 - dim_left + ((realpos * dim_left * 10) / (fixedpos - 1) + 5) / 10;
                                }
                            int trans_left = getTransLeft();
                            if (trans_left > 0)
                                if (realpos+1 < fixedpos) {
                                    op = 255 - trans_left + ((realpos * trans_left * 10) / (fixedpos - 1) + 5) / 10;
                                }
                        }
                        else
                        if (realpos > fixedpos) {
                            /* pos after selected item */
                            int dim_right = getDimRight();
                            if (dim_right > 0)
                                if (realpos-1 > fixedpos) {
                                    bn = 255 - (((realpos - fixedpos - 1) * dim_right * 10) / (this->h_items - fixedpos - 2) + 5) / 10;
                                }
                            int trans_right = getTransRight();
                            if (trans_right > 0)
                                if (realpos-1 > fixedpos) {
                                    op = 255 - (((realpos - fixedpos - 1) * trans_right * 10) / (this->h_items - fixedpos - 2) + 5) / 10;
                                }
                        }

                        /* check if focused */
                        if (!isFocused()) {
                            bn-= getDimItems();
                            op-= getTransItems();
                        }

                        /* set brightness */
                        if (bn < 0) bn = 0;
                        if (bn > 255) bn = 255;
                        this->children.at(i)->setBrightness(bn, false);

                        /* set opacity */
                        if (op < 0) op = 0;
                        if (op > 255) op = 255;
                        this->children.at(i)->setOpacity((unsigned char)op, false);
                    }

                    /* switch the visibility */
                    this->children.at(i)->setVisible(visibleAfter, false);
                }
            }
        }
    }

    // unlock me
    unlock();

    return;
}

void MMSMenuWidget::initParentWindow(void) {
	if (!this->rootwindow) return;

	/* get the parent window */
	this->parent_window = NULL;
	string pw = getParentWindow();
	if (pw!="") {
		MMSWindow *p = this->rootwindow->getParent(true);
		if (p)
			this->parent_window = p->findWindow(pw);
	}
	if (!this->parent_window)
		this->parent_window = this->rootwindow;
}

void MMSMenuWidget::setRootWindow(MMSWindow *root, MMSWindow *parentroot) {
	MMSWidget::setRootWindow(root, parentroot);
	initParentWindow();
}


void MMSMenuWidget::switchArrowWidgets() {
    // connect arrow widgets
    loadArrowWidgets();

    // get columns
    unsigned int cols = getCols();

    // arrow support is not needed for menus with fixed selection position
    if (getFixedPos() >= 0) {
        // select the correct arrows
        if (cols > 1) {
            // horizontal menu
            if (this->da->leftArrowWidget)
                this->da->leftArrowWidget->setSelected(true);
            if (this->da->rightArrowWidget)
                this->da->rightArrowWidget->setSelected(true);
            if (this->da->upArrowWidget)
                this->da->upArrowWidget->setSelected(false);
            if (this->da->downArrowWidget)
                this->da->downArrowWidget->setSelected(false);
        }
        else {
            // vertical menu
            if (this->da->leftArrowWidget)
                this->da->leftArrowWidget->setSelected(false);
            if (this->da->rightArrowWidget)
                this->da->rightArrowWidget->setSelected(false);
            if (this->da->upArrowWidget)
                this->da->upArrowWidget->setSelected(true);
            if (this->da->downArrowWidget)
                this->da->downArrowWidget->setSelected(true);
        }
        return;
    }

    // switch arrow widgets
    if (this->da->leftArrowWidget) {
        if (this->px == 0)
            this->da->leftArrowWidget->setSelected(false);
        else
            this->da->leftArrowWidget->setSelected(true);
    }

    if (this->da->upArrowWidget) {
        if (this->py == 0)
            this->da->upArrowWidget->setSelected(false);
        else
            this->da->upArrowWidget->setSelected(true);
    }

    if (this->da->rightArrowWidget) {
        unsigned int columns = cols;
        if (columns > children.size())
            columns = children.size();
        if ((int)(columns - this->px) > this->h_items)
            this->da->rightArrowWidget->setSelected(true);
        else
            this->da->rightArrowWidget->setSelected(false);
    }

    if (this->da->downArrowWidget) {
        if ((int)(children.size() / cols + ((children.size() % cols)?1:0) - this->py) > this->v_items)
            this->da->downArrowWidget->setSelected(true);
        else
            this->da->downArrowWidget->setSelected(false);
    }
}

void MMSMenuWidget::emitOnReturnForParents(MMSMenuWidget *orw) {
	// first all parents recursive on to the stack
	if (this->parent_menu)
		this->parent_menu->emitOnReturnForParents(orw);
	// fire the onReturn event
	this->onReturn->emit(orw);
}

bool MMSMenuWidget::callOnReturn() {

	if (!switchToSubMenu()) {
		// call onReturn for all parents
		if (this->parent_menu)
			this->parent_menu->emitOnReturnForParents(this);

		// close all sub menus
		switchBackToParentMenu(MMSDIRECTION_NOTSET, true);

		// return true, so widget will call onReturn for this menu
		return true;
	}

	// i have switched the menu, so onReturn should not called
	return false;
}

bool MMSMenuWidget::switchToSubMenu() {

	// get access to the submenu struct
	unsigned int sel = getSelected();
	if (this->back_item == (int)sel) {
		// the user has chosen the go-back-item
		switchBackToParentMenu();
		return true;
	}
	if (sel>=this->iteminfos.size()) return false;
	MMSMENUITEMINFOS *sm = &(this->iteminfos.at(sel));

	// get access to the submenu window
	if ((!this->rootwindow)||(sm->name=="")) return false;
	if (!sm->window) {
		MMSWindow *p = this->parent_window->getParent();
		if (!p) return false;
		sm->window=p->findWindow(sm->name);
	}
	if (!sm->window) return false;

	// get access to the menu widget of the submenu window
	if (!sm->menu) {
		sm->menu = (MMSMenuWidget *)sm->window->findWidgetType(MMSWIDGETTYPE_MENU);
		if (!sm->menu) return false;
	}

	// switch to the other menu (window)
	this->curr_submenu = sel;
	sm->menu->parent_menu = this;
	sm->menu->setSelected(0);
	sm->window->setFocus();
	if (memcmp(&this->parent_window->geom, &sm->window->geom, sizeof(sm->window->geom))==0)
		// same geom for me and submenu, hide me
		this->parent_window->hide();
	sm->window->show();
	return true;
}

bool MMSMenuWidget::switchBackToParentMenu(MMSDIRECTION direction, bool closeall) {
	// check if we can go back to the parent menu
	if (!this->parent_menu) return false;
	MMSFBRectangle pgeom = this->parent_menu->parent_window->geom;
	MMSFBRectangle mgeom = this->parent_window->geom;
	switch (direction) {
		case MMSDIRECTION_LEFT:
			if (pgeom.x >= mgeom.x) return false;
			break;
		case MMSDIRECTION_RIGHT:
			if (pgeom.x+pgeom.w <= mgeom.x+mgeom.w) return false;
			break;
		case MMSDIRECTION_UP:
			if (pgeom.y >= mgeom.y) return false;
			break;
		case MMSDIRECTION_DOWN:
			if (pgeom.y+pgeom.h <= mgeom.y+mgeom.h) return false;
			break;
		default:
			break;
	}

	// okay, switch back
	if (this->parent_window->getFocus())
		this->parent_menu->parent_window->setFocus();
	this->parent_window->hide();
	this->parent_menu->parent_window->show();
	this->parent_menu->curr_submenu = -1;
	MMSMenuWidget *menu = this->parent_menu;
	this->parent_menu = NULL;
	if (closeall)
		menu->switchBackToParentMenu(direction, closeall);
	return true;
}

void MMSMenuWidget::setSliders() {
    MMSSliderWidget *s;

    // lock me
    lock();

    /* get columns */
    unsigned int cols = getCols();

    if(this->da->vSliderWidget && (s = dynamic_cast<MMSSliderWidget*>(this->da->vSliderWidget))) {
        unsigned int pos = 0;
        int size = (int)children.size() - 1;
        if (size > 0)
            pos = (this->y * 100) / (size / cols + ((size % cols)?1:0));
    	s->setPosition(pos);
    }

    if ((this->da->hSliderWidget)&&(cols>1) && (s = dynamic_cast<MMSSliderWidget*>(this->da->hSliderWidget))) {
        unsigned int pos = 0;
        int size = (int)children.size() - 1;
        if (size >= (int)cols) size = cols - 1;
        if (size > 0)
            pos = (this->x * 100) / size;
    	s->setPosition(pos);
    }

    // unlock me
    unlock();
}

void MMSMenuWidget::selectItem(MMSWidget *item, bool set, bool refresh, bool refreshall) {

    if(!item)
      return;

    // lock me
    lock();

    if (selimage) {
    	// we have an selection image, so we have to enable refresh for specified item
    	if (!item->checkRefreshStatus()) {
    		item->enableRefresh(true);
    	}
    }

    item->setSelected(set, refresh);

    if (refreshall) {
        // refresh is required
        enableRefresh();

        this->refresh();
    }
    if (set) {
        this->onSelectItem->emit(item);
    }

    // unlock me
    unlock();
}




#define MMSMENUWIDGET_GET_SLOOP(sloop) \
	{ sloop = getSmoothDelay(); \
	if (!sloop) { sloop = 5; this->frame_delay = 0;	this->frame_delay_set = true; } \
	else sloop = getFrameNum(sloop); }

#define MMSMENUWIDGET_SSLEEP { msleep(this->frame_delay); }

#define MMSMENUWIDGET_GET_SSTART { if (!this->frame_delay_set) start_ts = getMTimeStamp(); }

#define MMSMENUWIDGET_GET_SEND \
	{ if (!this->frame_delay_set) { \
	end_ts = getMTimeStamp(); \
    this->frame_delay = getFrameDelay(start_ts, end_ts); \
    fd_sum+= frame_delay; } }

#define MMSMENUWIDGET_CALC_DELAY \
	{ if (!this->frame_delay_set) { \
    this->frame_delay = fd_sum / (sloop - 1); \
    this->frame_delay_set = true; } }



bool MMSMenuWidget::onBeforeAnimation(MMSPulser *pulser) {
	//unlock mmsfb as it is not necessary before the first frame draw
	mmsfb->unlock();

	// init animation
	switch (this->pulser_mode) {
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_UP:
		this->scrolling_offset = 0;
		this->anim_factor = getItemVMargin() * 2 + this->item_h;
		break;
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT:
		this->scrolling_offset = 0;
		this->anim_factor = getItemHMargin() * 2 + this->item_w;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_UP:
		this->selection_offset_x = 0;
		this->selection_offset_y = 0;
		this->anim_factor = getItemVMargin() * 2 + this->item_h;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_LEFT:
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_RIGHT:
		this->selection_offset_x = 0;
		this->selection_offset_y = 0;
		this->anim_factor = getItemHMargin() * 2 + this->item_w;
		break;
	}

	return true;
}

bool MMSMenuWidget::onAnimation(MMSPulser *pulser) {
	// next offset
	switch (this->pulser_mode) {
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT:
		this->scrolling_offset = this->anim_offset - (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		break;
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_UP:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT:
		this->scrolling_offset = this->anim_offset + (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_DOWN:
		this->selection_offset_y = this->anim_offset + (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		this->selection_offset_y*= this->anim_jumpover + 1;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_UP:
		this->selection_offset_y = this->anim_offset - (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		this->selection_offset_y*= this->anim_jumpover + 1;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_LEFT:
		this->selection_offset_x = this->anim_offset - (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		this->selection_offset_x*= this->anim_jumpover + 1;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_RIGHT:
		this->selection_offset_x = this->anim_offset + (int)(((this->anim_factor * pulser->getOffset()) / MMSMENUWIDGET_ANIM_MAX_OFFSET) + 0.5);
		this->selection_offset_x*= this->anim_jumpover + 1;
		break;
	}

	// lock mmsfb to ensure single thread access to mmsgui
	mmsfb->lock();

	// refresh is required
    enableRefresh();

    // update screen
	this->refresh();

	// unlock mmsfb to enable other threads to
	// update the screen between frames
	mmsfb->unlock();
	return true;
}

void MMSMenuWidget::onAfterAnimation(MMSPulser *pulser) {

	// reset at the end of the animation
	switch (this->pulser_mode) {
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_UP:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT:
		this->scrolling_offset = 0;
		break;
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_UP:
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_LEFT:
	case MMSMENUWIDGET_PULSER_MODE_MOVESEL_RIGHT:
		this->selection_offset_x = 0;
		this->selection_offset_y = 0;
		break;
	}

	//relock mmsfb because the last frame unlocked mmsfb
	mmsfb->lock();
	return;
}


void MMSMenuWidget::startAnimation(MMSMENUWIDGET_PULSER_MODE pulser_mode, double anim_offset, int anim_jumpover) {

	MMSSEQUENCEMODE	seq_mode;

	// get source sequence mode
	switch (pulser_mode) {
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_DOWN:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_UP:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT:
	case MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT:
		// smooth scrolling mode
		seq_mode = this->smooth_scrolling;
		break;
	default:
		// smooth selection mode
		seq_mode = this->smooth_selection;
		break;
	}

	// save offset and jump over cnt
	this->anim_offset = anim_offset;
	this->anim_jumpover = anim_jumpover;

	// init pulser and start it
	this->pulser.setStepsPerSecond(MMSMENUWIDGET_ANIM_STEPS_PER_SECOND);
	switch (seq_mode) {
	case MMSSEQUENCEMODE_LOG:
		this->pulser.setMaxOffset(MMSMENUWIDGET_ANIM_MAX_OFFSET, MMSPULSER_SEQ_LOG_SOFT_START_AND_END);
		break;
	case MMSSEQUENCEMODE_LOG_SOFT_START:
		this->pulser.setMaxOffset(MMSMENUWIDGET_ANIM_MAX_OFFSET, MMSPULSER_SEQ_LOG_SOFT_START);
		break;
	case MMSSEQUENCEMODE_LOG_SOFT_END:
		this->pulser.setMaxOffset(MMSMENUWIDGET_ANIM_MAX_OFFSET, MMSPULSER_SEQ_LOG_SOFT_END);
		break;
	default:
		this->pulser.setMaxOffset(MMSMENUWIDGET_ANIM_MAX_OFFSET, MMSPULSER_SEQ_LINEAR);
		break;
	}
	this->pulser_mode = pulser_mode;
	this->pulser.start(false);
}



bool MMSMenuWidget::scrollDownEx(unsigned int count, bool refresh, bool test, bool leave_selection) {
    bool pyChanged = false;
    int oldx=0;
    int oldy;
    unsigned int cols;
    int fixedpos;

    // check something
    if (count==0 || children.empty()) {
        return false;
    }

    // get settings
    cols = getCols();
    fixedpos = getFixedPos();

    // test for deactivated items
    while((this->x + (this->y + count) * cols) < children.size()) {
        if(children.at(this->x + (this->y + count) * cols)->isActivated()) break;
        count++;
    }

    // normal menu or fixed selection?
    if (fixedpos < 0) {
        // normal menu
        if (!leave_selection) {
        	// we have to change the selected menu item!!!
	        if (this->x + (this->y + count) * cols >= children.size()) {
	            if (this->x == 0) {
	                // really nothing to scroll?
	                if (getVLoop()) {
	                    // I should not give up the focus
	                    if (this->y) {
	                        return scrollUpEx(this->y, refresh, test, leave_selection);
	                    }
						return true;
	                }

	                return false;
	            }

	            for (int i = (int)this->x - 1; i >= 0; i--)
	                if (i + (this->y + count) * cols < children.size()) {
	                    // save old and set new x selection
	                    oldx = this->x;
	                    if (!test)
	                        this->x = i;
	                    break;
	                }

	            if (!oldx) {
	                // really nothing to scroll?
	                if (getVLoop()) {
	                    // I should not give up the focus
	                    if (this->y) {
	                        return scrollUpEx(this->y, refresh, test, leave_selection);
	                    }
						return true;
	                }

	                return false;
	            }
	        }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

            // callback
            this->onBeforeScroll->emit(this);

            // save old and set new y selection
	        oldy = this->y;
	        this->y+=count;

	        // recalculate scroll position
	        int ypy = this->y - this->py;
	        if (ypy >= this->v_items) {
	            this->py = this->y - this->v_items + 1;
	            pyChanged = true;
	        }
	        else
	        if (ypy < 0) {
	            this->py = this->y;
	            pyChanged = true;
	        }
	        if (oldx) {
	            if (this->x < this->px) {
	                this->px = this->x;
	                pyChanged = true;
	            }
	        }

	        // get access to widgets
			unsigned int olditem_index = ((oldx)?oldx:this->x) + oldy * cols;
			unsigned int item_index = this->x + this->y * cols;
	        MMSWidget *olditem = (olditem_index < children.size()) ? children.at(olditem_index) : NULL;
	        MMSWidget *item    = (item_index    < children.size()) ? children.at(item_index)    : NULL;

	        if (!pyChanged) {
	            // not scrolled, switch focus between visible children
				selectItem(olditem, false, true);

				if ((selimage)&&(this->smooth_selection)&&(refresh)&&(oldy < this->y)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
					startAnimation(MMSMENUWIDGET_PULSER_MODE_MOVESEL_DOWN,
											-(double)(getItemVMargin() * 2 + this->item_h),
											count - 1);
	            }

	        	// switch on new selection
				selectItem(item, true, refresh);
	        }
	        else {
	            // scrolled, switch focus needs recalculate children
	            selectItem(olditem, false, false);

	            if ((this->smooth_scrolling)&&(refresh)&&(oldy < this->y)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_DOWN,
											(double)(getItemVMargin() * 2 + this->item_h),
											count - 1);
	            }

	            if (refresh)
	                recalculateChildren();

	            selectItem(item, true, false, refresh);
	        }
        }
        else {
        	// we have to leave the selected menu item asis!!!
            if (this->x + (this->py + this->v_items + count - 1) * cols >= children.size()) {
                return false;
            }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

	        // recalculate scroll position
            this->py++;

            // refresh is required
            enableRefresh();

            if (refresh) {
                recalculateChildren();
                this->refresh();
            }
        }

        // set the sliders
        setSliders();

        return true;
    }
    else {
        // menu with fixed selection
        if (cols == 1) {
            // in test mode we can say that we can scroll
            if (test) {
                return true;
            }

            // callback
            this->onBeforeScroll->emit(this);

            // correct menu with one column
            count%=children.size();

            // save old and set new y selection
            oldy = this->y;
            this->y+=count;

            if (this->y >= (int)children.size()) {
                // go back to begin of the list (round robin)
                this->y = this->y - children.size();
            }

            // recalculate scroll position
            this->py = this->y;

	        // get access to widgets
	        MMSWidget *olditem = (oldy    < children.size()) ? children.at(oldy)    : NULL;
	        MMSWidget *item    = (this->y < children.size()) ? children.at(this->y) : NULL;

            // switch focus and recalculate children
            selectItem(olditem, false, false);

            if (refresh)
                recalculateChildren();

            selectItem(item, true, false, refresh);

            // set the sliders
            setSliders();

            return true;
        }
        else {
            // menu with more than one column cannot be scrolled down in fixed selection mode
            return false;
        }
    }
}

bool MMSMenuWidget::scrollUpEx(unsigned int count, bool refresh, bool test, bool leave_selection) {
    bool pyChanged = false;
    int oldy;
    unsigned int cols;
    int fixedpos;

    // check something
    if (count==0 || children.empty()) {
        return false;
    }

    // get settings
    cols = getCols();
    fixedpos = getFixedPos();

    // test for deactivated items
    while(int(this->x + (this->y - count) * cols) > 0) {
        if(children.at(this->x + (this->y - count) * cols)->isActivated()) break;
        count++;
    }

    // normal menu or fixed selection?
    if (fixedpos < 0) {
        // normal menu
        if (!leave_selection) {
        	// we have to change the selected menu item!!!
	        if (this->y < (int)count) {
	            // really nothing to scroll?
	            if (getVLoop()) {
	                // I should not give up the focus
	                unsigned int lines = this->children.size() /* / cols */;
	                // if (this->children.size() % cols > 0) lines++;
	                if ((int)lines - (int)this->y > 1) {
	                    return scrollDownEx(lines - this->y - 1, refresh, test, leave_selection);
	                }
					return true;
	            }

	            return false;
	        }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

            // callback
            this->onBeforeScroll->emit(this);

            // save old and set new y selection
	        oldy = this->y;
	        this->y-=count;

	        // recalculate scroll position
	        int ypy = this->y - this->py;
	        if (ypy < 0) {
	            this->py = this->y;
	            pyChanged = true;
	        }
	        else
	        if (ypy >= this->v_items) {
	            this->py = this->y - this->v_items + 1;
	            pyChanged = true;
	        }

	        // get access to widgets
			unsigned int olditem_index = this->x + oldy * cols;
			unsigned int item_index = this->x + this->y * cols;
	        MMSWidget *olditem = (olditem_index < children.size()) ? children.at(olditem_index) : NULL;
	        MMSWidget *item    = (item_index    < children.size()) ? children.at(item_index)    : NULL;

	        if (!pyChanged) {
	            // not scrolled, switch focus between visible children
				selectItem(olditem, false, true);

            	// selection animation?
				if ((selimage)&&(this->smooth_selection)&&(refresh)&&(oldy > this->y)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
					startAnimation(MMSMENUWIDGET_PULSER_MODE_MOVESEL_UP,
											getItemVMargin() * 2 + this->item_h,
											count - 1);
	            }

	        	// switch on new selection
				selectItem(item, true, refresh);
	        }
	        else {
	            // scrolled, switch focus needs recalculate children
	            selectItem(olditem, false, false);

	            if ((this->smooth_scrolling)&&(refresh)&&(oldy > this->y)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_UP,
											-(double)(getItemVMargin() * 2 + this->item_h),
											count - 1);
	            }

	            if (refresh)
	                recalculateChildren();

	            selectItem(item, true, false, refresh);
	        }
        }
        else {
        	// we have to leave the selected menu item asis!!!
            if (this->py < (int)count) {
                return false;
            }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

	        // recalculate scroll position
            this->py--;

            // refresh is required
            enableRefresh();

            if (refresh) {
                recalculateChildren();
                this->refresh();
            }
        }

        // set the sliders
        setSliders();

        return true;
    }
    else {
        // menu with fixed selection
        if (cols == 1) {
            // in test mode we can say that we can scroll
            if (test) {
                return true;
            }

            // callback
            this->onBeforeScroll->emit(this);

            // correct menu with one column
            count%=children.size();

            // save old and set new y selection
            oldy = this->y;
            this->y-=count;

            if ((int)this->y < 0) {
                // go back to end of the list (round robin)
                this->y = children.size() + (int)this->y;
            }

            // recalculate scroll position
            this->py = this->y;

	        // get access to widgets
	        MMSWidget *olditem = (oldy    < children.size()) ? children.at(oldy)    : NULL;
	        MMSWidget *item    = (this->y < children.size()) ? children.at(this->y) : NULL;

            // switch focus and recalculate children
            selectItem(olditem, false, false);

            if (refresh)
                recalculateChildren();

            selectItem(item, true, false, refresh);

            // set the sliders
            setSliders();

            return true;
        }
        else {
            // menu with more than one column cannot be scrolled up in fixed selection mode
            return false;
        }
    }
}


bool MMSMenuWidget::scrollRightEx(unsigned int count, bool refresh, bool test, bool leave_selection) {
    bool pxChanged = false;
    int oldx;
    int oldy=0;
    unsigned int cols;
    int fixedpos;

    // check something
    if (count==0 || children.empty()) {
        return false;
    }

    // get settings
    cols = getCols();
    fixedpos = getFixedPos();

    // normal menu or fixed selection?
    if (fixedpos < 0) {
        // normal menu
        if (!leave_selection) {
        	// we have to change the selected menu item!!!
	        if (this->x + count + this->y * cols >= children.size()) {
	            if ((this->x + count >= cols) || (this->y == 0)) {
	                // really nothing to scroll?
	                if (getHLoop()) {
	                    // I should not give up the focus
	                    if (this->x) {
							if (children.size() <= cols) {
								return scrollLeftEx(this->x, refresh, test, leave_selection);
							}
							else {
								if (!scrollLeftEx(this->x, false, test, leave_selection)) {
								    return false;
								}
								return scrollUpEx(1, refresh, test, leave_selection);
							}
	                    }
	                    else {
	                        return true;
	                    }
	                }

	                return false;
	            }

	            for (int i = (int)this->y - 1; i >= 0; i--)
	                if (this->x + count + i * cols < children.size()) {
	                    oldy = this->y;
	                    if (!test)
	                        this->y = i;
	                    break;
	                }

	            if (!oldy) {
	                // really nothing to scroll?
	                if (getHLoop()) {
	                    // I should not give up the focus
	                    if (this->x) {
	                        return scrollLeftEx(this->x, refresh, test, leave_selection);
	                    }
						return true;
	                }

	                return false;
	            }
	        }
	        else
	        if (this->x + count >= cols) {
	            // really nothing to scroll?
	            if (getHLoop()) {
	                // I should not give up the focus
	                if (this->x) {
						if (children.size() <= cols) {
							return scrollLeftEx(this->x, refresh, test, leave_selection);
						}
						else {
							if (!scrollLeftEx(this->x, false, test, leave_selection)) {
							    return false;
							}
							return scrollDownEx(1, refresh, test, leave_selection);
						}
	                }
	                else {
	                    return true;
	                }
	            }

	            return false;
	        }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

            // callback
            this->onBeforeScroll->emit(this);

            // save old and set new x selection
	        oldx = this->x;
	        this->x+=count;

	        // recalculate scroll position
	        int xpx = this->x - this->px;
	        if (xpx >= this->h_items) {
	            this->px = this->x - this->h_items + 1;
	            pxChanged = true;
	        }
	        else
	        if (xpx < 0) {
	            this->px = this->x;
	            pxChanged = true;
	        }
	        if (oldy) {
	            if (this->y < this->py) {
	                this->py = this->y;
	                pxChanged = true;
	            }
	        }

	        // get access to widgets
			unsigned int olditem_index = oldx + ((oldy)?oldy:this->y) * cols;
			unsigned int item_index = this->x + this->y * cols;
	        MMSWidget *olditem = (olditem_index < children.size()) ? children.at(olditem_index) : NULL;
	        MMSWidget *item    = (item_index    < children.size()) ? children.at(item_index)    : NULL;

	        if (!pxChanged) {
	            // not scrolled, switch focus between visible children
				selectItem(olditem, false, true);

				if ((selimage)&&(this->smooth_selection)&&(refresh)&&(oldx < this->x)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_MOVESEL_RIGHT,
											-(double)(getItemHMargin() * 2 + this->item_w),
											count - 1);
	            }

	        	// switch on new selection
				selectItem(item, true, refresh);
	        }
	        else {
	            // scrolled, switch focus needs recalculate children
	            selectItem(olditem, false, false);

	            if ((this->smooth_scrolling)&&(refresh)&&(oldx < this->x)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT,
											getItemHMargin() * 2 + this->item_w,
											count - 1);
	            }

	            if (refresh)
	                recalculateChildren();

	            selectItem(item, true, false, refresh);
	        }
        }
        else {
        	// we have to leave the selected menu item asis!!!
	        if (this->px + this->h_items + count - 1 >= ((cols<=children.size())?cols:children.size())) {
	            return false;
	        }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

	        // recalculate scroll position
            this->px++;

            // refresh is required
            enableRefresh();

            if (refresh) {
                recalculateChildren();
                this->refresh();
            }
        }

        // set the sliders
        setSliders();

        return true;
    }
    else {
        // menu with fixed selection
        if (cols > 1) {
            // in test mode we can say that we can scroll
            if (test) {
                return true;
            }

            // callback
            this->onBeforeScroll->emit(this);

            if ((this->smooth_scrolling)&&(refresh)) {
				// do the animation
				// input: animation mode, animation offset, number of menu items to jump over
            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_RIGHT, 0, count - 1);
            }

			// correct menu with more than one column
            count%=cols;

            // save old and set new x selection
            oldx = this->x;
            this->x+=count;

            if (this->x >= (int)cols) {
                // go back to begin of the first row (round robin)
                this->x = this->x - cols;
            }

            if (this->x >= (int)children.size()) {
                // go back to begin of the list (round robin)
                this->x = this->x - children.size();
            }

            // recalculate scroll position
            this->px = this->x;

	        // get access to widgets
	        MMSWidget *olditem = (oldx    < children.size()) ? children.at(oldx)    : NULL;
	        MMSWidget *item    = (this->x < children.size()) ? children.at(this->x) : NULL;

            if ((smooth_scrolling)&&(refresh)) {
            	// reset the blend value
            	olditem->setBlend(0, false);
            	item->setBlend(0, false);
            }

            // switch focus and recalculate children
            selectItem(olditem, false, false);

            if (refresh)
                recalculateChildren();

            selectItem(item, true, false, refresh);

            // set the sliders
            setSliders();

            return true;
        }
        else {
            // menu with only one column cannot be scrolled right in fixed selection mode
            return false;
        }
    }
}

bool MMSMenuWidget::scrollLeftEx(unsigned int count, bool refresh, bool test, bool leave_selection) {
    bool pxChanged = false;
    int oldx;
    unsigned int cols;
    int fixedpos;

    // check something
    if (count==0 || children.empty()) {
        return false;
    }

    // get settings
    cols = getCols();
    fixedpos = getFixedPos();

    // normal menu or fixed selection?
    if (fixedpos < 0) {
        // normal menu
        if (!leave_selection) {
	        if (this->x < (int)count) {
	            // really nothing to scroll?
	            if (getHLoop()) {
	                // I should not give up the focus
	                unsigned int columns;
	                if (cols < this->children.size())
	                    columns = cols;
	                else
	                    columns = this->children.size();
	                if ((int)columns - (int)this->x > 1) {
						if (children.size() <= cols) {
							return scrollRightEx(columns - this->x - 1, refresh, test, leave_selection);
						}
						else {
							if (!scrollRightEx(columns - this->x - 1, false, test, leave_selection)) {
							    return false;
							}
							if (this->y > 0) {
								return scrollUpEx(1, refresh, test, leave_selection);
							}
							else {
								return scrollDownEx((children.size() - cols) / cols, refresh, test, leave_selection);
							}
						}
	                }
	                else {
	                    return true;
	                }
	            }

	            return false;
	        }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

            // callback
            this->onBeforeScroll->emit(this);

            // save old and set new x selection
	        oldx = this->x;
	        this->x-=count;

	        // recalculate scroll position
	        int xpx = this->x - this->px;
	        if (xpx < 0) {
	            this->px = this->x;
	            pxChanged = true;
	        }
	        else
	        if (xpx >= this->h_items) {
	            this->px = this->x - this->h_items + 1;
	            pxChanged = true;
	        }

	        // get access to widgets
			unsigned int olditem_index = oldx + this->y * cols;
			unsigned int item_index = this->x + this->y * cols;
	        MMSWidget *olditem = (olditem_index < children.size()) ? children.at(olditem_index) : NULL;
	        MMSWidget *item    = (item_index    < children.size()) ? children.at(item_index)    : NULL;

	        if (!pxChanged) {
	            // not scrolled, switch focus between visible children
				selectItem(olditem, false, true);

	            if ((selimage)&&(this->smooth_selection)&&(refresh)&&(oldx > this->x)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_MOVESEL_LEFT,
											getItemHMargin() * 2 + this->item_w,
											count - 1);
	            }

	        	// switch on new selection
				selectItem(item, true, refresh);
			}
	        else {
	            // scrolled, switch focus needs recalculate children
	            selectItem(olditem, false, false);

	            if ((this->smooth_scrolling)&&(refresh)&&(oldx > this->x)) {
					// do the animation
					// input: animation mode, animation offset, number of menu items to jump over
	            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT,
											-(double)(getItemHMargin() * 2 + this->item_w),
											count - 1);
	            }

	            if (refresh)
	                recalculateChildren();

	            selectItem(item, true, false, refresh);
	        }
        }
        else {
        	// we have to leave the selected menu item asis!!!
            if (this->px < (int)count) {
                return false;
            }

	        // in test mode we can say that we can scroll
	        if (test) {
	            return true;
	        }

	        // recalculate scroll position
            this->px--;

            // refresh is required
            enableRefresh();

            if (refresh) {
                recalculateChildren();
                this->refresh();
            }
        }

        // set the sliders
        setSliders();

        return true;
    }
    else {
        // menu with fixed selection
        if (cols > 1) {
            // in test mode we can say that we can scroll
            if (test) {
                return true;
            }

            // callback
            this->onBeforeScroll->emit(this);

            if ((this->smooth_scrolling)&&(refresh)) {
				// do the animation
				// input: animation mode, animation offset, number of menu items to jump over
            	startAnimation(MMSMENUWIDGET_PULSER_MODE_SCROLL_LEFT, 0, count - 1);
            }


			// correct menu with more than one column
            count%=cols;

            // save old and set new x selection
            oldx = this->x;
            this->x-=count;

            if ((int)this->x < 0) {
                // go back to end of the first row (round robin)
                this->x = (int)cols + (int)this->x;

                if (this->x >= (int)children.size()) {
                    // go back to begin of the list (round robin)
                    this->x = children.size() - (cols - this->x);
                }
            }

            // recalculate scroll position
            this->px = this->x;

	        // get access to widgets
	        MMSWidget *olditem = (oldx    < children.size()) ? children.at(oldx)    : NULL;
	        MMSWidget *item    = (this->x < children.size()) ? children.at(this->x) : NULL;

            if ((smooth_scrolling)&&(refresh)) {
            	/* reset the blend value */
            	olditem->setBlend(0, false);
            	item->setBlend(0, false);
            }

            // switch focus and recalculate children
            selectItem(olditem, false, false);

            if (refresh)
                recalculateChildren();

            selectItem(item, true, false, refresh);

            // set the sliders
            setSliders();

            return true;
        }
        else {
            // menu with only one column cannot be scrolled right in fixed selection mode
            return false;
        }
    }
}

bool MMSMenuWidget::scrollDown(unsigned int count, bool refresh, bool test, bool leave_selection) {

	if (this->children.size()==0)
		return false;

	if ((!test)&&(smooth_scrolling)&&(refresh)) {
	    int fixedpos = getFixedPos();
	    if (fixedpos >= 0) {
			if (getCols() == 1) {
		    	/* this is a vertical menu and we have to check which is the shortest way - up or down scrolling */
		    	/* it's important to know because of smooth scrolling */
		    	count = count % this->children.size();
	            if (fixedpos >= this->v_items)
	                fixedpos = (this->v_items - 1) / 2;

		    	if (count > (this->children.size() / 2) - (fixedpos - ((this->v_items - 1) / 2))) {
		    		count = this->v_items-count;
		    		while (count--)
		    			scrollUpEx(1, refresh, test, leave_selection);
		    		return true;
		    	}

	    		while (count--)
	    			scrollDownEx(1, refresh, test, leave_selection);
	    		return true;
			}
	    }
	}

////
/*	if (!test) {
	static int iii=0;
	bool jjj=false;
	if (iii<1) {
	printf("start:%d\n", time(NULL));
	iii++;
	jjj=true;
	}
	if (this->name=="loc_menu") {
		int gggg=0;
		while (gggg++<20) {
			int ggggg = 0;
			while (ggggg++<15)
				scrollDownEx(1, true, false, false);
			while (ggggg-->0)
				scrollUpEx(1, true, false, false);
		}
	}
	if (jjj) {
	printf("end:%d\n", time(NULL));
	}
	}*/
////


	bool ret = scrollDownEx(count, refresh, test, leave_selection);

	if ((!ret)&&(!test))
		// nothing to scroll
		if (this->parent_menu) {
			// if we have a parent menu, we do only leave this menu if we switch back to the parent
			switchBackToParentMenu(MMSDIRECTION_DOWN);
			return true;
		}

	return ret;
}

bool MMSMenuWidget::scrollUp(unsigned int count, bool refresh, bool test, bool leave_selection) {

	if (this->children.size()==0)
		return false;

	if ((!test)&&(smooth_scrolling)&&(refresh)) {
	    int fixedpos = getFixedPos();
	    if (fixedpos >= 0) {
			if (getCols() == 1) {
		    	/* this is a vertical menu and we have to check which is the shortest way - up or down scrolling */
		    	/* it's important to know because of smooth scrolling */
		    	count = count % this->children.size();
	            if (fixedpos >= this->v_items)
	                fixedpos = (this->v_items - 1) / 2;

		    	if (count > (this->children.size() / 2) - (fixedpos - ((this->v_items - 1) / 2))) {
		    		count = this->v_items-count;
		    		while (count--)
		    			scrollDownEx(1, refresh, test, leave_selection);
		    		return true;
		    	}

	    		while (count--)
	    			scrollUpEx(1, refresh, test, leave_selection);
	    		return true;
			}
	    }
	}

	bool ret = scrollUpEx(count, refresh, test, leave_selection);

	if ((!ret)&&(!test))
		// nothing to scroll
		if (this->parent_menu) {
			// if we have a parent menu, we do only leave this menu if we switch back to the parent
			switchBackToParentMenu(MMSDIRECTION_UP);
			return true;
		}

	return ret;
}

bool MMSMenuWidget::scrollRight(unsigned int count, bool refresh, bool test, bool leave_selection) {

	if (this->children.size()==0)
		return false;

	////
/*		if (!test) {
		static int iiii=0;
		bool jjj=false;
		if (iiii<1) {
		printf("startX:%d\n", time(NULL));
		iiii++;
		jjj=true;
		}
		if (this->name=="switcher_menu") {
			int gggg=0;
			while (gggg++<1000) {
				scrollRightEx(1, true, false, false);
			}
		}
		if (jjj) {
		printf("endX:%d\n", time(NULL));
		}
		}*/
	////


	if ((!test)&&(smooth_scrolling)&&(refresh)) {
	    int fixedpos = getFixedPos();
	    if (fixedpos >= 0) {
			if (getCols() != 1) {
		    	/* this is a horizontal menu and we have to check which is the shortest way - left or right scrolling */
		    	/* it's important to know because of smooth scrolling */
		    	count = count % this->children.size();
	            if (fixedpos >= this->h_items)
	                fixedpos = (this->h_items - 1) / 2;

		    	if (count > (this->children.size() / 2) - (fixedpos - ((this->h_items - 1) / 2))) {
		    		count = this->children.size()-count;
		    		while (count--)
		    			scrollLeftEx(1, refresh, test, leave_selection);
		    		return true;
		    	}

	    		while (count--)
	    			scrollRightEx(1, refresh, test, leave_selection);
	    		return true;
			}
	    }
	}

	bool ret = scrollRightEx(count, refresh, test, leave_selection);

	if ((!ret)&&(!test))
		// nothing to scroll
		if (this->parent_menu) {
			// if we have a parent menu, we do only leave this menu if we switch back to the parent
			switchBackToParentMenu(MMSDIRECTION_RIGHT);
			return true;
		}

	return ret;
}

bool MMSMenuWidget::scrollLeft(unsigned int count, bool refresh, bool test, bool leave_selection) {

	if (this->children.size()==0)
		return false;

	if ((!test)&&(smooth_scrolling)&&(refresh)) {
	    int fixedpos = getFixedPos();
	    if (fixedpos >= 0) {
			if (getCols() != 1) {
		    	/* this is a horizontal menu and we have to check which is the shortest way - left or right scrolling */
		    	/* it's important to know because of smooth scrolling */
		    	count = count % this->children.size();
	            if (fixedpos >= this->h_items)
	                fixedpos = (this->h_items - 1) / 2;

		    	if (count > (this->children.size() / 2) - (fixedpos - ((this->h_items - 1) / 2))) {
		    		count = this->children.size()-count;
		    		while (count--)
		    			scrollRightEx(1, refresh, test, leave_selection);
		    		return true;
				}

	    		while (count--)
	    			scrollLeftEx(1, refresh, test, leave_selection);
	    		return true;
			}
	    }
	}

	bool ret = scrollLeftEx(count, refresh, test, leave_selection);

	if ((!ret)&&(!test))
		// nothing to scroll
		if (this->parent_menu) {
			// if we have a parent menu, we do only leave this menu if we switch back to the parent
			switchBackToParentMenu(MMSDIRECTION_LEFT);
			return true;
		}

	return ret;
}

bool MMSMenuWidget::scrollTo(int posx, int posy, bool refresh, bool *changed, MMSWIDGET_SCROLL_MODE mode, MMSFBRectangle *inputrect) {

	// searching the affected menu item
	for (unsigned int i = 0; i < this->children.size(); i++) {
		if (!this->children.at(i)->isVisible())
			continue;
		MMSFBRectangle mygeom = this->children.at(i)->getGeometry();
		if   ((posx >= mygeom.x)&&(posy >= mygeom.y)
			&&(posx < mygeom.x + mygeom.w)&&(posy < mygeom.y + mygeom.h)) {
			switch (mode) {
			case MMSWIDGET_SCROLL_MODE_SETSELECTED:
				// that's the right menu item, scroll smooth to the position
				setSelected(i, refresh, changed, false);
				break;
			case MMSWIDGET_SCROLL_MODE_SETSELECTED | MMSWIDGET_SCROLL_MODE_RMPRESSED:
				// that's the right menu item, scroll smooth to the position
				this->children.at(i)->setPressed(false, false);
				setSelected(i, refresh, changed, false);
				break;
			case MMSWIDGET_SCROLL_MODE_SETPRESSED:
				// that's the right menu item, set pressed status
				if (changed) *changed = true;
				if (inputrect) *inputrect = mygeom;
				this->children.at(i)->setPressed(true, refresh);
				break;
			case MMSWIDGET_SCROLL_MODE_RMPRESSED:
				// that's the right menu item, remove pressed status
				if (changed) *changed = true;
				this->children.at(i)->setPressed(false, refresh);
				break;
			}
			return true;
		}
	}

	return false;
}

void MMSMenuWidget::setItemTemplate(MMSWidget *itemTemplate) {
	bool b;

	if (!itemTemplate)
        throw MMSWidgetError(0, "item template not set");

    /* we need menu items which can be selected */
    if (!itemTemplate->getSelectable(b))
        throw MMSWidgetError(0, "widget cannot be selected");
    if (!b)
    	throw MMSWidgetError(0, "widget cannot be selected");

    /* we need menu items which can be selected and we must switch focusable off */
    if (itemTemplate->getFocusable(b))
    	if (b)
    		itemTemplate->setFocusable(false, false);
    itemTemplate->unsetFocusableForAllChildren(false);
    itemTemplate->setVisible(false, false);

    /* item template can be set once only */
    if (this->itemTemplate)
        throw MMSWidgetError(0, "item template can be set once only");

    this->itemTemplate = itemTemplate;
}

MMSWidget *MMSMenuWidget::getItemTemplate() {
    return this->itemTemplate;
}

MMSWidget *MMSMenuWidget::newItem(int item, MMSWidget *widget) {
    MMSMENUITEMINFOS	iteminfo;

    if (!widget) {
    	// no widget given, create widget from template
		if (!this->itemTemplate)
			throw MMSWidgetError(0, "item template not set");

		widget = itemTemplate->copyWidget();
    }

    // lock me
    lock();

    widget->setParent(this);
    widget->setRootWindow(this->rootwindow);
	iteminfo.name = "";
	iteminfo.window = NULL;
	iteminfo.menu = NULL;
    if (item > 0) {
    	if (item > (int)this->children.size())
    		item = -1;
    }
    if (item < 0) {
    	// push new item at the end of the list
    	this->children.push_back(widget);
		this->iteminfos.push_back(iteminfo);
    }
    else {
    	// get currently selected icon
    	unsigned int sitem = getSelected();

    	// insert at position item
    	this->children.insert(this->children.begin() + item, widget);
		this->iteminfos.insert(this->iteminfos.begin() + item, iteminfo);

	    if (item <= (int)sitem) {
	    	// item before the selected item inserted, so have to change the selection
			setSelected(sitem + 1, false);
	    }
    }

    recalculateChildren();

    if (widget->isVisible()) {
        // refresh is required
    	enableRefresh();

        this->refresh();
    }

    // unlock me
    unlock();

    return widget;
}


void MMSMenuWidget::deleteItem(unsigned int item) {

    // lock me
    lock();

    // check size
	if (item >= this->children.size()) {
	    // unlock me
	    unlock();

		return;
	}

	// get currently selected icon
	unsigned int sitem = getSelected();

	// delete item
    delete this->children.at(item);
    this->children.erase(this->children.begin()+item);
    this->iteminfos.erase(this->iteminfos.begin()+item);

    // recalc and refresh
    recalculateChildren();

    if (item < sitem) {
    	// item before the selected item was deleted, so have to change the selection
		setSelected(sitem - 1, false);
    }
    else
    if (item == sitem) {
    	// selected item was deleted, so we have to select the item at this position
    	if (sitem < this->children.size())
    		setSelected(sitem, false);
    	else
		if (sitem > 0)
    		setSelected(sitem - 1, false);
    }

    // refresh is required
    enableRefresh();

    this->refresh();

    // unlock me
    unlock();
}


void MMSMenuWidget::clear() {
    // lock me
    lock();

    for(int i = (int)this->children.size() - 1; i >= 0 ; i--) {
        delete this->children.at(i);
        this->children.erase(this->children.end()-1);
        this->iteminfos.erase(this->iteminfos.end()-1);
    }

    this->x = 0;
    this->y = 0;
    this->px = 0;
    this->py = 0;
    this->firstFocus = false;
    this->firstSelection = false;

    recalculateChildren();

    // refresh is required
    enableRefresh();

    this->refresh();

    // unlock me
    unlock();
}

void MMSMenuWidget::setFocus(bool set, bool refresh, MMSInputEvent *inputevent) {
	/* switch the brightness of the menu items */
    if (set) {
        /* get the focus -> dim up and/or decrease transparency */
        if ((!MMSWidget::isFocused())&&(this->firstFocus)) {
            for (unsigned int i = 0; i < children.size(); i++) {
                children.at(i)->setBrightness(children.at(i)->getBrightness() + getDimItems(), false);
                children.at(i)->setOpacity(children.at(i)->getOpacity() + getTransItems(), false);
            }
        }
    }
    else {
        /* loose the focus -> dim down and/or increase transparency */
        if ((MMSWidget::isFocused())||(!this->firstFocus)) {
            for (unsigned int i = 0; i < children.size(); i++) {
                children.at(i)->setBrightness(children.at(i)->getBrightness() - getDimItems(), false);
                children.at(i)->setOpacity(children.at(i)->getOpacity() - getTransItems(), false);
            }
        }
    }
    this->firstFocus = true;

    /* set the focus */
    if (!this->firstSelection) {
        if (!children.empty()) {
            MMSWidget::setFocus(set, false, inputevent);
			string inputmode = "";
			getInputModeEx(inputmode);
			if (strToUpr(inputmode) == "CLICK")
				selectItem(children.at(0), false, refresh);
			else
				selectItem(children.at(0), true, refresh);
        }
        else
            MMSWidget::setFocus(set, refresh, inputevent);
        this->firstSelection = true;
    }
    else {
        MMSWidget::setFocus(set, refresh, inputevent);
		string inputmode = "";
		getInputModeEx(inputmode);
		if (strToUpr(inputmode) == "CLICK")
			selectItem(getSelectedItem(), false, refresh);
    }
}

bool MMSMenuWidget::setSelected(unsigned int item, bool refresh, bool *changed, bool joined) {
	bool c = false;
	if (changed)
		*changed = c;

    if (!getConfig())
        return false;

    if (item >= children.size())
        return false;

    if (!this->firstSelection) {
        if (item == 0)
            if (!children.empty())
                selectItem(children.at(0), refresh);
        this->firstSelection = true;
    }

    unsigned int cols = getCols();
    unsigned int mx = item % cols;
    unsigned int my = item / cols;

    // scroll left-down
    if (((int)mx < this->x)&&((int)my > this->y)) {
        if (scrollLeft(this->x - mx, false))
       		scrollDown(my - this->y, refresh);
    	c = true;
    }
    else
    // scroll right-down
    if (((int)mx > this->x)&&((int)my > this->y)) {
        if (scrollRight(mx - this->x, false))
      		scrollDown(my - this->y, refresh);
    	c = true;
    }
    else
    // scroll left-up
    if (((int)mx < this->x)&&((int)my < this->y)) {
        if (scrollUp(this->y - my, false))
       		scrollLeft(this->x - mx, refresh);
    	c = true;
    }
    else
    // scroll right-up
    if (((int)mx > this->x)&&((int)my < this->y)) {
        if (scrollUp(this->y - my, false))
       		scrollRight(mx - this->x, refresh);
    	c = true;
    }
    else
    // scroll down
    if ((int)my > this->y) {
   		scrollDown(my - this->y, refresh);
    	c = true;
    }
    else
    // scroll up
    if ((int)my < this->y) {
   		scrollUp(this->y - my, refresh);
    	c = true;
    }
    else
    // scroll left
    if ((int)mx < this->x) {
   		scrollLeft(this->x - mx, refresh);
    	c = true;
    }
    else
    // scroll right
    if ((int)mx > this->x) {
   		scrollRight(mx - this->x, refresh);
    	c = true;
    }

    if (!c) {
    	// preventive set selected true to reset it
    	MMSWidget *item = getSelectedItem();
    	if (item)
    		item->setSelected(true);
    }

	if (changed)
		*changed = c;

    return true;
}

bool MMSMenuWidget::setSelected(unsigned int item, bool refresh) {
	return setSelected(item, refresh, NULL, false);
}

unsigned int MMSMenuWidget::getSelected() {
    return (this->x + this->y * getCols());
}

MMSWidget *MMSMenuWidget::getItem(unsigned int item) {
    if (item < this->children.size())
        return this->children.at(item);
    return NULL;
}

MMSWidget *MMSMenuWidget::getSelectedItem() {
    return getItem(getSelected());
}

unsigned int MMSMenuWidget::getSize() {
    return this->children.size();
}

unsigned int MMSMenuWidget::getVItems() {
    return this->v_items;
}

unsigned int MMSMenuWidget::getHItems() {
    return this->h_items;
}



bool MMSMenuWidget::setSubMenuName(unsigned int item, const char *name) {
	if (item >= this->iteminfos.size()) return false;
	iteminfos.at(item).name = name;
	iteminfos.at(item).window = NULL;
	return true;
}

bool MMSMenuWidget::setSubMenuName(unsigned int item, string &name) {
	return setSubMenuName(item, name.c_str());
}

bool MMSMenuWidget::setBackItem(unsigned int item) {
/*	if ((item!=-1)&&(item >= this->children.size())) return false;   -- item is unsigned (mattmax) */
	if (item >= this->children.size()) return false;
	this->back_item = item;
	return true;
}


/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETMENU(x) \
    if (this->myMenuWidgetClass.is##x()) return myMenuWidgetClass.get##x(); \
    else if ((menuWidgetClass)&&(menuWidgetClass->is##x())) return menuWidgetClass->get##x(); \
    else return this->da->theme->menuWidgetClass.get##x();

#define GETMENU_X(x,y) \
    if (this->myMenuWidgetClass.is##x()) { y = myMenuWidgetClass.get##x(); return true; } \
    else if ((menuWidgetClass)&&(menuWidgetClass->is##x())) { y = menuWidgetClass->get##x(); return true; } \
    else { y = this->da->theme->menuWidgetClass.get##x(); return true; }

MMSTaffFile *MMSMenuWidget::getTAFF() {
    MMSTaffFile *node;
    if ((node=myMenuWidgetClass.getTAFF()))
        return node;
    if ((menuWidgetClass)&&((node=menuWidgetClass->getTAFF())))
        return node;
    return this->da->theme->menuWidgetClass.getTAFF();
}

string MMSMenuWidget::getItemWidth() {
    GETMENU(ItemWidth);
}

string MMSMenuWidget::getItemHeight() {
    GETMENU(ItemHeight);
}

unsigned int MMSMenuWidget::getItemHMargin() {
    GETMENU(ItemHMargin);
}

unsigned int MMSMenuWidget::getItemVMargin() {
    GETMENU(ItemVMargin);
}

unsigned int MMSMenuWidget::getCols() {
    if (this->myMenuWidgetClass.isCols())
    	return myMenuWidgetClass.getCols();
    else if ((menuWidgetClass)&&(menuWidgetClass->isCols()))
    	return menuWidgetClass->getCols();
    else
    	return this->da->theme->menuWidgetClass.getCols();


    //GETMENU(Cols);
}

unsigned int MMSMenuWidget::getDimItems() {
    GETMENU(DimItems);
}

int MMSMenuWidget::getFixedPos() {
    GETMENU(FixedPos);
}

bool MMSMenuWidget::getHLoop() {
    GETMENU(HLoop);
}

bool MMSMenuWidget::getVLoop() {
    GETMENU(VLoop);
}

unsigned int MMSMenuWidget::getTransItems() {
    GETMENU(TransItems);
}

unsigned int MMSMenuWidget::getDimTop() {
    GETMENU(DimTop);
}

unsigned int MMSMenuWidget::getDimBottom() {
    GETMENU(DimBottom);
}

unsigned int MMSMenuWidget::getDimLeft() {
    GETMENU(DimLeft);
}

unsigned int MMSMenuWidget::getDimRight() {
    GETMENU(DimRight);
}

unsigned int MMSMenuWidget::getTransTop() {
    GETMENU(TransTop);
}

unsigned int MMSMenuWidget::getTransBottom() {
    GETMENU(TransBottom);
}

unsigned int MMSMenuWidget::getTransLeft() {
    GETMENU(TransLeft);
}

unsigned int MMSMenuWidget::getTransRight() {
    GETMENU(TransRight);
}

string MMSMenuWidget::getZoomSelWidth() {
    GETMENU(ZoomSelWidth);
}

string MMSMenuWidget::getZoomSelHeight() {
    GETMENU(ZoomSelHeight);
}

string MMSMenuWidget::getZoomSelShiftX() {
    GETMENU(ZoomSelShiftX);
}

string MMSMenuWidget::getZoomSelShiftY() {
    GETMENU(ZoomSelShiftY);
}

MMSSEQUENCEMODE MMSMenuWidget::getSmoothScrolling() {
    GETMENU(SmoothScrolling);
}

string MMSMenuWidget::getParentWindow() {
    GETMENU(ParentWindow);
}

bool MMSMenuWidget::getSelImagePath(string &selimagepath) {
    GETMENU_X(SelImagePath, selimagepath);
}

bool MMSMenuWidget::getSelImageName(string &selimagename) {
    GETMENU_X(SelImageName, selimagename);
}

MMSSEQUENCEMODE MMSMenuWidget::getSmoothSelection() {
    GETMENU(SmoothSelection);
}

unsigned int MMSMenuWidget::getSmoothDelay() {
    GETMENU(SmoothDelay);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSMenuWidget::setItemWidth(string itemwidth, bool refresh) {
    myMenuWidgetClass.setItemWidth(itemwidth);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setItemHeight(string itemheight, bool refresh) {
    myMenuWidgetClass.setItemHeight(itemheight);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setItemHMargin(unsigned int itemhmargin, bool refresh) {
    myMenuWidgetClass.setItemHMargin(itemhmargin);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setItemVMargin(unsigned int itemvmargin, bool refresh) {
    myMenuWidgetClass.setItemVMargin(itemvmargin);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setCols(unsigned int cols, bool refresh) {
    myMenuWidgetClass.setCols(cols);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setDimItems(unsigned int dimitems, bool refresh) {
    myMenuWidgetClass.setDimItems(dimitems);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setFixedPos(int fixedpos, bool refresh) {
    myMenuWidgetClass.setFixedPos(fixedpos);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setHLoop(bool hloop) {
    myMenuWidgetClass.setHLoop(hloop);
}

void MMSMenuWidget::setVLoop(bool vloop) {
    myMenuWidgetClass.setVLoop(vloop);
}

void MMSMenuWidget::setTransItems(unsigned int transitems, bool refresh) {
    myMenuWidgetClass.setTransItems(transitems);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setDimTop(unsigned int dimtop, bool refresh) {
    myMenuWidgetClass.setDimTop(dimtop);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setDimBottom(unsigned int dimbottom, bool refresh) {
    myMenuWidgetClass.setDimBottom(dimbottom);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setDimLeft(unsigned int dimleft, bool refresh) {
    myMenuWidgetClass.setDimLeft(dimleft);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setDimRight(unsigned int dimright, bool refresh) {
    myMenuWidgetClass.setDimRight(dimright);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setTransTop(unsigned int transtop, bool refresh) {
    myMenuWidgetClass.setTransTop(transtop);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setTransBottom(unsigned int transbottom, bool refresh) {
    myMenuWidgetClass.setTransBottom(transbottom);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setTransLeft(unsigned int transleft, bool refresh) {
    myMenuWidgetClass.setTransLeft(transleft);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setTransRight(unsigned int transright, bool refresh) {
    myMenuWidgetClass.setTransRight(transright);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setZoomSelWidth(string zoomselwidth, bool refresh) {
    myMenuWidgetClass.setZoomSelWidth(zoomselwidth);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setZoomSelHeight(string zoomselheight, bool refresh) {
    myMenuWidgetClass.setZoomSelHeight(zoomselheight);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setZoomSelShiftX(string zoomselshiftx, bool refresh) {
    myMenuWidgetClass.setZoomSelShiftX(zoomselshiftx);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setZoomSelShiftY(string zoomselshifty, bool refresh) {
    myMenuWidgetClass.setZoomSelShiftY(zoomselshifty);

    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setSmoothScrolling(MMSSEQUENCEMODE seq_mode) {
    myMenuWidgetClass.setSmoothScrolling(seq_mode);
    this->smooth_scrolling = seq_mode;
}

void MMSMenuWidget::setParentWindow(string parentwindow) {
    myMenuWidgetClass.setParentWindow(parentwindow);
    initParentWindow();
}

void MMSMenuWidget::setSelImagePath(string selimagepath, bool load, bool refresh) {
    myMenuWidgetClass.setSelImagePath(selimagepath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage);
            string path, name;
            if (!getSelImagePath(path)) path = "";
            if (!getSelImageName(name)) name = "";
            this->selimage = this->rootwindow->im->getImage(path, name);
        }


    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setSelImageName(string selimagename, bool load, bool refresh) {
    myMenuWidgetClass.setSelImageName(selimagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage);
            string path, name;
            if (!getSelImagePath(path)) path = "";
            if (!getSelImageName(name)) name = "";
            this->selimage = this->rootwindow->im->getImage(path, name);
        }


    // refresh is required
    enableRefresh();

    if (refresh)
        this->refresh();
}

void MMSMenuWidget::setSmoothSelection(MMSSEQUENCEMODE seq_mode) {
    myMenuWidgetClass.setSmoothSelection(seq_mode);
    this->smooth_selection = seq_mode;
}

void MMSMenuWidget::setSmoothDelay(unsigned int smoothdelay) {
    myMenuWidgetClass.setSmoothDelay(smoothdelay);
    this->frame_delay = 0;
    this->frame_delay_set = false;
}


void MMSMenuWidget::updateFromThemeClass(MMSMenuWidgetClass *themeClass) {
	if (themeClass->isItemWidth())
        setItemWidth(themeClass->getItemWidth());
   if (themeClass->isItemHeight())
        setItemHeight(themeClass->getItemHeight());
   if (themeClass->isItemHMargin())
        setItemHMargin(themeClass->getItemHMargin());
   if (themeClass->isItemVMargin())
        setItemVMargin(themeClass->getItemVMargin());
   if (themeClass->isCols())
        setCols(themeClass->getCols());
   if (themeClass->isDimItems())
        setDimItems(themeClass->getDimItems());
   if (themeClass->isFixedPos())
        setFixedPos(themeClass->getFixedPos());
   if (themeClass->isHLoop())
        setHLoop(themeClass->getHLoop());
   if (themeClass->isVLoop())
        setVLoop(themeClass->getVLoop());
   if (themeClass->isTransItems())
        setTransItems(themeClass->getTransItems());
   if (themeClass->isDimTop())
        setDimTop(themeClass->getDimTop());
   if (themeClass->isDimBottom())
        setDimBottom(themeClass->getDimBottom());
   if (themeClass->isDimLeft())
        setDimLeft(themeClass->getDimLeft());
   if (themeClass->isDimRight())
        setDimRight(themeClass->getDimRight());
   if (themeClass->isTransTop())
        setTransTop(themeClass->getTransTop());
   if (themeClass->isTransBottom())
        setTransBottom(themeClass->getTransBottom());
   if (themeClass->isTransLeft())
        setTransLeft(themeClass->getTransLeft());
   if (themeClass->isTransRight())
        setTransRight(themeClass->getTransRight());
   if (themeClass->isZoomSelWidth())
        setZoomSelWidth(themeClass->getZoomSelWidth());
   if (themeClass->isZoomSelHeight())
        setZoomSelHeight(themeClass->getZoomSelHeight());
   if (themeClass->isZoomSelShiftX())
        setZoomSelShiftX(themeClass->getZoomSelShiftX());
   if (themeClass->isZoomSelShiftY())
        setZoomSelShiftY(themeClass->getZoomSelShiftY());
   if (themeClass->isSmoothScrolling())
        setSmoothScrolling(themeClass->getSmoothScrolling());
   if (themeClass->isParentWindow())
        setParentWindow(themeClass->getParentWindow());
   if (themeClass->isSelImagePath())
       setSelImagePath(themeClass->getSelImagePath());
   if (themeClass->isSelImageName())
       setSelImageName(themeClass->getSelImageName());
   if (themeClass->isSmoothSelection())
        setSmoothSelection(themeClass->getSmoothSelection());
   if (themeClass->isSmoothDelay())
        setSmoothDelay(themeClass->getSmoothDelay());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/

