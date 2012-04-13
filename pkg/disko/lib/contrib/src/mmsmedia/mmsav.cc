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

#include "mmsmedia/mmsav.h"
#ifdef __HAVE_DIRECTFB__
#include <directfb_version.h>
#endif
#include <string.h>
#include <stdlib.h>

#include "mmsmedia/mmsgst.h"


MMS_CREATEERROR(MMSAVError);


#ifdef __HAVE_GSTREAMER__

static void* gstPlayRoutine(GST_DISKOVIDEOSINK_DATA	*gst_diskovideosink_data) {

	printf("uri %s\n", gst_diskovideosink_data->uri.c_str());

	if (gst_diskovideosink_data->pipeline) {
		// playing gstreamer pipe
		mmsGstPlay(gst_diskovideosink_data->pipeline);
	}

	return NULL;
}

#endif




#ifdef __HAVE_XINE__

#ifdef __HAVE_DIRECTFB__
DFBResult dfbres;
#define THROW_DFB_ERROR(dfbres,msg) {if (dfbres) { string s1 = msg; string s2 = DirectFBErrorString((DFBResult)dfbres); throw MMSAVError(dfbres,s1 + " [" + s2 + "]"); }else{ throw MMSAVError(0,msg); }}
#endif

/**
 * Callback, that will be called before a frame is drawn.
 *
 * It checks if the ratio has changed and sets some variables if so.
 *
 * @param   cdata       [in/out]    pointer to VODESC structure
 * @param   width       [in]        width of dvd stream
 * @param   height      [in]        height of dvd stream
 * @param   ratio       [in]        ratio of dvd stream
 * @param   format      [in]        pixel format of dvd stream
 * @param   dest_rect   [out]       the current rectangle will be returned
 */
#ifdef __HAVE_DIRECTFB__
static void dfb_output_cb(void *cdata, int width, int height, double ratio,
						  DFBSurfacePixelFormat format, DFBRectangle* dest_rect) {
    VODESC *vodesc = (VODESC *) cdata;
    int    newH, newW;

	//DEBUGOUT("\noutput_cb %d:%d:%f:%d:%d",width,height,ratio,vodesc->windsc.width,vodesc->windsc.height);
    if (vodesc->ratio != ratio) {
    	newW = (int)((double)(vodesc->windsc.height) * ratio + 0.5);
    	newH = (int)((double)(vodesc->windsc.width) / ratio + 0.5);

    	/* ratio has changed */
        if (ratio<1.0) {
        	if(newW > vodesc->windsc.width) {
        		vodesc->rect.w = vodesc->windsc.width;
        		vodesc->rect.h = newH;
        	} else {
				vodesc->rect.w = newW;
				vodesc->rect.h = vodesc->windsc.height;
        	}
            vodesc->rect.x = (vodesc->windsc.width - vodesc->rect.w) / 2;
            vodesc->rect.y = 0;
        } else {
        	if(newH > vodesc->windsc.height) {
        		vodesc->rect.h = vodesc->windsc.height;
                vodesc->rect.w = newW;
        	} else {
        		vodesc->rect.w = vodesc->windsc.width;
        		vodesc->rect.h = newH;
        	}
            vodesc->rect.x = 0;
            vodesc->rect.y = (vodesc->windsc.height - vodesc->rect.h) / 2;
        }

        /* save other infos */
        vodesc->format = format;
        vodesc->ratio  = ratio;
        vodesc->width  = width;
        vodesc->height = height;
        /* clear surface */
        vodesc->winsurface->clear();
        vodesc->winsurface->flip();
        vodesc->winsurface->clear();
    }

    /* return the current rect */
    *dest_rect=(DFBRectangle&)vodesc->rect;
    //DEBUGOUT("\nrect %d:%d:%d:%d",dest_rect->x,dest_rect->y,dest_rect->w,dest_rect->h);

}
#endif


/*static void printFrameFormat(int frame_format) {
	switch(frame_format) {
		case XINE_VORAW_YV12:
			printf("YV12 frame\n");
			break;
		case XINE_VORAW_YUY2:
			printf("YUY2 frame\n");
			break;
		case XINE_VORAW_RGB:
			printf("RGB frame\n");
			break;
		default:
			printf("unknown frame format\n");
	}
}*/


