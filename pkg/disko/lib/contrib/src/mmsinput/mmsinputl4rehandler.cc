/*
 * Copyright 2012 Ksys Labs LLC
 * Contact: <ivan.loskutov@ksyslabs.org>
 * Porting Disko UI Framework to L4Re.
 *
 */

#include "mmsinput/mmsinputl4rehandler.h"
#include "mmsinput/mmskeymap.h"
#include <typeinfo>

/* input */
#include <l4/util/util.h>
#include <l4/input/libinput.h>

#include <pthread-l4.h>

#include <stdio.h>

pthread_mutex_t lock_queue = PTHREAD_MUTEX_INITIALIZER;

std::deque<struct l4input>   MMSInputL4REHandler::events_queue = std::deque<struct l4input>();

void MMSInputL4REHandler::l4event_cb(struct l4input *ev)
{
    // add to queue
    pthread_mutex_lock(&lock_queue);
    MMSInputL4REHandler::events_queue.push_back(*ev);
    pthread_mutex_unlock(&lock_queue);
}

MMSInputL4REHandler::MMSInputL4REHandler(MMS_INPUT_DEVICE device)
{
#ifdef __L4_RE__
    //printf("MMSInputL4REHandler::MMSInputL4REHandler\n");

    this->config = new MMSConfigData();

    /* get the screen rectangle */
    this->screen_rect.x = config->getVRect().x;
    this->screen_rect.y = config->getVRect().y;
    this->screen_rect.w = config->getVRect().w;
    this->screen_rect.h = config->getVRect().h;

    /* get the pointer rectangle */
    this->pointer_rect.x = config->getTouchRect().x;
    this->pointer_rect.y = config->getTouchRect().y;
    this->pointer_rect.w = config->getTouchRect().w;
    this->pointer_rect.h = config->getTouchRect().h;
    if ((this->pointer_rect.w<=0)||(this->pointer_rect.h<=0))
        if (config->getPointer()!=MMSFB_PM_FALSE) {
            // no touch rect given but pointer needed
            this->pointer_rect.x = this->screen_rect.x;
            this->pointer_rect.y = this->screen_rect.y;
            this->pointer_rect.w = this->screen_rect.w;
            this->pointer_rect.h = this->screen_rect.h;
        }

    /* calculate a factor between screen and pointer rectangle */
    if ((this->pointer_rect.w > 0)&&(this->pointer_rect.h > 0)) {
        this->xfac = (100 * this->screen_rect.w) / this->pointer_rect.w;
        this->yfac = (100 * this->screen_rect.h) / this->pointer_rect.h;
        this->pointer_xpos = this->pointer_old_xpos = this->screen_rect.x + this->screen_rect.w / 2;
        this->pointer_ypos = this->pointer_old_ypos = this->screen_rect.y + this->screen_rect.h / 2;
    }
    else {
        /* this means that touch pad/screen is not used */
        this->pointer_rect.w = 0;
        this->pointer_rect.h = 0;
    }

    TRACEOUT("L4REINPUT", "screen_rect x=%d, y=%d, h=%d, w=%d\n", screen_rect.x, screen_rect.y, screen_rect.h, screen_rect.w);
    TRACEOUT("L4REINPUT", "pointer_rect x=%d, y=%d, h=%d, w=%d\n", pointer_rect.x, pointer_rect.y, pointer_rect.h, pointer_rect.w);

    // set callback for events from L4 input
    l4input_init(17, MMSInputL4REHandler::l4event_cb);
#else
    throw MMSError(0,(string)typeid(this).name() + " is empty. compile L4Re support!");
#endif
}

MMSInputL4REHandler::~MMSInputL4REHandler()  {
}

void MMSInputL4REHandler::grabEvents(MMSInputEvent *inputevent) {
    bool queue_empty = false;
    MMSInputEvent   event;
    struct l4input  ev;

    do {
        pthread_mutex_lock(&lock_queue);
        queue_empty = MMSInputL4REHandler::events_queue.empty();
        if (!queue_empty)
        {
            ev = MMSInputL4REHandler::events_queue.front();
            MMSInputL4REHandler::events_queue.pop_front();
        }
        pthread_mutex_unlock(&lock_queue);


        if (queue_empty) {
            usleep(10000);
        }
        else {
            if (translateEvent(ev, &event)) {
                *inputevent = event;
                break;
            }
        }

    } while(1);
}

