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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/kd.h>
#include <linux/vt.h>
#include "mmsgui/fb/mmsfbdev.h"

#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBDev is not initialized");return false;}

MMSFBDev::MMSFBDev() {
	// init fb vals
	this->isinitialized = false;
	this->fd = -1;
	this->framebuffer_base = NULL;
	this->reset_console_accel = false;
	memset(this->modes, 0, sizeof(this->modes));
	this->modes_cnt = 0;
	memset(this->layers, 0, sizeof(this->layers));
	this->layers_cnt = 0;
	this->active_screen = 0;

	// init terminal vals
	this->vt.fd0 = -1;
	this->vt.fd = -1;
	this->vt.number = -1;
	this->vt.previous = -1;
	this->vt.org_fb = -1;
}

MMSFBDev::~MMSFBDev() {
	closeDevice();
}

void MMSFBDev::printFixScreenInfo() {
	char id[17];
	id[16]=0;
    printf("MMSFBDev: fix screen info ------------\n");
    printf("    device         = %s\n", this->device_file.c_str());
    memcpy(id, fix_screeninfo.id, 16);
    printf("    id             = %s\n", id);
    printf("    smem_start     = 0x%x\n", (unsigned int)fix_screeninfo.smem_start);
    printf("    smem_len       = %d\n", fix_screeninfo.smem_len);
    printf("    type           = %d\n", fix_screeninfo.type);
    printf("    type_aux       = %d\n", fix_screeninfo.type_aux);
    printf("    visual         = %d\n", fix_screeninfo.visual);
    printf("    xpanstep       = %d\n", fix_screeninfo.xpanstep);
    printf("    ypanstep       = %d\n", fix_screeninfo.ypanstep);
    printf("    ywrapstep      = %d\n", fix_screeninfo.ywrapstep);
    printf("    line_length    = %d\n", fix_screeninfo.line_length);
    printf("    mmio_start     = 0x%x\n", (unsigned int)fix_screeninfo.mmio_start);
    printf("    mmio_len       = %d\n", fix_screeninfo.mmio_len);
    printf("    accel          = %d\n", fix_screeninfo.accel);
    printf("    reserved[3]    = %d, %d, %d\n", fix_screeninfo.reserved[0], fix_screeninfo.reserved[1], fix_screeninfo.reserved[2]);
}

void MMSFBDev::printVarScreenInfo() {
    printf("MMSFBDev: var screen info ------------\n");
    printf("    xres           = %d\n", var_screeninfo.xres);
    printf("    yres           = %d\n", var_screeninfo.yres);
    printf("    xres_virtual   = %d\n", var_screeninfo.xres_virtual);
    printf("    yres_virtual   = %d\n", var_screeninfo.yres_virtual);
    printf("    xoffset        = %d\n", var_screeninfo.xoffset);
    printf("    yoffset        = %d\n", var_screeninfo.yoffset);
    printf("    bits_per_pixel = %d\n", var_screeninfo.bits_per_pixel);
    printf("    grayscale      = %d\n", var_screeninfo.grayscale);
    printf("    red            = %d(offs=%d)\n", var_screeninfo.red.length, var_screeninfo.red.offset);
    printf("    green          = %d(offs=%d)\n", var_screeninfo.green.length, var_screeninfo.green.offset);
    printf("    blue           = %d(offs=%d)\n", var_screeninfo.blue.length, var_screeninfo.blue.offset);
    printf("    transp         = %d(offs=%d)\n", var_screeninfo.transp.length, var_screeninfo.transp.offset);
    printf("    nonstd         = %d\n", var_screeninfo.nonstd);
    printf("    activate       = %d\n", var_screeninfo.activate);
    printf("    height         = %d\n", var_screeninfo.height);
    printf("    width          = %d\n", var_screeninfo.width);
    printf("    accel_flags    = %d\n", var_screeninfo.accel_flags);

    printf("    pixclock       = %d\n", var_screeninfo.pixclock);
    printf("    left_margin    = %d\n", var_screeninfo.left_margin);
    printf("    right_margin   = %d\n", var_screeninfo.right_margin);
    printf("    upper_margin   = %d\n", var_screeninfo.upper_margin);
    printf("    lower_margin   = %d\n", var_screeninfo.lower_margin);
    printf("    hsync_len      = %d\n", var_screeninfo.hsync_len);
    printf("    vsync_len      = %d\n", var_screeninfo.vsync_len);
    printf("    sync           = %d\n", var_screeninfo.sync);
    printf("    vmode          = %d\n", var_screeninfo.vmode);
    printf("    rotate         = %d\n", var_screeninfo.rotate);
    printf("    accel_flags    = %d\n", var_screeninfo.accel_flags);
    printf("    reserved[5]    = %d, %d, %d, %d, %d\n", var_screeninfo.reserved[0], var_screeninfo.reserved[1], var_screeninfo.reserved[2], var_screeninfo.reserved[3], var_screeninfo.reserved[4]);
}