void raw_frame_cb(void *user_data, int frame_format, int frame_width, int frame_height, double frame_aspect, void *data0, void *data1, void *data2) {
	MMSRAW_USERDATA *userd =(MMSRAW_USERDATA *)user_data;
/*	printf("-------\nframe format: ");
	printFrameFormat(frame_format);
	printf("frame_width: %d\n", frame_width);
	printf("frame_height: %d\n", frame_height);
	printf("frame_height: %f\n", frame_aspect);
	printf("plane0: %p\n", data0);
	printf("plane1: %p\n", data1);
	printf("plane2: %p\n", data2);
	printf("-------\n");*/

	if(!userd) {
		//there is no window to draw on!
		return;
	}

    if (userd->lastaspect != frame_aspect) {
    	// format changed
		printf("format change %f\n", frame_aspect);
    	int newW = (int)((double)(userd->size.h) * frame_aspect + 0.5);
    	int newH = (int)((double)(userd->size.w) / frame_aspect + 0.5);

    	/* ratio has changed */
        if (frame_aspect<1.0) {
        	if(newW > userd->size.w) {
        		userd->dest.w = userd->size.w;
        		userd->dest.h = newH;
        	} else {
        		userd->dest.w = newW;
        		userd->dest.h = userd->size.w;
        	}

        	userd->dest.x = (userd->size.w - userd->dest.w) / 2;
        	userd->dest.y = 0;
        } else {
        	if(newH > userd->size.h) {
        		userd->dest.h = userd->size.h;
        		userd->dest.w = newW;
        	} else {
        		userd->dest.w = userd->size.w;
        		userd->dest.h = newH;
        	}
        	userd->dest.x = (userd->size.w - userd->dest.w) / 2;;
        	userd->dest.y = (userd->size.h - userd->dest.h) / 2;
        }

        userd->lastaspect  = frame_aspect;
        /* clear surface */
        userd->surf->clear();
    	userd->surf->flip(NULL);
        userd->surf->clear();

        // printf("w,h,x,y: %d, %d, %d, %d\n", userd->dest.w,userd->dest.h,userd->dest.x,userd->dest.y);
        userd->dest.w&=~0x01;
        userd->dest.h&=~0x01;
        userd->dest.x&=~0x01;
        userd->dest.y&=~0x01;
        // printf("w,h,x,y: %d, %d, %d, %d\n", userd->dest.w,userd->dest.h,userd->dest.x,userd->dest.y);

        // delete iterim surface
		if (userd->interim) {
			delete userd->interim;
			userd->interim = NULL;
		}
    }


/*
////// TEST //////////////////////////////////////
static unsigned char rrrrr_buf[2048*768];
static int rrrrr=0;
if(!rrrrr) {
	rrrrr=1;
	for (int i = 0; i < sizeof(rrrrr_buf)/4; i++) {
		rrrrr_buf[i*4] = 0xff;
		rrrrr_buf[i*4+1] = 0x00;
		rrrrr_buf[i*4+2] = 0x00;
		rrrrr_buf[i*4+3] = 0x00;
	}
}
data0 = rrrrr_buf;
frame_format = XINE_VORAW_YUY2;
//////////////////////////////////////////////////
*/

	if (userd->surf_pixelformat == MMSFB_PF_YV12) {
		// the destination has YV12 pixelformat
		if ((frame_format != XINE_VORAW_YV12) && (!userd->interim)) {
			// have to allocate iterim buffer
			switch (frame_format) {
			case XINE_VORAW_YUY2:
			case XINE_VORAW_RGB:
				// we get YUY2 or RGB24 data, allocate interim buffer for YV12 convertion
				userd->interim = new MMSFBSurface(frame_width, frame_height, MMSFB_PF_YV12);
				break;
			}
		}

		if (userd->interim) {
			// blit to interim and then stretch it
			switch (frame_format) {
			case XINE_VORAW_YUY2:
				// source is YUY2
				userd->interim->blitBuffer(data0, frame_width*2, MMSFB_PF_YUY2,
										   frame_width, frame_height, NULL, 0, 0);
				break;
			case XINE_VORAW_RGB:
				// source is RGB24
				userd->interim->blitBuffer(data0, frame_width*3, MMSFB_PF_RGB24,
										   frame_width, frame_height, NULL, 0, 0);
				break;
			}
			userd->surf->stretchBlit(userd->interim, NULL, &userd->dest);
		} else {
			// source is YV12
			MMSFBExternalSurfaceBuffer buf;
			buf.ptr = data0;
			buf.pitch = frame_width;
			buf.ptr2 = data1;
			buf.pitch2 = frame_width / 2;
			buf.ptr3 = data2;
			buf.pitch3 = frame_width / 2;

			userd->surf->stretchBlitBuffer(&buf, MMSFB_PF_YV12,
										   frame_width, frame_height, NULL, &userd->dest);
		}
	}
	else {
		// destination with any other pixelformat
		if (frame_format == XINE_VORAW_YV12) {
    		// we get YV12 data
			if (!userd->interim) {
				// allocate interim buffer for YV12 stretch blit
				userd->interim = new MMSFBSurface(userd->dest.w, userd->dest.h, MMSFB_PF_YV12);
				if (userd->interim) userd->interim->setBlittingFlags(MMSFB_BLIT_ANTIALIASING);
			}
    	}

		if (userd->interim) {
			// source is YV12
			MMSFBExternalSurfaceBuffer buf;
			buf.ptr = data0;
			buf.pitch = frame_width;
			buf.ptr2 = data1;
			buf.pitch2 = frame_width / 2;
			buf.ptr3 = data2;
			buf.pitch3 = frame_width / 2;
			MMSFBRectangle mydest = userd->dest;
			mydest.x = 0;
			mydest.y = 0;

			userd->interim->stretchBlitBuffer(&buf, MMSFB_PF_YV12,
									   frame_width, frame_height, NULL, &mydest);
			userd->surf->blit(userd->interim, NULL, userd->dest.x, userd->dest.y);

		} else {
			// source is RGB24
			userd->surf->stretchBlitBuffer(data0, frame_width*3, MMSFB_PF_RGB24,
										   frame_width, frame_height, NULL, &userd->dest);
		}
	}


	if(userd->numOverlays > 0) {
		int rw = (userd->dest.w << 10) / frame_width;
		int rh = (userd->dest.h << 10) / frame_height;

		// save and set blitting flags
		MMSFBBlittingFlags saved_flags;
		userd->surf->getBlittingFlags(&saved_flags);
		userd->surf->setBlittingFlags(MMSFB_BLIT_BLEND_ALPHACHANNEL | MMSFB_BLIT_ANTIALIASING);

		if (!userd->overlayInterim) {
			// create interim surface for stretching overlays
			userd->overlayInterim = new MMSFBSurface(userd->size.w, userd->size.h, MMSFB_PF_ARGB);
		}

		// for all overlays
		for (int i = 0; i < userd->numOverlays; ++i) {
			// get overlay infos
			raw_overlay_t *ovl = &(userd->overlays[i]);

			// calc x/y offsets
			int x = (ovl->ovl_x * rw) >> 10;
			if(userd->size.w > userd->dest.w) {
				x += (userd->size.w - userd->dest.w) >> 1;
			}
			int y = (ovl->ovl_y * rh) >> 10;
			if(userd->size.h > userd->dest.h) {
				y += (userd->size.h - userd->dest.h) >> 1;
			}

			// stretch to interim
			MMSFBRectangle dest_rect;
			dest_rect.x = 0;
			dest_rect.y = 0;
			dest_rect.w = (ovl->ovl_w * rw) >> 10;
			dest_rect.h = (ovl->ovl_h * rh) >> 10;
			userd->overlayInterim->stretchBlitBuffer(ovl->ovl_rgba, ovl->ovl_w * 4, MMSFB_PF_ARGB, ovl->ovl_w, ovl->ovl_h, NULL, &dest_rect);

			// blit to target surface
			userd->surf->blit(userd->overlayInterim, &dest_rect, x, y);
		}

		// restore blitting flags
		userd->surf->setBlittingFlags(saved_flags);
	}

	userd->surf->flip(NULL);
}

/**
 * Callback, that will be called each time an overlay state changes @see: xine.h
 */
void raw_overlay_cb(void *user_data, int num_ovl, raw_overlay_t *overlays_array) {
	MMSRAW_USERDATA *userd =(MMSRAW_USERDATA *)user_data;

	userd->numOverlays = num_ovl;
	userd->overlays = overlays_array;
}


/**
 * Callback, that will be called after a frame is drawn.
 *
 * It sets clipping areas and does the flipping.
 *
 * @param   cdata       [in/out]    pointer to VODESC structure
 */
