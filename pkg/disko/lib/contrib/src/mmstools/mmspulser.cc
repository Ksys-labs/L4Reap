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

#include "mmstools/mmspulser.h"
#include "mmstools/tools.h"
#include "mmstools/mmserror.h"
#include <math.h>

MMSPulser::MMSPulser() : MMSThread("MMSPulser") {
	// animation is not running
	this->animRunning = false;

	// set stacksize to 128kb, will be allocated if started with start(true)
	setStacksize(128*1024);

	// set attributes
	setStepsPerSecond(25);
	setMaxCPUUsage(100);
	setMaxFrameRate(25);
	setMaxOffset(0);
	setDuration(0);

	// reset all other values
	reset();
}

MMSPulser::~MMSPulser() {
}

void MMSPulser::reset() {
	// reset all values
	this->cancel				= false;
	this->recalc_requested		= true;
	this->recalc_cnt			= 0;
	this->recalc_interval		= 3;
	this->step_len				= 0;
	this->offset				= 0;
	this->offset_curve			= 0;
	this->process_time			= 0;
	this->frame_delay			= 0;
	this->frame_rate			= 0;
	this->frames				= 0;
	this->times_buf_pos			= 0;
	this->times_buf_cnt			= 0;
	this->real_duration			= 0;
	this->onAnimation_counter 	= 0;

	if (this->max_offset > 0) {
		// if max offset is set, recalculate every loop
		this->recalc_interval = 1;
	}

	// use special seq_modes
	switch (this->seq_mode) {
	case MMSPULSER_SEQ_LINEAR:
	case MMSPULSER_SEQ_LOG_SOFT_START:
	case MMSPULSER_SEQ_LOG_SOFT_END:
	case MMSPULSER_SEQ_LOG_SOFT_START_AND_END:
		this->offset = 1;
		this->offset_curve = 0;
		calcCurve(this->offset, this->offset_curve);
		break;
	case MMSPULSER_SEQ_LINEAR_DESC:
	case MMSPULSER_SEQ_LOG_DESC_SOFT_START:
	case MMSPULSER_SEQ_LOG_DESC_SOFT_END:
	case MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END:
		this->offset = this->max_offset - 1;
		this->offset_curve = this->max_offset;
		calcCurve(this->offset, this->offset_curve);
		break;
	default:
		break;
	}
}

void MMSPulser::calcCurve(double &offset, double &offset_curve) {
	// curve calculation
	if (this->max_offset > 0) {
		switch (this->seq_mode) {
		case MMSPULSER_SEQ_LINEAR:
			if (this->seq_start <= 0)
				offset_curve = offset;
			else
				offset_curve = this->seq_start + (offset * this->seq_range) / this->max_offset;
			break;
		case MMSPULSER_SEQ_LINEAR_DESC:
			if (this->seq_start <= 0)
				offset_curve = offset;
			else
				offset_curve = (offset * this->seq_range) / this->max_offset;
			break;
		case MMSPULSER_SEQ_LOG_SOFT_START:
			// check offset, because log(1) is zero
			if (this->max_offset - offset > 1) {
				offset_curve = this->seq_start
								+ this->seq_range * (1 - (log(this->max_offset - offset) / this->max_offset_log));
			}
			else {
				// last offset
				offset_curve = this->max_offset;
			}
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START:
			// check offset, because log(1) is zero
			if (offset > 1) {
				offset_curve = this->seq_start
								- this->seq_range * (1 - (log(offset) / this->max_offset_log));
			}
			else {
				// last offset
				offset_curve = 0;
			}
			break;
		case MMSPULSER_SEQ_LOG_SOFT_END:
			// check offset, because log(1) is zero
			if (offset == 1) offset++;
			offset_curve = this->seq_start
							+ this->seq_range * (log(offset) / this->max_offset_log);
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_END:
			// check offset, because log(1) is zero
			if (this->max_offset - offset == 1) offset--;
			offset_curve = this->seq_start
							- this->seq_range * (log(this->max_offset - offset) / this->max_offset_log);
			break;
		case MMSPULSER_SEQ_LOG_SOFT_START_AND_END:
			if (offset_curve < this->max_offset / 2) {
				if (this->max_offset - offset > 0) {
					offset_curve = this->seq_start
									+ this->seq_range * (1 - (log(this->max_offset - offset) / this->max_offset_log));
					if (offset_curve >= this->max_offset / 2) {
						offset = this->max_offset - offset + 1;
						calcCurve(offset, offset_curve);
					}
				}
				else {
					// log(0)
					offset_curve = this->max_offset;
					offset = 1;
					calcCurve(offset, offset_curve);
				}
			}
			else {
				offset_curve = (this->max_offset - this->seq_range)
								+ this->seq_range * (log(offset) / this->max_offset_log);
			}
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END:
			if (offset_curve > this->max_offset / 2) {
				if (offset > 0) {
					offset_curve = (this->max_offset - this->seq_range)
									+ this->seq_range * (log(offset) / this->max_offset_log);
					if (offset_curve <= this->max_offset / 2) {
						offset = this->max_offset - offset - 1;
						calcCurve(offset, offset_curve);
					}
				}
				else {
					// log(0)
					offset_curve = 0;
					offset = this->max_offset - 1;
					calcCurve(offset, offset_curve);
				}
			}
			else {
				offset_curve = this->seq_range * (1 - (log(this->max_offset - offset) / this->max_offset_log));
			}
			break;

		default:
			offset_curve = 0;
			break;
		}
	}
	else
		offset_curve = 0;
}

