/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re. Original copyrights follow below.
 *
 */

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

#include <cstring>
#include "mmsgui/fb/mmsfbwindowmanager.h"
#include "mmsinfo/mmsinfo.h"
#include "mmsgui/fb/mmsfb.h"

/* initialize the mmsfbwindowmanager object */
MMSFBWindowManager *mmsfbwindowmanager = new MMSFBWindowManager();

#define INITCHECK  if(!this->layer){MMSFB_SetError(0,"not initialized");return false;}

MMSFBWindowManager::MMSFBWindowManager() {
    // init me
    this->layer = NULL;
    this->layer_surface = NULL;
    this->dst_surface = NULL;
    this->layer_pixelformat = MMSFB_PF_NONE;
    this->high_freq_surface = NULL;
    this->high_freq_saved_surface = NULL;
    this->high_freq_region.x1 = 0;
    this->high_freq_region.y1 = 0;
    this->high_freq_region.x2 = 0;
    this->high_freq_region.y2 = 0;
    this->high_freq_lastflip = 0;
    this->mmsfbwinmanthread = NULL;

    // init pointer values
    this->show_pointer = false;
    this->pointer_posx = -1;
    this->pointer_posy = -1;
    this->pointer_rect.x = 0;
    this->pointer_rect.y = 0;
    this->pointer_rect.w = 0;
    this->pointer_rect.h = 0;
    this->pointer_region.x1 = 0;
    this->pointer_region.y1 = 0;
    this->pointer_region.x2 = 0;
    this->pointer_region.y2 = 0;
    this->pointer_surface = NULL;
    this->pointer_opacity = 0;
    this->button_pressed = false;
    this->pointer_fadecnt = 0;
}

MMSFBWindowManager::~MMSFBWindowManager() {
    for (unsigned int i=0; i < this->windows.size(); i++) {
        delete this->windows.at(i).window;
    }
}

bool MMSFBWindowManager::init(MMSFBLayer *layer, bool show_pointer) {

    // check if already initialized
    if (this->layer) {
        MMSFB_SetError(0, "already initialized");
        return false;
    }

    // start my thread
    if (!this->mmsfbwinmanthread) {
	    this->mmsfbwinmanthread = new MMSFBWindowManagerThread(&this->high_freq_surface,
	                                                     	&this->high_freq_saved_surface,
	                                                     	&this->high_freq_lastflip,
	                                                     	&this->lock);
	    if (this->mmsfbwinmanthread)
	    	mmsfbwinmanthread->start();
    }

    // set values
    this->layer = layer;
    this->show_pointer = show_pointer;

    DEBUGMSG("MMSGUI", "MMSFBWindowManager: get layer surface");

    // get the surface of the layer
    if (!this->layer->getSurface(&this->layer_surface))
        return false;
    this->dst_surface = this->layer_surface;

    // get the pixelformat of the layer surface
    if (!this->layer_surface->getPixelFormat(&this->layer_pixelformat))
    	return false;

    // get the pixelformat, create a little temp surface
	this->pixelformat = MMSFB_PF_NONE;
	this->ogl_mode = false;
	MMSFBSurface *ts;
    if (this->layer->createSurface(&ts, 8, 1)) {
    	// okay, get the pixelformat from surface
    	ts->getPixelFormat(&this->pixelformat);
    	this->ogl_mode = (ts->allocated_by == MMSFBSurfaceAllocatedBy_ogl);
    	delete ts;
    }

    // use taff?
	this->usetaff = false;
    switch (this->pixelformat) {
    case MMSFB_PF_ARGB:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_ARGB;
    	break;
    case MMSFB_PF_AiRGB:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_AiRGB;
    	break;
    case MMSFB_PF_AYUV:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_AYUV;
    	break;
    case MMSFB_PF_ARGB4444:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_ARGB4444;
    	break;
    case MMSFB_PF_RGB16:
    	// if running in RGB16 mode, we use an ARGB surface for the mouse pointer
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_ARGB;
    	break;
    case MMSFB_PF_ABGR:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_ABGR;
    	break;
    default:
    	break;
    }

    return true;
}

bool MMSFBWindowManager::reset() {

    // check if initialized
    INITCHECK;

    // reset the high freq surface pointer
    this->high_freq_surface = NULL;
	this->high_freq_saved_surface = NULL;
	this->high_freq_lastflip = 0;

	return true;
}

bool MMSFBWindowManager::getLayer(MMSFBLayer **layer) {

    // check if initialized
    INITCHECK;

    // return the layer
    *layer = this->layer;

    return true;
}

void MMSFBWindowManager::lockWM() {
    // stop parallel processing
    lock.lock();
}

void MMSFBWindowManager::unlockWM() {
    // unlock
    lock.unlock();
}

bool MMSFBWindowManager::addWindow(MMSFBWindow *window) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

    /* search for duplicate items */
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
            lock.unlock();
            return false;
        }

    /* add window */
    AVAILABLE_WINDOWS awin;
    awin.window = window;
    awin.vrect.x = 0;
    awin.vrect.y = 0;
    awin.vrect.w = 0;
    awin.vrect.h = 0;
    this->windows.push_back(awin);

    /* unlock */
    lock.unlock();

    return true;
}

