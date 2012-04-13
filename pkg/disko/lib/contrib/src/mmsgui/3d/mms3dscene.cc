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

#include "mmsgui/3d/mms3dscene.h"
#include <string.h>

MMS3DScene::MMS3DScene() {
	this->objects_cnt = 0;
	this->objects[this->objects_cnt] = NULL;
}

int MMS3DScene::newObject(MMS3DObject *object) {
	if (this->objects_cnt >= OBJ_SIZE) {
		// no more space
		return OBJ_NOTSET;
	}

	// get new object
	this->objects[this->objects_cnt] = &this->objbuf[this->objects_cnt];

	// reset it
	memset(this->objects[this->objects_cnt], 0, sizeof(MMS3D_OBJECT));
	this->objects[this->objects_cnt]->parent   = NULL;
	this->objects[this->objects_cnt]->vertices = -1;
	this->objects[this->objects_cnt]->normals  = -1;
	this->objects[this->objects_cnt]->texcoords= -1;
	this->objects[this->objects_cnt]->indices  = -1;
	this->objects[this->objects_cnt]->material = -1;
	this->objects[this->objects_cnt]->texture  = -1;

	// add to children
	this->children.push_back(object);

	// return index
	this->objects_cnt++;
	this->objects[this->objects_cnt] = NULL;
	return this->objects_cnt - 1;
}

MMS3D_OBJECT *MMS3DScene::getObject(int object) {
	return this->objects[object];
}

bool MMS3DScene::getResultMatrix(MMS3DMatrix result) {
	return this->matrixStack.getResultMatrix(result);
}

void MMS3DScene::getMeshArrays(MMS3D_VERTEX_ARRAY ***varrays, MMS3D_INDEX_ARRAY ***iarrays) {
	mms3dpm.getArrays(varrays, iarrays);
}

void MMS3DScene::getObjects(MMS3D_OBJECT ***objects) {
	*objects = this->objects;
}

void MMS3DScene::setBaseMatrix(MMS3DMatrix matrix) {
	this->matrixStack.setBaseMatrix(matrix);
}

void MMS3DScene::reset() {
	this->matrixStack.clear();
}

bool MMS3DScene::scale(float sx, float sy, float sz) {
	return this->matrixStack.scale(sx, sy, sz);
}

bool MMS3DScene::translate(float tx, float ty, float tz) {
	return this->matrixStack.translate(tx, ty, tz);
}

bool MMS3DScene::rotate(float angle, float x, float y, float z) {
	return this->matrixStack.rotate(angle, x, y, z);
}

bool MMS3DScene::genMatrices() {
	// get result matrix of scene used as base matrix for objects
	MMS3DMatrix base_matrix;
	if (!getResultMatrix(base_matrix)) return false;

	// though the flat list of scene objects
	for (int i = 0; i < this->children.size(); i++) {
		// ignore objects which are children of objects
		if (this->children.at(i)->parent) continue;

		// set the base matrix of the object
		this->children.at(i)->setBaseMatrix(base_matrix);

		// generate the matrices for the object and it's children
		this->children.at(i)->genMatrices();
	}

	return true;
}