bool MMSFBDev::buildPixelFormat() {
	this->layers[0].pixelformat = MMSFB_PF_NONE;
    switch (var_screeninfo.transp.length) {
    case 0:
    	// pixelformat with no alphachannel
    	if    ((var_screeninfo.red.length == 5)  && (var_screeninfo.green.length == 6) && (var_screeninfo.blue.length == 5)
    		&& (var_screeninfo.red.offset == 11) && (var_screeninfo.green.offset == 5) && (var_screeninfo.blue.offset == 0)) {
    		this->layers[0].pixelformat = MMSFB_PF_RGB16;
    	}
    	else
    	if    ((var_screeninfo.red.length == 8)  && (var_screeninfo.green.length == 8) && (var_screeninfo.blue.length == 8)
    		&& (var_screeninfo.red.offset == 16) && (var_screeninfo.green.offset == 8) && (var_screeninfo.blue.offset == 0)) {
    		if (var_screeninfo.bits_per_pixel == 24)
    			this->layers[0].pixelformat = MMSFB_PF_RGB24;
    		else
    			this->layers[0].pixelformat = MMSFB_PF_RGB32;
    	}
    	else
    	if    ((var_screeninfo.red.length == 8) && (var_screeninfo.green.length == 8) && (var_screeninfo.blue.length == 8)
    		&& (var_screeninfo.red.offset == 0) && (var_screeninfo.green.offset == 8) && (var_screeninfo.blue.offset == 16)) {
    		if (var_screeninfo.bits_per_pixel == 24)
    			this->layers[0].pixelformat = MMSFB_PF_BGR24;
    	}
    	else
    	if    ((var_screeninfo.red.length == 5) && (var_screeninfo.green.length == 5) && (var_screeninfo.blue.length == 5)
    		&& (var_screeninfo.red.offset == 0) && (var_screeninfo.green.offset == 5) && (var_screeninfo.blue.offset == 10)) {
    		if (var_screeninfo.bits_per_pixel == 16)
    			this->layers[0].pixelformat = MMSFB_PF_BGR555;
    	}
    	else
    	if    ((var_screeninfo.red.length == 0) && (var_screeninfo.green.length == 0) && (var_screeninfo.blue.length == 0)
    		&& (var_screeninfo.red.offset == 0) && (var_screeninfo.green.offset == 0) && (var_screeninfo.blue.offset == 0)) {
    		if (var_screeninfo.bits_per_pixel == 4)
    			this->layers[0].pixelformat = MMSFB_PF_A4;
    		else
			if (var_screeninfo.bits_per_pixel == 16)
				this->layers[0].pixelformat = MMSFB_PF_YUY2;
			else
    			this->layers[0].pixelformat = MMSFB_PF_NONE;
    	}
    	break;
    case 8:
    	// pixelformat with 8 bit alphachannel
    	if    ((var_screeninfo.red.length == 8)  && (var_screeninfo.green.length == 8) && (var_screeninfo.blue.length == 8)
    		&& (var_screeninfo.red.offset == 16) && (var_screeninfo.green.offset == 8) && (var_screeninfo.blue.offset == 0)) {
    		this->layers[0].pixelformat = MMSFB_PF_ARGB;
    	}
    	else
    	if    ((var_screeninfo.red.length == 8)  && (var_screeninfo.green.length == 8) && (var_screeninfo.blue.length == 8)
    		&& (var_screeninfo.red.offset == 0) && (var_screeninfo.green.offset == 8) && (var_screeninfo.blue.offset == 16)) {
    		this->layers[0].pixelformat = MMSFB_PF_ABGR;
    	}
    	break;
    }

    if (this->layers[0].pixelformat != MMSFB_PF_NONE) {
   		printf("MMSFBDev: current pixelformat is %s\n", getMMSFBPixelFormatString(this->layers[0].pixelformat).c_str());
    	return true;
    }

	return false;
}

bool MMSFBDev::readModes() {
	// reset mode list
	this->modes_cnt = 0;

	// open fb.modes
	FILE *fp;
	if (!(fp = fopen("/etc/fb.modes","r")))
		return false;

	// through all lines
	char line[128];
	while (fgets(line, sizeof(line)-1, fp)) {
		char label[32];
		if (sscanf(line, "mode \"%31[^\"]\"", label) == 1) {
			// mode found
			bool geom_set = false;
			bool timings_set = false;
			struct fb_var_screeninfo *mode = &this->modes[this->modes_cnt];
			memset(mode, 0, sizeof(struct fb_var_screeninfo));

			// parse for settings
			while (fgets(line, sizeof(line)-1, fp) && !(strstr(line, "endmode"))) {
				char value[16];
				int dummy;
				if (sscanf(line, " geometry %d %d %d %d %d",
								&mode->xres, &mode->yres, &dummy, &dummy, &mode->bits_per_pixel) == 5)
					geom_set = true;
				else
				if (sscanf(line, " timings %d %d %d %d %d %d %d",
								&mode->pixclock, &mode->left_margin,  &mode->right_margin,
								&mode->upper_margin, &mode->lower_margin, &mode->hsync_len, &mode->vsync_len) == 7)
					timings_set = true;
				else
				if ((sscanf(line, " hsync %15s", value) == 1) && !strcasecmp(value, "high"))
					mode->sync |= FB_SYNC_HOR_HIGH_ACT;
				else
				if ((sscanf(line, " vsync %15s", value) == 1) && !strcasecmp(value, "high"))
					mode->sync |= FB_SYNC_VERT_HIGH_ACT;
				else
				if ((sscanf(line, " csync %15s", value) == 1) && !strcasecmp(value, "high"))
					mode->sync |= FB_SYNC_COMP_HIGH_ACT;
				else
				if ((sscanf(line, " gsync %15s", value) == 1) && !strcasecmp(value, "true"))
					mode->sync |= FB_SYNC_ON_GREEN;
				else
				if ((sscanf(line, " extsync %15s", value) == 1) && !strcasecmp(value, "true"))
					mode->sync |= FB_SYNC_EXT;
				else
				if ((sscanf(line, " bcast %15s", value) == 1) && !strcasecmp(value, "true"))
					mode->sync |= FB_SYNC_BROADCAST;
				else
				if ((sscanf(line, " laced %15s", value) == 1) && !strcasecmp(value, "true"))
					mode->vmode |= FB_VMODE_INTERLACED;
				else
				if ((sscanf(line, " double %15s", value) == 1) && !strcasecmp(value, "true"))
					mode->vmode |= FB_VMODE_DOUBLE;
			}

			if (geom_set &&	timings_set) {
				// add mode to list
				this->modes_cnt++;

			    printf("MMSFBDev: mode %s (%dx%d, %d bits) loaded from /etc/fb.modes\n", label, mode->xres, mode->yres, mode->bits_per_pixel);
			}
			else {
				// ignore mode
			    printf("MMSFBDev: ignore mode %s (%dx%d, %d bits) from /etc/fb.modes\n", label, mode->xres, mode->yres, mode->bits_per_pixel);
			}
		}
	}

	fclose (fp);
	return true;
}