bool MMSFBWindowManager::removeWindow(MMSFBWindow *window) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

    /* search for item */
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {

            /* hide the window before removing */
            hideWindow(window);

            /* remove it from list */
            this->windows.erase(this->windows.begin()+i);

            /* unlock */
            lock.unlock();
            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::raiseToTop(MMSFBWindow *window, int zlevel) {

    // check if initialized
    INITCHECK;

    // stop parallel processing
    lock.lock();

    // get requested zorder index
    zlevel = this->vwins.size() - 1 - zlevel;
    if (zlevel < 0)
    	zlevel = 0;
    else
    if (zlevel >= (int)this->vwins.size())
    	zlevel = this->vwins.size() - 1;

    // search for item within visible list
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            // window found
            if ((int)i < zlevel) {
                // window is not at the top or the requested zlevel, raise it now
            	// if the requested zlevel is less than the current zlevel, nothing will be done
                VISIBLE_WINDOWS vw = this->vwins.at(i);
                this->vwins.erase(this->vwins.begin()+i);
                this->vwins.insert(this->vwins.begin()+zlevel, vw);

                // change the windows list
                if ((int)this->vwins.size() <= zlevel+1) {
                	// at the top
                    for (unsigned int i=0; i < this->windows.size(); i++)
                        if (this->windows.at(i).window == window) {
                            // window found
                            if (i < this->windows.size()-1) {
                            	// put at the top
                                AVAILABLE_WINDOWS aw = this->windows.at(i);
                                this->windows.erase(this->windows.begin()+i);
                                this->windows.push_back(aw);
                            }
                            break;
                        }
                }
                else {
                    for (unsigned int i=0; i < this->windows.size(); i++)
                        if (this->windows.at(i).window == window) {
                            // window found
                            if (i < this->windows.size()-1) {
                            	// put it behind the swin
            					MMSFBWindow *swin = this->vwins.at(zlevel + 1).window;
                                for (unsigned int j=0; j < this->windows.size(); j++)
                                    if (this->windows.at(j).window == swin) {
                                        // window found
                                        AVAILABLE_WINDOWS aw = this->windows.at(i);
                                        this->windows.insert(this->windows.begin()+j, aw);
                                        if (i>=j) i++;
                                        this->windows.erase(this->windows.begin()+i);
                                        break;
                                    }
                            }
                            break;
                        }
                }

                // draw the window
                flipSurface(vw.surface, NULL, true);
            }

            // unlock
            lock.unlock();
            return true;
        }


    // not found
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::lowerToBottom(MMSFBWindow *window) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

    /* search for item */
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
            /* window found */
            if (i > 0) {
                /* window is not at the bottom, lower it now */
                AVAILABLE_WINDOWS aw = this->windows.at(i);
                this->windows.erase(this->windows.begin()+i);
                this->windows.insert(this->windows.begin(), aw);

                /* search for item within visible list */
                for (unsigned int j=0; j < this->vwins.size(); j++)
                    if (this->vwins.at(j).window == window) {
                        /* window found */
                        if (j > 0) {
                            /* window is not at the bottom, lower it now */
                            VISIBLE_WINDOWS vw = this->vwins.at(j);
                            this->vwins.erase(this->vwins.begin()+j);
                            this->vwins.insert(this->vwins.begin(), vw);

                            /* draw the window */
                            flipSurface(vw.surface, NULL, true);
                        }
                    }
            }

            /* unlock */
            lock.unlock();
            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}


bool MMSFBWindowManager::loadWindowConfig(MMSFBWindow *window, VISIBLE_WINDOWS *vwin) {
    vwin->window = window;
    vwin->window->getSurface(&vwin->surface);
    MMSFBWindowConfig winconf;
    vwin->window->getConfiguration(&winconf);
    vwin->vrect.x = 0;
    vwin->vrect.y = 0;
    vwin->vrect.w = 0;
    vwin->vrect.h = 0;
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
            vwin->vrect = this->windows.at(i).vrect;
            break;
        }
	vwin->region.x1 = winconf.posx;
	vwin->region.y1 = winconf.posy;
	vwin->region.x2 = vwin->region.x1 + winconf.surface_config.w - 1;
	vwin->region.y2 = vwin->region.y1 + winconf.surface_config.h - 1;
    if ((vwin->vrect.w > 0)&&(vwin->vrect.h > 0)) {
    	// visible rectangle set, so have to adjust region
    	MMSFBRegion sr = vwin->region;
		sr.x1 += vwin->vrect.x;
		sr.y1 += vwin->vrect.y;
		sr.x2 = sr.x1 + vwin->vrect.w - 1;
		sr.y2 = sr.y1 + vwin->vrect.h - 1;
		if (sr.x1 < vwin->region.x1)
			sr.x1 = vwin->region.x1;
		if (sr.y1 < vwin->region.y1)
			sr.y1 = vwin->region.y1;
		if (sr.x2 > vwin->region.x2)
			sr.x2 = vwin->region.x2;
		if (sr.y2 > vwin->region.y2)
			sr.y2 = vwin->region.y2;
		if ((sr.x1 <= vwin->region.x2)&&(sr.y1 <= vwin->region.y2)&&(sr.x2 >= vwin->region.x1)&&(sr.y2 >= vwin->region.y1)) {
			// adjusted region okay, set it
			vwin->region = sr;
		}
		else {
			// window is outside it's visible region
			vwin->region.x1 = 0;
			vwin->region.y1 = 0;
			vwin->region.x2 = -1;
			vwin->region.y2 = -1;
		}
    }
    vwin->alphachannel = winconf.surface_config.surface_buffer->alphachannel;
    vwin->opacity = winconf.opacity;
    vwin->lastflip = 0;
    vwin->islayersurface = false;
    vwin->saved_surface = NULL;
    return true;
}

