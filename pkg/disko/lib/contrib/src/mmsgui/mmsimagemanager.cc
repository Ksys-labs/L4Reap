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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "mmsgui/mmsimagemanager.h"
#include "mmsgui/mmsgifloader.h"
#include "mmsgui/mmsfbmanager.h"


MMSImageManager::MMSImageManager(MMSFBLayer *layer) {
	if (!layer) {
		// use default layer
		layer = mmsfbmanager.getGraphicsLayer();
	}

	// save layer
	this->layer = layer;

    // get the pixelformat, create a little temp surface
	this->pixelformat = MMSFB_PF_NONE;
	MMSFBSurface *ts;
    if (this->layer->createSurface(&ts, 8, 1)) {
    	// okay, get the pixelformat from surface
    	ts->getPixelFormat(&this->pixelformat);
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
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_RGB16;
    	break;
    case MMSFB_PF_ABGR:
    	this->usetaff = true;
    	this->taffpf = MMSTAFF_PF_ABGR;
    	break;
    default:
    	break;
    }
}

MMSImageManager::~MMSImageManager() {
    /* free all surfaces */
    for (unsigned int i = 0; i < this->images.size(); i++) {
        for (int j = 0; j < this->images.at(i)->sufcount; j++)
            if (this->images.at(i)->suf[j].surface)
                delete this->images.at(i)->suf[j].surface;
        delete this->images.at(i);
    }
}


