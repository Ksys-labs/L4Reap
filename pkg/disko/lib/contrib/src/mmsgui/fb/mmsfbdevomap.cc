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

#ifdef __HAVE_FBDEV__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "mmsgui/fb/mmsfbdevomap.h"
#include <sys/ioctl.h>
#include <cstring>
#include "mmsgui/fb/omapfb.h"


#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBDevOmap is not initialized");return false;}

MMSFBDevOmap::MMSFBDevOmap() {
	this->osd0.fbdev	= NULL;
	this->vid.fbdev		= NULL;
	this->osd1.fbdev	= NULL;
	this->primary		= NULL;
}

MMSFBDevOmap::~MMSFBDevOmap() {
	closeDevice();
}

bool MMSFBDevOmap::openDevice(int id) {
	char dev[100];
	sprintf(dev, "/dev/fb%d", id);

	if (id < 0 || id > 2) {
		printf("MMSFBDevOmap: unknown device %s\n", dev);
		return false;
	}

	// new device
	MMSFBDev *fbdev = new MMSFBDev();

	if ((fbdev) && (!fbdev->openDevice(dev, (!this->osd0.fbdev && !this->vid.fbdev && !this->osd1.fbdev) ? this->console : MMSFBDEV_NO_CONSOLE))) {
		// delete uninitialized fbdev object
		delete fbdev;
		return false;
	}

	if ((fbdev) && (memcmp(fbdev->fix_screeninfo.id, "omapfb", 6) == 0)) {

		fbdev->onGenFBPixelFormat.connect(sigc::mem_fun(this,&MMSFBDevOmap::onGenFBPixelFormatDev));
		fbdev->onDisable.connect(sigc::mem_fun(this,&MMSFBDevOmap::onDisableDev));
		fbdev->onActivate.connect(sigc::mem_fun(this,&MMSFBDevOmap::onActivateDev));

		switch (id) {
		case 0:
			this->osd0.fbdev = fbdev;
			strcpy(this->osd0.device, dev);
			this->osd0.width = 0;
			this->primary = &this->osd0;
			if (this->console != MMSFBDEV_NO_CONSOLE) {
				// disable device
				this->osd0.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			}
			break;
		case 1:
			this->vid.fbdev = fbdev;
			strcpy(this->vid.device, dev);
			this->vid.width = 0;
			if (!this->primary)
				this->primary = &this->vid;
			// disable device
			this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			break;
		case 2:
			this->osd1.fbdev = fbdev;
			strcpy(this->osd1.device, dev);
			//this->osd1.width = -1;
			this->primary = &this->osd1;
			// disable device
			this->osd1.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			break;
		}
	}
	else {
		// not supported
		if (fbdev) {
			printf("MMSFBDevOmap: unsupported accelerator %d (%.16s)\n", fbdev->fix_screeninfo.accel, fbdev->fix_screeninfo.id);
			delete fbdev;
		}
		return false;
	}

	return true;
}

bool MMSFBDevOmap::openDevice(char *device_file, int console) {
	// close the device if opened
	closeDevice();

	// we do not initialize the devices here, but dynamically within testLayer() / initLayer()
	this->console = console;
	this->isinitialized = true;
	return true;
}

void MMSFBDevOmap::closeDevice() {
	// close frame buffers
	if (this->osd1.fbdev) {
		delete this->osd1.fbdev;
		this->osd1.fbdev = NULL;
	}
	if (this->vid.fbdev) {
		delete this->vid.fbdev;
		this->vid.fbdev = NULL;
	}
	if (this->osd0.fbdev) {
		delete this->osd0.fbdev;
		this->osd0.fbdev = NULL;
	}
	this->primary = NULL;

	// reset all other
	this->isinitialized = false;
}

bool MMSFBDevOmap::waitForVSync() {
	// is initialized?
	INITCHECK;

	if (!this->primary)
		return false;

	if (!this->primary->fbdev)
		return false;

	static const int s = 0;
	if (ioctl(this->primary->fbdev->fd, OMAPFB_WAITFORVSYNC, &s)) {
		// failed, well then???
	}

	return true;
}