bool MMSFBWindowManager::showWindow(MMSFBWindow *window, bool locked, bool refresh) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    if (!locked)
        lock.lock();

    /* search for item */
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
            /* search for duplicate items */
            for (unsigned int j=0; j < this->vwins.size(); j++)
                if (this->vwins.at(j).window == window) {
                    /* the window is already visible */
                    if (!locked)
                        lock.unlock();
                    return false;
                }

            /* prepare new list item */
            VISIBLE_WINDOWS vwin;
            loadWindowConfig(window, &vwin);

            /* add window to visible list */
            bool inserted = false;
            if (i < this->windows.size()-1) {
                /* first searching for the right position within the window stack */
                for (unsigned int j=0; j < this->vwins.size() && !inserted; j++)
                    for (unsigned int k=0; k < this->windows.size() && !inserted; k++)
                        if (this->vwins.at(j).window == this->windows.at(k).window)
                            if (k > i) {
                                /* insert the window */
                                this->vwins.insert(this->vwins.begin()+j, vwin);
                                inserted = true;
                                break;
                            }
            }
            if (!inserted)
                /* insert at the end (this is the top) */
                this->vwins.push_back(vwin);

            /* draw the window */
            flipSurface(vwin.surface, NULL, true, refresh);

            /* unlock */
            if (!locked)
                lock.unlock();

            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::hideWindow(MMSFBWindow *window, bool locked, bool refresh) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    if (!locked)
        lock.lock();

    /* search for item */
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            /* redraw the window with no opacity because must redrawing other windows */
            this->vwins.at(i).opacity = 0;

            flipSurface(this->vwins.at(i).surface, NULL, true, refresh);

            if (this->high_freq_surface==this->vwins.at(i).surface) {
                /* i was the high_freq_surface */
                this->high_freq_surface = NULL;
                this->high_freq_saved_surface = NULL;
                this->high_freq_lastflip = 0;
            }

            /* remove it from list */
            this->vwins.erase(this->vwins.begin()+i);

            /* unlock */
            if (!locked)
                lock.unlock();

            return true;
        }

    /* not found */
    if (!locked)
        lock.unlock();
    return false;
}