bool MMSFBDev::openDevice(char *device_file, int console) {
	// close the device if opened
	closeDevice();

    if (device_file) {
        // open given device
    	this->fd = open(device_file, O_RDWR);
        if (this->fd < 0) {
        	printf("MMSFBDev: opening device %s failed\n", device_file);
            return false;
        }
        this->device_file = device_file;
    }
    else {
        // open standard device
        this->fd = open("/dev/fb0", O_RDWR);
        if (this->fd < 0) {
            this->fd = open("/dev/fb/0", O_RDWR);
            if (this->fd < 0) {
            	printf("MMSFBDev: opening device /dev/fb0 and /dev/fb/0 failed\n");
                return false;
            }
            this->device_file = "/dev/fb/0";
        }
        else
            this->device_file = "/dev/fb0";
    }

    // file descriptor should be closed when an exec function is invoked
    fcntl(this->fd, F_SETFD, FD_CLOEXEC);

    // build device abbreviation
    memset(this->device, 0, sizeof(this->device));
    sprintf(this->device, "fb0");
    if (this->device_file.substr(0, 8) == "/dev/fb/")
        sprintf(this->device, "fb%s", this->device_file.substr(8, 5).c_str());
    else
    if (this->device_file.substr(0, 7) == "/dev/fb")
        sprintf(this->device, "fb%s", this->device_file.substr(7, 5).c_str());

    // read video modes
    readModes();
	printf("MMSFBDev: %d modes loaded from /etc/fb.modes\n", this->modes_cnt);

    // initialize virtual terminal
	if (console >= -1)
		if (!vtOpen(console)) {
			closeDevice();
			return false;
		}

    // get fix screen infos
    if (ioctl(this->fd, FBIOGET_FSCREENINFO, &this->fix_screeninfo) < 0) {
    	printf("MMSFBDev: could not get fix screen infos from %s\n", this->device_file.c_str());
    	closeDevice();
        return false;
    }
    printFixScreenInfo();

    // map framebuffer to memory (try shared, if it doesn't work, i.e. on non-mmu based machines, try private)
    if ((this->framebuffer_base=mmap(NULL, this->fix_screeninfo.smem_len,
                                     PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0)) == MAP_FAILED) {
        if ((this->framebuffer_base=mmap(NULL, this->fix_screeninfo.smem_len,
                                         PROT_READ | PROT_WRITE, MAP_PRIVATE, this->fd, 0)) == MAP_FAILED) {
			printf("MMSFBDev: could not mmap framebuffer memory for %s\n", this->device_file.c_str());
			this->framebuffer_base = NULL;
			closeDevice();
			return false;
        }
    }

    // get variable screen infos
    if (ioctl(this->fd, FBIOGET_VSCREENINFO, &this->org_var_screeninfo) < 0) {
    	printf("MMSFBDev: could not get var screen infos from %s\n", this->device_file.c_str());
    	closeDevice();
        return false;
    }

    // disable console acceleration
    this->var_screeninfo = this->org_var_screeninfo;
    this->var_screeninfo.accel_flags = 0;
    if (ioctl(this->fd, FBIOPUT_VSCREENINFO, &this->var_screeninfo) < 0) {
    	printf("MMSFBDev: could not disable console acceleration for %s\n", this->device_file.c_str());
    	closeDevice();
        return false;
    }
    printVarScreenInfo();

	// build the preset pixelformat
	buildPixelFormat();

    // all initialized :)
	this->isinitialized = true;

    return true;
}

void MMSFBDev::closeDevice() {
	// reset virtual terminal
	vtClose();

	// free resources if allocated
	if (this->reset_console_accel) {
	    // reset console acceleration
	    ioctl(this->fd, FBIOPUT_VSCREENINFO, &this->org_var_screeninfo);
		this->reset_console_accel = false;
	}

	if (this->framebuffer_base) {
		munmap(this->framebuffer_base, this->fix_screeninfo.smem_len);
		this->framebuffer_base = NULL;
	}

	if (this->fd != -1) {
        close(this->fd);
        this->fd = -1;
	}

	// reset all other
	this->isinitialized = false;
	memset(this->modes, 0, sizeof(this->modes));
	this->modes_cnt = 0;
	memset(this->layers, 0, sizeof(this->layers));
	this->layers_cnt = 0;
	this->active_screen = 0;
}