bool MMSFBDevOmap::panDisplay(int buffer_id, void *framebuffer_base) {
	// is initialized?
	INITCHECK;

	if (this->osd0.fbdev && framebuffer_base == this->osd0.fbdev->framebuffer_base) {
		// Graphic layer (OSD0)
		if (this->osd0.fbdev)
			return this->osd0.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}
	else
	if (this->vid.fbdev && framebuffer_base == this->vid.fbdev->framebuffer_base) {
		// Video layer (VID)
		if (this->vid.fbdev)
			return this->vid.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}
	else
	if (this->osd1.fbdev && framebuffer_base == this->osd1.fbdev->framebuffer_base) {
		// Graphic layer (OSD1)
		if (this->osd1.fbdev)
			return this->osd1.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}

	// check framebuffer_base pointer
	printf("MMSFBDevOmap: framebuffer base pointer not correct\n");
	return false;
}

bool MMSFBDevOmap::testLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
	    if (!this->osd0.fbdev) openDevice(0);
		if (!this->osd0.fbdev) {
			printf("MMSFBDevOmap: OSD Layer %d not initialized\n", layer_id);
			return false;
		}
		return true;
	case 1:
		// Video layer
	    if (!this->vid.fbdev) openDevice(1);
		if (!this->vid.fbdev) {
			printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
			return false;
		}
		return true;
	case 2:
		// OSD layer
	    if (!this->osd1.fbdev) openDevice(2);
		if (!this->osd1.fbdev) {
			printf("MMSFBDevOmap: OSD Layer %d not initialized\n", layer_id);
			return false;
		}
		return true;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}