bool MMSFBWindowManager::flipSurface(MMSFBSurface *surface, MMSFBRegion *region,
                                     bool locked, bool refresh) {
    VISIBLE_WINDOWS *vw = NULL;
    MMSFBRegion     ls_region;
    bool            high_freq = false;
    bool            cleared = false;
    bool			win_found = false;

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    if (!locked)
        lock.lock();

    if (this->ogl_mode) {
    	// running in OpenGL mode
    	// note: GLX can only flip the complete screen!!!
    	//       EGL too, but currently we run EGL with FRONTONLY, so we do not need a layer flip
#ifdef  __HAVE_GLX__
		surface = NULL;
		region = NULL;
#endif
    }

/*
if (region)
printf("#1winman: region = %d,%d,%d,%d\n", region->x1, region->y1, region->x2, region->y2);
else
printf("#1winman: region = NULL\n");
*/

    /* search for item */
    if (surface) {
        /* surface given */
        for (unsigned int i=0; i < this->vwins.size(); i++) {
			if (this->vwins.at(i).surface == surface) {
				// surface found
				vw = &(this->vwins.at(i));
				ls_region = vw->region;

				// calculate the affected region on the layer surface
				if (region != NULL) {
					// only a region
					if (region->x1 > 0) {
						ls_region.x2 = ls_region.x1 + region->x2;
						ls_region.x1 = ls_region.x1 + region->x1;
					}
					else
						ls_region.x2 = ls_region.x1 + region->x2;
					if (region->y1 > 0) {
						ls_region.y2 = ls_region.y1 + region->y2;
						ls_region.y1 = ls_region.y1 + region->y1;
					}
					else
						ls_region.y2 = ls_region.y1 + region->y2;

		            if ((vw->vrect.w > 0)&&(vw->vrect.h > 0)) {
		            	// visible rectangle set, check calculated ls_region
		            	ls_region.x1 -= vw->vrect.x;
		            	ls_region.y1 -= vw->vrect.y;
		            	ls_region.x2 -= vw->vrect.x;
		            	ls_region.y2 -= vw->vrect.y;

		            	if (ls_region.x1 < vw->region.x1)
		            		ls_region.x1 = vw->region.x1;
		            	if (ls_region.y1 < vw->region.y1)
		            		ls_region.y1 = vw->region.y1;
		            	if (ls_region.x2 > vw->region.x2)
		            		ls_region.x2 = vw->region.x2;
		            	if (ls_region.y2 > vw->region.y2)
		            		ls_region.y2 = vw->region.y2;

		            	if ((ls_region.x1 > ls_region.x2)||(ls_region.y1 > ls_region.y2)) {
		            		// wrong region
		            		vw = NULL;
		            		break;
		            	}
		            }
				}

				// check region
				if (ls_region.x1 < 0) {
					ls_region.x2+= ls_region.x1;
					ls_region.x1 = 0;
				}
				if (ls_region.y1 < 0) {
					ls_region.y2+= ls_region.y1;
					ls_region.y1 = 0;
				}
				int ls_w, ls_h;
				if (this->dst_surface->getSize(&ls_w, &ls_h)) {
					if (ls_region.x2 >= ls_w)
						ls_region.x2 = ls_w - 1;
					if (ls_region.y2 >= ls_h)
						ls_region.y2 = ls_h - 1;
				}

				break;
			}
		}

        if (!vw) {
            /* not found */
            if (!locked)
                lock.unlock();
            return false;
        }
    }
    else {
        // no surface given, have to redraw a layer region?
        if (region == NULL) {
            // no
/*            if (!locked)
                lock.unlock();
            return false;*/

        	if (!this->dst_surface->getSize(&ls_region.x2, &ls_region.y2)) {
				if (!locked)
					lock.unlock();
				return false;
        	}

        	ls_region.x1=0;
        	ls_region.y1=0;
        	ls_region.x2--;
        	ls_region.y2--;
        }
        else {
			// take this region
			ls_region = *region;
        }
    }

    if ((region == NULL)&&(vw)) {
        /* this is only for whole (window) surfaces with an high flip frequency */
        struct  timeval tv;
        /* get the flip time */
        gettimeofday(&tv, NULL);
        int newfliptime = (((int)tv.tv_sec)%1000000)*1000+((int)tv.tv_usec)/1000;
        int diff = newfliptime - vw->lastflip;

        if ((diff > 0)&&(diff < 50)) {
            /* more than 20 pictures per second comes from this surface */
            high_freq = true;
        }

        if (vw->saved_surface) {
            /* save the frames if window works direct on the layer surface */
            if (vw->lastflip % 1000 < 40) {
            }
        }

        vw->lastflip = newfliptime;
    }

    if (high_freq) {
        /* this surface has an high flip frequency */
        if (!this->high_freq_surface) {
            /* this->high_freq_surface is not set, set it now */
            this->high_freq_region = ls_region;
            this->high_freq_lastflip = vw->lastflip;
            this->high_freq_surface = vw->surface;
            this->high_freq_saved_surface = vw->saved_surface;
        }
        else
            /* update the high_freq_lastflip */
            this->high_freq_lastflip = vw->lastflip;
    }
    else {
        bool check = (this->high_freq_surface!=NULL);
        if ((check)&&(vw))
            check = (this->high_freq_surface!=vw->surface);
        if (check) {
            /* high_freq_surface is set and i am not this surface */
            /* check if i am within high_freq_region */
            if ((this->high_freq_region.x1 <= ls_region.x1)
              &&(this->high_freq_region.y1 <= ls_region.y1)
              &&(this->high_freq_region.x2 >= ls_region.x2)
              &&(this->high_freq_region.y2 >= ls_region.y2)) {
                /* yes, have to flip nothing */
                if (!locked)
                    lock.unlock();
                return true;
            }
        }
        else {
            if ((this->high_freq_surface)&&(vw))
                /* update the high_freq_lastflip */
                this->high_freq_lastflip = vw->lastflip;
        }
    }


    // set the region of the layer surface
    this->dst_surface->setClip(&ls_region);


    // check if i have to clear the background
    if (!vw)
        cleared = true;
    else
        cleared = (!((vw->alphachannel==false)&&(vw->opacity==255)));


//printf("#2winman: flip windows\n");

    // two loops for optimized DEPTH TEST
    // FIRST:  find lowest window which is to blit
    // SECOND: blit the window stack beginning from lowest window
	int lowest_win = 0;
	MMSFBRegion tmpreg = MMSFBRegion(0,0,0,0);
	for (int depth_test = 1; depth_test >= 0; depth_test--) {
		// searching for affected windows and draw parts of it (in the second loop)
		for (unsigned int i = lowest_win; i < this->vwins.size(); i++) {
			VISIBLE_WINDOWS *aw = &(this->vwins.at(i));
			MMSFBRegion myreg = aw->region;
			MMSFBRegion *myregion = &myreg;

			// if the window has no opacity then continue
			if (!aw->opacity)
				continue;

			// check if layer surface
			if (aw->islayersurface)
				if (!cleared)
					continue;

			if (!((myregion->x2 < ls_region.x1)||(myregion->y2 < ls_region.y1)
				||(myregion->x1 > ls_region.x2)||(myregion->y1 > ls_region.y2))) {
				// the window is affected
				if (depth_test) {
					// FIRST loop: DEPTH TEST
					if (myregion->x1 <= tmpreg.x1 && myregion->y1 <= tmpreg.y1
							&& myregion->x2 >= tmpreg.x2 && myregion->y2 >= tmpreg.y2) {
						if ((!aw->alphachannel) || (MMSFBSURFACE_READ_BUFFER(aw->surface).opaque)) {
							if (aw->opacity == 0xff) {
								tmpreg = *myregion;
								lowest_win = i;
							}
						}
					}
				}
				else {
					// SECOND loop: blit affected window

					// calc source and destination
					MMSFBRectangle src_rect;
					int dst_x = ls_region.x1;
					int dst_y = ls_region.y1;

					src_rect.x = ls_region.x1 - myregion->x1;
					if (src_rect.x < 0) {
						dst_x-= src_rect.x;
						src_rect.x = 0;
					}

					src_rect.y = ls_region.y1 - myregion->y1;
					if (src_rect.y < 0) {
						dst_y-= src_rect.y;
						src_rect.y = 0;
					}

					src_rect.w = myregion->x2 - myregion->x1 + 1 - src_rect.x;
					if (myregion->x2 > ls_region.x2)
						src_rect.w-= myregion->x2 - ls_region.x2;

					src_rect.h = myregion->y2 - myregion->y1 + 1 - src_rect.y;
					if (myregion->y2 > ls_region.y2)
						src_rect.h-= myregion->y2 - ls_region.y2;

					if ((aw->vrect.w > 0)&&(aw->vrect.h > 0)) {
						// visible rectangle set, so have to adjust source offset
						src_rect.x += aw->vrect.x;
						src_rect.y += aw->vrect.y;
					}
/*
printf("#3winman: flip window id = %d, opaque = %d, src_rect = %d,%d %dx%d, dst = %d,%d\n",
						i, MMSFBSURFACE_READ_BUFFER(aw->surface).opaque,
						src_rect.x, src_rect.y, src_rect.w, src_rect.h, dst_x, dst_y);
*/

					// set the blitting flags and color
					if ((aw->alphachannel)&&((win_found)||(!this->dst_surface->config.surface_buffer->alphachannel))) {
						// the window has an alphachannel
						if (!(MMSFBSURFACE_READ_BUFFER(aw->surface).opaque)) {
							// (semi-)transparent surface buffer
							if (aw->opacity < 255) {
								this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) (MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
								this->dst_surface->setColor(0, 0, 0, aw->opacity);
							}
							else {
								this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_BLEND_ALPHACHANNEL);
							}
						}
						else {
							// opaque surface buffer, we do not need MMSFB_BLIT_BLEND_ALPHACHANNEL
							if (aw->opacity < 255) {
								this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_BLEND_COLORALPHA);
								this->dst_surface->setColor(0, 0, 0, aw->opacity);
							}
							else {
								this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_NOFX);
							}
						}

						// first window?
						if (!win_found) {
							// yes, clear the layer before blitting the first window surface
							if (cleared)
								this->dst_surface->clear();
							win_found = true;
						}
					}
					else {
						// the window has no alphachannel
						if (aw->opacity < 255) {
							this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_BLEND_COLORALPHA);
							this->dst_surface->setColor(0, 0, 0, aw->opacity);
						}
						else {
							this->dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_NOFX);
						}

						// first window?
						if (!win_found) {
							// yes, clear the layer before blitting the first window surface
							// but only, if the first window does not use the whole layer region
							// else we do not have to clear the layer region and can save CPU
							if (cleared)
								if ((aw->opacity < 255)||((dst_x != ls_region.x1) || (dst_y != ls_region.y1)
								 || (dst_x + src_rect.w <= ls_region.x2) || (dst_y + src_rect.h <= ls_region.y2))) {
									this->dst_surface->clear();
								}

							win_found = true;
						}
					}

					// check if layer surface and blit
					if (aw->islayersurface) {
						if (aw->saved_surface) {
							this->dst_surface->blit(aw->saved_surface, &src_rect, dst_x, dst_y);
						}
					}
					else {
						this->dst_surface->blit(aw->surface, &src_rect, dst_x, dst_y);
					}
				}
			}
		}
	}



    if (!win_found) {
        // if no window is drawn, check if we have to clear the layer region
	    if (cleared)
	        this->dst_surface->clear();
    }

    // draw the pointer
    drawPointer(&ls_region);

	// reset the clip
    this->dst_surface->setClip(NULL);

    // make changes visible
    if (refresh)
    	this->dst_surface->flip(&ls_region);


    // unlock
    if (!locked)
        lock.unlock();

    return true;
}

