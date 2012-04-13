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

#include "mmsgui/3d/mms3dmatrixstack.h"

MMS3DMatrixStack::MMS3DMatrixStack() {
	clear();
	this->base_matrix_set = false;
	this->result_matrix_set = false;
}

bool MMS3DMatrixStack::add(MOP mop, float p0, float p1, float p2, float p3) {
	if (this->ms_cnt >= MS_SIZE) return false;

	// new item
	MSI *msi = &ms[this->ms_cnt];
	this->ms_cnt++;

	// setup item
	msi->mop = mop;
	msi->params[0] = p0;
	msi->params[1] = p1;
	msi->params[2] = p2;
	msi->params[3] = p3;

	return true;
}

void MMS3DMatrixStack::clear() {
	this->ms_cnt = 0;
	this->result_matrix_set = false;
}

void MMS3DMatrixStack::setBaseMatrix(MMS3DMatrix base_matrix) {
	if (this->base_matrix_set) {
		if (equalMatrix(this->base_matrix, base_matrix)) {
			// this base matrix has been already set
			return;
		}
	}

	// set new base matrix
	copyMatrix(this->base_matrix, base_matrix);
	this->base_matrix_set = true;
	this->result_matrix_set = false;
}

bool MMS3DMatrixStack::getResultMatrix(MMS3DMatrix result_matrix) {
	if (!this->base_matrix_set) return false;

	if (!this->result_matrix_set) {
		// result matrix not generated, do it now
		copyMatrix(this->result_matrix, this->base_matrix);

		// generate result
		for (int i = this->ms_cnt - 1; i >= 0; i--) {
			MSI *msi = &ms[i];
			switch (msi->mop) {
			case MOP_SCALE:
				scaleMatrix(this->result_matrix, msi->params[0], msi->params[1], msi->params[2]);
				break;
			case MOP_TRANSLATE:
				translateMatrix(this->result_matrix, msi->params[0], msi->params[1], msi->params[2]);
				break;
			case MOP_ROTATE:
				rotateMatrix(this->result_matrix, msi->params[0], msi->params[1], msi->params[2], msi->params[3]);
				break;
			}
		}

		// result matrix is now available
		this->result_matrix_set = true;
	}

	// return result
	copyMatrix(result_matrix, this->result_matrix);

	return true;
}

bool MMS3DMatrixStack::scale(float sx, float sy, float sz) {
	if (sx == 1.0f && sy == 1.0f && sz == 1.0f) return true;
	this->result_matrix_set = false;
	return add(MOP_SCALE, sx, sy, sz, 0);
}

bool MMS3DMatrixStack::translate(float tx, float ty, float tz) {
	if (tx == 0.0f && ty == 0.0f && tz == 0.0f) return true;
	this->result_matrix_set = false;
	return add(MOP_TRANSLATE, tx, ty, tz, 0);
}

bool MMS3DMatrixStack::rotate(float angle, float x, float y, float z) {
	if (angle == 0.0f) return true;
	this->result_matrix_set = false;
	return add(MOP_ROTATE, angle, x, y, z);
}