MMSFBSurface *MMSImageManager::getImage(const string &path, const string &filename, MMSIM_DESC_SUF **surfdesc,
										int mirror_size, bool gen_taff) {
    string		imagefile;
    MMSIM_DESC	*im_desc = NULL;
    int			reload_image = -1;

    // build filename
    imagefile = path;
    if (imagefile != "") imagefile+= "/";
    imagefile += filename;
    if (imagefile == "")
        return NULL;
    if (imagefile.substr(imagefile.size()-1,1)=="/")
        return NULL;

	if (gen_taff) {
		// check if we have to switch off the taff file generation
		if (strToUpr(imagefile.substr(0,7)) == "HTTP://") {
			gen_taff = false;
		}
		else
		if (strToUpr(imagefile.substr(0,6)) == "FTP://") {
			gen_taff = false;
		}
	}

    // lock threads
    this->lock.lock();

    DEBUGMSG("MMSGUI", "Load request for path=%s, name=%s", path.c_str(), filename.c_str());

    // searching within images list
    for (unsigned int i = 0; i < this->images.size(); i++) {
        if (this->images.at(i)->imagefile == imagefile) {
            // already loaded, check if the file has changed
            struct stat statbuf;
            if (stat(imagefile.c_str(), &statbuf)==0) {
                if (statbuf.st_mtime != this->images.at(i)->mtime) {
                    // file was modified, reload it
                    reload_image = (int)i;
                    this->images.at(i)->mtime = statbuf.st_mtime;
                    break;
                }
                else {
                    // do not reload
					DEBUGMSG("MMSGUI", "Reusing already loaded image path=%s, name=%s", path.c_str(), filename.c_str());
                    this->images.at(i)->usecount++;
                    if (surfdesc)
                        *surfdesc = this->images.at(i)->suf;
                    this->lock.unlock();
                    return this->images.at(i)->suf[0].surface;
                }
            }
            else {
                // do not reload
				DEBUGMSG("MMSGUI", "Reusing already loaded image path=%s, name=%s", path.c_str(), filename.c_str());
                this->images.at(i)->usecount++;
                if (surfdesc)
                    *surfdesc = this->images.at(i)->suf;
                this->lock.unlock();
                return this->images.at(i)->suf[0].surface;
            }
        }
    }

    // init im_desc
    im_desc = new MMSIM_DESC;
    memset(im_desc->suf, 0, sizeof(im_desc->suf));
    im_desc->suf[0].delaytime = im_desc->suf[1].delaytime = MMSIM_DESC_SUF_END;
    im_desc->sufcount = 0;
    im_desc->loading = false;

	DEBUGMSG("MMSGUI", "Loading image path=%s, name=%s", path.c_str(), filename.c_str());

    // first try to load GIF formated files
    if (isGIF(imagefile)) {
        // it's an GIF file
        im_desc->imagefile = imagefile;

        if (reload_image < 0) {
            // get the modification time of the file
            struct stat statbuf;
            if (stat(imagefile.c_str(), &statbuf)==0)
                im_desc->mtime = statbuf.st_mtime;
            else
                im_desc->mtime = 0;
        }

        if (reload_image < 0) {
            // load it
            MMSGIFLoader *gifloader = new MMSGIFLoader(im_desc, this->layer);
            gifloader->start();
            gifloader->block();

            if (im_desc->sufcount > 0) {
            	DEBUGMSG("MMSGUI", "ImageManager has loaded: '%s'", imagefile.c_str());

                // add to images list and return the surface
                im_desc->usecount = 1;
                this->images.push_back(im_desc);
                if (surfdesc)
                    *surfdesc = this->images.at(this->images.size()-1)->suf;
                this->lock.unlock();
                return im_desc->suf[0].surface;
            }
            else {
                // failed to load
            	DEBUGMSG("MMSGUI", "cannot load image file '%s'",imagefile.c_str());
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }
        }
        else {
            // increase usecount
            this->images.at(reload_image)->usecount++;

//TODO
            delete im_desc;
            if (surfdesc)
                *surfdesc = this->images.at(reload_image)->suf;
            this->lock.unlock();
            return this->images.at(reload_image)->suf[0].surface;
        }
    }
    else {
        // failed, try to read from taff?
/*
struct  timeval tv;
gettimeofday(&tv, NULL);
DEBUGOUT("start > %d\n", tv.tv_usec);
*/



    	if (this->usetaff) {
    		// yes, try with taff
    		// assume: the taffpf (supported taff pixelformat) is correctly set
	    	// first : try to read taff image without special pixelformat
    		// second: try with pixelformat from my surfaces
    		bool retry = false;
			MMSTaffFile *tafff = NULL;
    		do {
    			if (retry) {
	    			retry = false;
	    			DEBUGOUT("ImageManager, retry\n");

    				// have to convert taff with special destination pixelformat
	    			if (gen_taff) {
	    				// here we have to use *.taff file, which should store converted image data
						tafff = new MMSTaffFile(imagefile + ".taff", NULL,
												"", MMSTAFF_EXTERNAL_TYPE_IMAGE);
	    			}
	    			else {
	    				// hold converted image data in memory, do NOT write it to an *.taff file
						tafff = new MMSTaffFile("", NULL,
												"", MMSTAFF_EXTERNAL_TYPE_IMAGE,
												false, false, false, true, false);
	    			}

	    			if (tafff) {
        				// set external file and requested pixelformat
	    				tafff->setExternal(imagefile, MMSTAFF_EXTERNAL_TYPE_IMAGE);
	    				DEBUGOUT("ImageManager, taffpf = %d\n", taffpf);

	    				if (config.getGraphicsLayer().outputtype == MMSFB_OT_OGL) {
							// for ogl we don't need premultiplied images
							tafff->setDestinationPixelFormat(taffpf, false);
	    				}
	    				else {
	    					// use premultiplied images
	    					tafff->setDestinationPixelFormat(taffpf, true);
	    				}

	    				// set mirror size
	    				tafff->setMirrorEffect(mirror_size);

	    				// rotate image by 180°?
	    				tafff->rotate180(MMSFBBase_rotate180);

	    				// convert it
	    				if (!tafff->convertExternal2TAFF()) {
	    					// conversion failed
	    					delete tafff;
	    					tafff = NULL;
	    					break;
	    				}

	    				if (gen_taff) {
	    					// delete this tafff instance, because it will be re-loaded
	    					delete tafff;
	    					tafff = NULL;
	    				}
        			}
    			}

				// load image, but do not auto rewrite taff because have to set special attributes like mirror effect
    			if (gen_taff) {
					tafff = new MMSTaffFile(imagefile + ".taff", NULL,
											imagefile, MMSTAFF_EXTERNAL_TYPE_IMAGE,
											false, false, false, false, false);
    			}
    			else {
					if (!tafff) {
						retry = true;
						continue;
    				}
    			}
    			if (tafff) {
    				if (!tafff->isLoaded()) {
        				// set special attributes like mirror effect
    					tafff->setMirrorEffect(mirror_size);

	    				// rotate image by 180°?
	    				tafff->rotate180(MMSFBBase_rotate180);

	    				// convert it
	    				if (!tafff->convertExternal2TAFF()) {
	    					// conversion failed
	    					delete tafff;
	    					break;
	    				}
	    				delete tafff;
	    				tafff = NULL;

	    				// here we have to read from *.taff file, because special attributes will be written to file
	    				tafff = new MMSTaffFile(imagefile + ".taff", NULL,
	    	    								"", MMSTAFF_EXTERNAL_TYPE_IMAGE);
    				}
    			}
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
				    	bool		img_alphachannel = true;
				    	bool		img_rotate_180 = false;

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
				    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_alphachannel:
				    			img_alphachannel = (value_int);
				    			break;
				    		case MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_rotate_180:
				    			img_rotate_180 = (value_int);
				    			break;
				    		}
				    	}

				    	if (img_pixelformat != taffpf) {
				    		DEBUGOUT("ImageManager, taffpf = %d\n", (int)taffpf);
				    		// the image from the file has not the same pixelformat as the surface
				    		if (!retry) {
				    			// retry with surface pixelformat
				    			DEBUGOUT("ImageManager, request new pixf\n");
				    			retry = true;
				    			delete tafff;
				    			continue;
				    		}
				    		else
				    			retry = false;
				    	}
				    	else
				    	if (img_mirror_size != mirror_size) {
				    		DEBUGOUT("ImageManager, mirror_size = %d\n", (int)mirror_size);
				    		// the image from the file has not the same mirror_size
				    		if (!retry) {
				    			// retry with given mirror_size
				    			DEBUGOUT("ImageManager, request new mirror_size\n");
				    			retry = true;
				    			delete tafff;
				    			continue;
				    		}
				    		else
				    			retry = false;
				    	}
				    	else
						if (img_premultiplied && (config.getGraphicsLayer().outputtype == MMSFB_OT_OGL)) {
							DEBUGOUT("ImageManager, premultiplied image\n");
							// for ogl we don't need premultiplied images
							if (!retry) {
								// retry without pre-multiplication
								DEBUGOUT("ImageManager, retry without pre-multiplication\n");
								retry = true;
								delete tafff;
								continue;
							}
							else
								retry = false;
						}
						else
						if (!img_premultiplied && (config.getGraphicsLayer().outputtype != MMSFB_OT_OGL)) {
							DEBUGOUT("ImageManager, image not premultiplied\n");
							// we use premultiplied images
							if (!retry) {
								// retry with pre-multiplication
								DEBUGOUT("ImageManager, retry with pre-multiplication\n");
								retry = true;
								delete tafff;
								continue;
							}
							else
								retry = false;
						}
						else
						if (img_rotate_180 && !MMSFBBase_rotate180) {
				    		DEBUGOUT("ImageManager, taff image is rotated by 180 degree, but NOT requested\n");
				    		if (!retry) {
				    			// reset rotation
				    			DEBUGOUT("ImageManager, reset rotation\n");
				    			retry = true;
				    			delete tafff;
				    			continue;
				    		}
				    		else
				    			retry = false;
						}
						else
						if (!img_rotate_180 && MMSFBBase_rotate180) {
				    		DEBUGOUT("ImageManager, taff image is NOT rotated by 180 degree, but requested\n");
				    		if (!retry) {
				    			// retry with rotation
				    			DEBUGOUT("ImageManager, rotate 180 degree\n");
				    			retry = true;
				    			delete tafff;
				    			continue;
				    		}
				    		else
				    			retry = false;
						}
						else
				    	if ((img_width)&&(img_height)&&(img_pitch)&&(img_size)&&(img_buf)) {
				        	// successfully read
//				    		DEBUGOUT("ImageManager, use pixf = %d\n", (int)taffpf);
				            im_desc->imagefile = imagefile;

				            if (reload_image < 0) {
				                // get the modification time of the file
				                struct stat statbuf;
				                if (stat(imagefile.c_str(), &statbuf)==0)
				                    im_desc->mtime = statbuf.st_mtime;
				                else
				                    im_desc->mtime = 0;
				            }

				            if (reload_image < 0) {
//printf("ImageManager has loaded: '%s'   - %s\n", imagefile.c_str(),(img_alphachannel)?"alpha":"no alpha");
				                // create a surface
				                if (!this->layer->createSurface(&(im_desc->suf[0].surface), img_width, img_height, this->pixelformat)) {
				                    DEBUGMSG("MMSGUI", "cannot create surface for image file '%s'", imagefile.c_str());
				                    delete im_desc;
				                    this->lock.unlock();
				                    return NULL;
				                }
				                im_desc->sufcount = 1;

								// blit from external buffer to surface
								im_desc->suf[0].surface->blitBuffer(img_buf, img_pitch, this->pixelformat,
																	img_width, img_height, NULL, 0, 0,
																	!img_alphachannel);

				                // free
				                delete tafff;
				                tafff = NULL;

				                DEBUGMSG("MMSGUI", "ImageManager has loaded: '%s'", imagefile.c_str());

				                // add to images list and return the surface
				                im_desc->usecount = 1;
				                this->images.push_back(im_desc);
				                if (surfdesc)
				                    *surfdesc = this->images.at(this->images.size()-1)->suf;
				                this->lock.unlock();
				                return im_desc->suf[0].surface;
				            }
				            else {
				                // increase usecount
				                this->images.at(reload_image)->usecount++;

				                // check if I have to resize the surface
				                int w, h;
				                this->images.at(reload_image)->suf[0].surface->getSize(&w, &h);
				                if ((w != img_width) || (h != img_height))
				                    this->images.at(reload_image)->suf[0].surface->resize(img_width, img_height);

				                // copy img_buf to the surface
				                char *suf_ptr;
				                int suf_pitch;
				                im_desc->suf[0].surface->lock(MMSFB_LOCK_WRITE, (void**)&suf_ptr, &suf_pitch);

				                if (img_pitch == suf_pitch)
				                	// copy in one block
				                	memcpy(suf_ptr, img_buf, img_pitch * img_height);
				                else {
				                	// copy each line
				                	char *img_b = (char*)img_buf;
				                	for (int i = 0; i < img_height; i++) {
				                		memcpy(suf_ptr, img_b, img_pitch);
				                		suf_ptr+=suf_pitch;
				                		img_b+=img_pitch;
				                	}
				                }
				                im_desc->suf[0].surface->unlock();

				                // free
				                delete tafff;
				                tafff = NULL;

				                DEBUGMSG("MMSGUI", "ImageManager has reloaded: '%s'", imagefile.c_str());

				                // return the surface
				                delete im_desc;
				                if (surfdesc)
				                    *surfdesc = this->images.at(reload_image)->suf;
				                this->lock.unlock();
				                return this->images.at(reload_image)->suf[0].surface;
				            }
				    	}
		    		}

		            // free
		            delete tafff;
		            tafff = NULL;
		        }
    		} while (retry);
    	}


