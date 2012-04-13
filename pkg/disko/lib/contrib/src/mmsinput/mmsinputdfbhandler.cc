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

#include "mmsinput/mmsinputdfbhandler.h"
#include "mmsinput/mmskeymap.h"
#include <typeinfo>
#ifdef __HAVE_DIRECTFB__
#include <directfb_keynames.h>

MMSKeySymbol getKeyFromDFB(DFBInputDeviceKeySymbol sym) {
	switch(sym) {
		case DIKS_BACKSPACE: return MMSKEY_BACKSPACE;
		case DIKS_TAB: return MMSKEY_TAB;
		case DIKS_RETURN: return MMSKEY_RETURN;
		case DIKS_CANCEL: return MMSKEY_CANCEL;
		case DIKS_ESCAPE: return MMSKEY_ESCAPE;
		case DIKS_SPACE: return MMSKEY_SPACE;
		case DIKS_EXCLAMATION_MARK: return MMSKEY_EXCLAMATION_MARK;
		case DIKS_QUOTATION: return MMSKEY_QUOTATION;
		case DIKS_NUMBER_SIGN: return MMSKEY_NUMBER_SIGN;
		case DIKS_DOLLAR_SIGN: return MMSKEY_DOLLAR_SIGN;
		case DIKS_PERCENT_SIGN: return MMSKEY_PERCENT_SIGN;
		case DIKS_AMPERSAND: return MMSKEY_AMPERSAND;
		case DIKS_APOSTROPHE: return MMSKEY_APOSTROPHE;
		case DIKS_PARENTHESIS_LEFT: return MMSKEY_PARENTHESIS_LEFT;
		case DIKS_PARENTHESIS_RIGHT: return MMSKEY_PARENTHESIS_RIGHT;
		case DIKS_ASTERISK: return MMSKEY_ASTERISK;
		case DIKS_PLUS_SIGN: return MMSKEY_PLUS_SIGN;
		case DIKS_COMMA: return MMSKEY_COMMA;
		case DIKS_MINUS_SIGN: return MMSKEY_MINUS_SIGN;
		case DIKS_PERIOD: return MMSKEY_PERIOD;
		case DIKS_SLASH: return MMSKEY_SLASH;
		case DIKS_0: return MMSKEY_0;
		case DIKS_1: return MMSKEY_1;
		case DIKS_2: return MMSKEY_2;
		case DIKS_3: return MMSKEY_3;
		case DIKS_4: return MMSKEY_4;
		case DIKS_5: return MMSKEY_5;
		case DIKS_6: return MMSKEY_6;
		case DIKS_7: return MMSKEY_7;
		case DIKS_8: return MMSKEY_8;
		case DIKS_9: return MMSKEY_9;
		case DIKS_COLON: return MMSKEY_COLON;
		case DIKS_SEMICOLON: return MMSKEY_SEMICOLON;
		case DIKS_LESS_THAN_SIGN: return MMSKEY_LESS_THAN_SIGN;
		case DIKS_EQUALS_SIGN: return MMSKEY_EQUALS_SIGN;
		case DIKS_GREATER_THAN_SIGN: return MMSKEY_GREATER_THAN_SIGN;
		case DIKS_QUESTION_MARK: return MMSKEY_QUESTION_MARK;
		case DIKS_AT: return MMSKEY_AT;
		case DIKS_CAPITAL_A: return MMSKEY_CAPITAL_A;
		case DIKS_CAPITAL_B: return MMSKEY_CAPITAL_B;
		case DIKS_CAPITAL_C: return MMSKEY_CAPITAL_C;
		case DIKS_CAPITAL_D: return MMSKEY_CAPITAL_D;
		case DIKS_CAPITAL_E: return MMSKEY_CAPITAL_E;
		case DIKS_CAPITAL_F: return MMSKEY_CAPITAL_F;
		case DIKS_CAPITAL_G: return MMSKEY_CAPITAL_G;
		case DIKS_CAPITAL_H: return MMSKEY_CAPITAL_H;
		case DIKS_CAPITAL_I: return MMSKEY_CAPITAL_I;
		case DIKS_CAPITAL_J: return MMSKEY_CAPITAL_J;
		case DIKS_CAPITAL_K: return MMSKEY_CAPITAL_K;
		case DIKS_CAPITAL_L: return MMSKEY_CAPITAL_L;
		case DIKS_CAPITAL_M: return MMSKEY_CAPITAL_M;
		case DIKS_CAPITAL_N: return MMSKEY_CAPITAL_N;
		case DIKS_CAPITAL_O: return MMSKEY_CAPITAL_O;
		case DIKS_CAPITAL_P: return MMSKEY_CAPITAL_P;
		case DIKS_CAPITAL_Q: return MMSKEY_CAPITAL_Q;
		case DIKS_CAPITAL_R: return MMSKEY_CAPITAL_R;
		case DIKS_CAPITAL_S: return MMSKEY_CAPITAL_S;
		case DIKS_CAPITAL_T: return MMSKEY_CAPITAL_T;
		case DIKS_CAPITAL_U: return MMSKEY_CAPITAL_U;
		case DIKS_CAPITAL_V: return MMSKEY_CAPITAL_V;
		case DIKS_CAPITAL_W: return MMSKEY_CAPITAL_W;
		case DIKS_CAPITAL_X: return MMSKEY_CAPITAL_X;
		case DIKS_CAPITAL_Y: return MMSKEY_CAPITAL_Y;
		case DIKS_CAPITAL_Z: return MMSKEY_CAPITAL_Z;
		case DIKS_SQUARE_BRACKET_LEFT: return MMSKEY_SQUARE_BRACKET_LEFT;
		case DIKS_BACKSLASH: return MMSKEY_BACKSLASH;
		case DIKS_SQUARE_BRACKET_RIGHT: return MMSKEY_SQUARE_BRACKET_RIGHT;
		case DIKS_CIRCUMFLEX_ACCENT: return MMSKEY_CIRCUMFLEX_ACCENT;
		case DIKS_UNDERSCORE: return MMSKEY_UNDERSCORE;
		case DIKS_GRAVE_ACCENT: return MMSKEY_GRAVE_ACCENT;
		case DIKS_SMALL_A: return MMSKEY_SMALL_A;
		case DIKS_SMALL_B: return MMSKEY_SMALL_B;
		case DIKS_SMALL_C: return MMSKEY_SMALL_C;
		case DIKS_SMALL_D: return MMSKEY_SMALL_D;
		case DIKS_SMALL_E: return MMSKEY_SMALL_E;
		case DIKS_SMALL_F: return MMSKEY_SMALL_F;
		case DIKS_SMALL_G: return MMSKEY_SMALL_G;
		case DIKS_SMALL_H: return MMSKEY_SMALL_H;
		case DIKS_SMALL_I: return MMSKEY_SMALL_I;
		case DIKS_SMALL_J: return MMSKEY_SMALL_J;
		case DIKS_SMALL_K: return MMSKEY_SMALL_K;
		case DIKS_SMALL_L: return MMSKEY_SMALL_L;
		case DIKS_SMALL_M: return MMSKEY_SMALL_M;
		case DIKS_SMALL_N: return MMSKEY_SMALL_N;
		case DIKS_SMALL_O: return MMSKEY_SMALL_O;
		case DIKS_SMALL_P: return MMSKEY_SMALL_P;
		case DIKS_SMALL_Q: return MMSKEY_SMALL_Q;
		case DIKS_SMALL_R: return MMSKEY_SMALL_R;
		case DIKS_SMALL_S: return MMSKEY_SMALL_S;
		case DIKS_SMALL_T: return MMSKEY_SMALL_T;
		case DIKS_SMALL_U: return MMSKEY_SMALL_U;
		case DIKS_SMALL_V: return MMSKEY_SMALL_V;
		case DIKS_SMALL_W: return MMSKEY_SMALL_W;
		case DIKS_SMALL_X: return MMSKEY_SMALL_X;
		case DIKS_SMALL_Y: return MMSKEY_SMALL_Y;
		case DIKS_SMALL_Z: return MMSKEY_SMALL_Z;
		case DIKS_CURLY_BRACKET_LEFT: return MMSKEY_CURLY_BRACKET_LEFT;
		case DIKS_VERTICAL_BAR: return MMSKEY_VERTICAL_BAR;
		case DIKS_CURLY_BRACKET_RIGHT: return MMSKEY_CURLY_BRACKET_RIGHT;
		case DIKS_TILDE: return MMSKEY_TILDE;
		case DIKS_DELETE: return MMSKEY_DELETE;
		case DIKS_CURSOR_LEFT: return MMSKEY_CURSOR_LEFT;
		case DIKS_CURSOR_RIGHT: return MMSKEY_CURSOR_RIGHT;
		case DIKS_CURSOR_UP: return MMSKEY_CURSOR_UP;
		case DIKS_CURSOR_DOWN: return MMSKEY_CURSOR_DOWN;
		case DIKS_INSERT: return MMSKEY_INSERT;
		case DIKS_HOME: return MMSKEY_HOME;
		case DIKS_END: return MMSKEY_END;
		case DIKS_PAGE_UP: return MMSKEY_PAGE_UP;
		case DIKS_PAGE_DOWN: return MMSKEY_PAGE_DOWN;
		case DIKS_PRINT: return MMSKEY_PRINT;
		case DIKS_PAUSE: return MMSKEY_PAUSE;
		case DIKS_OK: return MMSKEY_OK;
		case DIKS_SELECT: return MMSKEY_SELECT;
		case DIKS_GOTO: return MMSKEY_GOTO;
		case DIKS_CLEAR: return MMSKEY_CLEAR;
		case DIKS_POWER: return MMSKEY_POWER;
		case DIKS_POWER2: return MMSKEY_POWER2;
		case DIKS_OPTION: return MMSKEY_OPTION;
		case DIKS_MENU: return MMSKEY_MENU;
		case DIKS_HELP: return MMSKEY_HELP;
		case DIKS_INFO: return MMSKEY_INFO;
		case DIKS_TIME: return MMSKEY_TIME;
		case DIKS_VENDOR: return MMSKEY_VENDOR;
		case DIKS_ARCHIVE: return MMSKEY_ARCHIVE;
		case DIKS_PROGRAM: return MMSKEY_PROGRAM;
		case DIKS_CHANNEL: return MMSKEY_CHANNEL;
		case DIKS_FAVORITES: return MMSKEY_FAVORITES;
		case DIKS_EPG: return MMSKEY_EPG;
		case DIKS_PVR: return MMSKEY_PVR;
		case DIKS_MHP: return MMSKEY_MHP;
		case DIKS_LANGUAGE: return MMSKEY_LANGUAGE;
		case DIKS_TITLE: return MMSKEY_TITLE;
		case DIKS_SUBTITLE: return MMSKEY_SUBTITLE;
		case DIKS_ANGLE: return MMSKEY_ANGLE;
		case DIKS_ZOOM: return MMSKEY_ZOOM;
		case DIKS_MODE: return MMSKEY_MODE;
		case DIKS_KEYBOARD: return MMSKEY_KEYBOARD;
		case DIKS_PC: return MMSKEY_PC;
		case DIKS_SCREEN: return MMSKEY_SCREEN;
		case DIKS_TV: return MMSKEY_TV;
		case DIKS_TV2: return MMSKEY_TV2;
		case DIKS_VCR: return MMSKEY_VCR;
		case DIKS_VCR2: return MMSKEY_VCR2;
		case DIKS_SAT: return MMSKEY_SAT;
		case DIKS_SAT2: return MMSKEY_SAT2;
		case DIKS_CD: return MMSKEY_CD;
		case DIKS_TAPE: return MMSKEY_TAPE;
		case DIKS_RADIO: return MMSKEY_RADIO;
		case DIKS_TUNER: return MMSKEY_TUNER;
		case DIKS_PLAYER: return MMSKEY_PLAYER;
		case DIKS_TEXT: return MMSKEY_TEXT;
		case DIKS_DVD: return MMSKEY_DVD;
		case DIKS_AUX: return MMSKEY_AUX;
		case DIKS_MP3: return MMSKEY_MP3;
		case DIKS_PHONE: return MMSKEY_PHONE;
		case DIKS_AUDIO: return MMSKEY_AUDIO;
		case DIKS_VIDEO: return MMSKEY_VIDEO;
		case DIKS_INTERNET: return MMSKEY_INTERNET;
		case DIKS_MAIL: return MMSKEY_MAIL;
		case DIKS_NEWS: return MMSKEY_NEWS;
		case DIKS_DIRECTORY: return MMSKEY_DIRECTORY;
		case DIKS_LIST: return MMSKEY_LIST;
		case DIKS_CALCULATOR: return MMSKEY_CALCULATOR;
		case DIKS_MEMO: return MMSKEY_MEMO;
		case DIKS_CALENDAR: return MMSKEY_CALENDAR;
		case DIKS_EDITOR: return MMSKEY_EDITOR;
		case DIKS_RED: return MMSKEY_RED;
		case DIKS_GREEN: return MMSKEY_GREEN;
		case DIKS_YELLOW: return MMSKEY_YELLOW;
		case DIKS_BLUE: return MMSKEY_BLUE;
		case DIKS_CHANNEL_UP: return MMSKEY_CHANNEL_UP;
		case DIKS_CHANNEL_DOWN: return MMSKEY_CHANNEL_DOWN;
		case DIKS_BACK: return MMSKEY_BACK;
		case DIKS_FORWARD: return MMSKEY_FORWARD;
		case DIKS_FIRST: return MMSKEY_FIRST;
		case DIKS_LAST: return MMSKEY_LAST;
		case DIKS_VOLUME_UP: return MMSKEY_VOLUME_UP;
		case DIKS_VOLUME_DOWN: return MMSKEY_VOLUME_DOWN;
		case DIKS_MUTE: return MMSKEY_MUTE;
		case DIKS_AB: return MMSKEY_AB;
		case DIKS_PLAYPAUSE: return MMSKEY_PLAYPAUSE;
		case DIKS_PLAY: return MMSKEY_PLAY;
		case DIKS_STOP: return MMSKEY_STOP;
		case DIKS_RESTART: return MMSKEY_RESTART;
		case DIKS_SLOW: return MMSKEY_SLOW;
		case DIKS_FAST: return MMSKEY_FAST;
		case DIKS_RECORD: return MMSKEY_RECORD;
		case DIKS_EJECT: return MMSKEY_EJECT;
		case DIKS_SHUFFLE: return MMSKEY_SHUFFLE;
		case DIKS_REWIND: return MMSKEY_REWIND;
		case DIKS_FASTFORWARD: return MMSKEY_FASTFORWARD;
		case DIKS_PREVIOUS: return MMSKEY_PREVIOUS;
		case DIKS_NEXT: return MMSKEY_NEXT;
		case DIKS_BEGIN: return MMSKEY_BEGIN;
		case DIKS_DIGITS: return MMSKEY_DIGITS;
		case DIKS_TEEN: return MMSKEY_TEEN;
		case DIKS_TWEN: return MMSKEY_TWEN;
		case DIKS_BREAK: return MMSKEY_BREAK;
		case DIKS_EXIT: return MMSKEY_EXIT;
		case DIKS_SETUP: return MMSKEY_SETUP;
		case DIKS_CURSOR_LEFT_UP: return MMSKEY_CURSOR_LEFT_UP;
		case DIKS_CURSOR_LEFT_DOWN: return MMSKEY_CURSOR_LEFT_DOWN;
		case DIKS_CURSOR_UP_RIGHT: return MMSKEY_CURSOR_UP_RIGHT;
		case DIKS_CURSOR_DOWN_RIGHT: return MMSKEY_CURSOR_DOWN_RIGHT;
		case DIKS_F1: return MMSKEY_F1;
		case DIKS_F2: return MMSKEY_F2;
		case DIKS_F3: return MMSKEY_F3;
		case DIKS_F4: return MMSKEY_F4;
		case DIKS_F5: return MMSKEY_F5;
		case DIKS_F6: return MMSKEY_F6;
		case DIKS_F7: return MMSKEY_F7;
		case DIKS_F8: return MMSKEY_F8;
		case DIKS_F9: return MMSKEY_F9;
		case DIKS_F10: return MMSKEY_F10;
		case DIKS_F11: return MMSKEY_F11;
		case DIKS_F12: return MMSKEY_F12;
		case DIKS_SHIFT: return MMSKEY_SHIFT;
		case DIKS_CONTROL: return MMSKEY_CONTROL;
		case DIKS_ALT: return MMSKEY_ALT;
		case DIKS_ALTGR: return MMSKEY_ALTGR;
		case DIKS_META: return MMSKEY_META;
		case DIKS_SUPER: return MMSKEY_SUPER;
		case DIKS_HYPER: return MMSKEY_HYPER;
		case DIKS_CAPS_LOCK: return MMSKEY_CAPS_LOCK;
		case DIKS_NUM_LOCK: return MMSKEY_NUM_LOCK;
		case DIKS_SCROLL_LOCK: return MMSKEY_SCROLL_LOCK;
		case DIKS_DEAD_ABOVEDOT: return MMSKEY_DEAD_ABOVEDOT;
		case DIKS_DEAD_ABOVERING: return MMSKEY_DEAD_ABOVERING;
		case DIKS_DEAD_ACUTE: return MMSKEY_DEAD_ACUTE;
		case DIKS_DEAD_BREVE: return MMSKEY_DEAD_BREVE;
		case DIKS_DEAD_CARON: return MMSKEY_DEAD_CARON;
		case DIKS_DEAD_CEDILLA: return MMSKEY_DEAD_CEDILLA;
		case DIKS_DEAD_CIRCUMFLEX: return MMSKEY_DEAD_CIRCUMFLEX;
		case DIKS_DEAD_DIAERESIS: return MMSKEY_DEAD_DIAERESIS;
		case DIKS_DEAD_DOUBLEACUTE: return MMSKEY_DEAD_DOUBLEACUTE;
		case DIKS_DEAD_GRAVE: return MMSKEY_DEAD_GRAVE;
		case DIKS_DEAD_IOTA: return MMSKEY_DEAD_IOTA;
		case DIKS_DEAD_MACRON: return MMSKEY_DEAD_MACRON;
		case DIKS_DEAD_OGONEK: return MMSKEY_DEAD_OGONEK;
		case DIKS_DEAD_SEMIVOICED_SOUND: return MMSKEY_DEAD_SEMIVOICED_SOUND;
		case DIKS_DEAD_TILDE: return MMSKEY_DEAD_TILDE;
		case DIKS_DEAD_VOICED_SOUND: return MMSKEY_DEAD_VOICED_SOUND;
		case DIKS_CUSTOM0: return MMSKEY_CUSTOM0;
		case DIKS_CUSTOM1: return MMSKEY_CUSTOM1;
		case DIKS_CUSTOM2: return MMSKEY_CUSTOM2;
		case DIKS_CUSTOM3: return MMSKEY_CUSTOM3;
		case DIKS_CUSTOM4: return MMSKEY_CUSTOM4;
		case DIKS_CUSTOM5: return MMSKEY_CUSTOM5;
		case DIKS_CUSTOM6: return MMSKEY_CUSTOM6;
		case DIKS_CUSTOM7: return MMSKEY_CUSTOM7;
		case DIKS_CUSTOM8: return MMSKEY_CUSTOM8;
		case DIKS_CUSTOM9: return MMSKEY_CUSTOM9;
		case DIKS_CUSTOM10: return MMSKEY_CUSTOM10;
		case DIKS_CUSTOM11: return MMSKEY_CUSTOM11;
		case DIKS_CUSTOM12: return MMSKEY_CUSTOM12;
		case DIKS_CUSTOM13: return MMSKEY_CUSTOM13;
		case DIKS_CUSTOM14: return MMSKEY_CUSTOM14;
		case DIKS_CUSTOM15: return MMSKEY_CUSTOM15;
		case DIKS_CUSTOM16: return MMSKEY_CUSTOM16;
		case DIKS_CUSTOM17: return MMSKEY_CUSTOM17;
		case DIKS_CUSTOM18: return MMSKEY_CUSTOM18;
		case DIKS_CUSTOM19: return MMSKEY_CUSTOM19;
		case DIKS_CUSTOM20: return MMSKEY_CUSTOM20;
		case DIKS_CUSTOM21: return MMSKEY_CUSTOM21;
		case DIKS_CUSTOM22: return MMSKEY_CUSTOM22;
		case DIKS_CUSTOM23: return MMSKEY_CUSTOM23;
		case DIKS_CUSTOM24: return MMSKEY_CUSTOM24;
		case DIKS_CUSTOM25: return MMSKEY_CUSTOM25;
		case DIKS_CUSTOM26: return MMSKEY_CUSTOM26;
		case DIKS_CUSTOM27: return MMSKEY_CUSTOM27;
		case DIKS_CUSTOM28: return MMSKEY_CUSTOM28;
		case DIKS_CUSTOM29: return MMSKEY_CUSTOM29;
		case DIKS_CUSTOM30: return MMSKEY_CUSTOM30;
		case DIKS_CUSTOM31: return MMSKEY_CUSTOM31;
		case DIKS_CUSTOM32: return MMSKEY_CUSTOM32;
		case DIKS_CUSTOM33: return MMSKEY_CUSTOM33;
		case DIKS_CUSTOM34: return MMSKEY_CUSTOM34;
		case DIKS_CUSTOM35: return MMSKEY_CUSTOM35;
		case DIKS_CUSTOM36: return MMSKEY_CUSTOM36;
		case DIKS_CUSTOM37: return MMSKEY_CUSTOM37;
		case DIKS_CUSTOM38: return MMSKEY_CUSTOM38;
		case DIKS_CUSTOM39: return MMSKEY_CUSTOM39;
		case DIKS_CUSTOM40: return MMSKEY_CUSTOM40;
		case DIKS_CUSTOM41: return MMSKEY_CUSTOM41;
		case DIKS_CUSTOM42: return MMSKEY_CUSTOM42;
		case DIKS_CUSTOM43: return MMSKEY_CUSTOM43;
		case DIKS_CUSTOM44: return MMSKEY_CUSTOM44;
		case DIKS_CUSTOM45: return MMSKEY_CUSTOM45;
		case DIKS_CUSTOM46: return MMSKEY_CUSTOM46;
		case DIKS_CUSTOM47: return MMSKEY_CUSTOM47;
		case DIKS_CUSTOM48: return MMSKEY_CUSTOM48;
		case DIKS_CUSTOM49: return MMSKEY_CUSTOM49;
		case DIKS_CUSTOM50: return MMSKEY_CUSTOM50;
		case DIKS_CUSTOM51: return MMSKEY_CUSTOM51;
		case DIKS_CUSTOM52: return MMSKEY_CUSTOM52;
		case DIKS_CUSTOM53: return MMSKEY_CUSTOM53;
		case DIKS_CUSTOM54: return MMSKEY_CUSTOM54;
		case DIKS_CUSTOM55: return MMSKEY_CUSTOM55;
		case DIKS_CUSTOM56: return MMSKEY_CUSTOM56;
		case DIKS_CUSTOM57: return MMSKEY_CUSTOM57;
		case DIKS_CUSTOM58: return MMSKEY_CUSTOM58;
		case DIKS_CUSTOM59: return MMSKEY_CUSTOM59;
		case DIKS_CUSTOM60: return MMSKEY_CUSTOM60;
		case DIKS_CUSTOM61: return MMSKEY_CUSTOM61;
		case DIKS_CUSTOM62: return MMSKEY_CUSTOM62;
		case DIKS_CUSTOM63: return MMSKEY_CUSTOM63;
		case DIKS_CUSTOM64: return MMSKEY_CUSTOM64;
		case DIKS_CUSTOM65: return MMSKEY_CUSTOM65;
		case DIKS_CUSTOM66: return MMSKEY_CUSTOM66;
		case DIKS_CUSTOM67: return MMSKEY_CUSTOM67;
		case DIKS_CUSTOM68: return MMSKEY_CUSTOM68;
		case DIKS_CUSTOM69: return MMSKEY_CUSTOM69;
		case DIKS_CUSTOM70: return MMSKEY_CUSTOM70;
		case DIKS_CUSTOM71: return MMSKEY_CUSTOM71;
		case DIKS_CUSTOM72: return MMSKEY_CUSTOM72;
		case DIKS_CUSTOM73: return MMSKEY_CUSTOM73;
		case DIKS_CUSTOM74: return MMSKEY_CUSTOM74;
		case DIKS_CUSTOM75: return MMSKEY_CUSTOM75;
		case DIKS_CUSTOM76: return MMSKEY_CUSTOM76;
		case DIKS_CUSTOM77: return MMSKEY_CUSTOM77;
		case DIKS_CUSTOM78: return MMSKEY_CUSTOM78;
		case DIKS_CUSTOM79: return MMSKEY_CUSTOM79;
		case DIKS_CUSTOM80: return MMSKEY_CUSTOM80;
		case DIKS_CUSTOM81: return MMSKEY_CUSTOM81;
		case DIKS_CUSTOM82: return MMSKEY_CUSTOM82;
		case DIKS_CUSTOM83: return MMSKEY_CUSTOM83;
		case DIKS_CUSTOM84: return MMSKEY_CUSTOM84;
		case DIKS_CUSTOM85: return MMSKEY_CUSTOM85;
		case DIKS_CUSTOM86: return MMSKEY_CUSTOM86;
		case DIKS_CUSTOM87: return MMSKEY_CUSTOM87;
		case DIKS_CUSTOM88: return MMSKEY_CUSTOM88;
		case DIKS_CUSTOM89: return MMSKEY_CUSTOM89;
		case DIKS_CUSTOM90: return MMSKEY_CUSTOM90;
		case DIKS_CUSTOM91: return MMSKEY_CUSTOM91;
		case DIKS_CUSTOM92: return MMSKEY_CUSTOM92;
		case DIKS_CUSTOM93: return MMSKEY_CUSTOM93;
		case DIKS_CUSTOM94: return MMSKEY_CUSTOM94;
		case DIKS_CUSTOM95: return MMSKEY_CUSTOM95;
		case DIKS_CUSTOM96: return MMSKEY_CUSTOM96;
		case DIKS_CUSTOM97: return MMSKEY_CUSTOM97;
		case DIKS_CUSTOM98: return MMSKEY_CUSTOM98;
		case DIKS_CUSTOM99: return MMSKEY_CUSTOM99;
		case DIKS_NULL: return MMSKEY_NULL;
		default: return MMSKEY_UNKNOWN;
	}

}
#endif

