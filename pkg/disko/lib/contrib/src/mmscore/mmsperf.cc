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

#include "mmscore/mmsperf.h"
#include <stdio.h>
#include <sys/time.h>


// static variables
bool MMSPerf::initialized	= false;
MMSMutex MMSPerf::lockme;
struct timeval MMSPerf::start_time;
MMSFBPERF_MEASURING_LIST MMSPerf::fillrect;
MMSFBPERF_MEASURING_LIST MMSPerf::drawline;
MMSFBPERF_MEASURING_LIST MMSPerf::drawstring;
MMSFBPERF_MEASURING_LIST MMSPerf::blit;
MMSFBPERF_MEASURING_LIST MMSPerf::stretchblit;
MMSFBPERF_MEASURING_LIST MMSPerf::xshmputimage;
MMSFBPERF_MEASURING_LIST MMSPerf::xvshmputimage;
MMSFBPERF_MEASURING_LIST MMSPerf::vsync;
MMSFBPERF_MEASURING_LIST MMSPerf::swapdisplay;
MMSFBPERF_MEASURING_LIST_VKEY MMSPerf::vkey;


MMSPerf::MMSPerf() {
	if (!this->initialized) {
		// first initialization
		reset();
		this->initialized = true;
	}
}

MMSPerf::~MMSPerf() {
}

void MMSPerf::lock() {
	lockme.lock();
}

void MMSPerf::unlock() {
	lockme.unlock();
}

void MMSPerf::reset() {
	// reset statistic infos
	lock();
	memset(this->fillrect, 0, sizeof(this->fillrect));
	memset(this->drawline, 0, sizeof(this->drawline));
	memset(this->drawstring, 0, sizeof(this->drawstring));
	memset(this->blit, 0, sizeof(this->blit));
	memset(this->stretchblit, 0, sizeof(this->stretchblit));
	memset(this->xshmputimage, 0, sizeof(this->xshmputimage));
	memset(this->xvshmputimage, 0, sizeof(this->xvshmputimage));
	memset(this->vsync, 0, sizeof(this->vsync));
	memset(this->swapdisplay, 0, sizeof(this->swapdisplay));
	memset(this->vkey, 0, sizeof(this->vkey));
	gettimeofday(&this->start_time, NULL);
	unlock();
}

unsigned int MMSPerf::getDuration() {
	struct timeval end_time;
	gettimeofday(&end_time, NULL);

	unsigned int ms = (end_time.tv_sec - this->start_time.tv_sec) * 1000;
	if (end_time.tv_usec >= this->start_time.tv_usec)
		ms+= (end_time.tv_usec - this->start_time.tv_usec) / 1000;
	else
		ms-= (this->start_time.tv_usec - end_time.tv_usec) / 1000;
	if (ms == 0) ms = 1;

	return ms;
}

void MMSPerf::addMeasuringVals(MMSFBPERF_MEASURING_VALS *summary, MMSFBPERF_MEASURING_VALS *add_sum) {
	// add sum
	summary->calls+= add_sum->calls;
	summary->usecs+= add_sum->usecs;

	if (add_sum->mpixels > 0 && add_sum->rpixels > 0) {
		// re-calculate m/r pixels
		summary->mpixels+= add_sum->mpixels;	lock();

		summary->rpixels+= add_sum->rpixels;
		summary->mpixels+= summary->rpixels / 1000000;
		summary->rpixels%= 1000000;

		// re-calculate mpps
		summary->mpps = summary->mpixels * 1000;
		if (summary->usecs > 1000) summary->mpps/= summary->usecs / 1000;
		if (summary->usecs > 0) summary->mpps+= summary->rpixels / summary->usecs;
	}
}

void MMSPerf::addMeasuringVals(MMSFBPERF_MEASURING_VALS_VKEY *summary, MMSFBPERF_MEASURING_VALS_VKEY *add_sum) {
	// add sum
	summary->calls+= add_sum->calls;
	summary->usecs+= add_sum->usecs;
}