void MMSPulser::threadMain() {

	// call connected onBeforeAnimation callbacks
    if (!this->onBeforeAnimation.emit(this)) {
    	// one of the connected callbacks has failed, do not start the animation
    	return;
    }

	// get start timestamp
	this->anim_start = getMTimeStamp();

	while (1) {
		// start and end timestamps for CPU average calculation
		unsigned int start_ts;
		unsigned int end_ts;

		// get start timestamp if needed
		if (this->recalc_requested) {
			start_ts = getMTimeStamp();
		}

		if (this->cancel) {
			// we should stop the animation
			break;
		}

		// call connected onAnimation callbacks
	    if (!this->onAnimation.emit(this)) {
	    	// one of the connected callbacks has "failed", stop the animation
		    break;
	    }
	    this->onAnimation_counter++;

		// recalc speed parameters?
		if (this->recalc_requested) {
			// get end timestamp
			end_ts = getMTimeStamp();

			// get process time
			// we collect up to MMSPULSER_TIMES_BUF_SIZE times in a ring buffer
			// and calculate the CPU average
			unsigned int times = 0;
			unsigned int diff = getMDiff(start_ts, end_ts);
			this->times_buf[times_buf_pos++] = diff;
			if (this->times_buf_pos >= MMSPULSER_TIMES_BUF_SIZE) this->times_buf_pos = 0;
			if (this->times_buf_cnt < MMSPULSER_TIMES_BUF_SIZE) this->times_buf_cnt++;
			for (unsigned int i = 0; i < this->times_buf_cnt; i++) times+= this->times_buf[i];
			this->process_time = (times + diff / 2) / this->times_buf_cnt;

			// calculate step length and frame delay
			int max_duration = 1000 / this->steps_per_second;
			if (this->seq_mode == MMSPULSER_SEQ_LOG_SOFT_START_AND_END || this->seq_mode == MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END) {
				// for "soft start and end" modes we have to divide max_duration in halves
				max_duration/= 2;
			}
			int total_time = (this->process_time * 100) / this->max_cpu_usage;
			this->step_len = (total_time + max_duration - 1) / max_duration;
			this->frame_delay = max_duration * this->step_len - this->process_time;
			if (this->frame_delay <= 0) this->frame_delay = 1;
			int frame_duration = this->frame_delay + this->process_time;
			this->frame_rate = 1000 / frame_duration;

			if (this->frame_rate > this->max_frame_rate) {
				// calculated frame rate is greater than max frame rate
				// so we have to calculate back the step length and the frame delay
				this->frame_delay = 1000 / this->max_frame_rate - this->process_time;
				frame_duration = this->frame_delay + this->process_time;
				this->step_len = (frame_duration + max_duration - 1) / max_duration;
				this->frame_delay = max_duration * this->step_len - this->process_time;
				if (this->frame_delay <= 0) this->frame_delay = 1;
				frame_duration = this->frame_delay + this->process_time;
				this->frame_rate = 1000 / frame_duration;
			}

			// mark as calculated
			this->recalc_cnt = 1;
			this->recalc_requested = (this->recalc_interval <= 1);
		}
		else {
			// recalc not needed
			this->recalc_cnt++;
			if (this->recalc_cnt >= this->recalc_interval) {
				// after recalc_interval times, recalc requested
				this->recalc_cnt = 0;
				this->recalc_requested = true;
			}
		}

		// sleeping...
        usleep((this->frame_delay>0)?this->frame_delay*1000:1000);

		// increase the frame counter
		this->frames++;

		// increase/decrease offset with step length
		switch (this->seq_mode) {
		case MMSPULSER_SEQ_LINEAR:
		case MMSPULSER_SEQ_LOG_SOFT_START:
		case MMSPULSER_SEQ_LOG_SOFT_END:
		case MMSPULSER_SEQ_LOG_SOFT_START_AND_END:
			this->offset+= this->step_len;
			break;
		case MMSPULSER_SEQ_LINEAR_DESC:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_END:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END:
			this->offset-= this->step_len;
			break;
		}

		// curve calculation
		calcCurve(this->offset, this->offset_curve);

    	// stop the animation?
    	bool stop = false;
    	if (this->max_offset > 0) {
    		if   ((this->seq_mode == MMSPULSER_SEQ_LINEAR)
				||(this->seq_mode == MMSPULSER_SEQ_LOG_SOFT_START)
				||(this->seq_mode == MMSPULSER_SEQ_LOG_SOFT_END)
    			||(this->seq_mode == MMSPULSER_SEQ_LOG_SOFT_START_AND_END)) {
    			// ascending modes
				if (this->offset_curve > 0) {
					if (this->offset_curve >= this->max_offset) {
						// offset is exceeded, stop the animation
						stop = true;
					}
				}
				else {
					if (this->offset >= this->max_offset) {
						// offset is exceeded, stop the animation
						stop = true;
					}
				}
    		}
    		else {
    			// descending modes
				if (this->offset_curve != 0) {
					if (this->offset_curve < 0) {
						// offset is exceeded, stop the animation
						stop = true;
					}
				}
				else {
					if (this->offset < 0) {
						// offset is exceeded, stop the animation
						stop = true;
					}
				}
    		}
    	}

    	// stop the animation?
    	if ((this->duration) && (this->real_duration > this->duration)) {
    		// requested duration reached, stop the animation
    		stop = true;
    	}

    	// get real animation duration
    	this->anim_end = getMTimeStamp();
    	this->real_duration = getMDiff(this->anim_start, this->anim_end);

    	if (stop) {
    		// break the loop
    		break;
    	}
	}

	// call connected onAfterAnimation callbacks
    this->onAfterAnimation.emit(this);
}