bool MMSFBDev::isInitialized() {
	return this->isinitialized;
}

bool MMSFBDev::waitForVSync() {
	// is initialized?
	INITCHECK;

	// default fbdev does support only primary screen 0
	if (this->active_screen != 0) {
    	printf("MMSFBDev: screen %d is not supported\n", this->active_screen);
		return false;
	}

	static const int s = 0;
	if (ioctl(this->fd, FBIO_WAITFORVSYNC, &s)) {
		// failed, well then???
	}

	return true;
}

bool MMSFBDev::panDisplay(int buffer_id, void *framebuffer_base) {
	// is initialized?
	INITCHECK;

	// check framebuffer_base pointer
	if (framebuffer_base) {
		if (framebuffer_base != this->framebuffer_base) {
			printf("MMSFBDev: framebuffer base pointer not correct\n");
			return false;
		}
	}

	// calc new y offset
	int yoffset = buffer_id * this->var_screeninfo.yres;
	if ((yoffset < 0) || (yoffset + this->var_screeninfo.yres > this->var_screeninfo.yres_virtual)) {
		return false;
	}
	int xoffset_save = this->var_screeninfo.xoffset;
	int yoffset_save = this->var_screeninfo.yoffset;

	// set new x/y offsets
	this->var_screeninfo.xoffset = 0;
	this->var_screeninfo.yoffset = yoffset;
	if (this->fix_screeninfo.ypanstep)
		this->var_screeninfo.vmode &= ~FB_VMODE_YWRAP;
	else
		this->var_screeninfo.vmode |= FB_VMODE_YWRAP;

	// switch display
	this->var_screeninfo.activate = FB_ACTIVATE_VBL;
	if (ioctl(this->fd, FBIOPAN_DISPLAY, &this->var_screeninfo) < 0) {
    	printf("MMSFBDev: display panning not supported\n");
		this->var_screeninfo.xoffset = xoffset_save;
		this->var_screeninfo.yoffset = yoffset_save;
		return false;
	}

	return true;
}

bool MMSFBDev::testLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	// default fbdev does support only primary layer 0 on primary screen 0
	if (layer_id != 0) {
    	printf("MMSFBDev: layer %d is not supported\n", layer_id);
		return false;
	}

	return true;
}

bool MMSFBDev::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	// is initialized?
	INITCHECK;

	// default fbdev does support only primary layer 0 on primary screen 0
	if (layer_id != 0) {
    	printf("MMSFBDev: layer %d is not supported\n", layer_id);
		return false;
	}

	//switch video mode
	if (!setMode(width, height, pixelformat, backbuffer))
		return false;

	if (width <= 0 || height <= 0) {
		// the layer is disabled now
		this->layers[layer_id].isinitialized = false;
		return true;
	}

	// save dimension of the layer
	this->layers[layer_id].width = this->var_screeninfo.xres;
	this->layers[layer_id].height = this->var_screeninfo.yres;

	// save the buffers
	memset(&this->layers[layer_id].buffers, 0, sizeof(this->layers[layer_id].buffers));
	switch (backbuffer) {
	case 2:
		this->layers[layer_id].buffers[2].ptr  = ((char *)this->framebuffer_base)
												+ 2 * this->fix_screeninfo.line_length * this->var_screeninfo.yres;
		this->layers[layer_id].buffers[2].pitch= this->fix_screeninfo.line_length;
		this->layers[layer_id].buffers[2].hwbuffer = true;
	case 1:
		this->layers[layer_id].buffers[1].ptr  = ((char *)this->framebuffer_base)
												 + this->fix_screeninfo.line_length * this->var_screeninfo.yres;
		this->layers[layer_id].buffers[1].pitch= this->fix_screeninfo.line_length;
		this->layers[layer_id].buffers[1].hwbuffer = true;
	case 0:
		this->layers[layer_id].buffers[0].ptr  = this->framebuffer_base;
		this->layers[layer_id].buffers[0].pitch= this->fix_screeninfo.line_length;
		this->layers[layer_id].buffers[0].hwbuffer = true;
		break;
	default:
		return false;
	}

	// layer is initialized
	this->layers[layer_id].isinitialized = true;

	// this layer is on screen 0 (default)
	this->active_screen = 0;

	return true;
}

bool MMSFBDev::releaseLayer(int layer_id) {
	printf("MMSFBDev: layer %d cannot be released\n", layer_id);
	return false;
}

bool MMSFBDev::restoreLayer(int layer_id) {
	printf("MMSFBDev: layer %d cannot be restored\n", layer_id);
	return false;
}

bool MMSFBDev::getPixelFormat(int layer_id, MMSFBSurfacePixelFormat *pf) {
	// is initialized?
	INITCHECK;

	// is layer initialized?
	if (!this->layers[layer_id].isinitialized)
		return false;

	// return pixelformat
	*pf = this->layers[layer_id].pixelformat;
	return true;
}

bool MMSFBDev::getPhysicalMemory(unsigned long *mem) {
	// is initialized?
	INITCHECK;
    *mem = this->fix_screeninfo.smem_start;
	return true;
}

