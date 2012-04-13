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

#include "mmsinput/mmsinputlisthread.h"

#include <string.h>
#include <errno.h>

#ifndef __LIS_DEBUG__
#undef MSG2OUT
#define MSG2OUT(ident, msg...)
#endif


// keycode translation table e.g. for remote controls
MMSKeySymbol MMSInputLISThread_extkeycodes [] = {
	MMSKEY_UNKNOWN,
	MMSKEY_ESCAPE,
	MMSKEY_1,
	MMSKEY_2,
	MMSKEY_3,
	MMSKEY_4,
	MMSKEY_5,
	MMSKEY_6,
	MMSKEY_7,
	MMSKEY_8,
	MMSKEY_9,
	MMSKEY_0,
	MMSKEY_MINUS_SIGN,
	MMSKEY_EQUALS_SIGN,
	MMSKEY_BACKSPACE,
	MMSKEY_TAB,
	MMSKEY_SMALL_Q,
	MMSKEY_SMALL_W,
	MMSKEY_SMALL_E,
	MMSKEY_SMALL_R,
	MMSKEY_SMALL_T,
	MMSKEY_SMALL_Y,
	MMSKEY_SMALL_U,
	MMSKEY_SMALL_I,
	MMSKEY_SMALL_O,
	MMSKEY_SMALL_P,
	MMSKEY_SQUARE_BRACKET_LEFT,
	MMSKEY_SQUARE_BRACKET_RIGHT,
	MMSKEY_RETURN,
	MMSKEY_CONTROL,
	MMSKEY_SMALL_A,
	MMSKEY_SMALL_S,
	MMSKEY_SMALL_D,
	MMSKEY_SMALL_F,
	MMSKEY_SMALL_G,
	MMSKEY_SMALL_H,
	MMSKEY_SMALL_J,
	MMSKEY_SMALL_K,
	MMSKEY_SMALL_L,
	MMSKEY_SEMICOLON,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_SHIFT,
	MMSKEY_BACKSLASH,
	MMSKEY_SMALL_Z,
	MMSKEY_SMALL_X,
	MMSKEY_SMALL_C,
	MMSKEY_SMALL_V,
	MMSKEY_SMALL_B,
	MMSKEY_SMALL_N,
	MMSKEY_SMALL_M,
	MMSKEY_COMMA,
	MMSKEY_PERIOD,
	MMSKEY_SLASH,
	MMSKEY_SHIFT,
	MMSKEY_UNKNOWN,
	MMSKEY_ALT,
	MMSKEY_SPACE,
	MMSKEY_CAPS_LOCK,
	MMSKEY_F1,
	MMSKEY_F2,
	MMSKEY_F3,
	MMSKEY_F4,
	MMSKEY_F5,
	MMSKEY_F6,
	MMSKEY_F7,
	MMSKEY_F8,
	MMSKEY_F9,
	MMSKEY_F10,
	MMSKEY_NUM_LOCK,
	MMSKEY_SCROLL_LOCK,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_BACKSLASH,
	MMSKEY_UNKNOWN,
	MMSKEY_LESS_THAN_SIGN,
	MMSKEY_F11,
	MMSKEY_F12,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_RETURN,
	MMSKEY_CONTROL,
	MMSKEY_UNKNOWN,
	MMSKEY_PRINT,
	MMSKEY_ALTGR,
	MMSKEY_UNKNOWN,
	MMSKEY_HOME,
	MMSKEY_CURSOR_UP,
	MMSKEY_PAGE_UP,
	MMSKEY_CURSOR_LEFT,
	MMSKEY_CURSOR_RIGHT,
	MMSKEY_END,
	MMSKEY_CURSOR_DOWN,
	MMSKEY_PAGE_DOWN,
	MMSKEY_INSERT,
	MMSKEY_DELETE,
	MMSKEY_UNKNOWN,
	MMSKEY_MUTE,
	MMSKEY_VOLUME_DOWN,
	MMSKEY_VOLUME_UP,
	MMSKEY_POWER,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_PAUSE,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_META,
	MMSKEY_META,
	MMSKEY_SUPER,
	MMSKEY_STOP,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_HELP,
	MMSKEY_MENU,
	MMSKEY_CALCULATOR,
	MMSKEY_SETUP,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_CUSTOM1,
	MMSKEY_CUSTOM2,
	MMSKEY_INTERNET,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_MAIL,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_BACK,
	MMSKEY_FORWARD,
	MMSKEY_EJECT,
	MMSKEY_EJECT,
	MMSKEY_EJECT,
	MMSKEY_NEXT,
	MMSKEY_PLAYPAUSE,
	MMSKEY_PREVIOUS,
	MMSKEY_STOP,
	MMSKEY_RECORD,
	MMSKEY_REWIND,
	MMSKEY_PHONE,
	MMSKEY_UNKNOWN,
	MMSKEY_SETUP,
	MMSKEY_UNKNOWN,
	MMSKEY_SHUFFLE,
	MMSKEY_EXIT,
	MMSKEY_UNKNOWN,
	MMSKEY_EDITOR,
	MMSKEY_PAGE_UP,
	MMSKEY_PAGE_DOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_PLAY,
	MMSKEY_PAUSE,
	MMSKEY_CUSTOM3,
	MMSKEY_CUSTOM4,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_PLAY,
	MMSKEY_FASTFORWARD,
	MMSKEY_UNKNOWN,
	MMSKEY_PRINT,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_AUDIO,
	MMSKEY_HELP,
	MMSKEY_MAIL,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_CANCEL,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,
	MMSKEY_UNKNOWN,

	// special keys...
	MMSKEY_OK,
	MMSKEY_SELECT,
	MMSKEY_GOTO,
	MMSKEY_CLEAR,
	MMSKEY_POWER2,
	MMSKEY_OPTION,
	MMSKEY_INFO,
	MMSKEY_TIME,
	MMSKEY_VENDOR,
	MMSKEY_ARCHIVE,
	MMSKEY_PROGRAM,
	MMSKEY_CHANNEL,
	MMSKEY_FAVORITES,
	MMSKEY_EPG,
	MMSKEY_PVR,
	MMSKEY_MHP,
	MMSKEY_LANGUAGE,
	MMSKEY_TITLE,
	MMSKEY_SUBTITLE,
	MMSKEY_ANGLE,
	MMSKEY_ZOOM,
	MMSKEY_MODE,
	MMSKEY_KEYBOARD,
	MMSKEY_SCREEN,
	MMSKEY_PC,
	MMSKEY_TV,
	MMSKEY_TV2,
	MMSKEY_VCR,
	MMSKEY_VCR2,
	MMSKEY_SAT,
	MMSKEY_SAT2,
	MMSKEY_CD,
	MMSKEY_TAPE,
	MMSKEY_RADIO,
	MMSKEY_TUNER,
	MMSKEY_PLAYER,
	MMSKEY_TEXT,
	MMSKEY_DVD,
	MMSKEY_AUX,
	MMSKEY_MP3,
	MMSKEY_AUDIO,
	MMSKEY_VIDEO,
	MMSKEY_DIRECTORY,
	MMSKEY_LIST,
	MMSKEY_MEMO,
	MMSKEY_CALENDAR,
	MMSKEY_RED,
	MMSKEY_GREEN,
	MMSKEY_YELLOW,
	MMSKEY_BLUE,
	MMSKEY_CHANNEL_UP,
	MMSKEY_CHANNEL_DOWN,
	MMSKEY_FIRST,
	MMSKEY_LAST,
	MMSKEY_AB,
	MMSKEY_NEXT,
	MMSKEY_RESTART,
	MMSKEY_SLOW,
	MMSKEY_SHUFFLE,
	MMSKEY_FASTFORWARD,
	MMSKEY_PREVIOUS,
	MMSKEY_NEXT,
	MMSKEY_DIGITS,
	MMSKEY_TEEN,
	MMSKEY_TWEN,
	MMSKEY_BREAK
};



