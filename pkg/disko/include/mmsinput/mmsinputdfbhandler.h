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

#ifndef MMSINPUTDFBHANDLER_H_
#define MMSINPUTDFBHANDLER_H_

#include "mmsgui/mmsguitools.h"
#include "mmsbase/mmsbase.h"
#include "mmsinput/mmsinputhandler.h"


#ifdef  __HAVE_DIRECTFB__
	#ifndef DFBCHECK
		#define DFBCHECK( x... ) \
		{\
			 DFBResult err = x;\
			 if (err != DFB_OK) {\
				  fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );\
				  DirectFBErrorFatal( #x, err );\
			 }\
		}
	#endif



class MMSInputDFBHandler : public MMSInputHandler {
	private:
		MMSConfigData *config;

		IDirectFBInputDevice    *input;
		IDirectFB			    *dfb;
		IDirectFBEventBuffer    *keybuffer;
		bool					quit;

		DFBRectangle	screen_rect;
		DFBRectangle	pointer_rect;

		int				xfac;
		int				yfac;

		int				pointer_xpos;
		int				pointer_ypos;

		int				pointer_old_xpos;
		int				pointer_old_ypos;

		int				button_pressed;

	public:
		MMSInputDFBHandler(MMS_INPUT_DEVICE device);
		~MMSInputDFBHandler();
		void grabEvents(MMSInputEvent *inputevent);
};

#else
class MMSInputDFBHandler : public MMSInputHandler {
	public:
		MMSInputDFBHandler(MMS_INPUT_DEVICE device);
		~MMSInputDFBHandler();
		void grabEvents(MMSInputEvent *inputevent);
};

#endif

#endif /*MMSINPUTHANDLER_H_*/