bool MMSFBDev::getFrameBufferBase(unsigned char **base) {
	// is initialized?
	INITCHECK;
	*base = (unsigned char *)this->framebuffer_base;
	return true;
}

bool MMSFBDev::getFrameBufferPtr(int layer_id, MMSFBSurfacePlanesBuffer buffers, int *width, int *height) {
	// is initialized?
	INITCHECK;

	// is layer initialized?
	if (!this->layers[layer_id].isinitialized) {
		return false;
	}

	// return buffer infos
	if (buffers)
		memcpy(buffers, this->layers[layer_id].buffers, sizeof(this->layers[layer_id].buffers));
	*width = this->layers[layer_id].width;
	*height = this->layers[layer_id].height;

	return true;
}


bool MMSFBDev::mapMmio(unsigned char **mmio) {
	// is initialized?
	INITCHECK;

    *mmio = (unsigned char *)mmap(NULL, this->fix_screeninfo.mmio_len, PROT_READ | PROT_WRITE, MAP_SHARED,
								  this->fd, this->fix_screeninfo.smem_len);
    if (!*mmio) {
    	printf("MMSFBDev: could not mmap mmio buffer\n");
    	return false;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    unsigned long page_mask = page_size < 0 ? 0 : (page_size - 1);

    *mmio = (*mmio) + (this->fix_screeninfo.mmio_start & page_mask);

	return true;
}

bool MMSFBDev::unmapMmio(unsigned char *mmio) {
	// is initialized?
	INITCHECK;

    long page_size = sysconf(_SC_PAGESIZE);
    unsigned long page_mask = page_size < 0 ? 0 : (page_size - 1);

	munmap((void*)(mmio - (this->fix_screeninfo.mmio_start & page_mask)), this->fix_screeninfo.mmio_len);

	return true;
}

void MMSFBDev::genFBPixelFormat(MMSFBSurfacePixelFormat pf, unsigned int *nonstd_format, MMSFBPixelDef *pixeldef) {

	// generate std format
	if (nonstd_format) *nonstd_format = 0;
	getBitsPerPixel(pf, pixeldef);

	// try to get a fb specific format
	this->onGenFBPixelFormat.emit(pf, nonstd_format, pixeldef);
}

void MMSFBDev::disable(int fd, string device_file) {
	// have to disable the framebuffer
	if (!this->onDisable.emit(fd, device_file)) {
		// use FBIOPUT_VSCREENINFO and FBIOBLANK together
		struct fb_var_screeninfo var_screeninfo;
		ioctl(fd, FBIOGET_VSCREENINFO, &var_screeninfo);
		var_screeninfo.activate = FB_ACTIVATE_NOW;
		var_screeninfo.accel_flags = 0;
		var_screeninfo.xres = 0;
		var_screeninfo.yres = 0;
		var_screeninfo.xres_virtual = 0;
		var_screeninfo.yres_virtual = 0;
		var_screeninfo.xoffset = 0;
		var_screeninfo.yoffset = 0;
		var_screeninfo.grayscale = 0;
		ioctl(fd, FBIOPUT_VSCREENINFO, &var_screeninfo);
		ioctl(fd, FBIOBLANK, 1);
	}
}

bool MMSFBDev::activate(int fd, string device_file, struct fb_var_screeninfo *var_screeninfo,
						int width, int height, MMSFBSurfacePixelFormat pixelformat, bool switch_mode) {

	// try callback
	if (!this->onActivate.emit(fd, device_file, var_screeninfo, width, height, pixelformat, switch_mode)) {
		// no callback set or callback failed
		if (switch_mode) {
			if (ioctl(fd, FBIOPUT_VSCREENINFO, var_screeninfo) < 0) {
				printf("MMSFBDev: could not switch to mode %dx%d, pixelformat %s (%d bits, nonstd %d), %s\n",
						width, height, getMMSFBPixelFormatString(pixelformat).c_str(),
						var_screeninfo->bits_per_pixel, var_screeninfo->nonstd,
						device_file.c_str());
				return false;
			}
		}
	}

    // get fix screen infos
    if (ioctl(this->fd, FBIOGET_FSCREENINFO, &this->fix_screeninfo) < 0) {
    	printf("MMSFBDev: could not get fix screen infos from %s\n", this->device_file.c_str());
        return false;
    }
    printFixScreenInfo();

    // get variable screen infos
    if (ioctl(this->fd, FBIOGET_VSCREENINFO, &this->var_screeninfo) < 0) {
    	printf("MMSFBDev: could not get var screen infos from %s\n", this->device_file.c_str());
        return false;
    }
    printVarScreenInfo();

    return true;
}

bool MMSFBDev::setMode(int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	bool do_switch = false;

	// is initialized?
	INITCHECK;

	if (width <= 0 || height <= 0) {
		// have to disable the framebuffer
		disable(this->fd, this->device_file);
		return true;
	}

    // reload fix screen infos before changing it
    if (ioctl(this->fd, FBIOGET_FSCREENINFO, &this->fix_screeninfo) < 0) {
    	printf("MMSFBDev: could not get fix screen infos from %s\n", this->device_file.c_str());
        return false;
    }
    printFixScreenInfo();

    // reload variable screen infos before changing it
    if (ioctl(this->fd, FBIOGET_VSCREENINFO, &this->var_screeninfo) < 0) {
    	printf("MMSFBDev: could not get var screen infos from %s\n", this->device_file.c_str());
        return false;
    }
    printVarScreenInfo();

    if (backbuffer) {
    	if ((!this->fix_screeninfo.ypanstep)&&(!this->fix_screeninfo.ywrapstep)) {
        	printf("MMSFBDev: backbuffer requested, but hardware does not support it for %s\n", this->device_file.c_str());
            return false;
    	}
    }

	// get bits per pixel and its length/offset
	unsigned int nonstd_format;
	MMSFBPixelDef pixeldef;
	genFBPixelFormat(pixelformat, &nonstd_format, &pixeldef);

	// check if mode is already set
    if (!nonstd_format) {
    	// do it only if a std format is requested
		if    ((width == (int)this->var_screeninfo.xres) && (height == (int)this->var_screeninfo.yres)
			&& (pixeldef.bits == (int)this->var_screeninfo.bits_per_pixel) && (this->layers[0].pixelformat == pixelformat)
			&& (!backbuffer || (this->var_screeninfo.yres_virtual >= this->var_screeninfo.yres * (backbuffer+1)))) {
			// mode already set, no switch required
			printf("MMSFBDev: using preset mode %dx%d, pixelformat %s (%d bits), %s\n",
					width, height, getMMSFBPixelFormatString(pixelformat).c_str(), pixeldef.bits,
					this->device_file.c_str());

			// we have to activate the device (can be disabled), but mode switch is not required
		    activate(this->fd, this->device_file, &this->var_screeninfo, width, height, pixelformat, false);
			return true;
		}
    }

    if (!do_switch) {
		// searching for mode
		for (int cnt = 0; cnt < this->modes_cnt; cnt++) {
			struct fb_var_screeninfo *mode = &this->modes[cnt];
			if ((width == (int)mode->xres) && (height == (int)mode->yres) && (pixeldef.bits == (int)mode->bits_per_pixel)) {
				// mode matches, switch to it
				this->var_screeninfo = *mode;

				this->var_screeninfo.activate = FB_ACTIVATE_NOW;
				this->var_screeninfo.accel_flags = 0;

        	    this->var_screeninfo.nonstd = nonstd_format;

				this->var_screeninfo.red.length = pixeldef.red_length;
				this->var_screeninfo.red.offset = pixeldef.red_offset;
				this->var_screeninfo.green.length = pixeldef.green_length;
				this->var_screeninfo.green.offset = pixeldef.green_offset;
				this->var_screeninfo.blue.length = pixeldef.blue_length;
				this->var_screeninfo.blue.offset = pixeldef.blue_offset;
				this->var_screeninfo.transp.length = pixeldef.transp_length;
				this->var_screeninfo.transp.offset = pixeldef.transp_offset;

				this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
				this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
				this->var_screeninfo.xoffset = 0;
				this->var_screeninfo.yoffset = 0;
				this->var_screeninfo.grayscale = 0;

				do_switch = true;
				break;
			}
		}
    }

    if (!do_switch) {
        // no mode found
    	printf("MMSFBDev: no mode %dx%d, bit depth %d (%s) found in /etc/fb.modes\n",
    			width, height, pixeldef.bits, getMMSFBPixelFormatString(pixelformat).c_str());

    	// searching for mode with any pixelformat
        for (int cnt = 0; cnt < this->modes_cnt; cnt++) {
        	struct fb_var_screeninfo *mode = &this->modes[cnt];
        	if ((width == (int)mode->xres) && (height == (int)mode->yres)) {
        		// mode matches, switch to it
        	    this->var_screeninfo = *mode;

				printf("MMSFBDev: trying to use first mode %dx%d, bit depth %d from /etc/fb.modes\n", width, height, this->var_screeninfo.bits_per_pixel);

                this->var_screeninfo.activate = FB_ACTIVATE_NOW;
        	    this->var_screeninfo.accel_flags = 0;

        	    this->var_screeninfo.nonstd = nonstd_format;

        	    this->var_screeninfo.bits_per_pixel = pixeldef.bits;
        	    this->var_screeninfo.red.length = pixeldef.red_length;
    			this->var_screeninfo.red.offset = pixeldef.red_offset;
    			this->var_screeninfo.green.length = pixeldef.green_length;
    			this->var_screeninfo.green.offset = pixeldef.green_offset;
    			this->var_screeninfo.blue.length = pixeldef.blue_length;
    			this->var_screeninfo.blue.offset = pixeldef.blue_offset;
    			this->var_screeninfo.transp.length = pixeldef.transp_length;
    			this->var_screeninfo.transp.offset = pixeldef.transp_offset;

    			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
    			this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
    			this->var_screeninfo.xoffset = 0;
    			this->var_screeninfo.yoffset = 0;
    			this->var_screeninfo.grayscale = 0;

    			do_switch = true;
    			break;
        	}
    	}
    }

    if (!do_switch) {
        // no mode found
    	printf("MMSFBDev: no mode %dx%d with any bit depth found in /etc/fb.modes\n", width, height);

        // check if resolution has changed
    	if  ((width == (int)this->var_screeninfo.xres) && (height == (int)this->var_screeninfo.yres)) {
			// resolution has not changed, so try to change only the pixelformat
			printf("MMSFBDev: resolution is the same, so try to change the pixelformat to %s, %s\n",
					getMMSFBPixelFormatString(pixelformat).c_str(),
					this->device_file.c_str());

			this->var_screeninfo.activate = FB_ACTIVATE_NOW;
			this->var_screeninfo.accel_flags = 0;

    	    this->var_screeninfo.nonstd = nonstd_format;

    	    this->var_screeninfo.bits_per_pixel = pixeldef.bits;
			this->var_screeninfo.red.length = pixeldef.red_length;
			this->var_screeninfo.red.offset = pixeldef.red_offset;
			this->var_screeninfo.green.length = pixeldef.green_length;
			this->var_screeninfo.green.offset = pixeldef.green_offset;
			this->var_screeninfo.blue.length = pixeldef.blue_length;
			this->var_screeninfo.blue.offset = pixeldef.blue_offset;
			this->var_screeninfo.transp.length = pixeldef.transp_length;
			this->var_screeninfo.transp.offset = pixeldef.transp_offset;

			if (backbuffer) {
    			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
				this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
    			this->var_screeninfo.xoffset = 0;
    			this->var_screeninfo.yoffset = 0;
			}

			do_switch = true;
		}
    	else
		if  (this->layers[0].pixelformat == pixelformat) {
			// pixelformat has not changed, so try to change only the resolution
			printf("MMSFBDev: pixelformat is the same, so try to change the resolution to %dx%d, %s\n",
					width, height,
					this->device_file.c_str());

			this->var_screeninfo.activate = FB_ACTIVATE_NOW;
			this->var_screeninfo.accel_flags = 0;

			this->var_screeninfo.xres = width;
			this->var_screeninfo.yres = height;

			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
			this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
			this->var_screeninfo.xoffset = 0;
			this->var_screeninfo.yoffset = 0;
			this->var_screeninfo.grayscale = 0;

			do_switch = true;
		}
    }

	if (do_switch) {
		// switch now
	    if (!activate(this->fd, this->device_file, &this->var_screeninfo, width, height, pixelformat)) {
	        return false;
	    }

	    // check the result of activation
		if    ((width == (int)this->var_screeninfo.xres) && (height == (int)this->var_screeninfo.yres)
			&& (pixeldef.bits == (int)this->var_screeninfo.bits_per_pixel)) {

			printf("MMSFBDev: mode successfully switched to %dx%d, pixelformat %s (%d bits), %s\n",
					width, height, getMMSFBPixelFormatString(pixelformat).c_str(), pixeldef.bits,
					this->device_file.c_str());

			if (backbuffer) {
				if (this->var_screeninfo.yres_virtual < this->var_screeninfo.yres * (backbuffer+1)) {
					printf("MMSFBDev: buffer size %dx%d is to small (%dx%d requested), %s\n",
							this->var_screeninfo.xres_virtual, this->var_screeninfo.yres_virtual,
							this->var_screeninfo.xres, this->var_screeninfo.yres * (backbuffer+1),
							this->device_file.c_str());
					return false;
				}
			}
		}
		else {
			printf("MMSFBDev: mode switch to %dx%d, pixelformat %s (%d bits) failed, %s\n",
					width, height, getMMSFBPixelFormatString(pixelformat).c_str(), pixeldef.bits,
					this->device_file.c_str());
			return false;
		}

		// build the pixelformat
		if (!buildPixelFormat()) {
		    printf("MMSFBDev: unsupported pixelformat: r=%d(offs=%d), g=%d(offs=%d), b=%d(offs=%d), a=%d(offs=%d) (%d bit)\n",
					this->var_screeninfo.red.length,    this->var_screeninfo.red.offset,
					this->var_screeninfo.green.length,  this->var_screeninfo.green.offset,
					this->var_screeninfo.blue.length,   this->var_screeninfo.blue.offset,
					this->var_screeninfo.transp.length, this->var_screeninfo.transp.offset,
					this->var_screeninfo.bits_per_pixel);
			return false;
		}

		if (this->layers[0].pixelformat != pixelformat) {
			printf("MMSFBDev: pixelformat not correctly set, %s set, %s requested\n",
					getMMSFBPixelFormatString(this->layers[0].pixelformat).c_str(),
					getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////

bool MMSFBDev::vtOpen(int console) {
	// close the vt device if opened
	vtClose();

	// run a program in a new session
    setsid();

    // open the tty0 device
    this->vt.fd0 = open("/dev/tty0", O_RDONLY | O_NOCTTY);
    if (this->vt.fd0 < 0) {
    	if (errno == ENXIO) {
    		printf("MMSFBDev: virtual terminal not available (working without it)\n");
    		return true;
		}
		if (errno == ENOENT) {
			this->vt.fd0 = open("/dev/vc/0", O_RDONLY | O_NOCTTY);
			if (this->vt.fd0 < 0) {
				printf("MMSFBDev: opening device /dev/tty0 and /dev/vc/0 failed\n");
				return false;
			}
		}
		else {
			printf("MMSFBDev: opening device /dev/tty0 failed\n");
			return false;
		}
    }

    // get the current vt
    struct vt_stat vs;
    if (ioctl(this->vt.fd0, VT_GETSTATE, &vs) < 0) {
		printf("MMSFBDev: could not get vt state\n");
		vtClose();
		return false;
    }
    this->vt.previous = vs.v_active;
	printf("MMSFBDev: started from virtual terminal #%d\n", this->vt.previous);

	if (console >= 0) {
		this->vt.number = console;
	}
	else {
		// query the vt number which can be used
		int n = ioctl(this->vt.fd0, VT_OPENQRY, &this->vt.number);
		if ((n < 0) || (this->vt.number == -1)) {
			printf("MMSFBDev: query vt number failed\n");
			vtClose();
			return false;
		}
	}
	printf("MMSFBDev: using virtual terminal #%d\n", this->vt.number);

	// save original fb
    struct fb_con2fbmap c2f;
    c2f.console = this->vt.number;
    if (ioctl(this->fd, FBIOGET_CON2FBMAP, &c2f)) {
		printf("MMSFBDev: get original framebuffer failed for vt #%d\n", this->vt.number);
		vtClose();
		return false;
    }
    this->vt.org_fb = c2f.framebuffer;

    // set console for our fb
    struct stat fbs;
    if (fstat(this->fd, &fbs)) {
		printf("MMSFBDev: stat fb device failed\n");
	    this->vt.org_fb = -1;
		vtClose();
		return false;
    }
    c2f.framebuffer = (fbs.st_rdev & 0xFF) >> 5;
    c2f.console = this->vt.number;
    if (ioctl(this->fd, FBIOPUT_CON2FBMAP, &c2f) < 0) {
		printf("MMSFBDev: set console for framebuffer failed\n");
	    this->vt.org_fb = -1;
		vtClose();
		return false;
    }

	// switch to our vt
	while (ioctl(this->vt.fd0, VT_ACTIVATE, this->vt.number) < 0) {
		if (errno == EINTR) continue;
		printf("MMSFBDev: cannot switch (VT_ACTIVATE) to console #%d\n", this->vt.number);
		vtClose();
		return false;
	}
	while (ioctl(this->vt.fd0, VT_WAITACTIVE, this->vt.number) < 0) {
		if (errno == EINTR) continue;
		printf("MMSFBDev: cannot switch (VT_WAITACTIVE) to console #%d\n", this->vt.number);
		vtClose();
		return false;
	}
	usleep(50*1000);

    // open my tty device
	char tty[16];
    sprintf(tty, "/dev/tty%d", this->vt.number);
    this->vt.fd = open(tty, O_RDWR | O_NOCTTY);
    if (this->vt.fd < 0) {
		if (errno == ENOENT) {
			sprintf(tty, "/dev/vc/%d", this->vt.number);
			this->vt.fd = open(tty, O_RDWR | O_NOCTTY);
			if (this->vt.fd < 0) {
				printf("MMSFBDev: opening device /dev/tty%d and /dev/vc/%d failed\n", this->vt.number, this->vt.number);
				vtClose();
				return false;
			}
		}
		else {
			printf("MMSFBDev: opening device /dev/tty%d failed\n", this->vt.number);
			vtClose();
			return false;
		}
    }

    // attach to the device
    ioctl(this->vt.fd, TIOCSCTTY, 0);

    // switch cursor off
    const char cursor_off[] = "\033[?1;0;0c";
    write(this->vt.fd, cursor_off, sizeof(cursor_off));

    // put terminal into graphics mode
	ioctl(this->vt.fd, KDSETMODE, KD_GRAPHICS);

    // init keyboard
    ioctl(this->vt.fd, KDSKBMODE, K_MEDIUMRAW);
    ioctl(this->vt.fd, KDSKBLED, 0);
    tcgetattr(this->vt.fd, &this->saved_ts);
    struct termios ts;
    ts = this->saved_ts;
    ts.c_cc[VTIME] = 0;
    ts.c_cc[VMIN] = 1;
    ts.c_lflag &= ~(ICANON|ECHO|ISIG);
    ts.c_iflag = 0;
    tcsetattr(this->vt.fd, TCSAFLUSH, &ts);
    tcsetpgrp(this->vt.fd, getpgrp());

    return true;
}

void MMSFBDev::vtClose() {
	if (this->vt.fd != -1) {
		// close tty
		tcsetattr(this->vt.fd, TCSAFLUSH, &this->saved_ts);
		ioctl(this->vt.fd, KDSKBMODE, K_XLATE);
		ioctl(this->vt.fd, KDSETMODE, KD_TEXT);
	    const char cursor_on[] = "\033[?0;0;0c";
	    write(this->vt.fd, cursor_on, sizeof(cursor_on));
        close(this->vt.fd);
        this->vt.fd = -1;
	}

	if (this->vt.org_fb != -1) {
		// reset the fb for used console
	    struct fb_con2fbmap c2f;
	    c2f.framebuffer = this->vt.org_fb;
	    c2f.console = this->vt.number;
	    ioctl(this->fd, FBIOPUT_CON2FBMAP, &c2f);
		this->vt.org_fb = -1;
	}

	if (this->vt.previous != -1) {
		// switch back to previous vt
		ioctl(this->vt.fd0, VT_ACTIVATE, this->vt.previous);
		ioctl(this->vt.fd0, VT_WAITACTIVE, this->vt.previous);
		usleep(50*1000);
        ioctl(this->vt.fd0, VT_DISALLOCATE, this->vt.number);
		this->vt.number = -1;
		this->vt.previous = -1;
	}

	if (this->vt.fd0 != -1) {
		// close tty
        close(this->vt.fd0);
        this->vt.fd0 = -1;
	}
}

bool MMSFBDev::vtGetFd(int *fd) {
	if (this->vt.fd != -1) {
		*fd = this->vt.fd;
		return true;
	}
	return false;
}

#endif
