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

#include "mmsgui/fb/mmsfbdevmatrox.h"
#include <sys/ioctl.h>
#include <cstring>

#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBDevMatrox is not initialized");return false;}

MMSFBDevMatrox::MMSFBDevMatrox() {
	this->scart_rgb_cable = false;
	this->tv_std_pal = true;
	this->mmio_base = NULL;
}

MMSFBDevMatrox::~MMSFBDevMatrox() {
	closeDevice();
}

bool MMSFBDevMatrox::openDevice(char *device_file, int console) {
	// open frame buffer
	if (!MMSFBDev::openDevice(device_file, console))
		return false;

	// check fb accel
    switch (this->fix_screeninfo.accel) {
		case FB_ACCEL_MATROX_MGAG400:
			// okay
			break;
		default:
			// not supported
			printf("MMSFBDevMatrox: unsupported accelerator %d (%.16s)\n", this->fix_screeninfo.accel, this->fix_screeninfo.id);
			closeDevice();
			return false;
    }

    // map mmio
	if (!mapMmio(&this->mmio_base)) {
		closeDevice();
		return false;
	}

	return true;
}

void MMSFBDevMatrox::closeDevice() {
	if (this->mmio_base)
		unmapMmio(this->mmio_base);

	// close frame buffer
	MMSFBDev::closeDevice();
}

bool MMSFBDevMatrox::waitForVSync() {
	// is initialized?
	INITCHECK;

	switch (this->active_screen) {
	case 0:
		// default fbdev primary screen 0
	    return MMSFBDev::waitForVSync();
	case 1: {
		// TVOut screen
		volatile unsigned char *mmio = this->mmio_base;
		int vdisplay = ((!this->tv_std_pal) ? 480/2 : 576/2) + 1;

#ifdef FBIO_WAITFORVSYNC
		static const int s = 1;
		if (ioctl(this->fd, FBIO_WAITFORVSYNC, &s)) {
			while ((int)(mga_in32(mmio, C2VCOUNT) & 0x00000fff) != vdisplay);
		}
#else
		while ((int)(mga_in32(mmio, C2VCOUNT) & 0x00000fff) != vdisplay);
#endif
		}
		return true;
	default:
    	printf("MMSFBDevMatrox: screen %d is not supported\n", this->active_screen);
    	break;
	}

	return false;
}

bool MMSFBDevMatrox::testLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
	    return MMSFBDev::testLayer(layer_id);
	case 2:
		// TVOut layer
		return true;
	default:
    	printf("MMSFBDevMatrox: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}


bool MMSFBDevMatrox::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		return MMSFBDev::initLayer(layer_id, width, height, pixelformat, backbuffer);
	case 2:
		// TVOut layer
		// check input
		if (width != 720) {
			printf("MMSFBDevMatrox: TVOut needs layer width 720, but %d given\n", width);
			return false;
		}
		if ((height != 576)&&(height != 480)) {
			printf("MMSFBDevMatrox: TVOut needs layer height 576 (PAL) or 480 (NTSC), but %d given\n", height);
			return false;
		}
		if ((pixelformat != MMSFB_PF_I420)&&(pixelformat != MMSFB_PF_YV12)) {
			printf("MMSFBDevMatrox: TVOut needs pixelformat I420 or YV12, but %s given\n", getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}
		if (backbuffer) {
	    	printf("MMSFBDevMatrox: TVOut layer does not support a backbuffer\n");
			return false;
		}

		// set values
		this->layers[layer_id].width = width;
		this->layers[layer_id].height = height;
		this->layers[layer_id].pixelformat = pixelformat;

		// save buffers
		memset(&this->layers[layer_id].buffers, 0, sizeof(this->layers[layer_id].buffers));
		this->layers[layer_id].buffers[0].ptr = this->framebuffer_base;
		if (width % 128)
			this->layers[layer_id].buffers[0].pitch = ((width / 128) + 1) * 128;
		else
			this->layers[layer_id].buffers[0].pitch = width;
		this->layers[layer_id].buffers[0].ptr2 = (char*)this->layers[layer_id].buffers[0].ptr + this->layers[layer_id].height * this->layers[layer_id].buffers[0].pitch;
		this->layers[layer_id].buffers[0].ptr3 = (char*)this->layers[layer_id].buffers[0].ptr2 + (this->layers[layer_id].height * this->layers[layer_id].buffers[0].pitch) / 2;
		this->layers[layer_id].buffers[0].pitch2 = this->layers[layer_id].buffers[0].pitch / 2;
		this->layers[layer_id].buffers[0].pitch3 = this->layers[layer_id].buffers[0].pitch2;
		this->layers[layer_id].buffers[0].hwbuffer = true;

		// pal or ntsc?
		this->tv_std_pal = (this->layers[layer_id].height == 576);

		// switch to layer
		buildCRTC2Regs();
		buildCRTC2Buffer();
		enableCRTC2();

		// layer is initialized
		this->layers[layer_id].isinitialized = true;

		// this layer is on screen 1
		this->active_screen = 1;

		printf("MMSFBDevMatrox: TVOut layer %d initialized with %dx%d (%s), pixelformat %s\n",
				layer_id, width, height, (this->tv_std_pal)?"PAL":"NTSC", getMMSFBPixelFormatString(pixelformat).c_str());

		return true;
	default:
		printf("MMSFBDevMatrox: layer %d is not supported\n", layer_id);
		break;
	}

	return false;
}


