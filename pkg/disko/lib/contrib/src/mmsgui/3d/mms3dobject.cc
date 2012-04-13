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

#include "mmsgui/3d/mms3dobject.h"
#include "mmsgui/3d/mms3dscene.h"

MMS3DObject::MMS3DObject(MMS3DScene *scene) {
	this->scene = scene;

	// put new object into scene
	// note: this object will not be drawn but can have children
	this->obj_id = this->scene->newObject(this);

	this->parent = NULL;

	// set the base matrix of the object
	MMS3DMatrix base_matrix;
	if (this->scene->getResultMatrix(base_matrix))
		this->matrixStack.setBaseMatrix(base_matrix);
}

MMS3DObject::MMS3DObject(MMS3DScene *scene, int material, int texture) {
	this->scene = scene;

	// put new object into scene
	this->obj_id = this->scene->newObject(this);
	if (this->obj_id >= 0) {
		// get access to it
		MMS3D_OBJECT *obj = this->scene->getObject(this->obj_id);

		// set material and texture indices to object structure
		obj->material = material;
		obj->texture = texture;
	}

	this->parent = NULL;

	// set the base matrix of the object
	MMS3DMatrix base_matrix;
	if (this->scene->getResultMatrix(base_matrix))
		this->matrixStack.setBaseMatrix(base_matrix);
}

bool MMS3DObject::addObject(MMS3DObject *object) {
	if (!object) return false;
	if (object->obj_id >= 0) {
		MMS3D_OBJECT *obj = object->scene->getObject(object->obj_id);
		if (this->obj_id >= 0) {
			MMS3D_OBJECT *tobj = this->scene->getObject(this->obj_id);
			obj->parent = tobj;
		}
	}

	object->parent = this;

	// set the base matrix of the object
	MMS3DMatrix base_matrix;
	if (object->parent->getResultMatrix(base_matrix))
		object->matrixStack.setBaseMatrix(base_matrix);


	this->children.push_back(object);
	return true;
}

bool MMS3DObject::show() {
	if (this->obj_id >= 0) {
		MMS3D_OBJECT *obj = this->scene->getObject(this->obj_id);
		obj->shown = true;
		return true;
	}

	return false;
}

bool MMS3DObject::hide() {
	if (this->obj_id >= 0) {
		MMS3D_OBJECT *obj = this->scene->getObject(this->obj_id);
		obj->shown = false;
		return true;
	}

	return false;
}

bool MMS3DObject::cullFace(bool cullface) {
	if (this->obj_id >= 0) {
		MMS3D_OBJECT *obj = this->scene->getObject(this->obj_id);
		obj->cullface = cullface;
		return true;
	}

	return false;
}


void MMS3DObject::setBaseMatrix(MMS3DMatrix matrix) {
	this->matrixStack.setBaseMatrix(matrix);
}

bool MMS3DObject::getResultMatrix(MMS3DMatrix result) {
	if (!this->matrixStack.getResultMatrix(result))
		return false;

	// store the result matrix into object's attribute
	if (this->obj_id >= 0) {
		MMS3D_OBJECT *obj = this->scene->getObject(this->obj_id);
		copyMatrix(obj->matrix, result);
	}

	return true;
}

bool MMS3DObject::genMatrices() {
	// get result matrix of object used as base matrix for children
	MMS3DMatrix base_matrix;
	if (!getResultMatrix(base_matrix)) return false;

	// through children
	for (int i = 0; i < this->children.size(); i++) {
		// set the base matrix of the object
		this->children.at(i)->setBaseMatrix(base_matrix);

		// generate the matrices for the object and it's children
		this->children.at(i)->genMatrices();
	}

	return true;
}


void MMS3DObject::reset() {
	this->matrixStack.clear();
}

bool MMS3DObject::scale(float sx, float sy, float sz) {
	return this->matrixStack.scale(sx, sy, sz);
}

bool MMS3DObject::translate(float tx, float ty, float tz) {
	return this->matrixStack.translate(tx, ty, tz);
}

bool MMS3DObject::rotate(float angle, float x, float y, float z) {
	return this->matrixStack.rotate(angle, x, y, z);
}