int MMSPerf::getPerfVals(MMSFBPERF_MEASURING_LIST *mlist, const char *prefix, char *retbuf, int retbuf_size,
						   MMSFBPERF_MEASURING_VALS *summary) {
	char *retbuf_start=retbuf;
	char *retbuf_end = retbuf + retbuf_size;

	for (unsigned int buftype = 0; buftype < 2; buftype++) {
		for (unsigned int pf_cnt = 0; pf_cnt < MMSFB_PF_CNT; pf_cnt++) {
			for (unsigned int src_pf_cnt = 0; src_pf_cnt < MMSFB_PF_CNT; src_pf_cnt++) {
				for (unsigned int flags_cnt = 0; flags_cnt < MMSFBPERF_MAXFLAGS; flags_cnt++) {
					// get access to the infos and check if used
					MMSFBPERF_MEASURING_VALS *mv = &(*mlist)[buftype][pf_cnt][src_pf_cnt][flags_cnt];
					if (!mv->usecs) {
						// unused combination
						continue;
					}

					if (summary) {
						// add current measuring values to summary
						addMeasuringVals(summary, mv);
					}

					// fill the info line
					char buf[256];
					int cnt;
					memset(buf, ' ', sizeof(buf));

					cnt = sprintf(&buf[0],   "%s", prefix);
					buf[0 + cnt]   = ' ';

					cnt = sprintf(&buf[14],  "%c", (buftype)?'X':'O');
					buf[14 + cnt]   = ' ';

					cnt = sprintf(&buf[16],  "%s", getMMSFBPixelFormatString((MMSFBSurfacePixelFormat)pf_cnt).c_str());
					buf[16 + cnt]   = ' ';

					cnt = sprintf(&buf[25],  "%s", getMMSFBPixelFormatString((MMSFBSurfacePixelFormat)src_pf_cnt).c_str());
					buf[25 + cnt]   = ' ';

					cnt = sprintf(&buf[34],  "%05x", flags_cnt);
					buf[34 + cnt]   = ' ';

					cnt = sprintf(&buf[40],  "%d", mv->calls);
					buf[40 + cnt]   = ' ';

					cnt = sprintf(&buf[47],  "%d.%03d", mv->mpixels, mv->rpixels / 1000);
					buf[47 + cnt]  = ' ';

					cnt = sprintf(&buf[57],  "%d", mv->usecs);
					buf[57 + cnt]  = ' ';

					cnt = sprintf(&buf[69],  "%d", mv->mpps);
					cnt+=69;

					// print it to retbuf
					if (retbuf + cnt + 1 <= retbuf_end) {
						memcpy(retbuf, buf, cnt);
						retbuf+=cnt;
						*retbuf = '\n';
						retbuf++;
						*retbuf = 0;
					}
					else {
						// retbuf is full
						return -1;
					}
				}
			}
		}
	}

	return (int)(retbuf - retbuf_start);
}