MMSInputDFBHandler::MMSInputDFBHandler(MMS_INPUT_DEVICE device) {
#ifdef  __HAVE_DIRECTFB__
	if(DirectFBCreate(&(this->dfb))!= DFB_OK)
        return;

	DFBInputDeviceID dev;
	switch(device) {
		case MMS_INPUT_KEYBOARD:
			dev=DIDID_KEYBOARD;
			break;
		case MMS_INPUT_MOUSE:
			dev=DIDID_MOUSE;
			break;
		case MMS_INPUT_REMOTE:
			dev=DIDID_REMOTE;
			break;
		default:
			dev=DIDID_ANY;
	}

	this->config = new MMSConfigData();


	/*grab the input device */
    //DFBCHECK(dfb->GetInputDevice( dfb, device, &input ));

	/*create an input queue */
    //DFBCHECK(input->CreateEventBuffer( input, &keybuffer ));
//    DFBCHECK(dfb->CreateInputEventBuffer(dfb,DICAPS_KEYS, (DFBBoolean)1, &keybuffer));
	DFBCHECK(dfb->CreateInputEventBuffer(dfb,DICAPS_ALL, (DFBBoolean)1, &keybuffer));

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

	this->button_pressed = 0;

	this->quit=false;
#else
	throw MMSError(0,(string)typeid(this).name() + " is empty. compile DirectFB support!");
#endif
}