#ifdef __HAVE_DIRECTFB__
#if DIRECTFB_MAJOR_VERSION == 1
static void dfb_frame_cb(void *cdata) {
#else
static int dfb_frame_cb(void *cdata) {
#endif
    VODESC *vodesc = (VODESC *) cdata;
/* this was for GUI optimization which is currently not active

    if (vodesc->rect.y > 0) {
        DFBRegion reg;
        reg.x1=0;
        reg.y1=0;
        reg.x2=vodesc->rect.w-1;
        reg.y2=vodesc->rect.y-1;
        vodesc->winsurface->setClip(&reg);
        vodesc->winsurface->clear();
        vodesc->winsurface->setClip(NULL);
    }

    if (vodesc->rect.h < vodesc->windsc.height) {
        DFBRegion reg;
        reg.x1=0;
        reg.y1=vodesc->rect.y+vodesc->rect.h;
        reg.x2=vodesc->rect.w-1;
        reg.y2=vodesc->windsc.height-1;
        vodesc->winsurface->setClip(&reg);
        vodesc->winsurface->clear();
        vodesc->winsurface->setClip(NULL);
    }

    vodesc->winsurface->flip(NULL, (MMSFBSurfaceFlipFlags)(DSFLIP_ONSYNC));
*/

    /*DFBRegion reg;
    reg.x1=vodesc->rect.x;
    reg.y1=vodesc->rect.y;
    reg.x2=reg.x1 + vodesc->rect.w-1;
    reg.y2=reg.y1 + vodesc->rect.h-1;*/


//    vodesc->winsurface->flip(NULL, (MMSFBSurfaceFlipFlags)(DSFLIP_WAITFORSYNC));
    vodesc->winsurface->flip();


    //vodesc->winsurface->lock();

    /*          frame->surface->Unlock( frame->surface );

          frame->surface->Lock( frame->surface, DSLF_WRITE,
                               (void*)&frame->vo_frame.base[0],
                               (int *)&frame->vo_frame.pitches[0] );
*/

    //vodesc->winsurface->flip(NULL, (MMSFBSurfaceFlipFlags)(DSFLIP_ONSYNC));
    //vodesc->winsurface->flip(NULL, (MMSFBSurfaceFlipFlags)DSFLIP_NONE);

    //vodesc->winsurface->unlock();

#if DIRECTFB_MAJOR_VERSION < 1
    return 0;
#endif
}
#endif

typedef struct {
	xine_stream_t	*stream;
	int				pos;
	short			*status;
	const char 		*mrl;
	pthread_mutex_t	*lock;
} internalStreamData;

static void* xinePlayRoutine(void *data) {
	if(!data) return NULL;

	internalStreamData *streamData = (internalStreamData*)data;

	pthread_mutex_lock(streamData->lock);
	if(*(streamData->status) == MMSAV::STATUS_PLAYING)
	    xine_stop(streamData->stream);

	if(*(streamData->status) > MMSAV::STATUS_NONE)
	    xine_close(streamData->stream);

    if(!xine_open(streamData->stream, streamData->mrl) || !xine_play(streamData->stream, streamData->pos, 0)) {
        switch(xine_get_error(streamData->stream)) {
            case XINE_ERROR_NO_INPUT_PLUGIN :
                DEBUGMSG("MMSAV", "Error while trying to play stream: No input plugin");
                break;
            case XINE_ERROR_NO_DEMUX_PLUGIN :
                DEBUGMSG("MMSAV", "Error while trying to play stream: No demux plugin");
                break;
            case XINE_ERROR_DEMUX_FAILED :
                DEBUGMSG("MMSAV", "Error while trying to play stream: Error in demux plugin");
                break;
            case XINE_ERROR_INPUT_FAILED :
                DEBUGMSG("MMSAV", "Error while trying to play stream: Error in input plugin");
                break;
            case XINE_ERROR_MALFORMED_MRL :
                DEBUGMSG("MMSAV", "Error while trying to play stream: Malformed MRL");
                break;
            default:
                DEBUGMSG("MMSAV", "Unknown error while trying to play stream");
                break;
        }
        *(streamData->status) = MMSAV::STATUS_NONE;
    }
    else
    	*(streamData->status) = MMSAV::STATUS_PLAYING;

	pthread_mutex_unlock(streamData->lock);
	delete streamData;

	return NULL;
}

static void* stopRoutine(void *data) {
	if(!data) return NULL;

	internalStreamData *streamData = (internalStreamData*)data;
	pthread_mutex_lock(streamData->lock);
    xine_stop(streamData->stream);
    xine_set_param(streamData->stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
    xine_close(streamData->stream);
    *(streamData->status) = MMSAV::STATUS_STOPPED;
	pthread_mutex_unlock(streamData->lock);
	delete streamData;

    return NULL;
}

/**
 * Initializes some xine stuff.
 *
 * It creates the xine object that is used for
 * audio/video stream creation.
 * Then it loads the user's xine configuration and sets
 * the given verbosity.
 *
 * @exception   MMSAVError  cannot get a new xine object
 */
void MMSAV::xineInit() {
	/* get a new xine object */
    if (!(this->xine = xine_new()))
        throw MMSAVError(0, "Cannot get a new xine object");

    /* load xine config */
    string cfg;
    if(getenv("XINERC"))
        cfg = getenv("XINERC");
    else {
        if (getenv("HOME"))
            cfg = string(getenv("HOME")) + "/.xine";
        else
            cfg = "~/.xine";
        mkdir(cfg.c_str(), 755);
        cfg = cfg + "/config";
    }
    xine_config_load(this->xine, cfg.c_str());

    /* init xine */
    xine_init(this->xine);

    /* set verbosity */
    if(this->verbose)
        xine_engine_set_param(this->xine, XINE_PARAM_VERBOSITY, XINE_VERBOSITY_DEBUG);
    else
        xine_engine_set_param(this->xine, XINE_PARAM_VERBOSITY, XINE_VERBOSITY_NONE);
}


#endif


bool MMSAV::onHandleInput(MMSWindow *window, MMSInputEvent *input) {
	// send the input event to the media backend
	return sendEvent(input);
}

/**
 * Initializes everything that is needed my MMSAV.
 *
 * First it initializes xine.
 * Then it sets some internal variables and does some surface
 * flipping.
 *
 * @param   verbose [in]    if true the xine engine writes debug messages to stdout
 * @param   window  [in]    window that will be used for video output
 *
 * @see MMSAV::xineInit()
 *
 * @exception   MMSAVError  Cannot get a new xine object
 * @exception   MMSAVError  MMSFBSurface::clear() failed
 * @exception   MMSAVError  MMSFBSurface::flip() failed
 * @exception   MMSAVError  Cannot open the DFB video driver
 */
void MMSAV::initialize(const bool verbose, MMSWindow *window) {
    this->verbose          = verbose;
	this->window           = window;

	onHandleInputConnection.disconnect();
	if (window)
		onHandleInputConnection = window->onHandleInput->connect(sigc::mem_fun(this,&MMSAV::onHandleInput));


    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	// nothing to do, because the pipe will be created when starting playback
    	// and then we have only to put keyboard, mouse, touchscreen events to the pipe
#endif
    }
    else {
#ifdef __HAVE_XINE__

    DEBUGMSG("MMSMedia", "xineInit()...");

    /* initialize xine */
    xineInit();


    DEBUGMSG("MMSMedia", "xineInit() done.");

    memset(&this->userd, 0, sizeof(this->userd));

	if (mmsfb->getBackend() != MMSFB_BE_DFB) {
		this->rawvisual.raw_output_cb = raw_frame_cb;
		this->rawvisual.supported_formats = XINE_VORAW_YV12;
		if(window) {
			this->rawvisual.user_data = (void *)&(this->userd);
			this->rawvisual.raw_overlay_cb = raw_overlay_cb;
		} else {
			this->rawvisual.user_data = NULL;
		}
	}
	else {
#ifdef __HAVE_DIRECTFB__
		this->vodesc.format    = DSPF_UNKNOWN;
		this->vodesc.ratio     = 1.25;
		this->vodesc.width     = 720;
		this->vodesc.height    = 576;
		this->vodesc.rect.x    = 0;
		this->vodesc.rect.y    = 0;

		if(window) {
			window->getSurface()->getSize(&(this->vodesc.windsc.width), &(this->vodesc.windsc.height));
			this->vodesc.winsurface = window->getSurface();
			if(!this->vodesc.winsurface->clear())
				THROW_DFB_ERROR(dfbres, "MMSFBSurface::clear() failed");
			if(!this->vodesc.winsurface->flip())
				THROW_DFB_ERROR(dfbres, "MMSFBSurface::flip() failed");
		}
		this->vodesc.rect.w = this->vodesc.windsc.width;
		this->vodesc.rect.h = (int)((double)(this->vodesc.windsc.width) / this->vodesc.ratio + 0.5);
#endif
	}

	//this->vodesc.winsurface
	/* clear surface */

	if (mmsfb->getBackend() != MMSFB_BE_DFB) {
		if(window) {
			this->userd.surf=window->getSurface();
			this->userd.surf->setBlittingFlags(MMSFB_BLIT_ANTIALIASING);
			this->userd.surf->getPixelFormat(&this->userd.surf_pixelformat);
			int w,h;
			this->userd.surf->getSize(&w,&h);
			this->userd.size.x=0;
			this->userd.size.y=0;
			this->userd.size.w=w;
			this->userd.size.h=h;
			this->userd.lastaspect=0.0;
			this->userd.interim = NULL;
			this->userd.overlayInterim = NULL;
			this->userd.numOverlays = 0;
			this->userd.overlays = NULL;
		}
		DEBUGMSG("MMSMedia", "opening video driver...");
		/* open the video output driver */
		if (!(this->vo = xine_open_video_driver(this->xine, "raw",
									XINE_VISUAL_TYPE_RAW, (void*) &this->rawvisual)))
			throw MMSAVError(0, "Cannot open the XINE RAW video driver");
	}
	else {
#ifdef __HAVE_DIRECTFB__
            if(window) {
				if(((IDirectFBSurface *)vodesc.winsurface->getDFBSurface())->SetBlittingFlags((IDirectFBSurface *)vodesc.winsurface->getDFBSurface(), DSBLIT_NOFX) != DFB_OK)
					DEBUGMSG("MMSMedia", "set blitting failed");
				/* fill the visual structure for the video output driver */
				this->visual.destination  = (IDirectFBSurface *)vodesc.winsurface->getDFBSurface();
            }
            else
            	this->visual.destination = NULL;
            this->visual.subpicture   = NULL;
            this->visual.output_cb    = dfb_output_cb;
            this->visual.output_cdata = (void*) &(this->vodesc);
            this->visual.frame_cb     = dfb_frame_cb;
            this->visual.frame_cdata  = (void*) &(this->vodesc);
            //this->visual.destination->SetField( this->visual.destination, 0 );
            DEBUGMSG("MMSMedia", "opening video driver...");
            if (!(this->vo = xine_open_video_driver(this->xine, "DFB",
                                        XINE_VISUAL_TYPE_DFB, (void*) &this->visual)))
                        throw MMSAVError(0, "Cannot open the DFB video driver, please install directfb extras!");
#endif
	}
	DEBUGMSG("MMSMedia", "opening video driver done.");


    /* open the audio output driver */
    const char* const *ao_list;
    int i = 0;
    if(!(ao_list = xine_list_audio_output_plugins(this->xine)) || !*ao_list) {
        DEBUGMSG("MMSMedia", "No audio output plugins found");
        xine_engine_set_param(this->xine, XINE_PARAM_IGNORE_AUDIO, 1);
        this->ao=NULL;
        return;
    }
    do {
        DEBUGMSG("MMSMedia", "checking audio output '%s'...", ao_list[i]);

    	/* ignore file output */
        if(strcmp(ao_list[i], "file") == 0) {
        	i++;
        	continue;
        }
        else if(strcmp(ao_list[i], "none") == 0)
        {
            /* disable audio */
            xine_engine_set_param(this->xine, XINE_PARAM_IGNORE_AUDIO, 1);
            DEBUGMSG("MMSMedia", "Could not open audio driver, sound disabled!");
            break;
        }

        DEBUGMSG("MMSMedia", "opening audio output '%s'", ao_list[i]);
    }
    while(!(this->ao = xine_open_audio_driver(this->xine, ao_list[i++], NULL)));

    DEBUGMSG("MMSMedia", "Using audio driver '%s'", ao_list[i-1]);

#endif
    }
}



/**
 * Constructor
 *
 * Initializes private variables
 *
 * @note	It checks the chosen backend against the
 * 			supported ones. It simply switches the
 * 			support, because one of the backends has
 * 			to be implemented, otherwise mmsmedia
 * 			isn't build.
 */
MMSAV::MMSAV(MMSMEDIABackend _backend) :
	backend(_backend),
	window(NULL),
	surface(NULL),
	verbose(false),
	status(STATUS_NONE),
	pos(0)
#ifdef __HAVE_XINE__
	, xine(NULL)
	, vo(NULL)
	, ao(NULL)
	, stream(NULL)
	, queue(NULL)
#endif
{

	this->onError          = new sigc::signal<void, string>;
    this->onStatusChange   = new sigc::signal<void, const unsigned short, const unsigned short>;

    switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			gst_diskovideosink_data.uri = "";
			gst_diskovideosink_data.pipeline = NULL;
			break;
#else
			cerr << "MMSAV: Disko was build without gstreamer support. Switching to xine." << endl;
			this->backend = MMSMEDIA_BE_XINE;
#endif
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			pthread_mutex_init(&this->lock, NULL);
#else
			cerr << "MMSAV: Disko was build without xine support. Switching to gstreamer." << endl;
			this->backend = MMSMEDIA_BE_GST;
#endif
		default:
			// shouldn't be reached
			break;
    }
}