int MMSPerf::getPerfVals(MMSFBPERF_MEASURING_LIST_VKEY *mlist, char *retbuf, int retbuf_size,
						   MMSFBPERF_MEASURING_VALS_VKEY *summary) {
	char *retbuf_start=retbuf;
	char *retbuf_end = retbuf + retbuf_size;

	for (unsigned int vk = 0; vk < MMSFBPERF_MAXVKEYS; vk++) {
		// get access to the infos and check if used
		MMSFBPERF_MEASURING_VALS_VKEY *mv = &(*mlist)[vk];
		if (!mv->usecs) {
			// unused combination
			continue;
		}

		if (summary) {
			// add current measuring values to summary
			addMeasuringVals(summary, mv);
		}

		// fill the info line
		char buf[256];
		int cnt;
		memset(buf, ' ', sizeof(buf));
/*
		cnt = sprintf(&buf[0],   "%s", prefix);
		buf[0 + cnt]   = ' ';

		cnt = sprintf(&buf[14],  "%c", (buftype)?'X':'O');
		buf[14 + cnt]   = ' ';

		cnt = sprintf(&buf[16],  "%s", getMMSFBPixelFormatString((MMSFBSurfacePixelFormat)pf_cnt).c_str());
		buf[16 + cnt]   = ' ';

		cnt = sprintf(&buf[25],  "%s", getMMSFBPixelFormatString((MMSFBSurfacePixelFormat)src_pf_cnt).c_str());
		buf[25 + cnt]   = ' ';

		cnt = sprintf(&buf[34],  "%05x", flags_cnt);
		buf[34 + cnt]   = ' ';

		cnt = sprintf(&buf[40],  "%d", mv->calls);
		buf[40 + cnt]   = ' ';

		cnt = sprintf(&buf[47],  "%d.%03d", mv->mpixels, mv->rpixels / 1000);
		buf[47 + cnt]  = ' ';

		cnt = sprintf(&buf[57],  "%d", mv->usecs);
		buf[57 + cnt]  = ' ';

		cnt = sprintf(&buf[69],  "%d", mv->mpps);
		cnt+=69;
*/
		// print it to retbuf
		if (retbuf + cnt + 1 <= retbuf_end) {
			memcpy(retbuf, buf, cnt);
			retbuf+=cnt;
			*retbuf = '\n';
			retbuf++;
			*retbuf = 0;
		}
		else {
			// retbuf is full
			return -1;
		}
	}

	return (int)(retbuf - retbuf_start);
}

void MMSPerf::stopMeasuring(struct timeval *perf_stime, MMSFBPERF_MEASURING_VALS *mvals,
							  int sw, int sh, int dw, int dh) {
	// get stop time
	struct timeval perf_etime;
	gettimeofday(&perf_etime, NULL);

	lock();

	// count calls
	mvals->calls++;

	// calculate pixels
	if (dw <= 0 || dh <= 0)
		mvals->rpixels+= sw * sh;
	else
		mvals->rpixels+= ((sw + dw) / 2) * ((sh + dh) / 2);
	if (mvals->rpixels > 1000000) {
		mvals->mpixels+= mvals->rpixels / 1000000;
		mvals->rpixels%= 1000000;
	}

	// calculate time
	mvals->usecs+= (perf_etime.tv_sec - perf_stime->tv_sec) * 1000000;
	if (perf_etime.tv_usec >= perf_stime->tv_usec)
		mvals->usecs+= perf_etime.tv_usec - perf_stime->tv_usec;
	else
		mvals->usecs-= perf_stime->tv_usec - perf_etime.tv_usec;
	if (mvals->usecs == 0) mvals->usecs = 1;

	// calculate mpps (mega pixel per second)
	mvals->mpps= (mvals->mpixels * 1000000 + mvals->rpixels) / mvals->usecs;

	unlock();
}

void MMSPerf::stopMeasuring(struct timeval *perf_stime, MMSFBPERF_MEASURING_VALS_VKEY *mvals) {
	// get stop time
	struct timeval perf_etime;
	gettimeofday(&perf_etime, NULL);

	lock();

	// count calls
	mvals->calls++;

	// calculate time
	mvals->usecs+= (perf_etime.tv_sec - perf_stime->tv_sec) * 1000000;
	if (perf_etime.tv_usec >= perf_stime->tv_usec)
		mvals->usecs+= perf_etime.tv_usec - perf_stime->tv_usec;
	else
		mvals->usecs-= perf_stime->tv_usec - perf_etime.tv_usec;
	if (mvals->usecs == 0) mvals->usecs = 1;

	unlock();
}

void MMSPerf::startMeasuring(struct timeval *perf_stime) {
	// get start time
	gettimeofday(perf_stime, NULL);
}

void MMSPerf::stopMeasuringFillRectangle(struct timeval *perf_stime,
										   MMSFBSurface *surface, int w, int h) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	MMSFBDrawingFlags drawingflags = surface->config.drawingflags;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if ((pixelformat < MMSFB_PF_CNT) && (drawingflags < MMSFBPERF_MAXFLAGS))
		stopMeasuring(perf_stime, &this->fillrect[buftype][pixelformat][MMSFB_PF_NONE][drawingflags], w, h);
}