#ifdef  __HAVE_DIRECTFB__
        IDirectFBImageProvider  *imageprovider = NULL;
        DFBSurfaceDescription   surface_desc;

        /* failed, try it with DFB providers */
    	if (!loadImage(&imageprovider, "", imagefile)) {
        	DEBUGMSG("MMSGUI", "cannot load image file '%s'", imagefile.c_str());
            if (reload_image < 0) {
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }
            else {
                this->images.at(reload_image)->usecount++;
                delete im_desc;
                if (surfdesc)
                    *surfdesc = this->images.at(reload_image)->suf;
                this->lock.unlock();
                return this->images.at(reload_image)->suf[0].surface;
            }
        }
        im_desc->imagefile = imagefile;

        if (reload_image < 0) {
            /* get the modification time of the file */
            struct stat statbuf;
            if (stat(imagefile.c_str(), &statbuf)==0)
                im_desc->mtime = statbuf.st_mtime;
            else
                im_desc->mtime = 0;
        }


        /* get surface description */
        if (imageprovider->GetSurfaceDescription(imageprovider, &surface_desc)!=DFB_OK) {
            /* release imageprovider */
            imageprovider->Release(imageprovider);
            DEBUGMSG("MMSGUI", "cannot read surface desciption from image file '%s'", imagefile.c_str());
            if (reload_image < 0) {
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }
            else {
                this->images.at(reload_image)->usecount++;
                delete im_desc;
                if (surfdesc)
                    *surfdesc = this->images.at(reload_image)->suf;
                this->lock.unlock();
                return this->images.at(reload_image)->suf[0].surface;
            }
        }

        if (reload_image < 0) {
            /* create a surface */
            if (!this->layer->createSurface(&(im_desc->suf[0].surface), surface_desc.width, surface_desc.height, this->pixelformat)) {
                /* release imageprovider */
                imageprovider->Release(imageprovider);
                DEBUGMSG("MMSGUI", "cannot create surface for image file '%s'", imagefile.c_str());
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }
            im_desc->sufcount = 1;

            // check if dfb surface available
            if (!im_desc->suf[0].surface->getDFBSurface()) {
                /* release imageprovider */
                imageprovider->Release(imageprovider);
                delete im_desc->suf[0].surface;
                DEBUGMSG("MMSGUI", "cannot render image file '%s' because it is not a DFB surface", imagefile.c_str());
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }

            /* render to the surface */
            if (imageprovider->RenderTo(imageprovider, (IDirectFBSurface *)im_desc->suf[0].surface->getDFBSurface(), NULL)!=DFB_OK) {
                /* release imageprovider */
                imageprovider->Release(imageprovider);
                delete im_desc->suf[0].surface;
                DEBUGMSG("MMSGUI", "cannot render image file '%s'", imagefile.c_str());
                delete im_desc;
                this->lock.unlock();
                return NULL;
            }

            /* release imageprovider */
            imageprovider->Release(imageprovider);

/*
gettimeofday(&tv, NULL);
DEBUGOUT("end < %d\n", tv.tv_usec);
*/

            DEBUGMSG("MMSGUI", "ImageManager has loaded: '%s'", imagefile.c_str());

            /* add to images list and return the surface */
            im_desc->usecount = 1;
            this->images.push_back(im_desc);
            if (surfdesc)
                *surfdesc = this->images.at(this->images.size()-1)->suf;
            this->lock.unlock();
            return im_desc->suf[0].surface;
        }
        else {
            /* increase usecount */
            this->images.at(reload_image)->usecount++;

            /* check if I have to resize the surface */
            int w, h;
            this->images.at(reload_image)->suf[0].surface->getSize(&w, &h);
            if ((w != surface_desc.width) || (h != surface_desc.height))
                this->images.at(reload_image)->suf[0].surface->resize(surface_desc.width, surface_desc.height);

            /* render to the surface */
            if (imageprovider->RenderTo(imageprovider, (IDirectFBSurface *)this->images.at(reload_image)->suf[0].surface->getDFBSurface(), NULL)!=DFB_OK) {
                /* release imageprovider */
                imageprovider->Release(imageprovider);
                DEBUGMSG("MMSGUI", "cannot render image file '%s'", imagefile.c_str());
                delete im_desc;
                if (surfdesc)
                    *surfdesc = this->images.at(reload_image)->suf;
                this->lock.unlock();
                return this->images.at(reload_image)->suf[0].surface;
            }

            /* release imageprovider */
            imageprovider->Release(imageprovider);

            DEBUGMSG("MMSGUI", "ImageManager has reloaded: '%s'", imagefile.c_str());

            /* return the surface */
            delete im_desc;
            if (surfdesc)
                *surfdesc = this->images.at(reload_image)->suf;
            this->lock.unlock();
            return this->images.at(reload_image)->suf[0].surface;
        }