/**
 * Destructor
 *
 * Deletes sigc++-callbacks and closes all xine related
 * stuff.
 */
MMSAV::~MMSAV() {
	onHandleInputConnection.disconnect();

	if(this->onError) {
		this->onError->clear();
		delete this->onError;
	}

	if(this->onStatusChange) {
		this->onStatusChange->clear();
		delete this->onStatusChange;
	}

	if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
		mmsGstFree();
#endif
    }
    else {
#ifdef __HAVE_XINE__
		pthread_mutex_destroy(&this->lock);


		if(this->queue)
			xine_event_dispose_queue(this->queue);
		if(this->stream)
			xine_dispose(this->stream);
		if(this->ao)
			xine_close_audio_driver(this->xine, this->ao);
		if(this->vo)
			xine_close_video_driver(this->xine, this->vo);

		// dispose all registered post plugins
		map<string, xine_post_t*>::const_iterator i;
		for(i = audioPostPlugins.begin(); i != audioPostPlugins.end(); ++i)
			xine_post_dispose(this->xine, i->second);
		audioPostPlugins.erase(audioPostPlugins.begin(), audioPostPlugins.end());

		for(i = videoPostPlugins.begin(); i != videoPostPlugins.end(); ++i)
			xine_post_dispose(this->xine, i->second);
		videoPostPlugins.erase(videoPostPlugins.begin(), videoPostPlugins.end());

		// exit xine
		xine_exit(this->xine);

		if (this->userd.interim) {
			// delete interim (used for xine raw callback)
			delete this->userd.interim;
		}

		if (this->userd.overlayInterim) {
			// delete overlay interim (used for xine raw callback)
			delete this->userd.overlayInterim;
		}
#endif
    }
}

#ifdef __HAVE_XINE__



/**
 * Opens an audio/video object.
 *
 * It creates the xine stream, wires all registered
 * audio/video post plugins, sets the verbosity and
 * registers the event queue if given.
 *
 * @note    You have to register post plugins before
 * calling open().
 *
 * @param   queue_cb    [in]    xine event queue callback
 * @param	userData	[in]	data to be used in xine event callbacks
 *
 * @see MMSAV::registerAudioPostPlugin
 * @see MMSAV::registerVideoPostPlugin
 *
 * @exception   MMSAVError  Cannot get a new stream
 */