bool MMSPulser::start(bool separate_thread, bool wait) {
	// waiting for the end of the thread
	this->startlock.lock();
	while (isRunning()) {
		if (wait) {
			usleep(10000);
		}
		else {
			this->startlock.unlock();
			return false;
		}
	}

	// reset all values
	reset();

	if (separate_thread) {
		// start animation in a separate thread context
		bool ret = MMSThread::start();
		this->startlock.unlock();
		return ret;
	}
	else {
		// start animation in the context of the current thread
		this->animRunning = true;
		this->startlock.unlock();

		try {

			// do the animation
			threadMain();

		} catch(MMSError &error) {
		    DEBUGMSG(this->identity.c_str(), "Abort due to: " + error.getMessage());
		}

		this->animRunning = false;
	}

	return true;
}

bool MMSPulser::isRunning() {
	if (MMSThread::isRunning()) {
		// separate thread is running
		return true;
	}
	else {
		// check if running without separate thread
		return this->animRunning;
	}
}

void MMSPulser::stop() {
	this->startlock.lock();
	this->cancel = true;
	this->startlock.unlock();
}

bool MMSPulser::setStepsPerSecond(int steps_per_second) {
	// check & set
	if (steps_per_second <= 0) return false;
	if (steps_per_second > 255) return false;
	this->steps_per_second = steps_per_second;

	// recalculation requested
	recalc_cnt = 0;
	recalc_requested = true;

	return true;
}