bool MMSInputL4REHandler::translateEvent(struct l4input &ev, MMSInputEvent *inputevent)
{
    static int x = -1, y = -1;
    static int tx = -1, ty = -1;
    static int px = 0, py = 0;
    static char pressed = 0xff;
    TsCalibration cal_const;

    switch(ev.type)
    {
        case EV_KEY:
            if (ev.code == BTN_LEFT)
            {
                // mouse
                inputevent->type = ev.value ? MMSINPUTEVENTTYPE_BUTTONPRESS : MMSINPUTEVENTTYPE_BUTTONRELEASE;
                inputevent->posx = this->pointer_xpos;
                inputevent->posy = this->pointer_ypos;
                //printf("L4REINPUT MOUSE BUTTON %s at %dx%d\n", (ev.value ? "PRESS" : "RELEASE"), inputevent->posx, inputevent->posy);
            }
            else if (ev.code == BTN_TOUCH) {
                // touchscreen
                //printf("L4REINPUT Event: type EV_KEY, code BTN_TOUCH, value %d\n", ev.value);
                pressed = (ev.value ? 1 : 0);
                return false;
            }
            else {
                inputevent->type = ev.value ? MMSINPUTEVENTTYPE_KEYPRESS : MMSINPUTEVENTTYPE_KEYRELEASE;
                inputevent->key = translateKey(ev.code);
            }
            break;

        case EV_REL:
            if (ev.code == REL_X) {
                /* x axis */
                this->pointer_xpos += ev.value;
                if (this->pointer_xpos < this->screen_rect.x)
                    this->pointer_xpos = this->screen_rect.x;
                else
                    if (this->pointer_xpos > this->screen_rect.x + this->screen_rect.w - 1)
                        this->pointer_xpos = this->screen_rect.x + this->screen_rect.w - 1;
            }
            else
            if (ev.code == REL_Y) {
                /* x axis */
                this->pointer_ypos += ev.value;
                if (this->pointer_ypos < this->screen_rect.y)
                    this->pointer_ypos = this->screen_rect.y;
                else
                    if (this->pointer_ypos > this->screen_rect.y + this->screen_rect.h - 1)
                        this->pointer_ypos = this->screen_rect.y + this->screen_rect.h - 1;
            }

            if ((this->pointer_xpos != this->pointer_old_xpos)||(this->pointer_ypos != this->pointer_old_ypos)) {
                /* the position of the mouse pointer has changed */
                /* save the old pointer */
                this->pointer_old_xpos = this->pointer_xpos;
                this->pointer_old_ypos = this->pointer_ypos;

                /* fill the return structure */
                inputevent->type = MMSINPUTEVENTTYPE_AXISMOTION;
                inputevent->posx = this->pointer_xpos;
                inputevent->posy = this->pointer_ypos;

                //printf("L4REINPUT Event: type MMSINPUTEVENTTYPE_AXISMOTION, x=%d, y=%d\n", pointer_xpos, pointer_ypos);
            }
            else
                return false;
            break;

        case EV_ABS:
            //printf("L4REINPUT Event: type EV_ABS, code %d, value %d\n", ev.code, ev.value);
            if (ev.code == ABS_X) {
                //if ( pressed && abs(ev.value - x) < 5 )
                    tx = ev.value;
                    return false;
            }
            else if (ev.code == ABS_Y) {
                //if ( abs(ev.value - y) > 5 )
                    ty = ev.value;
            }
            if (tx == -1 || ty == -1)
                return false;

            if (!this->config->getTsMode())
            {
                cal_const = this->config->getTsCalibration();

                x = (float)tx*cal_const.aX + (float)ty*cal_const.bX + cal_const.dX;
                y = (float)tx*cal_const.aY + (float)ty*cal_const.bY + cal_const.dY;
            } else {
                x = tx;
                y = ty;
            }

            //printf("L4REINPUT Event: (%d,%d)\n", x, y);

            if(pressed == 1) {
                inputevent->type = MMSINPUTEVENTTYPE_AXISMOTION;
                inputevent->posx = x;
                inputevent->posy = y;
            }
            break;

        case EV_SYN:
            //printf("L4REINPUT Event: type EV_SYN, code %d, value %d\n", ev.code, ev.value);
            if(pressed != 0xff) {
                inputevent->type = (pressed ? MMSINPUTEVENTTYPE_BUTTONPRESS : MMSINPUTEVENTTYPE_BUTTONRELEASE);

                if (pressed) {
                    px = x;
                    py = y;
                    if (x<0 || y<0) {
                        // x or y coordinate not set, ignore the PRESS event
                        //x = -1;
                        //y = -1;
                        return false;
                    }
                    inputevent->posx = x;
                    inputevent->posy = y;
                    x = -1;
                    y = -1;
                }
                else {
                    if (x<0 || y<0) {
                        // x or y coordinate not set, check pressed coordinate
                        x = -1;
                        y = -1;
                        if (px<0 || py<0) {
                            // px or py coordinate not set, ignore the RELEASE event
                            return false;
                        }
                        else {
                            inputevent->posx = px;
                            inputevent->posy = py;
                        }
                    }
                    else {
                        inputevent->posx = x;
                        inputevent->posy = y;
                        x = -1;
                        y = -1;
                    }

                }
                //printf("L4REINPUT TOUCH BUTTON %s at %dx%d\n", (pressed ? "PRESS" : "RELEASE"), inputevent->posx, inputevent->posy);

                if (this->config->getTsMode() && inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS)
                {
                    inputevent->type = MMSINPUTEVENTTYPE_TSCALIBRATION;
                }

                return true;
            } else {
                //TODO: we discuss to return false in case AXISMOTION should not emitted
                return false;
            }
            return false;

        default:
            //printf("L4REINPUT Event: type %d, code %d, value %d\n", ev.type, ev.code, ev.value);
            return false;
    }
    return true;
}

MMSKeySymbol MMSInputL4REHandler::translateKey(unsigned short key)
{
    switch(key){
        case KEY_ESC    : return MMSKEY_ESCAPE;
        case KEY_UP     : return MMSKEY_CURSOR_UP;
        case KEY_LEFT   : return MMSKEY_CURSOR_LEFT;
        case KEY_RIGHT  : return MMSKEY_CURSOR_RIGHT;
        case KEY_DOWN   : return MMSKEY_CURSOR_DOWN;
        case KEY_ENTER  : return MMSKEY_RETURN;
        case KEY_A      : return MMSKEY_SMALL_A;
        default         : return MMSKEY_UNKNOWN;
    }
}