bool MMSFBWindowManager::setWindowOpacity(MMSFBWindow *window) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

    /* search for item */
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            /* reload windows config */
            loadWindowConfig(window, &(this->vwins.at(i)));

            /* redraw the window */
            flipSurface(this->vwins.at(i).surface, NULL, true);

            /* unlock */
            lock.unlock();

            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::setWindowPosition(MMSFBWindow *window, MMSFBRectangle *vrect) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

	// change visible rectangle and position in one step?
    if (vrect) {
    	// yes, change visible rectangle
        for (unsigned int i=0; i < this->windows.size(); i++) {
            if (this->windows.at(i).window == window) {
    			// set rect
            	this->windows.at(i).vrect = *vrect;
                break;
            }
        }
    }

    /* search for item */
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            /* get the old config */
            VISIBLE_WINDOWS old_vwin;
            old_vwin = this->vwins.at(i);

            /* reload windows config */
            loadWindowConfig(window, &(this->vwins.at(i)));

            /* moving high_freq_surface? */
            if (this->high_freq_surface == this->vwins.at(i).surface) {
                /* yes, reset it */
                mmsfbwindowmanager->flipSurface(this->high_freq_surface, NULL, true);
                this->high_freq_surface = NULL;
                this->high_freq_saved_surface = NULL;
                this->high_freq_lastflip = 0;
            }

            /* redraw the window */
            flipSurface(this->vwins.at(i).surface, NULL, true);

            /* redraw the old rects */
            if (old_vwin.region.y1 < this->vwins.at(i).region.y1) {
                /* redraw above */
                MMSFBRegion region;
                region = old_vwin.region;
                if (region.y2 >= this->vwins.at(i).region.y1)
                    region.y2 = this->vwins.at(i).region.y1 - 1;
                flipSurface(NULL, &region, true);
            }
            else
            if (old_vwin.region.y1 > this->vwins.at(i).region.y1) {
                /* redraw below */
                MMSFBRegion region;
                region = old_vwin.region;
                if (region.y1 <= this->vwins.at(i).region.y2)
                    region.y1 = this->vwins.at(i).region.y2 + 1;
                flipSurface(NULL, &region, true);
            }
            if (old_vwin.region.x1 < this->vwins.at(i).region.x1) {
                /* redraw left side */
                MMSFBRegion region;
                region = old_vwin.region;
                if  ((region.y2 >= this->vwins.at(i).region.y1)
                   &&(region.y1 <= this->vwins.at(i).region.y2)) {
                    if (region.x2 >= this->vwins.at(i).region.x1)
                        region.x2 = this->vwins.at(i).region.x1 - 1;
                    region.y1 = this->vwins.at(i).region.y1;
                    region.y2 = this->vwins.at(i).region.y2;
                    flipSurface(NULL, &region, true);
                }
            }
            else
            if (old_vwin.region.x1 > this->vwins.at(i).region.x1) {
                /* redraw right side */
                MMSFBRegion region;
                region = old_vwin.region;
                if  ((region.y2 >= this->vwins.at(i).region.y1)
                   &&(region.y1 <= this->vwins.at(i).region.y2)) {
                    if (region.x1 <= this->vwins.at(i).region.x2)
                        region.x1 = this->vwins.at(i).region.x2 + 1;
                    region.y1 = this->vwins.at(i).region.y1;
                    region.y2 = this->vwins.at(i).region.y2;
                    flipSurface(NULL, &region, true);
                }
            }

            /* unlock */
            lock.unlock();

            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::setWindowSize(MMSFBWindow *window, int w, int h) {

    /* check if initialized */
    INITCHECK;

    /* stop parallel processing */
    lock.lock();

    /* search for item which is visible */
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            /* found as visible window */
            VISIBLE_WINDOWS old_vwin;
            old_vwin = this->vwins.at(i);
            int old_w = old_vwin.region.x2 - old_vwin.region.x1 + 1;
            int old_h = old_vwin.region.y2 - old_vwin.region.y1 + 1;

            if ((old_w != w)||(old_h != h)) {
                /* hide the window without updating the screen */
                hideWindow(window, true, false);

                /* resizing surface */
                MMSFBSurface *surface;
                window->getSurface(&surface);
                surface->resize(w, h);

                /* search for item in the window list */
                for (unsigned int j=0; j < this->windows.size(); j++)
                    if (this->windows.at(j).window == window) {
                        // reset the visible region
                    	this->windows.at(j).vrect.x = 0;
                    	this->windows.at(j).vrect.y = 0;
                    	this->windows.at(j).vrect.w = 0;
                    	this->windows.at(j).vrect.h = 0;
                    	break;
                    }

                /* re-show it */
                if ((old_w <= w)&&(old_h <= h))
                    /* larger or equal */
                    showWindow(window, true, true);
                else {
                    /* new window is less than the old one */
                    showWindow(window, true, false);

                    /* flip the old region */
                    flipSurface(NULL, &old_vwin.region, true, true);
                }
            }

            /* unlock */
            lock.unlock();

            return true;
        }

    /* search for item in the window list */
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
            /* found as window which is currently not shown */
            /* resizing surface */
            MMSFBSurface *surface;
            window->getSurface(&surface);
            surface->resize(w, h);

            // reset the visible region
        	this->windows.at(i).vrect.x = 0;
        	this->windows.at(i).vrect.y = 0;
        	this->windows.at(i).vrect.w = 0;
        	this->windows.at(i).vrect.h = 0;

        	/* unlock */
            lock.unlock();

            return true;
        }

    /* not found */
    lock.unlock();
    return false;
}

