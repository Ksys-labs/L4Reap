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

#ifndef MMSPULSER_H_
#define MMSPULSER_H_

#include "mmstools/mmsthread.h"
#include "mmstools/mmstypes.h"

// size of the times buffer needed for calculation of the CPU average
#define MMSPULSER_TIMES_BUF_SIZE 10

//! Sequence mode for onAnimation callback.
/*!
Per default the sequence mode is set to MMSPULSER_OFFSET_LINEAR.
You can change this mode with the setMaxOffset() method. This has effect to
the return value of getOffset() which you should use in the onAnimation callback.
\see setMaxOffset()
\see getOffset()
*/
typedef enum {
	//! linear ascending
	MMSPULSER_SEQ_LINEAR = 0,
	//! linear descending
	MMSPULSER_SEQ_LINEAR_DESC,
	//! logarithmic ascending, animation starts smoothly and ends fast
	MMSPULSER_SEQ_LOG_SOFT_START,
	//! logarithmic descending, animation starts smoothly and ends fast
	MMSPULSER_SEQ_LOG_DESC_SOFT_START,
	//! logarithmic ascending, animation starts fast and ends smoothly
	MMSPULSER_SEQ_LOG_SOFT_END,
	//! logarithmic descending, animation starts fast and ends smoothly
	MMSPULSER_SEQ_LOG_DESC_SOFT_END,
	//! logarithmic ascending, animation starts smoothly and ends smoothly
	MMSPULSER_SEQ_LOG_SOFT_START_AND_END,
	//! logarithmic descending, animation starts smoothly and ends smoothly
	MMSPULSER_SEQ_LOG_DESC_SOFT_START_AND_END
} MMSPULSER_SEQ;



//! This class helps the MMSGUI and user specific applications to get smooth animations.
/*!
There are three callbacks: onBeforeAnimation, onAnimation and onAfterAnimation.
At least the onAnimation callback should be connected. This callback will be called
for each frame which is to draw.
The onBeforeAnimation callback can be used to setup something.
The onAfterAnimation callback will be called, if the animation is finished.
The initiator can be blocked during the animation (see start(false)). But it is also
possible that the animation is running in a separate thread (see start(true)).
\author Jens Schneider
*/
class MMSPulser : public MMSThread {
    private:
		//! helper mutex to perform a safe start
		MMSMutex	startlock;

		//! shows if the animation is running
    	bool	animRunning;

    	//! true if animation thread should stop
		bool	cancel;

    	//! recalculation requested?
    	bool	recalc_requested;

        //! after X loops, recalculate animation values
    	int		recalc_interval;

    	//! recalc loop counter
        int		recalc_cnt;

        //! current step length
    	int		step_len;

        //! maximum offset
    	double	max_offset;

    	//! natural logarithm of max_offset
    	double	max_offset_log;

    	//! sequence mode, used in conjunction with max_offset
    	MMSPULSER_SEQ seq_mode;

    	//! sequence range, used in conjunction with max_offset and seq_mode;
    	double	seq_range;

    	//! sequence start, used in conjunction with seq_range;
    	double	seq_start;

        //! current offset
    	double	offset;

        //! current offset, curve calculation in conjunction with seq_mode
    	double	offset_curve;

    	//! animation steps per second
    	int		steps_per_second;

        //! maximum cpu usage in percent
    	int		max_cpu_usage;

    	//! duration of the onAnimation callback
    	int		process_time;

    	//! delay in milliseconds between two frames
    	int		frame_delay;

    	//! maximum frame rate
    	int		max_frame_rate;

    	//! current frame rate
    	int		frame_rate;

    	//! frame counter
    	unsigned int frames;

    	//! ring buffer to calculate the CPU average
    	unsigned int times_buf[MMSPULSER_TIMES_BUF_SIZE];

    	//! next pos in the ring buffer to write to
    	unsigned int times_buf_pos;

    	//! filling level of the ring buffer
    	unsigned int times_buf_cnt;

    	//! start timestamp of the animation
    	unsigned int anim_start;

    	//! end timestamp of the animation
    	unsigned int anim_end;

    	//! requested duration in milliseconds of the animation
    	unsigned int duration;

    	//! real duration in milliseconds of the animation (calculated after the animation)
    	unsigned int real_duration;

    	//! counts the onAnimation calls, zero for the first onAnimation call
    	unsigned int onAnimation_counter;

    	//! Internal method: Reset all values.
    	void reset();

    	//! Internal method: Calculate the offset curve.
    	void calcCurve(double &offset, double &offset_curve);

    	//! Internal method: Main method for the animation timing control.
		void threadMain();

    public:
    	//! The constructor.
    	MMSPulser();

    	//! The destructor.
    	~MMSPulser();

        //! Start the animation.
        /*!
        \param separate_thread	true:  a new thread will be started,
                                false: the animation is running in callers thread context
                                       and the start() method will return if the animation is finished
        \param wait				waiting for end of animation and then start the new one
        \return true or false
        */
		bool start(bool separate_thread = true, bool wait = false);

        //! Check if the animation is running.
        /*!
        \return true or false
        */
		bool isRunning();

        //! Signals the animation thread that it has to stop immediately.
		void stop();

        //! Set animation steps per second.
        /*!
        \param steps_per_second 1..255 steps per second, default are 25 steps
        \return true, if parameter is accepted
        \see getStepsPerSecond()
        */
		bool setStepsPerSecond(int steps_per_second);

        //! Get steps per second.
        /*!
        \return steps per second
        \see setStepsPerSecond()
        */
		int getStepsPerSecond();