bool MMSFBDevMatrox::releaseLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevMatrox: layer %d cannot be released\n", layer_id);
		return false;
	case 2:
		// TVOut layer
		printf("MMSFBDevMatrox: layer %d cannot be released\n", layer_id);
		return false;
	default:
    	printf("MMSFBDevMatrox: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevMatrox::restoreLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevMatrox: layer %d cannot be restored\n", layer_id);
	    return false;
	case 2:
		// TVOut layer
		printf("MMSFBDevMatrox: layer %d cannot be restored\n", layer_id);
	    return false;
	default:
    	printf("MMSFBDevMatrox: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}


void MMSFBDevMatrox::buildCRTC2Regs() {

	// init g450/g550, but we do not support g400!
	this->crtc2_regs.c2ctrl = C2CTRL_C2PIXCLKSEL_CRISTAL;

	// we reset this register, so we clear also the bits of the sub-picture layer
	// we do not plan to use the sub-picture layer because of it's improper pixelformat
	this->crtc2_regs.c2datactrl = 0;

	// pal or ntsc?
	if (!this->tv_std_pal)
		this->crtc2_regs.c2datactrl |= C2DATACTRL_C2NTSCEN;

	// we do only support pixelformat MMSFB_PF_I420 or MMSFB_PF_YV12
	this->crtc2_regs.c2ctrl |= C2CTRL_C2DEPTH_YCBCR420;

	// set offset for interlaced but not separated mode
	this->crtc2_regs.c2offset = this->layers[2].buffers[0].pitch * 2;

	// set sync values dependent on tv standard pal/ntsc
	int hdisplay, htotal, vdisplay, vtotal;
	if (!this->tv_std_pal) {
		// ntsc
		hdisplay = 720;
		htotal = 858;
		vdisplay = 480 / 2;
		vtotal = 525 / 2;
	} else {
		// pal
		hdisplay = 720;
		htotal = 864;
		vdisplay = 576 / 2;
		vtotal = 625 / 2;
	}
	this->crtc2_regs.c2hparam = ((hdisplay - 8) << 16) | (htotal - 8);
	this->crtc2_regs.c2vparam = ((vdisplay - 1) << 16) | (vtotal - 1);
	this->crtc2_regs.c2misc = 0;
	this->crtc2_regs.c2misc |= (vdisplay + 1) << 16;
}

void MMSFBDevMatrox::setCRTC2Regs() {
	volatile unsigned char *mmio = this->mmio_base;

	mga_out32(mmio, this->crtc2_regs.c2ctrl,		C2CTRL);
	mga_out32(mmio, this->crtc2_regs.c2datactrl,	C2DATACTRL);
	mga_out32(mmio, this->crtc2_regs.c2hparam,		C2HPARAM);
	mga_out32(mmio, 0,							C2HSYNC);
	mga_out32(mmio, this->crtc2_regs.c2vparam,		C2VPARAM);
	mga_out32(mmio, 0,							C2VSYNC);
	mga_out32(mmio, this->crtc2_regs.c2offset,		C2OFFSET);
	mga_out32(mmio, this->crtc2_regs.c2misc,		C2MISC);
	mga_out32(mmio, 0,							C2PRELOAD);
}

void MMSFBDevMatrox::buildCRTC2Buffer() {
	// working in interlaced mode, so have to work with an field offset
	unsigned int field_offset = this->layers[2].buffers[0].pitch;

	// Y plane
	this->crtc2_regs.c2_plane1_start1 = this->fix_screeninfo.smem_start;
	this->crtc2_regs.c2_plane1_start0 = this->crtc2_regs.c2_plane1_start1 + field_offset;

	// interlaced, half field offset for U/Y planes
	field_offset /= 2;

	switch (this->layers[2].pixelformat) {
	case MMSFB_PF_I420:
		// U plane
		this->crtc2_regs.c2_plane2_start1 = this->crtc2_regs.c2_plane1_start1 + this->layers[2].height * this->layers[2].buffers[0].pitch;
		this->crtc2_regs.c2_plane2_start0 = this->crtc2_regs.c2_plane2_start1 + field_offset;
		// V plane
		this->crtc2_regs.c2_plane3_start1 = this->crtc2_regs.c2_plane2_start1 + (this->layers[2].height * this->layers[2].buffers[0].pitch) / 2;
		this->crtc2_regs.c2_plane3_start0 = this->crtc2_regs.c2_plane3_start1 + field_offset;
		break;
	case MMSFB_PF_YV12:
		// V plane
		this->crtc2_regs.c2_plane3_start1 = this->crtc2_regs.c2_plane1_start1 + this->layers[2].height * this->layers[2].buffers[0].pitch;
		this->crtc2_regs.c2_plane3_start0 = this->crtc2_regs.c2_plane3_start1 + field_offset;
		// U plane
		this->crtc2_regs.c2_plane2_start1 = this->crtc2_regs.c2_plane3_start1 + (this->layers[2].height * this->layers[2].buffers[0].pitch) / 2;
		this->crtc2_regs.c2_plane2_start0 = this->crtc2_regs.c2_plane2_start1 + field_offset;
		break;
	default:
		break;
	}
}


void MMSFBDevMatrox::setCRTC2Buffer() {
	volatile unsigned char *mmio = this->mmio_base;

	mga_out32(mmio, this->crtc2_regs.c2_plane1_start0, C2PLANE1START0);
	mga_out32(mmio, this->crtc2_regs.c2_plane1_start1, C2PLANE1START1);
	mga_out32(mmio, this->crtc2_regs.c2_plane2_start0, C2PLANE2START0);
	mga_out32(mmio, this->crtc2_regs.c2_plane2_start1, C2PLANE2START1);
	mga_out32(mmio, this->crtc2_regs.c2_plane3_start0, C2PLANE3START0);
	mga_out32(mmio, this->crtc2_regs.c2_plane3_start1, C2PLANE3START1);
}


void MMSFBDevMatrox::switchCRTC2(bool on) {
	volatile unsigned char *mmio = this->mmio_base;

	if (on)
		this->crtc2_regs.c2ctrl |= C2CTRL_C2EN;
	else
		this->crtc2_regs.c2ctrl &= ~C2CTRL_C2EN;
	mga_out32(mmio, this->crtc2_regs.c2ctrl, C2CTRL);

	if (on)
		this->crtc2_regs.c2ctrl &= ~C2CTRL_C2PIXCLKDIS;
	else
		this->crtc2_regs.c2ctrl |= C2CTRL_C2PIXCLKDIS;
	mga_out32(mmio, this->crtc2_regs.c2ctrl, C2CTRL);

	if (!on) {
		this->crtc2_regs.c2ctrl &= ~C2CTRL_C2INTERLACE;
		mga_out32(mmio, this->crtc2_regs.c2ctrl, C2CTRL);
	}
}


bool MMSFBDevMatrox::enableCRTC2() {
	volatile unsigned char *mmio = this->mmio_base;
	unsigned char val;

	val = mga_in_dac(mmio, XGENIOCTRL);
	val |= 0x40;
	mga_out_dac(mmio, val, XGENIOCTRL);
	val = mga_in_dac(mmio, XGENIODATA);
	val &= ~0x40;
	mga_out_dac(mmio, val, XGENIODATA);

	val = mga_in_dac(mmio, XPWRCTRL);
	val |= XPWRCTRL_DAC2PDN | XPWRCTRL_CFIFOPDN;
	mga_out_dac(mmio, val, XPWRCTRL);

	val = mga_in_dac(mmio, XDISPCTRL);
	val &= ~XDISPCTRL_DAC2OUTSEL_MASK;
	val |= XDISPCTRL_DAC2OUTSEL_TVE;
	mga_out_dac(mmio, val, XDISPCTRL);

	if (this->scart_rgb_cable) {
		val = mga_in_dac(mmio, XSYNCCTRL);
		val &= ~(XSYNCCTRL_DAC2HSOFF | XSYNCCTRL_DAC2VSOFF | XSYNCCTRL_DAC2HSPOL | XSYNCCTRL_DAC2VSPOL);
		mga_out_dac(mmio, val, XSYNCCTRL);
	}

	disableMaven();

	switchCRTC2(false);

	setCRTC2Regs();

	setCRTC2Buffer();

	switchCRTC2(true);

	setMavenRegs();

	this->crtc2_regs.c2ctrl |= C2CTRL_C2INTERLACE;
	this->crtc2_regs.c2ctrl |= 0x1000;
	while ((mga_in32(mmio, C2VCOUNT) & 0x00000fff) != 1);
	while ((mga_in32(mmio, C2VCOUNT) & 0x00000fff) != 0);
	mga_out32(mmio, this->crtc2_regs.c2ctrl, C2CTRL);

	enableMaven();

	return true;
}

bool MMSFBDevMatrox::disableCRTC2() {
	volatile unsigned char *mmio = this->mmio_base;
	unsigned char val;

	disableMaven();
	switchCRTC2(false);

	val = mga_in_dac(mmio, XGENIOCTRL);
	val &= ~0x40;
	mga_out_dac(mmio, val, XGENIOCTRL);
	val = mga_in_dac(mmio, XGENIODATA);
	val &= ~0x40;
	mga_out_dac(mmio, val, XGENIODATA);

	val = mga_in_dac(mmio, XPWRCTRL);
	val &= ~(XPWRCTRL_DAC2PDN | XPWRCTRL_CFIFOPDN);
	mga_out_dac(mmio, val, XPWRCTRL);

	val = mga_in_dac(mmio, XDISPCTRL);
	val &= ~XDISPCTRL_DAC2OUTSEL_MASK;
	val |= XDISPCTRL_DAC2OUTSEL_DIS;
	mga_out_dac(mmio, val, XDISPCTRL);

	return true;
}


void MMSFBDevMatrox::setMavenRegs() {
	volatile unsigned char *mmio = this->mmio_base;

 	if (this->tv_std_pal) {
 		// pal
 		maven_out8(mmio, 0x2a, 0x00);
 		maven_out8(mmio, 0x09, 0x01);
 		maven_out8(mmio, 0x8a, 0x02);
 		maven_out8(mmio, 0xcb, 0x03);
 		maven_out8(mmio, 0x00, 0x04);
 		maven_out8(mmio, 0x18, 0x2c);
 		maven_out8(mmio, 0x7e, 0x08);
 		maven_out8(mmio, 0x8a, 0x0a);
 		maven_out8(mmio, 0x3a, 0x09);
 		maven_out8(mmio, 0x1a, 0x29);
 		maven_out8(mmio, 0xb4, 0x31);
 		maven_out8(mmio, 0x00, 0x32);
 		maven_out8(mmio, 0x9c, 0x17);
 		maven_out8(mmio, 0x01, 0x18);
 		maven_out8(mmio, 0x38, 0x0b);
 		maven_out8(mmio, 0x28, 0x0c);
 		maven_out8(mmio, 0x00, 0x35);
 		maven_out8(mmio, 0x46, 0x10);
 		maven_out8(mmio, 0x01, 0x11);
 		maven_out8(mmio, 0x46, 0x0e);
 		maven_out8(mmio, 0x01, 0x0f);
 		maven_out8(mmio, 0xea, 0x1e);
 		maven_out8(mmio, 0x00, 0x1f);
 		maven_out8(mmio, 0xbb, 0x20);
 		maven_out8(mmio, 0xbb, 0x22);
 		maven_out8(mmio, 0x00, 0x25);
 		maven_out8(mmio, 0x49, 0x34);
 		maven_out8(mmio, 0x16, 0x33);
 		maven_out8(mmio, 0x00, 0x19);
 		maven_out8(mmio, 0x1a, 0x12);
 		maven_out8(mmio, 0x22, 0x3b);
 		maven_out8(mmio, 0x2a, 0x13);
 		maven_out8(mmio, 0x22, 0x39);
 		maven_out8(mmio, 0x05, 0x1d);
 		maven_out8(mmio, 0x02, 0x3a);
 		maven_out8(mmio, 0x00, 0x24);
 		maven_out8(mmio, 0x1c, 0x14);
 		maven_out8(mmio, 0x3d, 0x15);
 		maven_out8(mmio, 0x14, 0x16);
 		maven_out8(mmio, 0x07, 0x2d);
 		maven_out8(mmio, 0x7e, 0x2e);
 		maven_out8(mmio, 0x02, 0x2f);
 		maven_out8(mmio, 0x54, 0x30);
 		maven_out8(mmio, 0xfe, 0x1a);
 		maven_out8(mmio, 0x7e, 0x1b);
 		maven_out8(mmio, 0x60, 0x1c);
 		maven_out8(mmio, 0x00, 0x23);
 		maven_out8(mmio, 0x08, 0x26);
 		maven_out8(mmio, 0x00, 0x28);
 		maven_out8(mmio, 0x04, 0x27);
 		maven_out8(mmio, 0x07, 0x21);
 		maven_out8(mmio, 0x55, 0x2a);
 		maven_out8(mmio, 0x01, 0x2b);
 		maven_out8(mmio, 0x00, 0x35);
 		maven_out8(mmio, 0x46, 0x3c);
 		maven_out8(mmio, 0x00, 0x3d);
 		maven_out8(mmio, 0xb9, 0x37);
 		maven_out8(mmio, 0xdd, 0x38);

 		maven_out8(mmio, 0x17, 0x82);
 		maven_out8(mmio, 0x00, 0x83);
 		maven_out8(mmio, 0x01, 0x84);
 		maven_out8(mmio, 0x00, 0x85);
 	}
 	else {
 		// ntsc
 		maven_out8(mmio, 0x21, 0x00);
 		maven_out8(mmio, 0xf0, 0x01);
 		maven_out8(mmio, 0x7c, 0x02);
 		maven_out8(mmio, 0x1f, 0x03);
 		maven_out8(mmio, 0x00, 0x04);
 		maven_out8(mmio, 0x20, 0x2c);
 		maven_out8(mmio, 0x7e, 0x08);
 		maven_out8(mmio, 0x76, 0x0a);
 		maven_out8(mmio, 0x44, 0x09);
 		maven_out8(mmio, 0x11, 0x29);
 		maven_out8(mmio, 0xb4, 0x31);
 		maven_out8(mmio, 0x00, 0x32);
 		maven_out8(mmio, 0x83, 0x17);
 		maven_out8(mmio, 0x01, 0x18);
 		maven_out8(mmio, 0x49, 0x0b);
 		maven_out8(mmio, 0x00, 0x0c);
 		maven_out8(mmio, 0x00, 0x35);
 		maven_out8(mmio, 0x42, 0x10);
 		maven_out8(mmio, 0x03, 0x11);
 		maven_out8(mmio, 0x4e, 0x0e);
 		maven_out8(mmio, 0x03, 0x0f);
 		maven_out8(mmio, 0xea, 0x1e);
 		maven_out8(mmio, 0x00, 0x1f);
 		maven_out8(mmio, 0xae, 0x20);
 		maven_out8(mmio, 0xae, 0x22);
 		maven_out8(mmio, 0x00, 0x25);
 		maven_out8(mmio, 0x02, 0x34);
 		maven_out8(mmio, 0x14, 0x33);
 		maven_out8(mmio, 0x00, 0x19);
 		maven_out8(mmio, 0x17, 0x12);
 		maven_out8(mmio, 0x15, 0x3b);
 		maven_out8(mmio, 0x21, 0x13);
 		maven_out8(mmio, 0x15, 0x39);
 		maven_out8(mmio, 0x05, 0x1d);
 		maven_out8(mmio, 0x05, 0x3a);
 		maven_out8(mmio, 0x02, 0x24);
 		maven_out8(mmio, 0x1b, 0x14);
 		maven_out8(mmio, 0x1b, 0x15);
 		maven_out8(mmio, 0x24, 0x16);
 		maven_out8(mmio, 0x0f, 0x2d);
 		maven_out8(mmio, 0x78, 0x2e);
 		maven_out8(mmio, 0x00, 0x2f);
 		maven_out8(mmio, 0x00, 0x30);
 		maven_out8(mmio, 0x0f, 0x1a);
 		maven_out8(mmio, 0x0f, 0x1b);
 		maven_out8(mmio, 0x60, 0x1c);
 		maven_out8(mmio, 0x01, 0x23);
 		maven_out8(mmio, 0x0a, 0x26);
 		maven_out8(mmio, 0x00, 0x28);
 		maven_out8(mmio, 0x05, 0x27);
 		maven_out8(mmio, 0x04, 0x21);
 		maven_out8(mmio, 0xff, 0x2a);
 		maven_out8(mmio, 0x03, 0x2b);
 		maven_out8(mmio, 0x00, 0x35);
 		maven_out8(mmio, 0x42, 0x3c);
 		maven_out8(mmio, 0x03, 0x3d);
 		maven_out8(mmio, 0xbd, 0x37);
 		maven_out8(mmio, 0xda, 0x38);

 		maven_out8(mmio, 0x14, 0x82);
 		maven_out8(mmio, 0x00, 0x83);
 		maven_out8(mmio, 0x01, 0x84);
 		maven_out8(mmio, 0x00, 0x85);
 	}
}


void MMSFBDevMatrox::enableMaven() {
	volatile unsigned char *mmio = this->mmio_base;

	if (this->scart_rgb_cable) {
		// SCART RGB
		maven_out8(mmio, (this->tv_std_pal) ? 0x41 : 0x43, 0x80);
	}
	else {
		// Composite / S-Video
		maven_out8(mmio, (this->tv_std_pal) ? 0x01 : 0x03, 0x80);
	}

	maven_out8(mmio, 0x00, 0x3e);
}


void MMSFBDevMatrox::disableMaven() {
	volatile unsigned char *mmio = this->mmio_base;

	maven_out8(mmio, 0x01, 0x3e);
	maven_out8(mmio, 0x00, 0x80);
}

#endif