bool MMSFBWindowManager::setWindowVisibleRectangle(MMSFBWindow *window, MMSFBRectangle *rect) {
	bool ret = false;

    // check if initialized
    INITCHECK;

    // stop parallel processing
    lock.lock();

    // search for item in available list
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
			// set rect
        	if (rect) {
        		this->windows.at(i).vrect = *rect;
        	}
        	else {
        		this->windows.at(i).vrect.x = 0;
        		this->windows.at(i).vrect.y = 0;
        		this->windows.at(i).vrect.w = 0;
        		this->windows.at(i).vrect.h = 0;
        	}

            ret = true;
            break;
        }

    // search for item in visible list
    for (unsigned int i=0; i < this->vwins.size(); i++)
        if (this->vwins.at(i).window == window) {
            // reload windows config
            loadWindowConfig(window, &(this->vwins.at(i)));

        	// redraw the window
            flipSurface(this->vwins.at(i).surface, NULL, true);

            ret = true;
            break;
        }

    lock.unlock();
    return ret;
}

bool MMSFBWindowManager::getWindowVisibleRectangle(MMSFBWindow *window, MMSFBRectangle *rect) {
	bool ret = false;

    // check if initialized
    INITCHECK;

    // stop parallel processing
    lock.lock();

    // search for item in available list
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
			// set rect
        	if (rect) *rect = this->windows.at(i).vrect;
        	if ((this->windows.at(i).vrect.w > 0)&&(this->windows.at(i).vrect.h > 0))
        		ret = true;
            break;
        }

    lock.unlock();
    return ret;
}

bool MMSFBWindowManager::getScreenshot(MMSFBWindow *window) {
	bool ret = false;

    // check if initialized
    INITCHECK;

    // stop parallel processing
    lock.lock();

    // search for item in available list
    for (unsigned int i=0; i < this->windows.size(); i++)
        if (this->windows.at(i).window == window) {
        	// window found, hide the window if shown
        	bool shown = window->isShown();
        	window->hide();

        	// switch the dst_surface
        	MMSFBSurface *saved_suf = this->dst_surface;
        	if (window->getSurface(&this->dst_surface)) {
				// draw the complete screen to the surface of the window
        		MMSFBRegion region;
        		this->dst_surface->getSize(&region.x2, &region.y2);
        		region.x1 = 0;
        		region.y1 = 0;
        		region.x2-= 1;
        		region.y2-= 1;
				flipSurface(NULL, &region, true, false);
        	}

        	// restore the dst_surface
        	this->dst_surface = saved_suf;

        	// show the window again
        	if (shown)
        		window->show();

        	ret = true;
            break;
        }

    lock.unlock();
    return ret;
}