MMSInputLISThread::MMSInputLISThread(MMSInputLISHandler *handler, int kb_fd) {
	this->handler = handler;
	this->kb_fd = kb_fd;
	this->dv_fd = -1;

	this->shift_pressed = false;
	this->altgr_pressed = false;
	this->is_caps_lock = false;
	this->button_pressed = false;

	this->lastX = this->lastY = -1;
}

MMSInputLISThread::MMSInputLISThread(MMSInputLISHandler *handler, MMSINPUTLISHANDLER_DEV *device) {
	this->handler = handler;
	this->kb_fd = -1;
	this->dv_fd = -1;
	this->device = *device;
}

MMSInputLISThread::~MMSInputLISThread() {
}

bool MMSInputLISThread::openDevice() {
	// close device if already opened
	closeDevice();

	MSG2OUT("MMSINPUTMANAGER", "Opening %s, type=%s (%s)\n",
						this->device.name.c_str(),
						this->device.type.c_str(),
						this->device.desc.c_str());

	// open the device
	if ((this->dv_fd = open(this->device.name.c_str(), O_RDWR)) < 0) {
		MSG2OUT("MMSINPUTMANAGER", "could not open device: %s\n", this->device.name.c_str());
		this->dv_fd = -1;
		return false;
	}

	// try to grab the device
	if (ioctl(this->dv_fd, EVIOCGRAB, 1)) {
		MSG2OUT("MMSINPUTMANAGER", "could not grab device: %s\n", this->device.name.c_str());
		close(this->dv_fd);
		this->dv_fd = -1;
		return false;
	}

	return true;
}

