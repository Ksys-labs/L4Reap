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
#include "mmscore/mmsperfinterface.h"

#include <stdio.h>

MMSPerfInterface::MMSPerfInterface(class MMSPerf *mmsfbperf) : MMSServerInterface("MMSPerfInterface") {
	this->mmsfbperf = mmsfbperf;
}

MMSPerfInterface::~MMSPerfInterface() {
}

void MMSPerfInterface::printHeader(char **rb, const char *title) {
	(*rb)+= sprintf(*rb, "\n");
	if (title) (*rb)+= sprintf(*rb, "%s\n\n", title);
	(*rb)+= sprintf(*rb, "Function      T Dest PF  Src PF   Flags Calls  MegaPix   MicroSecs   MP Per Sec\n");
	(*rb)+= sprintf(*rb, "-------------------------------------------------------------------------------\n");
}

void MMSPerfInterface::printSummary(char **rb, MMSFBPERF_MEASURING_VALS &summary) {

	if (summary.usecs) {
		// list above is not empty
		(*rb)+= sprintf(*rb, "-------------------------------------------------------------------------------\n");
	}

	// build summary
	int cnt;
	memset(*rb, ' ', 256);

	cnt = sprintf(&(*rb)[0],  "Summary");
	(*rb)[0 + cnt]   = ' ';

	cnt = sprintf(&(*rb)[40],  "%d", summary.calls);
	(*rb)[40 + cnt]   = ' ';

	cnt = sprintf(&(*rb)[47],  "%d.%03d", summary.mpixels, summary.rpixels / 1000);
	(*rb)[47 + cnt]  = ' ';

	cnt = sprintf(&(*rb)[57],  "%d", summary.usecs);
	(*rb)[57 + cnt]  = ' ';

	cnt = sprintf(&(*rb)[69],  "%d", summary.mpps);
	(*rb)+= 69 + cnt;

	(*rb)+= sprintf((*rb), "\n");
}

bool MMSPerfInterface::processRequest(string *request, string *answer) {

	if (!this->mmsfbperf) {
		printf("DISKO: MMSPerf not initialized, disko compiled with enable_actmon=yes?\n");
		return false;
	}

	if (request->substr(0, 12) == "GET_STATINFO") {
		// command: GET_STATINFO
		bool reset = false;
		if (request->find("RESET(TRUE)") != string::npos) {
			// reset perf values?
			reset = true;
		}

		this->mmsfbperf->lock();

		// char buffer to return
		char retbuf[65536];
		char *rb = retbuf;

		printHeader(&rb, "REPORT: DRAWING ROUTINES");
		MMSFBPERF_MEASURING_VALS sum_drawing;
		sum_drawing.calls = sum_drawing.mpixels = sum_drawing.rpixels = sum_drawing.usecs = sum_drawing.mpps = 0;
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->fillrect, 	"FILLRECT", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_drawing);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->drawline, 	"DRAWLINE", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_drawing);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->drawstring, "DRAWSTRING", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_drawing);
		printSummary(&rb, sum_drawing);

		printHeader(&rb, "REPORT: BLITTING ROUTINES");
		MMSFBPERF_MEASURING_VALS sum_blitting;
		sum_blitting.calls = sum_blitting.mpixels = sum_blitting.rpixels = sum_blitting.usecs = sum_blitting.mpps = 0;
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->blit,		 "BLIT", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_blitting);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->stretchblit, "STRETCHBLIT", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_blitting);
		printSummary(&rb, sum_blitting);

		printHeader(&rb, "REPORT: BACKEND ROUTINES");
		MMSFBPERF_MEASURING_VALS sum_backend;
		sum_backend.calls = sum_backend.mpixels = sum_backend.rpixels = sum_backend.usecs = sum_backend.mpps = 0;
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->xshmputimage,  "XSHMPUTIMAGE", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_backend);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->xvshmputimage, "XVSHMPUTIMAGE", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_backend);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->vsync,         "VSYNC", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_backend);
		rb+= this->mmsfbperf->getPerfVals(&this->mmsfbperf->swapdisplay,   "SWAPDISPLAY", rb, sizeof(retbuf) - (unsigned int)(rb - retbuf), &sum_backend);
		printSummary(&rb, sum_backend);

		rb+= sprintf(rb, "\n\nREPORT: OVER ALL SUMMARY\n\n");
		MMSFBPERF_MEASURING_VALS sum;
		sum.calls = sum.mpixels = sum.rpixels = sum.usecs = sum.mpps = 0;
		this->mmsfbperf->addMeasuringVals(&sum, &sum_drawing);
		this->mmsfbperf->addMeasuringVals(&sum, &sum_blitting);
		this->mmsfbperf->addMeasuringVals(&sum, &sum_backend);
		rb+= sprintf(rb, "        MegaPix       = %u.%u\n", sum.mpixels, sum.rpixels / 1000);
		rb+= sprintf(rb, "        MP Per Sec    = %u\n", sum.mpps);
		rb+= sprintf(rb, "        Consumed time = %d.%03ds\n", sum.usecs / 1000000, (sum.usecs % 1000000) / 1000);
		unsigned int dur = this->mmsfbperf->getDuration();
		rb+= sprintf(rb, "        Duration      = %d.%ds\n", dur / 1000, dur % 1000);

		// finalize
		rb+= sprintf(rb, "\n");

		*answer = retbuf;

		this->mmsfbperf->unlock();

		// reset statistic values?
		if (reset) {
			this->mmsfbperf->reset();
		}

		return true;
	}

	// unknown command
	return false;
}
