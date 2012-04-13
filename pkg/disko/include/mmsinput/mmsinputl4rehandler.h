/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re.
 *
 */

#ifndef MMSINPUTL4REHANDLER_H_
#define MMSINPUTL4REHANDLER_H_

#include "mmsgui/mmsguitools.h"
#include "mmsbase/mmsbase.h"
#include "mmsinput/mmsinputhandler.h"

#include <deque>

struct l4input;


class MMSInputL4REHandler : public MMSInputHandler {
	private:
        static std::deque<struct l4input>   events_queue;

        bool            shift_pressed;
        bool            alt_pressed;
        bool            ctrl_pressed;

        struct ScreenRectangle
        {
            int x, y, h, w;
        };

        struct ScreenRectangle screen_rect;
        struct ScreenRectangle pointer_rect;

        int             xfac;
        int             yfac;

        int             pointer_xpos;
        int             pointer_ypos;

        int             pointer_old_xpos;
        int             pointer_old_ypos;

        int             button_pressed;

        MMSConfigData   *config;

	public:
		MMSInputL4REHandler(MMS_INPUT_DEVICE device);
		~MMSInputL4REHandler();
		void grabEvents(MMSInputEvent *inputevent);

    private:
        static void l4event_cb(struct l4input *ev);

        bool translateEvent(struct l4input &ev, MMSInputEvent *inputevent);
        MMSKeySymbol translateKey(unsigned short key);

};

#endif /*MMSINPUTL4REHANDLER_H_*/