void MMSFBWindowManager::setPointerPosition(int pointer_posx, int pointer_posy, bool pressed) {
	// changed?
	if (this->button_pressed == pressed)
		if ((this->pointer_posx == pointer_posx)&&(this->pointer_posy == pointer_posy))
			return;
	this->button_pressed = pressed;

	switch (this->layer_pixelformat) {
	case MMSFB_PF_YV12:
	case MMSFB_PF_I420:
		// use even pointer position for this pixelformats
		this->pointer_posx = pointer_posx & ~0x01;
		this->pointer_posy = pointer_posy & ~0x01;
		break;
	default:
		// use normal odd/even positions
		this->pointer_posx = pointer_posx;
		this->pointer_posy = pointer_posy;
		break;
	}

	// do nothing more if pointer will not be shown
	if (!this->show_pointer)
		return;

	// surface of pointer initialized?
	if (!this->pointer_surface)
		if (!loadPointer()) {
			// not loaded, set a primitive pointer
			this->pointer_rect.w = 21;
			this->pointer_rect.h = 21;
		    if (this->layer->createSurface(&this->pointer_surface, this->pointer_rect.w, this->pointer_rect.h)) {
		    	this->pointer_surface->clear();
			    this->pointer_surface->setColor(255,255,255,255);
			    this->pointer_surface->drawLine(0,this->pointer_rect.h/2,this->pointer_rect.w-1,this->pointer_rect.h/2);
			    this->pointer_surface->drawLine(this->pointer_rect.w/2,0,this->pointer_rect.w/2,this->pointer_rect.h-1);
		    }
		    else
		    	this->pointer_surface = NULL;
		}

	// save the old region
	MMSFBRegion old_region = this->pointer_region;

	// set the rectangle/region position
	this->pointer_rect.x = this->pointer_posx - (this->pointer_rect.w >> 1);
	this->pointer_rect.y = this->pointer_posy - (this->pointer_rect.h >> 1);
	this->pointer_region.x1 = this->pointer_rect.x;
	this->pointer_region.y1 = this->pointer_rect.y;
	this->pointer_region.x2 = this->pointer_rect.x + this->pointer_rect.w - 1;
	this->pointer_region.y2 = this->pointer_rect.y + this->pointer_rect.h - 1;

	// set opacity
    this->pointer_opacity = 255;
	this->pointer_fadecnt = 0;

	// check if i have to flip one or two regions
	if   ((old_region.x1 > this->pointer_region.x2)
		||(old_region.y1 > this->pointer_region.y2)
		||(old_region.x2 < this->pointer_region.x1)
		||(old_region.y2 < this->pointer_region.y1)) {
		// two regions to be updated
		flipSurface(NULL, &this->pointer_region, false);
		if (old_region.x1 != old_region.x2)
			flipSurface(NULL, &old_region, false);
	}
	else {
		// one region
		if (old_region.x1 > this->pointer_region.x1)
			old_region.x1 = this->pointer_region.x1;
		else
			old_region.x2 = this->pointer_region.x2;
		if (old_region.y1 > this->pointer_region.y1)
			old_region.y1 = this->pointer_region.y1;
		else
			old_region.y2 = this->pointer_region.y2;
		flipSurface(NULL, &old_region, false);
	}
}

bool MMSFBWindowManager::getPointerPosition(int &pointer_posx, int &pointer_posy) {
	// set?
	if ((this->pointer_posx<0)||(this->pointer_posy<0))
		return false;
	pointer_posx = this->pointer_posx;
	pointer_posy = this->pointer_posy;
	return true;
}