void MMSInputLISThread::closeDevice() {
	// opened?
	if (this->dv_fd < 0)
		return;

	// release it
    ioctl(this->dv_fd, EVIOCGRAB, 0);
    close(this->dv_fd);
	this->dv_fd = -1;
}

MMSKeySymbol MMSInputLISThread::getSymbol(int code, unsigned short value) {
	unsigned char type  = KTYP(value);
	unsigned char index = KVAL(value);

	TRACEOUT("MMSINPUT", "KEYCODE: TYPE=%d(0x%x), INDEX=%d(0x%x), value=%d(0x%x)", type, type, index, index, value, value);

	switch (type) {
		case KT_FN:
            if (index < 12)
         	   return (MMSKeySymbol)(MMSKEY_F1 + index);
			break;
		case KT_LETTER:
		case KT_LATIN:
			switch (index) {
			case 0x1c: return MMSKEY_PRINT;
			case 0x7f: return MMSKEY_BACKSPACE;
			case 0x08: return MMSKEY_BACKSPACE;
			case 0x09: return MMSKEY_TAB;
			case 0x0d: return MMSKEY_RETURN;
			case 0x18: return MMSKEY_CANCEL;
			case 0x1b: return MMSKEY_ESCAPE;
			case 0x20: return MMSKEY_SPACE;
			case 0x21: return MMSKEY_EXCLAMATION_MARK;
			case 0x22: return MMSKEY_QUOTATION;
			case 0x23: return MMSKEY_NUMBER_SIGN;
			case 0x24: return MMSKEY_DOLLAR_SIGN;
			case 0x25: return MMSKEY_PERCENT_SIGN;
			case 0x26: return MMSKEY_AMPERSAND;
			case 0x27: return MMSKEY_APOSTROPHE;
			case 0x28: return MMSKEY_PARENTHESIS_LEFT;
			case 0x29: return MMSKEY_PARENTHESIS_RIGHT;
			case 0x2a: return MMSKEY_ASTERISK;
			case 0x2b: return MMSKEY_PLUS_SIGN;
			case 0x2c: return MMSKEY_COMMA;
			case 0x2d: return MMSKEY_MINUS_SIGN;
			case 0x2e: return MMSKEY_PERIOD;
			case 0x2f: return MMSKEY_SLASH;
			case 0x30: return MMSKEY_0;
			case 0x31: return MMSKEY_1;
			case 0x32: return MMSKEY_2;
			case 0x33: return MMSKEY_3;
			case 0x34: return MMSKEY_4;
			case 0x35: return MMSKEY_5;
			case 0x36: return MMSKEY_6;
			case 0x37: return MMSKEY_7;
			case 0x38: return MMSKEY_8;
			case 0x39: return MMSKEY_9;
			case 0x3a: return MMSKEY_COLON;
			case 0x3b: return MMSKEY_SEMICOLON;
			case 0x3c: return MMSKEY_LESS_THAN_SIGN;
			case 0x3d: return MMSKEY_EQUALS_SIGN;
			case 0x3e: return MMSKEY_GREATER_THAN_SIGN;
			case 0x3f: return MMSKEY_QUESTION_MARK;
			case 0x40: return MMSKEY_AT;
			case 0x41: return MMSKEY_CAPITAL_A;
			case 0x42: return MMSKEY_CAPITAL_B;
			case 0x43: return MMSKEY_CAPITAL_C;
			case 0x44: return MMSKEY_CAPITAL_D;
			case 0x45: return MMSKEY_CAPITAL_E;
			case 0x46: return MMSKEY_CAPITAL_F;
			case 0x47: return MMSKEY_CAPITAL_G;
			case 0x48: return MMSKEY_CAPITAL_H;
			case 0x49: return MMSKEY_CAPITAL_I;
			case 0x4a: return MMSKEY_CAPITAL_J;
			case 0x4b: return MMSKEY_CAPITAL_K;
			case 0x4c: return MMSKEY_CAPITAL_L;
			case 0x4d: return MMSKEY_CAPITAL_M;
			case 0x4e: return MMSKEY_CAPITAL_N;
			case 0x4f: return MMSKEY_CAPITAL_O;
			case 0x50: return MMSKEY_CAPITAL_P;
			case 0x51: return MMSKEY_CAPITAL_Q;
			case 0x52: return MMSKEY_CAPITAL_R;
			case 0x53: return MMSKEY_CAPITAL_S;
			case 0x54: return MMSKEY_CAPITAL_T;
			case 0x55: return MMSKEY_CAPITAL_U;
			case 0x56: return MMSKEY_CAPITAL_V;
			case 0x57: return MMSKEY_CAPITAL_W;
			case 0x58: return MMSKEY_CAPITAL_X;
			case 0x59: return MMSKEY_CAPITAL_Y;
			case 0x5a: return MMSKEY_CAPITAL_Z;
			case 0x5b: return MMSKEY_SQUARE_BRACKET_LEFT;
			case 0x5c: return MMSKEY_BACKSLASH;
			case 0x5d: return MMSKEY_SQUARE_BRACKET_RIGHT;
			case 0x5e: return MMSKEY_CIRCUMFLEX_ACCENT;
			case 0x5f: return MMSKEY_UNDERSCORE;
			case 0x60: return MMSKEY_GRAVE_ACCENT;
			case 0x61: return MMSKEY_SMALL_A;
			case 0x62: return MMSKEY_SMALL_B;
			case 0x63: return MMSKEY_SMALL_C;
			case 0x64: return MMSKEY_SMALL_D;
			case 0x65: return MMSKEY_SMALL_E;
			case 0x66: return MMSKEY_SMALL_F;
			case 0x67: return MMSKEY_SMALL_G;
			case 0x68: return MMSKEY_SMALL_H;
			case 0x69: return MMSKEY_SMALL_I;
			case 0x6a: return MMSKEY_SMALL_J;
			case 0x6b: return MMSKEY_SMALL_K;
			case 0x6c: return MMSKEY_SMALL_L;
			case 0x6d: return MMSKEY_SMALL_M;
			case 0x6e: return MMSKEY_SMALL_N;
			case 0x6f: return MMSKEY_SMALL_O;
			case 0x70: return MMSKEY_SMALL_P;
			case 0x71: return MMSKEY_SMALL_Q;
			case 0x72: return MMSKEY_SMALL_R;
			case 0x73: return MMSKEY_SMALL_S;
			case 0x74: return MMSKEY_SMALL_T;
			case 0x75: return MMSKEY_SMALL_U;
			case 0x76: return MMSKEY_SMALL_V;
			case 0x77: return MMSKEY_SMALL_W;
			case 0x78: return MMSKEY_SMALL_X;
			case 0x79: return MMSKEY_SMALL_Y;
			case 0x7a: return MMSKEY_SMALL_Z;
			case 0x7b: return MMSKEY_CURLY_BRACKET_LEFT;
			case 0x7c: return MMSKEY_VERTICAL_BAR;
			case 0x7d: return MMSKEY_CURLY_BRACKET_RIGHT;
			case 0x7e: return MMSKEY_TILDE;
			default:   return MMSKEY_UNKNOWN;
			}
			break;
		case KT_PAD:
			switch (value) {
			case K_P0:      return MMSKEY_0;
			case K_P1:      return MMSKEY_1;
			case K_P2:      return MMSKEY_2;
			case K_P3:      return MMSKEY_3;
			case K_P4:      return MMSKEY_4;
			case K_P5:      return MMSKEY_5;
			case K_P6:      return MMSKEY_6;
			case K_P7:      return MMSKEY_7;
			case K_P8:      return MMSKEY_8;
			case K_P9:      return MMSKEY_9;
			case K_PPLUS:   return MMSKEY_PLUS_SIGN;
			case K_PMINUS:  return MMSKEY_MINUS_SIGN;
			case K_PSTAR:   return MMSKEY_ASTERISK;
			case K_PSLASH:  return MMSKEY_SLASH;
			case K_PENTER:  return MMSKEY_RETURN;
			case K_PCOMMA:  return MMSKEY_COMMA;
			case K_PDOT:    return MMSKEY_PERIOD;
			case K_PPARENL: return MMSKEY_PARENTHESIS_LEFT;
			case K_PPARENR: return MMSKEY_PARENTHESIS_RIGHT;
			default:   return MMSKEY_UNKNOWN;
			}
			break;
	}

	switch (value) {
		case K_LEFT:    return MMSKEY_CURSOR_LEFT;
		case K_RIGHT:   return MMSKEY_CURSOR_RIGHT;
		case K_UP:      return MMSKEY_CURSOR_UP;
		case K_DOWN:    return MMSKEY_CURSOR_DOWN;
		case K_ENTER:   return MMSKEY_RETURN;
		case K_CTRL:    return MMSKEY_CONTROL;
		case K_SHIFT:   return MMSKEY_SHIFT;
		case K_ALT:     return MMSKEY_ALT;
		case K_ALTGR:   return MMSKEY_ALTGR;
		case K_INSERT:  return MMSKEY_INSERT;
		case K_REMOVE:  return MMSKEY_DELETE;
		case K_FIND:    return MMSKEY_HOME;
		case K_SELECT:  return MMSKEY_END;
		case K_PGUP:    return MMSKEY_PAGE_UP;
		case K_PGDN:    return MMSKEY_PAGE_DOWN;
		case K_NUM:     return MMSKEY_NUM_LOCK;
		case K_HOLD:    return MMSKEY_SCROLL_LOCK;
		case K_PAUSE:   return MMSKEY_PAUSE;
		case K_BREAK:   return MMSKEY_BREAK;
		case K_CAPS:    return MMSKEY_CAPS_LOCK;
		case K_P0:      return MMSKEY_INSERT;
		case K_P1:      return MMSKEY_END;
		case K_P2:      return MMSKEY_CURSOR_DOWN;
		case K_P3:      return MMSKEY_PAGE_DOWN;
		case K_P4:      return MMSKEY_CURSOR_LEFT;
		case K_P5:      return MMSKEY_BEGIN;
		case K_P6:      return MMSKEY_CURSOR_RIGHT;
		case K_P7:      return MMSKEY_HOME;
		case K_P8:      return MMSKEY_CURSOR_UP;
		case K_P9:      return MMSKEY_PAGE_UP;
		case K_PPLUS:   return MMSKEY_PLUS_SIGN;
		case K_PMINUS:  return MMSKEY_MINUS_SIGN;
		case K_PSTAR:   return MMSKEY_ASTERISK;
		case K_PSLASH:  return MMSKEY_SLASH;
		case K_PENTER:  return MMSKEY_RETURN;
		case K_PCOMMA:  return MMSKEY_COMMA;
		case K_PDOT:    return MMSKEY_PERIOD;
		case K_PPARENL: return MMSKEY_PARENTHESIS_LEFT;
		case K_PPARENR: return MMSKEY_PARENTHESIS_RIGHT;
    }

	return MMSKEY_UNKNOWN;
}