void MMSAV::xineOpen(xine_event_listener_cb_t queue_cb, void *userData) {
	if(this->stream) {
		DEBUGMSG("MMSAV", "xine stream already present, skipping xineOpen");
		return;
	}

    /* open stream */
    if (!(this->stream = xine_stream_new(this->xine, this->ao, this->vo)))
        throw MMSAVError(0, "Cannot get a new stream");

    /* wire all post plugins */
    for(map<string, xine_post_t*>::const_iterator i = videoPostPlugins.begin(); i != videoPostPlugins.end(); ++i)
        xine_post_wire_video_port(xine_get_video_source(stream), i->second->video_input[0]);
    for(map<string, xine_post_t*>::const_iterator i = audioPostPlugins.begin(); i != audioPostPlugins.end(); ++i)
        xine_post_wire_audio_port(xine_get_audio_source(stream), i->second->audio_input[0]);

    /* set verbosity */
    if(this->verbose)
        xine_set_param(this->stream, XINE_PARAM_VERBOSITY, XINE_VERBOSITY_DEBUG);
    else
        xine_set_param(this->stream, XINE_PARAM_VERBOSITY, XINE_VERBOSITY_NONE);

    /*if(this->vo)
         xine_set_param(this->stream, XINE_PARAM_VO_DEINTERLACE, false);
*/
    if(this->ao) {
        xine_set_param(this->stream, XINE_PARAM_AUDIO_MUTE, false);
        xine_set_param(this->stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, -1);
    }

    /* create event listener thread */
    if(queue_cb) {
        this->queue = xine_event_new_queue(this->stream);
        if(this->queue)
        	xine_event_create_listener_thread(this->queue, queue_cb, userData);
        else
        	DEBUGMSG("MMSMedia", "Could not create event listener");
    }
}

#endif

/**
 * Registers a xine audio post plugin.
 *
 * Post plugin will be initialized.
 *
 * @param   name    [in]    name of the post plugin
 *
 * @return  true if plugin could be initialized correctly
 */
bool MMSAV::registerAudioPostPlugin(string name) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	return true;
#endif
    }
    else {
#ifdef __HAVE_XINE__
		xine_post_t *p;

		if(!(p = xine_post_init(this->xine, name.c_str(), 1, &this->ao, NULL)))
			DEBUGMSG("MMSMedia", "Could not initialize audio post plugin %s", name.c_str());
		else {
			audioPostPlugins[name] = p;
			return true;
		}

		return false;
#endif
    }

    throw MMSAVError(0, "MMSAV::registerAudioPostPlugin() called but media backend does not match supported backends");
}

/**
 * Registers a xine video post plugin.
 *
 * Post plugin will be initialized.
 *
 * @param   name    [in]    name of the post plugin
 *
 * @return  true if plugin could be initialized correctly
 */
bool MMSAV::registerVideoPostPlugin(string name) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	return true;
#endif
    }
    else {
#ifdef __HAVE_XINE__
		xine_post_t *p;

		if(!(p = xine_post_init(this->xine, name.c_str(), 1, NULL, &this->vo)))
			DEBUGMSG("MMSMedia", "Could not initialize video post plugin %s", name.c_str());
		else {
			videoPostPlugins[name] = p;
			return true;
		}

		return false;
#endif
    }

    throw MMSAVError(0, "MMSAV::registerVideoPostPlugin() called but media backend does not match supported backends");
}

#ifdef __HAVE_XINE__
/**
 * Sets post plugin parameter.
 *
 * @param   plugins     [in]    map of post plugins
 * @param   name        [in]    name of post plugin that will be affected
 * @param   parameter   [in]    parameter to set
 * @param   value       [in]    value for the parameter
 *
 * @return  true if the parameter could be set
 */
bool MMSAV::setPostPluginParameter(map<string, xine_post_t*> plugins, string name, string parameter, string value) {
    xine_post_in_t              *postIn;
    xine_post_api_t             *postApi;
    xine_post_api_descr_t       *postApiDesc;
    xine_post_api_parameter_t   *postApiParam;
    char                        *data;

    // search for plugin
    if(!(postIn = (xine_post_in_t *)xine_post_input(plugins[name], "parameters"))) {
        DEBUGMSG("MMSMedia", "Could not set parameter for post plugin %s: Plugin not registered", name.c_str());
        return false;
    }

    postApi      = (xine_post_api_t *)postIn->data;
    postApiDesc  = postApi->get_param_descr();
    postApiParam = postApiDesc->parameter;
    data         = new char[postApiDesc->struct_size];
    postApi->get_parameters(plugins[name], (void*)data);

    while(postApiParam->type != POST_PARAM_TYPE_LAST)
    {
        if(strToUpr(string(postApiParam->name)) == strToUpr(parameter)) {
            if(postApiParam->type == POST_PARAM_TYPE_INT) {
                int iValue = atoi(value.c_str());

                // check if the name of an enumeration is used
                if(iValue == 0 && value.at(0) != '0') {
                    for(int i = 0; postApiParam->enum_values[i]; i++) {
                        if(value == postApiParam->enum_values[i]) {
                            iValue = i;
                            break;
                        }
                    }
                } else {
                    // check if value is out of range
                    if(iValue < postApiParam->range_min || iValue > postApiParam->range_max) {
                        DEBUGMSG("MMSMedia", "Could not set %s's %s to %s: Out of range", name.c_str(), parameter.c_str(), value.c_str());
                        return false;
                    }
                }

                // set value
                *(int *)(data + postApiParam->offset) = iValue;
                DEBUGMSG("MMSMedia", "%s: %s = %s", name.c_str(), parameter.c_str(), postApiParam->enum_values[*(int *)(data + postApiParam->offset)]);
            }
            else if(postApiParam->type == POST_PARAM_TYPE_DOUBLE) {
                double dValue = atof(value.c_str());

                // check if value is out of range
                if(dValue < postApiParam->range_min || dValue > postApiParam->range_max) {
                    DEBUGMSG("MMSMedia", "Could not set %s's %s to %s: Out of range", name.c_str(), parameter.c_str(), value.c_str());
                    return false;
                }

                // set value
               *(double *)(data + postApiParam->offset) = dValue;
               DEBUGMSG("MMSMedia", "%s: %s = %s", name.c_str(), parameter.c_str(), value.c_str());
            }
            else if(postApiParam->type == POST_PARAM_TYPE_BOOL) {
                bool bValue = false;
                if(value == "1" || strToUpr(value) == "TRUE")
                    bValue = true;
               *(bool *)(data + postApiParam->offset) = bValue;
               DEBUGMSG("MMSMedia", "%s: = %s", name.c_str(), parameter.c_str(), (bValue ? "true" : "false"));
            }
            else if(postApiParam->type == POST_PARAM_TYPE_CHAR) {
                char cValue = value.at(0);
               *(char *)(data + postApiParam->offset) = cValue;
               DEBUGMSG("MMSMedia", "%s: %s = %c", name.c_str(), parameter.c_str(), cValue);
            }
            else if(postApiParam->type == POST_PARAM_TYPE_STRING) {
                char *sValue = (char*)value.c_str();
               *(char **)(data + postApiParam->offset) = sValue;
               DEBUGMSG("MMSMedia", "%s: %s = %s", name.c_str(), parameter.c_str(), sValue);
            }
            break;
        }
        postApiParam++;
    }

    if(!postApi->set_parameters(plugins[name], (void *)data))
        DEBUGMSG("MMSMedia", "Error setting post plugin parameter");

    delete[] data;

    return true;
}
#endif