bool MMSFBWindowManager::loadPointer() {
#ifndef __L4_RE__
	string imagefile = (string)getPrefix() + "/share/disko/mmsgui/mmspointer.png";
#else
    string imagefile = "mmspointer.png";
#endif

	// try to read from taff?
	if (this->usetaff) {
		// yes, try with taff
		// assume: the taffpf (supported taff pixelformat) is correctly set
    	// first : try to read taff image without special pixelformat
		// second: try with pixelformat from my surfaces
		bool retry = false;
		do {
			MMSTaffFile *tafff;
			if (retry) {
    			retry = false;
    			DEBUGOUT("MMSFBWindowManager, retry\n");
				// have to convert taff with special destination pixelformat
				tafff = new MMSTaffFile(imagefile + ".taff", NULL,
	    								"", MMSTAFF_EXTERNAL_TYPE_IMAGE);
    			if (tafff) {
    				// set external file and requested pixelformat
    				tafff->setExternal(imagefile, MMSTAFF_EXTERNAL_TYPE_IMAGE);
    				DEBUGOUT("MMSFBWindowManager, taffpf = %d\n", taffpf);
    				tafff->setDestinationPixelFormat(taffpf);
    				// convert it
    				if (!tafff->convertExternal2TAFF()) {
    					// conversion failed
    					delete tafff;
    					break;
    				}
    				delete tafff;
    			}
			}

			// load image
			tafff = new MMSTaffFile(imagefile + ".taff", NULL,
									imagefile, MMSTAFF_EXTERNAL_TYPE_IMAGE);

			if (tafff) {
	    		if (tafff->isLoaded()) {

		    		// load the attributes
	    	    	int 		attrid;
	    	    	char 		*value_str;
	    	    	int  		value_int;
			    	void 		*img_buf = NULL;
			    	int 		img_width = 0;
			    	int 		img_height= 0;
			    	int 		img_pitch = 0;
			    	int 		img_size  = 0;
			    	MMSTAFF_PF 	img_pixelformat = MMSTAFF_PF_ARGB;
			    	bool 		img_premultiplied = true;
			    	int 		img_mirror_size = 0;

			    	while ((attrid=tafff->getNextAttribute(&value_str, &value_int, NULL))>=0) {
			    		switch (attrid) {
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_width:
			    			img_width = value_int;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_height:
			    			img_height = value_int;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pitch:
			    			img_pitch = value_int;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_size:
			    			img_size = value_int;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_data:
			    			img_buf = value_str;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pixelformat:
			    			img_pixelformat = (MMSTAFF_PF)value_int;
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_premultiplied:
			    			img_premultiplied = (value_int);
			    			break;
			    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_mirror_size:
			    			img_mirror_size = value_int;
			    			break;
			    		}
			    	}

			    	if (img_pixelformat != taffpf) {
			    		DEBUGOUT("MMSFBWindowManager, taffpf = %d\n", (int)taffpf);
			    		// the image from the file has not the same pixelformat as the surface
			    		if (!retry) {
			    			// retry with surface pixelformat
			    			DEBUGOUT("MMSFBWindowManager, request new pixf\n");
			    			retry = true;
			    			delete tafff;
			    			continue;
			    		}
			    		else
			    			retry = false;
			    	}
			    	else
			    	if ((img_width)&&(img_height)&&(img_pitch)&&(img_size)&&(img_buf)) {
			        	// successfully read, create a surface
						if (!this->layer->createSurface(&this->pointer_surface, img_width, img_height, this->pixelformat)) {
							DEBUGMSG("MMSFB", "cannot create surface for image file '%s'", imagefile.c_str());
							return false;
						}

						// blit from external buffer to surface
						this->pointer_surface->blitBuffer(img_buf, img_pitch, this->pixelformat,
															img_width, img_height, NULL, 0, 0);

						// free
						delete tafff;

						DEBUGMSG("MMSFB", "MMSFBWindowManager has loaded: '%s'", imagefile.c_str());

					    // set pointer width & height
					    this->pointer_rect.w = img_width;
					    this->pointer_rect.h = img_height;

					    return true;
			    	}
	    		}

	            // free
	            delete tafff;
	        }
		} while (retry);
	}


#ifdef  __HAVE_DIRECTFB__
    IDirectFBImageProvider *imageprov = NULL;
    DFBSurfaceDescription   surface_desc;

	// create image provider
    if (!mmsfb->createImageProvider(&imageprov, imagefile)) {
        if (imageprov)
        	imageprov->Release(imageprov);
        return false;
    }

    // get surface description
    if (imageprov->GetSurfaceDescription(imageprov, &surface_desc)!=DFB_OK) {
        imageprov->Release(imageprov);
        return false;
    }

    // create a surface
    if (!this->layer->createSurface(&this->pointer_surface, surface_desc.width, surface_desc.height)) {
        imageprov->Release(imageprov);
        return false;
    }

    // render to the surface
    if (imageprov->RenderTo(imageprov, (IDirectFBSurface *)this->pointer_surface->getDFBSurface(), NULL)!=DFB_OK) {
        imageprov->Release(imageprov);
        delete this->pointer_surface;
        return false;
    }

    // release imageprovider
    imageprov->Release(imageprov);

    // set pointer width & height
    this->pointer_rect.w = surface_desc.width;
    this->pointer_rect.h = surface_desc.height;
    return true;
#endif

	return false;
}

void MMSFBWindowManager::drawPointer(MMSFBRegion *region) {
	// should draw the pointer?
	if (!this->show_pointer)
		return;
	if ((this->pointer_posx<0)||(this->pointer_posy<0))
		return;
	if (!this->pointer_surface)
		return;
	if (this->pointer_opacity == 0)
		return;

	// blit the pointer surface with given opacity
	if (this->pointer_opacity < 255) {
		this->layer_surface->setBlittingFlags((MMSFBBlittingFlags) (MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
	    this->layer_surface->setColor(0, 0, 0, this->pointer_opacity);
	}
	else
		this->layer_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_BLEND_ALPHACHANNEL);
	this->layer_surface->blit(this->pointer_surface, NULL, this->pointer_rect.x, this->pointer_rect.y);
	this->layer_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_NOFX);
    this->layer_surface->setColor(0, 0, 0, 0);
}

unsigned char MMSFBWindowManager::getPointerOpacity() {
	return this->pointer_opacity;
}

void MMSFBWindowManager::setPointerOpacity(unsigned char opacity) {
	// set it
	this->pointer_opacity = opacity;
	this->pointer_fadecnt = 0;
	flipSurface(NULL, &this->pointer_region, false);
}

void MMSFBWindowManager::fadePointer() {
	if (!this->button_pressed) {
		if (this->pointer_opacity > 0) {
			if (this->pointer_fadecnt == 0)
				this->pointer_fadecnt = 1;
			else
				this->pointer_fadecnt*= 3;

			if (this->pointer_fadecnt >= 3) {
				// set it
				if (this->pointer_opacity > this->pointer_fadecnt / 3)
					this->pointer_opacity-= this->pointer_fadecnt / 3;
				else
					this->pointer_opacity = 0;
				flipSurface(NULL, &this->pointer_region, false);
			}
		}
	}
}