#endif
    }

    this->lock.unlock();
    return NULL;
}

MMSFBSurface *MMSImageManager::newImage(const string &name, unsigned int width, unsigned int height, MMSFBSurfacePixelFormat pixelformat) {
//    DFBSurfaceDescription   desc;
    MMSIM_DESC              *im_desc = NULL;


    // lock threads
    this->lock.lock();

    if (name != "") {
        /* search name within images list */
        for (unsigned int i = 0; i < this->images.size(); i++) {
            if (this->images.at(i)->name == name) {
                /* found, must not create a new image */
                this->images.at(i)->usecount++;
                this->lock.unlock();
                return this->images.at(i)->suf[0].surface;
            }
        }
    }

    /* init */
    im_desc = new MMSIM_DESC;
    memset(im_desc->suf, 0, sizeof(im_desc->suf));
    im_desc->suf[0].delaytime = im_desc->suf[1].delaytime = MMSIM_DESC_SUF_END;
    im_desc->sufcount = 0;
    im_desc->loading = false;

    im_desc->name = name;

/*    desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    desc.width = width;
    desc.height = height;
    desc.pixelformat = pixelformat;
*/
//    if (this->dfb->CreateSurface(this->dfb, &desc, &(im_desc.surface)) != DFB_OK)
    if (!this->layer->createSurface(&(im_desc->suf[0].surface), width, height, (pixelformat==MMSFB_PF_NONE)?this->pixelformat:pixelformat)) {
        this->lock.unlock();
        return NULL;
    }
    im_desc->sufcount = 1;
    im_desc->imagefile = "";

    /* add to images list and return the surface */
    im_desc->usecount = 1;
    this->images.push_back(im_desc);
    this->lock.unlock();
    return im_desc->suf[0].surface;
}