/**
 * Sets audio post plugin parameter.
 *
 * @param   name        [in]    name of post plugin that will be affected
 * @param   parameter   [in]    parameter to set
 * @param   value       [in]    value for the parameter
 *
 * @return  true if the parameter could be set
 *
 * @see     MMSAV::setPostPluginParameter
 * @see     MMSAV::setVideoPostPluginParameter
 */
bool MMSAV::setAudioPostPluginParameter(string name, string parameter, string value) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	return true;
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	return setPostPluginParameter(this->audioPostPlugins, name, parameter, value);
#endif
    }

    throw MMSAVError(0, "MMSAV::setAudioPostPluginParameter() called but media backend does not match supported backends");
}

/**
 * Sets video post plugin parameter.
 *
 * @param   name        [in]    name of post plugin that will be affected
 * @param   parameter   [in]    parameter to set
 * @param   value       [in]    value for the parameter
 *
 * @return  true if the parameter could be set
 *
 * @see     MMSAV::setPostPluginParameter
 * @see     MMSAV::setAudioPostPluginParameter
 */
bool MMSAV::setVideoPostPluginParameter(string name, string parameter, string value) {
    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__
    	return true;
#endif
    }
    else {
#ifdef __HAVE_XINE__
    	return setPostPluginParameter(this->videoPostPlugins, name, parameter, value);
#endif
    }

    throw MMSAVError(0, "MMSAV::setVideoPostPluginParameter() called but media backend does not match supported backends");
}



/**
 * Sets internal status of sound/video playback.
 *
 * It also emits a signal, which can be handled using the
 * sigc++ connectors.
 *
 * @param   status  [in]    status to set
 *
 * @see MMSAV::onStatusChange
 */
void MMSAV::setStatus(int status) {
    switch(status) {
        case STATUS_PLAYING :
            if(this->status != this->STATUS_NONE)
                this->onStatusChange->emit(this->status, status);
            this->status = status;
            return;
        case STATUS_PAUSED  :
        case STATUS_STOPPED :
        case STATUS_FFWD    :
        case STATUS_FFWD2   :
        case STATUS_SLOW    :
        case STATUS_SLOW2   :
            this->onStatusChange->emit(this->status, status);
            this->status = status;
            return;
        default:
            break;
    }

    this->onStatusChange->emit(status, status);
}

/**
 * Determines if a stream is currently being played.
 *
 * @return true if stream is being played
 */
bool MMSAV::isPlaying() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
	    	return (this->status == STATUS_PLAYING);
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->status == STATUS_PLAYING) {
				if(xine_get_status(this->stream)!=XINE_STATUS_PLAY) {
					this->setStatus(STATUS_STOPPED);
					return false;
				}
				else return true;
			}
			return false;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::isPlaying() called but media backend does not match supported backends");
}

/**
 * Determines if a stream is currently being paused.
 *
 * @return true if stream is being paused
 */
bool MMSAV::isPaused() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return false;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->status == STATUS_PAUSED) {
				if(xine_get_status(this->stream)!=XINE_STATUS_PLAY) {
					this->setStatus(STATUS_STOPPED);
					return false;
				}
				else return true;
			}
			return false;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::isPaused() called but media backend does not match supported backends");
}

/**
 * Determines if a stream is in stopped status.
 *
 * @return true if stream is being stopped
 */
bool MMSAV::isStopped() {
	return (this->status == STATUS_STOPPED);
}

/**
 * Starts playing.
 *
 * If the continue flag is set it tries to continue
 * at the position where it was stopped before.
 *
 * @param   mrl     [in]    mrl to play
 * @param   cont    [in]    if true it tries to continue at a position stopped before
 *
 * @exception   MMSAVError stream could not be opened
 */
void MMSAV::startPlaying(const string mrl, const bool cont) {
	DEBUGMSG("MMSAV", "currentMRL: %s mrl: %s status: %d", currentMRL.c_str(), mrl.c_str(), status);
	if((currentMRL == mrl) && (this->status == this->STATUS_PLAYING)) return;
	this->currentMRL = mrl;

    if (this->backend == MMSMEDIA_BE_GST) {
#ifdef __HAVE_GSTREAMER__


//TODO:new version of MMSAV
//TODO:version with playbin without disko video sink and without GST://

    	// disable VIDEO LAYER - only for test
    	MMSFBLayer *vl = mmsfbmanager.getVideoLayer();
    	if (vl != mmsfbmanager.getGraphicsLayer())
    		vl->releaseLayer();



    	// init gst pipe
    	this->gst_diskovideosink_data.pipeline = mmsGstInit(currentMRL, this->window->getSurface());
    	if (!this->gst_diskovideosink_data.pipeline)
    		return;
    	this->gst_diskovideosink_data.uri = currentMRL;

    	// play the pipe
    	pthread_t thread;
		if(pthread_create(&thread, NULL, (void* (*)(void*))gstPlayRoutine, &this->gst_diskovideosink_data) == 0)
			pthread_detach(thread);

#endif
    }
    else {
#ifdef __HAVE_XINE__

		if(!this->stream) this->xineOpen();

		if(!cont) this->pos = 0;

		/* start playing in extra thread to avoid blocking the application */
		//pthread_t thread;
		internalStreamData *streamData = new internalStreamData;
		streamData->stream = this->stream;
		streamData->pos    = this->pos;
		streamData->status = &(this->status);
		streamData->mrl    = mrl.c_str();
		streamData->lock   = &(this->lock);
		/*if(pthread_create(&thread, NULL, xinePlayRoutine, streamData) == 0)
			pthread_detach(thread);
		else*/
			xinePlayRoutine(streamData);

#endif
    }
}

/**
 * Continues playing.
 *
 * It only works if playing was stopped or speed
 * changed to other than normal.
 */
void MMSAV::play() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(!this->stream) return;

			if(this->status == this->STATUS_PAUSED  ||
			   this->status == this->STATUS_SLOW    ||
			   this->status == this->STATUS_SLOW2   ||
			   this->status == this->STATUS_FFWD    ||
			   this->status == this->STATUS_FFWD2) {
			   this->setStatus(this->STATUS_PLAYING);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::play() called but media backend does not match supported backends");
}

/**
 * Stops playing.
 *
 * @param	savePosition	[in]	if true stream position will be saved for continuation
 *
 * It saves the position, so if you call MMSAV::play()
 * afterwards with the continue flag set, it will continue
 * at this position.
 */
