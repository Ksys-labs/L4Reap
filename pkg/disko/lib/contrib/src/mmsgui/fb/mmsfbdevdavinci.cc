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

#include "mmsgui/fb/mmsfbdevdavinci.h"
#include <sys/ioctl.h>
#include <cstring>

#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBDevDavinci is not initialized");return false;}

MMSFBDevDavinci::MMSFBDevDavinci() {
	this->osd0.fbdev = NULL;
	this->osd1.fbdev = NULL;
	this->vid0.fbdev = NULL;
	this->vid1.fbdev = NULL;
}

MMSFBDevDavinci::~MMSFBDevDavinci() {
	closeDevice();
}

bool MMSFBDevDavinci::openDevice(char *device_file, int console) {
	// close the device if opened
	closeDevice();

	// open davinci frame buffers
	for (int i = 0; i < 4; i++) {
		MMSFBDev *fbdev;
		char      dev[100];
		sprintf(dev, "/dev/fb%d", i);
		fbdev = new MMSFBDev();
		if (!fbdev->openDevice(dev, (!i)?-1:-2)) {
			delete fbdev;
			closeDevice();
			return false;
		}

		if (memcmp(fbdev->fix_screeninfo.id, "dm_osd0_fb", 10) == 0) {
			this->osd0.fbdev = fbdev;
			strcpy(this->osd0.device, dev);
			this->osd0.width = -1;
		}
		else
		if (memcmp(fbdev->fix_screeninfo.id, "dm_vid0_fb", 10) == 0) {
			this->vid0.fbdev = fbdev;
			strcpy(this->vid0.device, dev);
			this->vid0.width = 0;
			// disable device
			this->vid0.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
		}
		else
		if (memcmp(fbdev->fix_screeninfo.id, "dm_osd1_fb", 10) == 0) {
			this->osd1.fbdev = fbdev;
			strcpy(this->osd1.device, dev);
			this->osd1.width = -1;
		}
		else
		if (memcmp(fbdev->fix_screeninfo.id, "dm_vid1_fb", 10) == 0) {
			this->vid1.fbdev = fbdev;
			strcpy(this->vid1.device, dev);
			this->vid1.width = 0;
			// disable device
			this->vid1.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
		}
		else {
			// not supported
			printf("MMSFBDevDavinci: unsupported accelerator %d (%.16s)\n", fbdev->fix_screeninfo.accel, fbdev->fix_screeninfo.id);
			delete fbdev;
			closeDevice();
			return false;
		}

		if (!i && !this->osd0.fbdev) {
			// osd0 must be at /dev/fb0
			printf("MMSFBDevDavinci: /dev/fb0 is not osd0\n");
			closeDevice();
			return false;
		}
	}

    // all initialized :)
	this->isinitialized = true;

	return true;
}

void MMSFBDevDavinci::closeDevice() {
	// close frame buffers
	if (this->vid1.fbdev) {
		delete this->vid1.fbdev;
		this->vid1.fbdev = NULL;
	}
	if (this->vid0.fbdev) {
		delete this->vid0.fbdev;
		this->vid0.fbdev = NULL;
	}
	if (this->osd1.fbdev) {
		delete this->osd1.fbdev;
		this->osd1.fbdev = NULL;
	}
	if (this->osd0.fbdev) {
		delete this->osd0.fbdev;
		this->osd0.fbdev = NULL;
	}

	// reset all other
	this->isinitialized = false;
}

bool MMSFBDevDavinci::waitForVSync() {
	// is initialized?
	INITCHECK;

	if (!this->osd0.fbdev)
		return false;

	static const int s = 0;
	if (ioctl(this->osd0.fbdev->fd, FBIO_WAITFORVSYNC, &s)) {
		// failed, well then???
	}

	return true;
}