bool MMSFBDevOmap::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	// is initialized?
	INITCHECK;

	if (!testLayer(layer_id)) {
		// layer not available
		return false;
	}

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		if   ((pixelformat != MMSFB_PF_ARGB)
			&&(pixelformat != MMSFB_PF_RGB32)
			&&(pixelformat != MMSFB_PF_RGB16)) {
			printf("MMSFBDevOmap: OSD Layer %d needs pixelformat ARGB, RGB32 or RGB16, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// enable OSD0
		if (this->osd0.fbdev->initLayer(0, width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->osd0.fbdev->layers[0].buffers, sizeof(this->osd0.fbdev->layers[0].buffers));

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: OSD Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

        	this->osd0.width = width;
        	this->osd0.height = height;
        	this->osd0.pixelformat = pixelformat;
        	this->osd0.backbuffer = backbuffer;

			return true;
		}
		return false;

	case 1:
		// Video layer (VID)
		if   (pixelformat != MMSFB_PF_I420) {
			printf("MMSFBDevOmap: Video Layer %d needs pixelformat I420 (==YUV420) but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// enable VID
		if (this->vid.fbdev->initLayer(0, width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->vid.fbdev->layers[0].buffers, sizeof(this->vid.fbdev->layers[0].buffers));

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: Video Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

        	this->vid.width = width;
        	this->vid.height = height;
        	this->vid.pixelformat = pixelformat;
        	this->vid.backbuffer = backbuffer;

			return true;
		}
		return false;

	case 2:
		// OSD layer (OSD1)
		if   ((pixelformat != MMSFB_PF_ARGB)
			&&(pixelformat != MMSFB_PF_RGB32)) {
			printf("MMSFBDevOmap: OSD Layer %d needs pixelformat ARGB or RGB32, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// enable OSD1
		if (this->osd1.fbdev->initLayer(0, width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->osd1.fbdev->layers[0].buffers, sizeof(this->osd1.fbdev->layers[0].buffers));

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: OSD Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

        	this->osd1.width = width;
        	this->osd1.height = height;
        	this->osd1.pixelformat = pixelformat;
        	this->osd1.backbuffer = backbuffer;

			return true;
		}
		return false;
	}

	return false;
}

bool MMSFBDevOmap::releaseLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevOmap: layer %d cannot be released\n", layer_id);
		return false;
	case 1:
		// Video layer (VID)
		if (this->vid.fbdev) {
			// disable
			this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			// close
			this->vid.fbdev->closeDevice();
			return true;
		}
		printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// OSD layer
		printf("MMSFBDevOmap: layer %d cannot be released\n", layer_id);
		return false;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevOmap::restoreLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevOmap: layer %d cannot be restored\n", layer_id);
	    return false;
	case 1:
		// Video layer (VID)
		if (this->vid.fbdev) {
			if (this->vid.fbdev->openDevice(this->vid.device, -2)) {
				if (!this->vid.width)
					return this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
				else
				if (this->vid.width > 0)
					return this->vid.fbdev->initLayer(0, this->vid.width, this->vid.height,
													  this->vid.pixelformat, this->vid.backbuffer);
				return true;
			}
			return false;
		}
		printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// OSD layer (OSD1)
		printf("MMSFBDevOmap: layer %d cannot be restored\n", layer_id);
	    return false;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevOmap::vtGetFd(int *fd) {
	if ((this->primary)&&(this->primary->fbdev)) {
		if (this->primary->fbdev->vt.fd != -1) {
			*fd = this->primary->fbdev->vt.fd;
			return true;
		}
	}
	return false;
}


bool MMSFBDevOmap::onGenFBPixelFormatDev(MMSFBSurfacePixelFormat pf, unsigned int *nonstd_format, MMSFBPixelDef *pixeldef) {

	if (nonstd_format) {
		switch (pf) {
		case MMSFB_PF_I420:
			*nonstd_format = OMAPFB_COLOR_YUV420;
			return true;
		default:
			break;
		}
	}
	return false;
}

bool MMSFBDevOmap::onDisableDev(int fd, string device_file) {
	// setup omap specific plane
	struct omapfb_plane_info plane_info;
	ioctl(fd, OMAPFB_QUERY_PLANE, &plane_info);
	plane_info.enabled = 0;

	printf("MMSFBDevOmap: disable plane, %s\n", device_file.c_str());
    if (ioctl(fd, OMAPFB_SETUP_PLANE, &plane_info)) {
    	printf("MMSFBDevOmap: could not disable plane, %s\n", device_file.c_str());
        return false;
    }

	return true;
}

bool MMSFBDevOmap::onActivateDev(int fd, string device_file, struct fb_var_screeninfo *var_screeninfo,
							     int width, int height, MMSFBSurfacePixelFormat pixelformat, bool switch_mode) {
	if (switch_mode) {
		if (ioctl(fd, FBIOPUT_VSCREENINFO, var_screeninfo) < 0) {
			printf("MMSFBDevOmap: could not switch to mode %dx%d, pixelformat %s (%d bits, nonstd %d), %s\n",
					width, height, getMMSFBPixelFormatString(pixelformat).c_str(),
					var_screeninfo->bits_per_pixel, var_screeninfo->nonstd,
					device_file.c_str());
			return false;
		}
	}

	// enable alpha blending
	if (var_screeninfo->transp.length) {
		printf("MMSFBDevOmap: set alpha blending!\n");
		int sysfd;
		sysfd = open("/sys/devices/platform/omapdss/manager0/alpha_blending_enabled",O_WRONLY);
		if(sysfd == -1) {
			printf("MMSFBDevOmap: could not access display manager (/sys/devices/platform/omapdss/manager0/alpha_blending_enabled)!\n");
		}
		write(sysfd,"1\n",2);
		close(sysfd);
	}

	// setup omap specific plane
    struct omapfb_plane_info plane_info;
    ioctl(fd, OMAPFB_QUERY_PLANE, &plane_info);
    plane_info.enabled = 1;
    plane_info.pos_x = 0;
    plane_info.pos_y = 0;
    plane_info.out_width = var_screeninfo->xres;
    plane_info.out_height = var_screeninfo->yres;

    printf("MMSFBDevOmap: enable plane, %s\n", device_file.c_str());
    if (ioctl(fd, OMAPFB_SETUP_PLANE, &plane_info)) {
    	printf("MMSFBDevOmap: could not enable plane, %s\n", device_file.c_str());
        return false;
    }

	return true;
}

#endif
