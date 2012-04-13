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

#ifndef MMS3DOBJECT_H_
#define MMS3DOBJECT_H_

#include "mmsgui/3d/mms3dmatrixstack.h"

#include <vector>

class MMS3DObject {
private:
	//! scene in which the object is drawn
	class MMS3DScene *scene;

	//! id of the object in the scene
	int obj_id;

	//! parent object or NULL
	MMS3DObject *parent;

	//! stores base matrix and matrix operations
	MMS3DMatrixStack	matrixStack;

	//! children objects
	vector<MMS3DObject*> children;


	void setBaseMatrix(MMS3DMatrix matrix);

	bool getResultMatrix(MMS3DMatrix result);

	bool genMatrices();

public:
	MMS3DObject(class MMS3DScene *scene);

	MMS3DObject(class MMS3DScene *scene, int material, int texture);

	bool addObject(MMS3DObject *object);

	bool show();

	bool hide();

	bool cullFace(bool cullface);

	void reset();

	bool scale(float sx, float sy, float sz);

	bool translate(float tx, float ty, float tz);

	bool rotate(float angle, float x, float y, float z);


	friend class MMS3DScene;
	friend class MMS3DRectangle;
	friend class MMS3DSphere;
	friend class MMS3DTorus;
	friend class MMS3DCylinder;
};

#endif /* MMS3DOBJECT_H_ */