int MMSPulser::getStepsPerSecond() {
	return this->steps_per_second;
}

bool MMSPulser::setMaxCPUUsage(int max_cpu_usage) {
	// check & set
	if (max_cpu_usage < 10) return false;
	if (max_cpu_usage > 100) return false;
	this->max_cpu_usage = max_cpu_usage;

	// recalculation requested
	recalc_cnt = 0;
	recalc_requested = true;

	return true;
}

int MMSPulser::getMaxCPUUsage() {
	return this->max_cpu_usage;
}

bool MMSPulser::setMaxFrameRate(int max_frame_rate) {
	// check & set
	if (max_frame_rate < 10) return false;
	if (max_frame_rate > 100) return false;
	this->max_frame_rate = max_frame_rate;

	// recalculation requested
	recalc_cnt = 0;
	recalc_requested = true;

	return true;
}

int MMSPulser::getMaxFrameRate() {
	return this->max_frame_rate;
}

int MMSPulser::getFrameRate() {
	return this->frame_rate;
}

int MMSPulser::getFrameDelay() {
	return this->frame_delay;
}

unsigned int MMSPulser::getFrames() {
	return this->frames;
}

int MMSPulser::getStepLength() {
	return this->step_len;
}

bool MMSPulser::setMaxOffset(double max_offset, MMSPULSER_SEQ seq_mode, double seq_range) {
	// check & set
	if ((max_offset < 2)&&(max_offset != 0)) return false;
	if ((seq_range < 2)&&(seq_range != 0)) return false;
	if (seq_range > max_offset) return false;
	this->max_offset = max_offset;
	this->seq_mode = seq_mode;
	this->seq_range = seq_range;

	// get the start point of the sequence
	if (this->seq_range <= 0) {
		// full, default
		switch (this->seq_mode) {
		case MMSPULSER_SEQ_LINEAR:
		case MMSPULSER_SEQ_LOG_SOFT_START:
		case MMSPULSER_SEQ_LOG_SOFT_END:
			this->seq_start = 0;
			this->seq_range = this->max_offset;
			break;
		case MMSPULSER_SEQ_LOG_SOFT_START_AND_END:
			this->seq_start = 0;
			this->seq_range = this->max_offset / 2;
			break;
		case MMSPULSER_SEQ_LINEAR_DESC:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_END:
			this->seq_start = this->max_offset;
			this->seq_range = this->max_offset;
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END:
			this->seq_start = this->max_offset;
			this->seq_range = this->max_offset / 2;
			break;
		}
	}
	else {
		// sequence should be only a little part of the max_offset range
		switch (this->seq_mode) {
		case MMSPULSER_SEQ_LINEAR:
		case MMSPULSER_SEQ_LOG_SOFT_END:
			this->seq_start = this->max_offset - this->seq_range;
			break;
		case MMSPULSER_SEQ_LINEAR_DESC:
		case MMSPULSER_SEQ_LOG_DESC_SOFT_END:
			this->seq_start = this->seq_range;
			break;
		case MMSPULSER_SEQ_LOG_SOFT_START:
			this->seq_start = 0;
			break;
		case MMSPULSER_SEQ_LOG_SOFT_START_AND_END:
			this->seq_start = 0;
			this->seq_range/= 2;
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START:
			this->seq_start = this->max_offset;
			break;
		case MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END:
			this->seq_start = this->max_offset;
			this->seq_range/= 2;
			break;
		}
	}

	// get the natural logarithm
	if (this->max_offset >= 2) {
		this->max_offset_log = log(this->max_offset);
	}
	else {
		this->max_offset_log = 0;
	}

	return true;
}

double MMSPulser::getOffset() {
	return this->offset_curve;
}

bool MMSPulser::setDuration(unsigned int duration) {
	this->duration = duration;
	return true;
}

unsigned int MMSPulser::getDuration() {
	return this->duration;
}

unsigned int MMSPulser::getRealDuration() {
	return this->real_duration;
}

unsigned int MMSPulser::getOnAnimationCounter() {
	return this->onAnimation_counter;
}