unsigned short MMSInputLISThread::readValue(unsigned char table, unsigned char index) {
	struct kbentry entry;
	entry.kb_table = table;
	entry.kb_index = index;
	entry.kb_value = 0;
	ioctl(this->kb_fd, KDGKBENT, &entry);
	return entry.kb_value;
}

MMSKeySymbol MMSInputLISThread::getKeyFromCode(bool pressed, unsigned char code) {
    unsigned short value;

	// try with normtab
	value = readValue(K_NORMTAB, code);
	MMSKeySymbol ks = getSymbol(code, value);

	// check special keys
    switch (ks) {
    case MMSKEY_SHIFT:
    	this->shift_pressed = pressed;
    	break;
    case MMSKEY_ALTGR:
    	this->altgr_pressed = pressed;
    	break;
    case MMSKEY_CAPS_LOCK:
    	if (!pressed) {
    		// work only with the key release event
   			this->is_caps_lock = !this->is_caps_lock;
			updateLED();
    	}
    	break;
    default:
        if ((this->shift_pressed)||(this->is_caps_lock)) {
            if (!this->altgr_pressed) {
            	// shift is pressed
            	value = readValue(K_SHIFTTAB, code);
            	ks = getSymbol(code, value);
            }
            else {
            	// shift+altgr is pressed
        		value = readValue(K_ALTSHIFTTAB, code);
        		ks = getSymbol(code, value);
            }
        }
        else
        if (this->altgr_pressed) {
        	// altgr is pressed
    		value = readValue(K_ALTTAB, code);
    		ks = getSymbol(code, value);
        }
    	break;
    }

    return ks;
}