MMSInputDFBHandler::~MMSInputDFBHandler()  {
#ifdef __HAVE_DIRECTFB__
	if(this->config) delete this->config;
#else
	throw MMSError(0,(string)typeid(this).name() + " is empty. compile DirectFB support!");
#endif
}

void MMSInputDFBHandler::grabEvents(MMSInputEvent *inputevent) {
#ifdef __HAVE_DIRECTFB__
	DFBInputEvent evt;

    bool event_buffered = false;
    inputevent->type = MMSINPUTEVENTTYPE_NONE;

	while(1) {
		/* wait for event with 100ms timeout */
		if (keybuffer->WaitForEventWithTimeout(keybuffer, 0, 100) != DFB_OK) {
			if (event_buffered)
				return;
			continue;
		}

	    /* process keybuffer */
	    if (keybuffer->PeekEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
	    	/* check if i have to get it */
			if (event_buffered)
		    	if ((evt.type == DIET_BUTTONRELEASE) && (inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS))
		    		/* if the next event is buttonrelease and the current buffered event is buttonpress */
		    		/* return the current event buttonpress */
		    		return;

	    	/* get the event */
		    if (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
		    	switch (evt.type) {
		    	case DIET_KEYPRESS:
		    			if (!this->button_pressed) {
			    			/* work with keypress event */
/*				        	symbol_name = (DFBKeySymbolName *)bsearch( &evt.key_symbol,keynames,sizeof(keynames) / sizeof(keynames[0]) - 1, sizeof(keynames[0]), compare_symbol );
				        	string name = symbol_name->name;
				        	logger.writeLog("key '" + name + "' pressed, id: " + iToStr(symbol_name->symbol));
*/
				        	/* fill the return structure */
				        	inputevent->type = MMSINPUTEVENTTYPE_KEYPRESS;
			        		inputevent->key = getKeyFromDFB(evt.key_symbol);
			        		return;
		    			}
		    			break;
		    	case DIET_KEYRELEASE:
		    			if (!this->button_pressed) {
			    			/* work with keyrelease event */
				        	/* fill the return structure */
				        	inputevent->type = MMSINPUTEVENTTYPE_KEYRELEASE;
			        		inputevent->key = getKeyFromDFB(evt.key_symbol);
			        		return;
		    			}
		    			break;
		    	case DIET_BUTTONPRESS:
		    			/* work with pressed buttons */
		    			if (this->pointer_rect.w) {
		    				if (evt.button == DIBI_LEFT) {
			    				/* work only with left button */
				    			/* fill the return structure */
					        	inputevent->type = MMSINPUTEVENTTYPE_BUTTONPRESS;
				        		inputevent->posx = this->pointer_xpos;
				        		inputevent->posy = this->pointer_ypos;

				    			/* buttonpress event */
				    			this->button_pressed++;

				    			if (this->button_pressed > 1)
				    				/* buttonpress event raised two times for touch pad/screens */
					        		return;

				    			/* all buttonpress events will be buffered */
				    			event_buffered = true;
			    			}
		    			}
		    			break;
		    	case DIET_BUTTONRELEASE:
	    				/* work with released buttons */
		    			if (this->pointer_rect.w) {
			    			if (evt.button == DIBI_LEFT) {
				    			/* fill the return structure */
					        	inputevent->type = MMSINPUTEVENTTYPE_BUTTONRELEASE;
				        		inputevent->posx = this->pointer_xpos;
				        		inputevent->posy = this->pointer_ypos;

				    			/* buttonrelease event */
				        		this->button_pressed--;

				    			if (!this->button_pressed)
				    				/* buttonrelease event raised two times for touch pad/screens */
					        		return;

				    			/* buttonrelease event will be buffered */
				    			event_buffered = true;
			    			}
		    			}
		    			break;
		    	case DIET_AXISMOTION:
		    			if (this->pointer_rect.w) {
				       	 	if (evt.axis == DIAI_X) {
				       	 		/* x axis */
					        	if (evt.flags & DIEF_AXISREL) {
					        		/* work with relative value (e.g. normal mouse move) */
									if ((evt.axisrel>1)||(evt.axisrel<-1))
										this->pointer_xpos+= evt.axisrel*2;
									else
										this->pointer_xpos+= evt.axisrel;
					        		if (this->pointer_xpos < this->screen_rect.x)
					        			this->pointer_xpos = this->screen_rect.x;
					        		else
					        		if (this->pointer_xpos > this->screen_rect.x + this->screen_rect.w - 1)
					        			this->pointer_xpos = this->screen_rect.x + this->screen_rect.w - 1;
					        	}
					        	else
					        		/* work with absolute position (e.g. touch pad/screen) */
					        		this->pointer_xpos = this->screen_rect.x
		  			        						 + ((this->xfac * (evt.axisabs - this->pointer_rect.x)) / 100);
				       	 	}
				       	 	else
			        	 	if (evt.axis == DIAI_Y) {
				       	 		/* y axis */
					        	if (evt.flags & DIEF_AXISREL) {
					        		/* work with relative value (e.g. normal mouse move) */
									if ((evt.axisrel>1)||(evt.axisrel<-1))
										this->pointer_ypos+= evt.axisrel*3;
									else
										this->pointer_ypos+= evt.axisrel;
					        		if (this->pointer_ypos < this->screen_rect.y)
					        			this->pointer_ypos = this->screen_rect.y;
					        		else
					        		if (this->pointer_ypos > this->screen_rect.y + this->screen_rect.h - 1)
					        			this->pointer_ypos = this->screen_rect.y + this->screen_rect.h - 1;
					        	}
					        	else
					        		/* work with absolute position (e.g. touch pad/screen) */
					        		this->pointer_ypos = this->screen_rect.y
		  			        						 + ((this->yfac * (evt.axisabs - this->pointer_rect.y)) / 100);
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

				        		/* if we are waiting for the second buttonpress event (touch pad/screen) */
				        		/* --> we cannot return directly */
				        		if (!event_buffered)
				        			return;
				       	 	}
		    			}
		    			break;
                    default:
	        		    break;
		    	}
			}
	    }
	}
#else
	throw MMSError(0,(string)typeid(this).name() + " is empty. compile DirectFB support!");
#endif
}

