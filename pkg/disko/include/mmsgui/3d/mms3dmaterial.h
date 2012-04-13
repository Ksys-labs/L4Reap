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

#ifndef MMS3DMATERIAL_H_
#define MMS3DMATERIAL_H_

#include "mmstools/mmstypes.h"

class MMS3DMaterial {
public:

	//! indices to the materials
	typedef enum {
		// index range 0..255 for user defined materials
		MAT_NOTSET = -1,
		MAT_MAX_USER_DEFINED = 255,

		// basic colors
		MAT_BLACK,
		MAT_RED,
		MAT_GREEN,
		MAT_BLUE,
		MAT_YELLOW,
		MAT_CYAN,
		MAT_MAGENTA,
		MAT_WHITE,

		// gemstone / glass
		MAT_JADE,
		MAT_JADE_LUCENT,
		MAT_OBSIDIAN,
		MAT_OBSIDIAN_LUCENT,
		MAT_PEARL,
		MAT_PEARL_LUCENT,
		MAT_RUBY,
		MAT_RUBY_LUCENT,
		MAT_EMERALD,
		MAT_EMERALD_LUCENT,
		MAT_TURQUOISE,
		MAT_TURQUOISE_LUCENT,

		// synthetic
		MAT_BLACK_PLASTIC,
		MAT_RED_PLASTIC,
		MAT_GREEN_PLASTIC,
		MAT_BLUE_PLASTIC,
		MAT_YELLOW_PLASTIC,
		MAT_CYAN_PLASTIC,
		MAT_MAGENTA_PLASTIC,
		MAT_WHITE_PLASTIC,
		MAT_BLACK_RUBBER,
		MAT_RED_RUBBER,
		MAT_GREEN_RUBBER,
		MAT_BLUE_RUBBER,
		MAT_YELLOW_RUBBER,
		MAT_CYAN_RUBBER,
		MAT_MAGENTA_RUBBER,
		MAT_WHITE_RUBBER,

		// metal
		MAT_BRONZE,
		MAT_BRONZE_POLISHED,
		MAT_CHROME,
		MAT_GOLD,
		MAT_GOLD_POLISHED,
		MAT_COPPER,
		MAT_COPPER_POLISHED,
		MAT_BRASS,
		MAT_SILVER,
		MAT_SILVER_POLISHED,
		MAT_TIN,

		// size needed for the buffer allocation
		MAT_SIZE
	} MAT;

private:

	//! materials
	static MMS3D_MATERIAL mat_buffer[MAT_SIZE];

	//! number of user defined materials
	static int material_cnt;

public:

	MMS3DMaterial();

	void getBuffer(MMS3D_MATERIAL **mat_buffer);

	int genMaterial(MMSFBColor emission, MMSFBColor ambient, MMSFBColor diffuse, MMSFBColor specular, unsigned char shininess);
};

#endif /* MMS3DMATERIAL_H_ */