void MMSInputLISThread::updateLED() {
	int locks = 0;
	if (this->is_caps_lock)
		locks |= K_CAPSLOCK;
    ioctl(this->kb_fd, KDSKBLED, locks);
}

MMSKeySymbol MMSInputLISThread::translateKey(int code) {
	if (code < 0)
		return MMSKEY_UNKNOWN;
	if (code >= (int)(sizeof(MMSInputLISThread_extkeycodes)/sizeof(MMSKeySymbol)))
		return MMSKEY_UNKNOWN;
	return MMSInputLISThread_extkeycodes[code];
}

bool MMSInputLISThread::translateEvent(struct input_event *linux_evt, MMSInputEvent *inputevent) {
	static int x = -1, y = -1;
	static int px = 0, py = 0;
	static char pressed = 0xff;

	TRACEOUT("MMSINPUT", "EVENT TYPE = %d, CODE = %d, VALUE = %d", linux_evt->type, linux_evt->code, linux_evt->value);

	if(linux_evt->type == EV_ABS) {
		if(this->device.touch.swapXY) {
			if(linux_evt->code == ABS_X) { linux_evt->code = ABS_Y; }
			else if(linux_evt->code == ABS_Y) { linux_evt->code = ABS_X; }
		}

		switch(linux_evt->code) {
			case ABS_X:
				x = linux_evt->value - this->device.touch.rect.x;

				if(this->device.touch.swapX) {
					x = this->device.touch.rect.w - x;
				}

				x*= this->device.touch.xFactor;

				TRACEOUT("MMSINPUT", "EVENT TYPE = EV_ABS, CODE = ABS_X, X = %d, XF = %f", x, this->device.touch.xFactor);

				break;
			case ABS_Y:
				y = linux_evt->value - this->device.touch.rect.y;

				if(this->device.touch.swapY) {
					y = this->device.touch.rect.h - y;
				}

				y*= this->device.touch.yFactor;

				TRACEOUT("MMSINPUT", "EVENT TYPE = EV_ABS, CODE = ABS_Y, Y = %d, YF = %f", y, this->device.touch.yFactor);

				break;
			case ABS_PRESSURE:
				/*
				 * if the touch driver doesn't send BTN_xxx events, use
				 * ABS_PRESSURE as indicator for pressed/released
				 */
				TRACEOUT("MMSINPUT", "EVENT TYPE = EV_ABS, CODE = ABS_PRESSURE, VALUE = %d", linux_evt->value);

				if(!this->device.touch.haveBtnEvents) {
					pressed = (linux_evt->value ? 1 : 0);
				}
				break;
			default:
				break;
		}
	} else if(linux_evt->type == EV_KEY) {
		switch(linux_evt->code) {
			case BTN_LEFT:
			case BTN_TOUCH:
				pressed = (linux_evt->value ? 1 : 0);
				break;
			default:
				inputevent->key = translateKey(linux_evt->code);
				if (inputevent->key == MMSKEY_UNKNOWN)
					return false;

				inputevent->type = linux_evt->value ? MMSINPUTEVENTTYPE_KEYPRESS : MMSINPUTEVENTTYPE_KEYRELEASE;
				TRACEOUT("MMSINPUT", "KEY %s %d", (pressed ? "PRESS" : "RELEASE"), inputevent->key);
				return true;
				break;
		}
	} else if(linux_evt->type == EV_SYN) {
		if(pressed != 0xff) {
			inputevent->type = (pressed ? MMSINPUTEVENTTYPE_BUTTONPRESS : MMSINPUTEVENTTYPE_BUTTONRELEASE);

			if (pressed) {
				px = x;
				py = y;
				if (x<0 || y<0) {
					// x or y coordinate not set, ignore the PRESS event
					x = -1;
					y = -1;
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

			TRACEOUT("MMSINPUT", "BUTTON %s at %dx%d", (pressed ? "PRESS" : "RELEASE"), inputevent->posx, inputevent->posy);

			pressed = 0xff;
		} else {
			inputevent->posx = x;
			inputevent->posy = y;
			inputevent->type = MMSINPUTEVENTTYPE_AXISMOTION;
		}

		return true;
	}

	return false;
}

void MMSInputLISThread::threadMain() {
	if (this->kb_fd >= 0) {
		// working for keyboard inputs
		unsigned char key;
		while (read(this->kb_fd, &key, 1) == 1) {
			// new event
			MMSInputEvent inputevent;
			if (key & 0x80) {
				inputevent.type = MMSINPUTEVENTTYPE_KEYRELEASE;

				TRACEOUT("MMSINPUT", "KEY RELEASE <<<");

				inputevent.key = getKeyFromCode(false, key & 0x7f);
			}
			else {
				inputevent.type = MMSINPUTEVENTTYPE_KEYPRESS;

				TRACEOUT("MMSINPUT", "KEY PRESS >>>");

				inputevent.key = getKeyFromCode(true, key & 0x7f);
			}

			this->handler->addEvent(&inputevent);
		}
	}
	else
	if (!this->device.name.empty()) {
		if(this->device.type != MMSINPUTLISHANDLER_DEVTYPE_UNKNOWN) {
			if (openDevice()) {
				while (1) {
					int readlen;
					struct input_event linux_evt;

					if (((readlen = read(this->dv_fd, (void*)&linux_evt, sizeof(linux_evt))) < 0) && (errno != EINTR))
						break;

					if (readlen <= 0)
						continue;

					if (readlen != sizeof(linux_evt))
						break;

					// new event
					MMSInputEvent inputevent;
					if (translateEvent(&linux_evt, &inputevent)) {
						this->handler->addEvent(&inputevent);
					}
				}

				// release the device
				closeDevice();
			}
		}
		else {
			MSG2OUT("MMSINPUTMANAGER", "Wrong type of device %s, type=%s (%s)\n",
								this->device.name.c_str(),
								this->device.type.c_str(),
								this->device.desc.c_str());
		}
	}
	else {
		printf("inputlisthread not correctly initialized");
	}
}

