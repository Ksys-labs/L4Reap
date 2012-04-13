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

#ifndef MMS3DMATRIXSTACK_H_
#define MMS3DMATRIXSTACK_H_

#include "mmstools/mmstypes.h"

class MMS3DMatrixStack {
private:
	//! matrix operations
	typedef enum {
		MOP_SCALE = 0,
		MOP_TRANSLATE,
		MOP_ROTATE
	} MOP;

	//! matrix stack item
	typedef struct {
		MOP		mop;
		float	params[4];
	} MSI;

	//! size of the stack
	#define MS_SIZE	50

	//! stack
	MSI ms[MS_SIZE];

	//! number of items in the stack
	int ms_cnt;

	//! base matrix (matrix before stack operations)
	MMS3DMatrix base_matrix;

	//! is base matrix set?
	bool base_matrix_set;

	//! result matrix (matrix after stack operations)
	MMS3DMatrix result_matrix;

	//! is result matrix set?
	bool result_matrix_set;


	bool add(MOP mop, float p0, float p1, float p2, float p3);

public:
	MMS3DMatrixStack();

	void clear();

	void setBaseMatrix(MMS3DMatrix base_matrix);

	bool getResultMatrix(MMS3DMatrix result_matrix);

	bool scale(float sx, float sy, float sz);

	bool translate(float tx, float ty, float tz);

	bool rotate(float angle, float x, float y, float z);
};

#endif /* MMS3DMATRIXSTACK_H_ */
