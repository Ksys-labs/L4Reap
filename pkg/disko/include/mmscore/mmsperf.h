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

#ifndef MMSPERF_H_
#define MMSPERF_H_

#include "mmstools/mmsthread.h"
#include "mmsgui/fb/mmsfbsurface.h"

#if MMSFB_BLIT_CNT > MMSFB_DRAW_CNT
	#define MMSFBPERF_MAXFLAGS	MMSFB_BLIT_CNT
#else
	#define MMSFBPERF_MAXFLAGS	MMSFB_DRAW_CNT
#endif

typedef struct {
	unsigned int calls;
	unsigned int mpixels;
	unsigned int rpixels;
	unsigned int usecs;
	unsigned int mpps;
} MMSFBPERF_MEASURING_VALS;

typedef MMSFBPERF_MEASURING_VALS MMSFBPERF_MEASURING_LIST[2][MMSFB_PF_CNT][MMSFB_PF_CNT][MMSFBPERF_MAXFLAGS];


#define MMSFBPERF_MAXVKEYS	1024

typedef struct {
	unsigned int calls;
	unsigned int usecs;
} MMSFBPERF_MEASURING_VALS_VKEY;

typedef MMSFBPERF_MEASURING_VALS_VKEY MMSFBPERF_MEASURING_LIST_VKEY[MMSFBPERF_MAXVKEYS];


class MMSPerf {
private:

	//! already initialized?
	static bool initialized;

	//! make it thread-safe
	static MMSMutex lockme;

	//! start of measuring
	static struct timeval start_time;

	//! statistic for fill rectangle routines
	static MMSFBPERF_MEASURING_LIST fillrect;

	//! statistic for draw line routines
	static MMSFBPERF_MEASURING_LIST drawline;

	//! statistic for draw string routines
	static MMSFBPERF_MEASURING_LIST drawstring;

	//! statistic for blitting routines
	static MMSFBPERF_MEASURING_LIST blit;

	//! statistic for stretchblit routines
	static MMSFBPERF_MEASURING_LIST stretchblit;

	//! statistic for xshmputimage routines
	static MMSFBPERF_MEASURING_LIST xshmputimage;

	//! statistic for xvshmputimage routines
	static MMSFBPERF_MEASURING_LIST xvshmputimage;

	//! statistic for vsync routines
	static MMSFBPERF_MEASURING_LIST vsync;

	//! statistic for swapdisplay routines
	static MMSFBPERF_MEASURING_LIST swapdisplay;


	//! statistic for vkey routines
	static MMSFBPERF_MEASURING_LIST_VKEY vkey;


	void lock();
	void unlock();

	//! reset statistic infos
	void reset();

	unsigned int getDuration();

	void addMeasuringVals(MMSFBPERF_MEASURING_VALS *summary, MMSFBPERF_MEASURING_VALS *add_sum);

	void addMeasuringVals(MMSFBPERF_MEASURING_VALS_VKEY *summary, MMSFBPERF_MEASURING_VALS_VKEY *add_sum);

	int getPerfVals(MMSFBPERF_MEASURING_LIST *mlist, const char *prefix, char *retbuf, int retbuf_size,
					MMSFBPERF_MEASURING_VALS *summary);

	int getPerfVals(MMSFBPERF_MEASURING_LIST_VKEY *mlist, char *retbuf, int retbuf_size,
					MMSFBPERF_MEASURING_VALS_VKEY *summary);

	void stopMeasuring(struct timeval *perf_stime, MMSFBPERF_MEASURING_VALS *mvals,
					   int sw = 0, int sh = 0, int dw = 0, int dh = 0);

	void stopMeasuring(struct timeval *perf_stime, MMSFBPERF_MEASURING_VALS_VKEY *mvals);

public:
	MMSPerf();
	~MMSPerf();

	void startMeasuring(struct timeval *perf_stime);

	void stopMeasuringFillRectangle(struct timeval *perf_stime,
									MMSFBSurface *surface, int w, int h);

	void stopMeasuringDrawLine(struct timeval *perf_stime,
							   MMSFBSurface *surface, int pixels);

	void stopMeasuringDrawString(struct timeval *perf_stime,
								 MMSFBSurface *surface, int w, int h);

	void stopMeasuringBlit(struct timeval *perf_stime,
						   MMSFBSurface *surface,
						   MMSFBSurfacePixelFormat src_pixelformat,
						   int sw, int sh);

	void stopMeasuringStretchBlit(struct timeval *perf_stime,
								  MMSFBSurface *surface,
								  MMSFBSurfacePixelFormat src_pixelformat,
								  int sw, int sh, int dw, int dh);

	void stopMeasuringXShmPutImage(struct timeval *perf_stime,
									MMSFBSurface *surface,
									int sw, int sh);

	void stopMeasuringXvShmPutImage(struct timeval *perf_stime,
									MMSFBSurface *surface,
									int sw, int sh, int dw, int dh);

	void stopMeasuringVSync(struct timeval *perf_stime,
							MMSFBSurface *surface);

	void stopMeasuringSwapDisplay(struct timeval *perf_stime,
									MMSFBSurface *surface);

    friend class MMSPerfInterface;
};

#endif /* MMSPERF_H_ */