bool MMSFBDevDavinci::panDisplay(int buffer_id, void *framebuffer_base) {
	// is initialized?
	INITCHECK;

	if   ((framebuffer_base == this->osd0.fbdev->framebuffer_base)
		||(framebuffer_base == this->osd1.fbdev->framebuffer_base)) {
		// Graphic layer (OSD0 and OSD1)
		if (this->osd0.fbdev)
			this->osd0.fbdev->panDisplay(buffer_id);
		if (this->osd1.fbdev)
			this->osd1.fbdev->panDisplay(buffer_id);
		return true;
	}
	else
	if (framebuffer_base == this->vid0.fbdev->framebuffer_base) {
		// Video layer (VID0)
		if (this->vid0.fbdev)
			return this->vid0.fbdev->panDisplay(buffer_id);
		return false;
	}
	else
	if (framebuffer_base == this->vid1.fbdev->framebuffer_base) {
		// Video layer (VID1)
		if (this->vid1.fbdev)
			return this->vid1.fbdev->panDisplay(buffer_id);
		return false;
	}

	// check framebuffer_base pointer
	printf("MMSFBDevDavinci: framebuffer base pointer not correct\n");
	return false;
}

bool MMSFBDevDavinci::testLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
	    return true;
	case 1:
		// Video layer (VID0)
		return true;
	case 2:
		// Video layer (VID1)
		return true;
	default:
    	printf("MMSFBDevDavinci: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}


bool MMSFBDevDavinci::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		if ((!this->osd0.fbdev) || (!this->osd1.fbdev)) {
			printf("MMSFBDevDavinci: OSD Layer %d not initialized\n", layer_id);
			return false;
		}

		if   ((pixelformat != MMSFB_PF_ARGB3565)
			&&(pixelformat != MMSFB_PF_RGB16)) {
			printf("MMSFBDevDavinci: OSD Layer %d needs pixelformat ARGB3565 or RGB16, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		if (backbuffer) {
			printf("MMSFBDevDavinci: OSD Layer %d does not support backbuffer handling\n", layer_id);
			return false;
		}

		// init the two davinci osd "windows"
		if (this->osd0.fbdev->initLayer(0, width, height, MMSFB_PF_RGB16, backbuffer)) {
			// init osd1 attribute plane
			if (this->osd1.fbdev->initLayer(0, width, height, MMSFB_PF_A4, backbuffer)) {
				// set values
				this->layers[layer_id].width = width;
				this->layers[layer_id].height = height;
				this->layers[layer_id].pixelformat = pixelformat;

				// save the buffers
				memcpy(this->layers[layer_id].buffers, this->osd0.fbdev->layers[0].buffers, sizeof(this->osd0.fbdev->layers[0].buffers));

				// merge OSD0 with OSD1
				if (pixelformat == MMSFB_PF_ARGB3565) {
					// set the alpha plane
					switch (backbuffer) {
					case 2:
						this->layers[layer_id].buffers[2].ptr2 = this->osd1.fbdev->layers[0].buffers[2].ptr;
						this->layers[layer_id].buffers[2].pitch2 = this->osd1.fbdev->layers[0].buffers[2].pitch;
					case 1:
						this->layers[layer_id].buffers[1].ptr2 = this->osd1.fbdev->layers[0].buffers[1].ptr;
						this->layers[layer_id].buffers[1].pitch2 = this->osd1.fbdev->layers[0].buffers[1].pitch;
					case 0:
						this->layers[layer_id].buffers[0].ptr2 = this->osd1.fbdev->layers[0].buffers[0].ptr;
						this->layers[layer_id].buffers[0].pitch2 = this->osd1.fbdev->layers[0].buffers[0].pitch;
						break;
					default:
						return false;
					}
				}

				// clear layer
				if (pixelformat == MMSFB_PF_ARGB3565) {
					MMSFBColor color(0x00, 0x00, 0x00, 0x00);
					mmsfb_fillrectangle_argb3565(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
												 0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				}
				else {
					MMSFBColor color(0x00, 0x00, 0x00, 0xff);
					mmsfb_fillrectangle_rgb16(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
											  0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				}

				// layer is initialized
				this->layers[layer_id].isinitialized = true;

				printf("MMSFBDevDavinci: OSD Layer %d initialized with %dx%dx%d, pixelformat %s\n",
							layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

	        	this->osd0.width = this->osd1.width = width;
	        	this->osd0.height = this->osd1.height = height;
	        	this->osd0.pixelformat = this->osd1.pixelformat = pixelformat;
	        	this->osd0.backbuffer = this->osd1.backbuffer = backbuffer;
				return true;
			}
		}
		return false;

	case 1:
		// Video layer (VID0)
		if (!this->vid0.fbdev) {
			printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
			return false;
		}

		if (pixelformat != MMSFB_PF_YUY2) {
			printf("MMSFBDevDavinci: Video Layer %d needs pixelformat YUY2, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// disable VID1
		if (this->vid1.fbdev) {
			this->vid1.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			this->vid1.width = 0;
		}

		// enable VID0
		if (this->vid0.fbdev->initLayer(0, width, height, MMSFB_PF_YUY2, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->vid0.fbdev->layers[0].buffers, sizeof(this->vid0.fbdev->layers[0].buffers));

			// clear layer
			MMSFBColor color(0x00, 0x00, 0x00, 0xff);
			mmsfb_fillrectangle_yuy2(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
			                         0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevDavinci: Video Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

        	this->vid0.width = width;
        	this->vid0.height = height;
        	this->vid0.pixelformat = pixelformat;
        	this->vid0.backbuffer = backbuffer;

        	return true;
		}
		return false;

	case 2:
		// Video layer (VID1)
		if (!this->vid1.fbdev) {
			printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
			return false;
		}

		if (pixelformat != MMSFB_PF_YUY2) {
			printf("MMSFBDevDavinci: Video Layer %d needs pixelformat YUY2, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// disable VID0
		if (this->vid0.fbdev) {
			this->vid0.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			this->vid0.width = 0;
		}

		// enable VID1
		if (this->vid1.fbdev->initLayer(0, width, height, MMSFB_PF_YUY2, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->vid1.fbdev->layers[0].buffers, sizeof(this->vid1.fbdev->layers[0].buffers));

			// clear layer
			MMSFBColor color(0x00, 0x00, 0x00, 0xff);
			mmsfb_fillrectangle_yuy2(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
			                         0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevDavinci: Video Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

        	this->vid1.width = width;
        	this->vid1.height = height;
        	this->vid1.pixelformat = pixelformat;
        	this->vid1.backbuffer = backbuffer;

        	return true;
		}
		return false;
	default:
		printf("MMSFBDevDavinci: layer %d is not supported\n", layer_id);
		break;
	}

	return false;
}

bool MMSFBDevDavinci::releaseLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevDavinci: layer %d cannot be released\n", layer_id);
		return false;
	case 1:
		// Video layer (VID0)
		if (this->vid0.fbdev) {
			// disable
			this->vid0.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			// close
			this->vid0.fbdev->closeDevice();
			return true;
		}
		printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// Video layer (VID1)
		if (this->vid1.fbdev) {
			// disable
			this->vid1.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			// close
			this->vid1.fbdev->closeDevice();
			return true;
		}
		printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
		return false;
	default:
    	printf("MMSFBDevDavinci: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevDavinci::restoreLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevDavinci: layer %d cannot be restored\n", layer_id);
	    return false;
	case 1:
		// Video layer (VID0)
		if (this->vid0.fbdev) {
			if (this->vid0.fbdev->openDevice(this->vid0.device, -2)) {
				if (!this->vid0.width)
					return this->vid0.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
				else
				if (this->vid0.width > 0)
					return this->vid0.fbdev->initLayer(0, this->vid0.width, this->vid0.height,
													   this->vid0.pixelformat, this->vid0.backbuffer);
				return true;
			}
			return false;
		}
		printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// Video layer (VID1)
		if (this->vid1.fbdev) {
			if (this->vid1.fbdev->openDevice(this->vid1.device, -2)) {
				if (!this->vid1.width)
					return this->vid1.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
				else
				if (this->vid1.width > 0)
					return this->vid1.fbdev->initLayer(0, this->vid1.width, this->vid1.height,
													   this->vid1.pixelformat, this->vid1.backbuffer);
				return true;
			}
			return false;
		}
		printf("MMSFBDevDavinci: Video Layer %d not initialized\n", layer_id);
		return false;
	default:
    	printf("MMSFBDevDavinci: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevDavinci::vtGetFd(int *fd) {
	if (this->osd0.fbdev) {
		if (this->osd0.fbdev->vt.fd != -1) {
			*fd = this->osd0.fbdev->vt.fd;
			return true;
		}
	}
	return false;
}

#endif