void MMSAV::stop(const bool savePosition) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE: {
#ifdef __HAVE_XINE__
			if(!this->stream) return;

			/* save position */
			if(savePosition)
				xine_get_pos_length(this->stream, &this->pos, NULL, NULL);

			/* stop xine in extra thread to avoid blocking the application */
			//pthread_t thread;
			internalStreamData *streamData = new internalStreamData;
			streamData->stream = this->stream;
			streamData->status = &(this->status);
			streamData->lock   = &(this->lock);
			/*if(pthread_create(&thread, NULL, stopRoutine, streamData) == 0)
				pthread_detach(thread);
			else*/
				stopRoutine(streamData);
			return;
#endif
			break;
		}
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::stop() called but media backend does not match supported backends");
}

/**
 * Pauses.
 *
 * It only works if stream is playing (can be slow,
 * ffwd etc.).
 */
void MMSAV::pause() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->status == this->STATUS_PLAYING ||
			   this->status == this->STATUS_SLOW    ||
			   this->status == this->STATUS_SLOW2   ||
			   this->status == this->STATUS_FFWD    ||
			   this->status == this->STATUS_FFWD2) {
			   this->setStatus(this->STATUS_PAUSED);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
				xine_set_param(this->stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::pause() called but media backend does not match supported backends");
}

/**
 * Playback will be switched to slow motion.
 *
 * There are two different speed settings for slow motion.
 * Twice as slow and four times as slow.
 *
 * @see MMSAV::ffwd()
 * @see MMSAV::rewind()
 */
void MMSAV::slow() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->status == this->STATUS_PLAYING || this->status == this->STATUS_PAUSED) {
				this->setStatus(this->STATUS_SLOW);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_SLOW_2);
			}
			else if(this->status == this->STATUS_SLOW) {
				this->setStatus(this->STATUS_SLOW2);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_SLOW_4);
			}
			else if(this->status == this->STATUS_FFWD) {
				this->setStatus(this->STATUS_PLAYING);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
			}
			else if(this->status == this->STATUS_FFWD2) {
				this->setStatus(this->STATUS_FFWD);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_FAST_2);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::slow() called but media backend does not match supported backends");
}

/**
 * Playback will be switched to fast forward.
 *
 * There are two different speed settings for fast forward.
 * Twice as fast and four times as fast.
 *
 * @see MMSAV::slow()
 * @see MMSAV::rewind()
 */
void MMSAV::ffwd() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__


	    	printf("ffwd-------------------------------------\n");


/*
static void
seek_to_time (GstElement *pipeline,
	      gint64      time_nanoseconds)
{
  if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET, time_nanoseconds,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
    g_print ("Seek failed!\n");
  }
}

 */
/*
	    	gst_element_seek(this->gst_diskovideosink_data.player,
	    			1.0,
	    			GST_FORMAT_TIME,
	    			GST_SEEK_FLAG_FLUSH,
	    			GST_SEEK_TYPE_SET,
	    			1000*1000*1000,
	    			GST_SEEK_TYPE_NONE,
	    			GST_CLOCK_TIME_NONE);*/
	    			         /*
	    	                                                         gdouble rate,
	    	                                                         GstFormat format,
	    	                                                         GstSeekFlags flags,
	    	                                                         GstSeekType cur_type,
	    	                                                         gint64 cur,
	    	                                                         GstSeekType stop_type,
	    	                                                         gint64 stop);*/

			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->status == this->STATUS_PLAYING || this->status == this->STATUS_PAUSED) {
				this->setStatus(this->STATUS_FFWD);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_FAST_2);
			}
			else if(this->status == this->STATUS_FFWD) {
				this->setStatus(this->STATUS_FFWD2);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_FAST_4);
			}
			else if(this->status == this->STATUS_SLOW) {
				this->setStatus(this->STATUS_PLAYING);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
			}
			else if(this->status == this->STATUS_SLOW2) {
				this->setStatus(this->STATUS_SLOW);
				xine_set_param(this->stream, XINE_PARAM_SPEED, XINE_SPEED_SLOW_2);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::ffwd() called but media backend does not match supported backends");
}

/**
 * Gets information about the length of the actual title
 * and the time of the current position in seconds.
 *
 * @param   pos     [out]   time in seconds of current position
 * @param   length  [out]   time in seconds of title length
 */
bool MMSAV::getTimes(int *pos, int *length) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return 0;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(!this->stream || !xine_get_pos_length(this->stream, NULL, pos, length)) return false;

			if(pos)    (*pos)    /= 1000;
			if(length) (*length) /= 1000;
			return true;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::getTimes() called but media backend does not match supported backends");
}

/**
 * Sets the brightness if video output is done.
 *
 * @param   count   [in]    amount of brightness
 *
 * @see     MMSAV::brightnessUp()
 * @see     MMSAV::brightnessDown()
 */
void MMSAV::setBrightness(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo)
				xine_set_param(this->stream, XINE_PARAM_VO_BRIGHTNESS, count);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::setBrightness() called but media backend does not match supported backends");
}

/**
 * Increases the brightness if video output is done.
 *
 * @param   count   [in]    amount of brightness to increase
 *
 * @see     MMSAV::setBrightness()
 * @see     MMSAV::brightnessDown()
 */
void MMSAV::brightnessUp(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_BRIGHTNESS);
				xine_set_param(this->stream, XINE_PARAM_VO_BRIGHTNESS, value + count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::brightnessUp() called but media backend does not match supported backends");
}

/**
 * Decreases the brightness if video output is done.
 *
 * @param   count   [in]    amount of brightness to decrease
 *
 * @see     MMSAV::setBrightness()
 * @see     MMSAV::brightnessUp()
 */
void MMSAV::brightnessDown(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_BRIGHTNESS);
				xine_set_param(this->stream, XINE_PARAM_VO_BRIGHTNESS, value - count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::brightnessDown() called but media backend does not match supported backends");
}

/**
 * Sets the contrast if video output is done.
 *
 * @param   count   [in]    amount of contrast
 *
 * @see     MMSAV::contrastUp()
 * @see     MMSAV::contrastDown()
 */
void MMSAV::setContrast(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo)
				xine_set_param(this->stream, XINE_PARAM_VO_CONTRAST, count);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::setContrast() called but media backend does not match supported backends");
}

/**
 * Increases the contrast if video output is done.
 *
 * @param   count   [in]    amount of contrast to increase
 *
 * @see     MMSAV::setContrast()
 * @see     MMSAV::contrastDown()
 */
void MMSAV::contrastUp(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_CONTRAST);
				xine_set_param(this->stream, XINE_PARAM_VO_CONTRAST, value + count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::contrastUp() called but media backend does not match supported backends");
}

/**
 * Decreases the contrast if video output is done.
 *
 * @param   count   [in]    amount of contrast to decrease
 *
 * @see     MMSAV::setContrast()
 * @see     MMSAV::contrastUp()
 */
void MMSAV::contrastDown(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_CONTRAST);
				xine_set_param(this->stream, XINE_PARAM_VO_CONTRAST, value - count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::contrastDown() called but media backend does not match supported backends");
}

/**
 * Sets the saturation if video output is done.
 *
 * @param   count   [in]    amount of saturation
 *
 * @see     MMSAV::saturationUp()
 * @see     MMSAV::saturationDown()
 */
void MMSAV::setSaturation(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo)
				xine_set_param(this->stream, XINE_PARAM_VO_SATURATION, count);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::setSaturation() called but media backend does not match supported backends");
}