        //! Set maximum CPU usage.
        /*!
        \param max_cpu_usage 10..100 percent, default is 75%
        \return true, if parameter is accepted
        \see getMaxCPUUsage()
        */
		bool setMaxCPUUsage(int max_cpu_usage);

        //! Get maximum CPU usage.
        /*!
        \return max CPU usage
        \see setMaxCPUUsage()
        */
		int getMaxCPUUsage();

        //! Set maximum frame rate.
        /*!
        \param max_frame_rate 10..100 frames per second, default are 25 fps
        \return true, if parameter is accepted
        \see getMaxFrameRate()
        */
		bool setMaxFrameRate(int max_frame_rate);

        //! Get maximum frame rate.
        /*!
        \return max frame rate
        \see setMaxFrameRate()
        */
		int getMaxFrameRate();

		//! Get the current frame rate.
        /*!
        \return current frame rate
        \note This value is valid during the animation (e.g. in the onAnimation callback).
        */
		int getFrameRate();

		//! Get the delay in milliseconds between two frames.
        /*!
        \return delay in milliseconds
        \note This value is valid during the animation (e.g. in the onAnimation callback).
        */
		int getFrameDelay();

		//! Get the frame counter.
        /*!
        \return frame counter
        \note This value is zero for the first time onAnimation callback is called.
        */
		unsigned int getFrames();

		//! Get the length of the current animation step.
        /*!
        \return length of the step
        \note This value is valid during the animation (e.g. in the onAnimation callback).
        \note This value is zero for the first time onAnimation callback is called.
        \see getOffset()
        */
		int getStepLength();

        //! Set the maximum offset returned by getOffset().
        /*!
        The animation will be stopped, if the maximum offset is exceeded.
        Additionally you can set the sequence mode and a sequence range. With these parameters
        you have influence to the return value of getOffset().
        \param max_offset	maximum offset (2..n), default 0 means that no maximum is set
							if maximum offset is not set (value 0), the seq_mode and seq_range will be ignored
        \param seq_mode		sequence mode, default is MMSPULSER_SEQ_LINEAR
        \param seq_range	the seq_range for the curve calculation (2..max_offset), default 0 is equal to max_offset
        \return true, if parameters are accepted
        \note If your onAnimation callback returns false, the Animation will be stopped at any time.
        \note The seq_mode has NO influence to the return value of getStepLength().
        \note With the parameter seq_range, you can get a flat curve at the end of the animation.
        \see getOffset()
        */
		bool setMaxOffset(double max_offset = 0, MMSPULSER_SEQ	seq_mode = MMSPULSER_SEQ_LINEAR,
						  double seq_range = 0);

		//! Get the offset of the animation.
        /*!
        After each onAnimation call the offset will be increased or decreased dependent on
		the seq_mode set with setMaxOffset().
        \return offset
        \note This value is valid during the animation (e.g. in the onAnimation callback).
        \see setMaxOffset()
        \see getStepLength()
        */
		double getOffset();

        //! Set the requested duration.
        /*!
        \param duration 0..n milliseconds, default is 0 and means endless animation
        \return true, if parameter is accepted
        \note If your onAnimation callback returns false, the Animation will be stopped at any time.
        \see getDuration()
        */
		bool setDuration(unsigned int duration);

		//! Get the requested duration in milliseconds of the animation.
        /*!
        \return requested duration in milliseconds
        \see setDuration()
        */
		unsigned int getDuration();

        //! Get the real duration in milliseconds of the animation.
        /*!
        \return real duration in milliseconds
        \note This value will be increased during the animation and has its final state at the
              end of the animation (e.g. in onAfterAnimation callback).
        */
		unsigned int getRealDuration();

        //! Get the number of onAnimation calls after the last onBeforeAnimation call.
        /*!
        \return number of calls
        \note This value is zero for the first time onAnimation is called.
        */
		unsigned int getOnAnimationCounter();

        //! Set one or more callbacks for the onBeforeAnimation event.
        /*!
        The connected callbacks will be called during threadMain().
        If at least one of the callbacks returns false, the animation will be stopped.

        A callback method must be defined like this:

			bool myclass::mycallbackmethod(MMSPulser *pulser);

			\param pulser	is the pointer to the caller (the pulser thread)

			\return true if the animation should continue, else false if the animation should stop

        To connect your callback to onBeforeAnimation do this:

            sigc::connection connection;
            connection = mypulser->onBeforeAnimation.connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onBeforeAnimation BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSPulser*>::accumulated<bool_accumulator> onBeforeAnimation;

        //! Set one or more callbacks for the onAnimation event.
        /*!
        The connected callbacks will be called during threadMain().
        If at least one of the callbacks returns false, the animation will be stopped.

        A callback method must be defined like this:

			bool myclass::mycallbackmethod(MMSPulser *pulser);

			\param pulser	is the pointer to the caller (the pulser thread)

			\return true if the animation should continue, else false if the animation should stop

        To connect your callback to onAnimation do this:

            sigc::connection connection;
            connection = mypulser->onAnimation.connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onAnimation BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSPulser*>::accumulated<bool_accumulator> onAnimation;

        //! Set one or more callbacks for the onAfterAnimation event.
        /*!
        The connected callbacks will be called during threadMain().

        A callback method must be defined like this:

			void myclass::mycallbackmethod(MMSPulser *pulser);

			\param pulser	is the pointer to the caller (the pulser thread)

        To connect your callback to onAfterAnimation do this:

            sigc::connection connection;
            connection = mypulser->onAfterAnimation.connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onAfterAnimation BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
		sigc::signal<void, MMSPulser*> onAfterAnimation;
};

#endif /*MMSPULSER_H_*/
