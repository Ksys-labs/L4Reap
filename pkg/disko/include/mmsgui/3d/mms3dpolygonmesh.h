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

#ifndef MMS3DPOLYGONMESH_H_
#define MMS3DPOLYGONMESH_H_

#include "mmstools/mmstypes.h"

class MMS3DPolygonMesh {
private:

	typedef enum {
		MMS3DPM_TYPE_RECTANGLE = 0,
		MMS3DPM_TYPE_SPHERE,
		MMS3DPM_TYPE_TORUS,
		MMS3DPM_TYPE_CYLINDER
	} MMS3DPM_TYPE;

	typedef struct {
		MMS3DPM_TYPE	type;
		float			identifier[4];
		int				vertices;
		int				normals;
		int				texcoords;
		int				indices;
	} MMS3DPM_ITEM;


	//! maximum number of separate meshes
	#define MMS3DPM_ITEM_MAX 50

	//! mesh items
	MMS3DPM_ITEM pm_items[MMS3DPM_ITEM_MAX];
	int pm_items_cnt;

	//! vertex arrays used by mesh items
	MMS3D_VERTEX_ARRAY vabuf[MMS3DPM_ITEM_MAX * 3];
	MMS3D_VERTEX_ARRAY *varrays[MMS3DPM_ITEM_MAX * 3 + 1];
	int varrays_cnt;

	//! index arrays used by mesh items
	MMS3D_INDEX_ARRAY iabuf[MMS3DPM_ITEM_MAX];
	MMS3D_INDEX_ARRAY *iarrays[MMS3DPM_ITEM_MAX + 1];
	int iarrays_cnt;


	void genRectangle(float width, float height,
						MMS3D_VERTEX_ARRAY	*vertices,
						MMS3D_VERTEX_ARRAY	*normals,
						MMS3D_VERTEX_ARRAY	*texcoords,
						MMS3D_INDEX_ARRAY		*indices);

	void genSphere(int numSlices, float radius,
					MMS3D_VERTEX_ARRAY	*vertices,
					MMS3D_VERTEX_ARRAY	*normals,
					MMS3D_VERTEX_ARRAY	*texcoords,
					MMS3D_INDEX_ARRAY		*indices);

	void genTorus(int numwraps, int numperwrap, float majorradius, float minorradius,
					MMS3D_VERTEX_ARRAY	*vertices,
					MMS3D_VERTEX_ARRAY	*normals,
					MMS3D_VERTEX_ARRAY	*texcoords,
					MMS3D_INDEX_ARRAY		*indices);

	void genCylinder(int numSlices, float height, float radius,
						MMS3D_VERTEX_ARRAY	*vertices,
						MMS3D_VERTEX_ARRAY	*normals,
						MMS3D_VERTEX_ARRAY	*texcoords,
						MMS3D_INDEX_ARRAY		*indices);


	int findPMItem(MMS3DPM_TYPE type, float identifier[4], int *vertices, int *normals, int *texcoords, int *indices);

	int newPMItem(MMS3DPM_TYPE type, float identifier[4], int *vertices, int *normals, int *texcoords, int *indices);


public:

	MMS3DPolygonMesh();

	void getArrays(MMS3D_VERTEX_ARRAY ***varrays, MMS3D_INDEX_ARRAY ***iarrays);

	bool genRectangle(float width, float height, int *vertices, int	*normals, int *texcoords, int *indices);

	bool genSphere(int numSlices, float radius, int	*vertices, int *normals, int *texcoords, int *indices);

	bool genTorus(int numwraps, int numperwrap, float majorradius, float minorradius,
					int *vertices, int *normals, int *texcoords, int *indices);

	bool genCylinder(int numSlices, float height, float radius,
						int *vertices, int *normals, int *texcoords, int *indices);
};

#endif /* MMS3DPOLYGONMESH_H_ */