void MMSPerf::stopMeasuringDrawLine(struct timeval *perf_stime,
									  MMSFBSurface *surface, int pixels) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	MMSFBDrawingFlags drawingflags = surface->config.drawingflags;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if ((pixelformat < MMSFB_PF_CNT) && (drawingflags < MMSFBPERF_MAXFLAGS))
		stopMeasuring(perf_stime, &this->drawline[buftype][pixelformat][MMSFB_PF_NONE][drawingflags], pixels, 1);
}

void MMSPerf::stopMeasuringDrawString(struct timeval *perf_stime,
										MMSFBSurface *surface, int w, int h) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	MMSFBDrawingFlags drawingflags = surface->config.drawingflags;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if ((pixelformat < MMSFB_PF_CNT) && (drawingflags < MMSFBPERF_MAXFLAGS))
		stopMeasuring(perf_stime, &this->drawstring[buftype][pixelformat][MMSFB_PF_NONE][drawingflags], w, h);
}

void MMSPerf::stopMeasuringBlit(struct timeval *perf_stime,
								  MMSFBSurface *surface,
								  MMSFBSurfacePixelFormat src_pixelformat,
								  int sw, int sh) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	MMSFBBlittingFlags blittingflags = surface->config.blittingflags;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if ((pixelformat < MMSFB_PF_CNT) && (src_pixelformat < MMSFB_PF_CNT) && (blittingflags < MMSFBPERF_MAXFLAGS))
		stopMeasuring(perf_stime, &this->blit[buftype][pixelformat][src_pixelformat][blittingflags], sw, sh);
}

void MMSPerf::stopMeasuringStretchBlit(struct timeval *perf_stime,
										 MMSFBSurface *surface,
										 MMSFBSurfacePixelFormat src_pixelformat,
										 int sw, int sh, int dw, int dh) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	MMSFBBlittingFlags blittingflags = surface->config.blittingflags;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if ((pixelformat < MMSFB_PF_CNT) && (src_pixelformat < MMSFB_PF_CNT) && (blittingflags < MMSFBPERF_MAXFLAGS))
		stopMeasuring(perf_stime, &this->stretchblit[buftype][pixelformat][src_pixelformat][blittingflags], sw, sh, dw, dh);
}

void MMSPerf::stopMeasuringXShmPutImage(struct timeval *perf_stime,
											MMSFBSurface *surface,
											int sw, int sh) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if (pixelformat < MMSFB_PF_CNT)
		stopMeasuring(perf_stime, &this->xshmputimage[buftype][pixelformat][pixelformat][MMSFB_BLIT_NOFX], sw, sh);
}

void MMSPerf::stopMeasuringXvShmPutImage(struct timeval *perf_stime,
											MMSFBSurface *surface,
											int sw, int sh, int dw, int dh) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if (pixelformat < MMSFB_PF_CNT)
		stopMeasuring(perf_stime, &this->xvshmputimage[buftype][pixelformat][pixelformat][MMSFB_BLIT_NOFX], sw, sh, dw, dh);
}

void MMSPerf::stopMeasuringVSync(struct timeval *perf_stime,
									MMSFBSurface *surface) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if (pixelformat < MMSFB_PF_CNT)
		stopMeasuring(perf_stime, &this->vsync[buftype][pixelformat][MMSFB_PF_NONE][MMSFB_BLIT_NOFX]);
}

void MMSPerf::stopMeasuringSwapDisplay(struct timeval *perf_stime,
											MMSFBSurface *surface) {

	// stop measuring for specified pixelformat and flags
	MMSFBSurfacePixelFormat pixelformat = surface->config.surface_buffer->pixelformat;
	unsigned int buftype = (!surface->config.surface_buffer->buffers[0].hwbuffer && !surface->config.surface_buffer->external_buffer)?0:1;
	if (pixelformat < MMSFB_PF_CNT)
		stopMeasuring(perf_stime, &this->swapdisplay[buftype][pixelformat][MMSFB_PF_NONE][MMSFB_BLIT_NOFX]);
}