void MMSImageManager::releaseImage(const string &path, const string &filename) {
    string imagefile;

    /* build filename */
    imagefile = path;
    if (imagefile != "") imagefile+= "/";
    imagefile += filename;
    if (imagefile == "")
        return;
    if (imagefile.substr(imagefile.size()-1,1)=="/")
        return;

    // lock threads
    this->lock.lock();

    /* search within images list */
    for (unsigned int i = 0; i < this->images.size(); i++) {
        if (this->images.at(i)->imagefile == imagefile) {
            /* surface does exist in memory */
            this->images.at(i)->usecount--;
            if (this->images.at(i)->usecount <= 0) {
                /* this surface is not used anymore */
            	DEBUGMSG("MMSGUI", "ImageManager deletes: '%s'", this->images.at(i)->imagefile.c_str());

                for (int j = 0; j < this->images.at(i)->sufcount; j++)
                    if (this->images.at(i)->suf[j].surface)
                        delete this->images.at(i)->suf[j].surface;

                delete this->images.at(i);
                this->images.erase(this->images.begin()+i);
                break;
            }
        }
    }

    this->lock.unlock();
}

void MMSImageManager::releaseImage(MMSFBSurface *surface) {
    /* NULL? */
    if (!surface) return;

    // lock threads
    this->lock.lock();

    /* search within images list */
    for (unsigned int i = 0; i < this->images.size(); i++) {
        if (this->images.at(i)->suf[0].surface == surface) {
            /* surface does exist in memory */
            this->images.at(i)->usecount--;
            if (this->images.at(i)->usecount <= 0) {
                /* this surface is not used anymore */
            	DEBUGMSG("MMSGUI", "ImageManager deletes: '%s'", this->images.at(i)->imagefile.c_str());

//printf("ImageManager deletes: '%s'\n", this->images.at(i)->imagefile.c_str());

                for (int j = 0; j < this->images.at(i)->sufcount; j++)
                    if (this->images.at(i)->suf[j].surface)
                        delete this->images.at(i)->suf[j].surface;

                delete this->images.at(i);
                this->images.erase(this->images.begin()+i);
                break;
            }
        }
    }

    this->lock.unlock();
}