/**
 * Increases the saturation if video output is done.
 *
 * @param   count   [in]    amount of saturation to increase
 *
 * @see     MMSAV::setSaturation()
 * @see     MMSAV::saturationDown()
 */
void MMSAV::saturationUp(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_SATURATION);
				xine_set_param(this->stream, XINE_PARAM_VO_SATURATION, value + count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::saturationUp() called but media backend does not match supported backends");
}

/**
 * Decreases the saturation if video output is done.
 *
 * @param   count   [in]    amount of saturation to decrease
 *
 * @see     MMSAV::setSaturation()
 * @see     MMSAV::saturationUp()
 */
void MMSAV::saturationDown(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_SATURATION);
				xine_set_param(this->stream, XINE_PARAM_VO_SATURATION, value - count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::saturationDown() called but media backend does not match supported backends");
}

/**
 * Sets the hue if video output is done.
 *
 * @param   count   [in]    amount of hue
 *
 * @see     MMSAV::hueUp()
 * @see     MMSAV::hueDown()
 */
void MMSAV::setHue(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo)
				xine_set_param(this->stream, XINE_PARAM_VO_HUE, count);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::setHue() called but media backend does not match supported backends");
}

/**
 * Increases the hue if video output is done.
 *
 * @param   count   [in]    amount of hue to increase
 *
 * @see     MMSAV::setHue()
 * @see     MMSAV::hueDown()
 */
void MMSAV::hueUp(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_HUE);
				xine_set_param(this->stream, XINE_PARAM_VO_HUE, value + count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0,"MMSAV::hueUp() called but media backend does not match supported backends");
}

/**
 * Decreases the hue if video output is done.
 *
 * @param   count   [in]    amount of hue to decrease
 *
 * @see     MMSAV::setHue()
 * @see     MMSAV::hueUp()
 */
void MMSAV::hueDown(int count) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->vo) {
				int value = xine_get_param(this->stream, XINE_PARAM_VO_HUE);
				xine_set_param(this->stream, XINE_PARAM_VO_HUE, value - count*500);
			}
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::hueDown() called but media backend does not match supported backends");
}

/**
 * Sets the volume of the audio output.
 *
 * @param   percent	[in]    volume in percent
 */
void MMSAV::setVolume(int percent) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			if(this->ao)
				xine_set_param(this->stream, XINE_PARAM_AUDIO_VOLUME, percent);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::setVolume() called but media backend does not match supported backends");
}

/**
 * Send a xine event to the engine
 *
 * @param   type    [in]    type of event
 * @param   data    [in]    event specific data
 * @param   datalen [in]    length of data
 */
void MMSAV::sendEvent(int type, void *data, int datalen) {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
			xine_event_t evt;

			evt.stream      = this->stream;
			evt.data        = data;
			evt.data_length = datalen;
			evt.type        = type;
			xine_event_send(this->stream, &evt);
			return;
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::sendEvent() called but media backend does not match supported backends");
}


bool MMSAV::sendKeyPress(MMSKeySymbol key) {
	switch(this->backend) {
	case MMSMEDIA_BE_GST: {
#ifdef __HAVE_GSTREAMER__
		return mmsGstSendKeyPress(gst_diskovideosink_data.pipeline, key);
#endif
		break;
	}
	case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		return false;
#endif
		break;
	default:
		// shouldn't be reached
		break;
	}

	throw MMSAVError(0, "MMSAV::sendKeyPress() called but media backend does not match supported backends");

	return false;
}

bool MMSAV::sendKeyRelease(MMSKeySymbol key) {
	switch(this->backend) {
	case MMSMEDIA_BE_GST: {
#ifdef __HAVE_GSTREAMER__
		return mmsGstSendKeyRelease(gst_diskovideosink_data.pipeline, key);
#endif
		break;
	}
	case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		return false;
#endif
		break;
	default:
		// shouldn't be reached
		break;
	}

	throw MMSAVError(0, "MMSAV::sendKeyRelease() called but media backend does not match supported backends");

	return false;
}

bool MMSAV::sendButtonPress(int posx, int posy) {
	switch(this->backend) {
	case MMSMEDIA_BE_GST: {
#ifdef __HAVE_GSTREAMER__
		return mmsGstSendButtonPress(gst_diskovideosink_data.pipeline, posx, posy);
#endif
		break;
	}
	case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		return false;
#endif
		break;
	default:
		// shouldn't be reached
		break;
	}

	throw MMSAVError(0, "MMSAV::sendButtonPress() called but media backend does not match supported backends");

	return false;
}

bool MMSAV::sendButtonRelease(int posx, int posy) {
	switch(this->backend) {
	case MMSMEDIA_BE_GST: {
#ifdef __HAVE_GSTREAMER__
		return mmsGstSendButtonRelease(gst_diskovideosink_data.pipeline, posx, posy);
#endif
		break;
	}
	case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		return false;
#endif
		break;
	default:
		// shouldn't be reached
		break;
	}

	throw MMSAVError(0, "MMSAV::sendButtonRelease() called but media backend does not match supported backends");

	return false;
}

bool MMSAV::sendAxisMotion(int posx, int posy) {
	switch(this->backend) {
	case MMSMEDIA_BE_GST: {
#ifdef __HAVE_GSTREAMER__
		return mmsGstSendAxisMotion(gst_diskovideosink_data.pipeline, posx, posy);
#endif
		break;
	}
	case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
		return false;
#endif
		break;
	default:
		// shouldn't be reached
		break;
	}

	throw MMSAVError(0, "MMSAV::sendAxisMotion() called but media backend does not match supported backends");

	return false;
}

bool MMSAV::sendEvent(MMSInputEvent *input) {
	// fire input events to the media backend
	switch (input->type) {
	case MMSINPUTEVENTTYPE_KEYPRESS:
		return sendKeyPress(input->key);
	case MMSINPUTEVENTTYPE_KEYRELEASE:
		return sendKeyRelease(input->key);
	case MMSINPUTEVENTTYPE_BUTTONPRESS:
		return sendButtonPress(input->posx, input->posy);
	case MMSINPUTEVENTTYPE_BUTTONRELEASE:
		return sendButtonRelease(input->posx, input->posy);
	case MMSINPUTEVENTTYPE_AXISMOTION:
		return sendAxisMotion(input->posx, input->posy);
	default:
		return false;
		break;
	}
}

/**
 * Returns true if stream contains a video stream.
 *
 * @return true if video stream
 */
bool MMSAV::hasVideo() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return true;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
	    	return (xine_get_stream_info(this->stream, XINE_STREAM_INFO_HAS_VIDEO) == 1);
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::hasVideo() called but media backend does not match supported backends");
}

/**
 * Returns true if stream contains an audio stream.
 *
 * @return true if audio stream
 */
bool MMSAV::hasAudio() {
	switch(this->backend) {
		case MMSMEDIA_BE_GST:
#ifdef __HAVE_GSTREAMER__
			return false;
#endif
			break;
		case MMSMEDIA_BE_XINE:
#ifdef __HAVE_XINE__
	    	return (xine_get_stream_info(this->stream, XINE_STREAM_INFO_HAS_AUDIO) == 1);
#endif
			break;
		default:
			// shouldn't be reached
			break;
	}

	throw MMSAVError(0, "MMSAV::hasAudio() called but media backend does not match supported backends");
}
