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

#include "mmsgui/3d/mms3dpolygonmesh.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


MMS3DPolygonMesh::MMS3DPolygonMesh() {

	// reset number of items to zero
	this->pm_items_cnt = 0;

	// init vertex and index array
	this->varrays_cnt = 0;
	this->varrays[varrays_cnt] = NULL;
	this->iarrays_cnt = 0;
	this->iarrays[iarrays_cnt] = NULL;
}

void MMS3DPolygonMesh::genRectangle(float width, float height,
					MMS3D_VERTEX_ARRAY	*vertices,
					MMS3D_VERTEX_ARRAY	*normals,
					MMS3D_VERTEX_ARRAY	*texcoords,
					MMS3D_INDEX_ARRAY		*indices) {


	// allocate memory for buffers
	if (vertices) {
		vertices->eSize = 2;
		vertices->eNum = 4;
		vertices->buf = (float *)malloc(sizeof(float) * vertices->eSize * vertices->eNum);
	}

	if (normals) {
		normals->eSize = 3;
		normals->eNum = 4;
		normals->buf = (float *)malloc(sizeof(float) * normals->eSize * normals->eNum);
	}

	if (texcoords) {
		texcoords->eSize = 2;
		texcoords->eNum = 4;
		texcoords->buf = (float *)malloc(sizeof(float) * texcoords->eSize * texcoords->eNum);
	}

	if (indices) {
		indices->type = MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP;
		indices->eNum = 4;
		indices->buf = (unsigned int *)malloc(sizeof(unsigned int) * indices->eNum);
	}



	// calculate buffers
	if (vertices) {
		vertices->buf[0*2 + 0] = -width/2;
		vertices->buf[0*2 + 1] = -height/2;
		vertices->buf[1*2 + 0] = width/2;
		vertices->buf[1*2 + 1] = -height/2;
		vertices->buf[2*2 + 0] = -width/2;
		vertices->buf[2*2 + 1] = height/2;
		vertices->buf[3*2 + 0] = width/2;
		vertices->buf[3*2 + 1] = height/2;
	}

	if (normals) {
		normals->buf[0*3 + 0] = 0;
		normals->buf[0*3 + 1] = 0;
		normals->buf[0*3 + 2] = 1;
		normals->buf[1*3 + 0] = 0;
		normals->buf[1*3 + 1] = 0;
		normals->buf[1*3 + 2] = 1;
		normals->buf[2*3 + 0] = 0;
		normals->buf[2*3 + 1] = 0;
		normals->buf[2*3 + 2] = 1;
		normals->buf[3*3 + 0] = 0;
		normals->buf[3*3 + 1] = 0;
		normals->buf[3*3 + 2] = 1;
	}

	if (texcoords) {
		texcoords->buf[0*2 + 0] = 0;
		texcoords->buf[0*2 + 1] = 0;
		texcoords->buf[1*2 + 0] = 1;
		texcoords->buf[1*2 + 1] = 0;
		texcoords->buf[2*2 + 0] = 0;
		texcoords->buf[2*2 + 1] = 1;
		texcoords->buf[3*2 + 0] = 1;
		texcoords->buf[3*2 + 1] = 1;
	}

	if (indices) {
		indices->buf[0] = 0;
		indices->buf[1] = 1;
		indices->buf[2] = 2;
		indices->buf[3] = 3;
	}
}

void MMS3DPolygonMesh::genSphere(int numSlices, float radius,
				MMS3D_VERTEX_ARRAY	*vertices,
				MMS3D_VERTEX_ARRAY	*normals,
				MMS3D_VERTEX_ARRAY	*texcoords,
				MMS3D_INDEX_ARRAY		*indices) {
	int i;
	int j;
	int numParallels = numSlices / 2;
	float angleStep = (2.0f * MMS3D_PI) / ((float) numSlices);



	// allocate memory for buffers
	if (vertices) {
		vertices->eSize = 3;
		vertices->eNum = ( numParallels + 1 ) * ( numSlices + 1 );
		vertices->buf = (float *)malloc(sizeof(float) * vertices->eSize * vertices->eNum);
	}

	if (normals) {
		normals->eSize = 3;
		normals->eNum = ( numParallels + 1 ) * ( numSlices + 1 );
		normals->buf = (float *)malloc(sizeof(float) * normals->eSize * normals->eNum);
	}

	if (texcoords) {
		texcoords->eSize = 2;
		texcoords->eNum = ( numParallels + 1 ) * ( numSlices + 1 );
		texcoords->buf = (float *)malloc(sizeof(float) * texcoords->eSize * texcoords->eNum);
	}

	if (indices) {
		indices->type = MMS3D_INDEX_ARRAY_TYPE_TRIANGLES;
		indices->eNum = numParallels * numSlices * 6;
		indices->buf = (unsigned int *)malloc(sizeof(unsigned int) * indices->eNum);
	}


	for ( i = 0; i < numParallels + 1; i++ ) {
	  for ( j = 0; j < numSlices + 1; j++ ) {
		 int vertex = ( i * (numSlices + 1) + j ) * vertices->eSize;

		 if (vertices) {
			vertices->buf[vertex + 0] = radius * sinf ( angleStep * (float)i ) *
											   sinf ( angleStep * (float)j );
			vertices->buf[vertex + 1] = radius * cosf ( angleStep * (float)i );
			vertices->buf[vertex + 2] = radius * sinf ( angleStep * (float)i ) *
											   cosf ( angleStep * (float)j );
		 }

		 if (normals) {
			normals->buf[vertex + 0] = vertices->buf[vertex + 0] / radius;
			normals->buf[vertex + 1] = vertices->buf[vertex + 1] / radius;
			normals->buf[vertex + 2] = vertices->buf[vertex + 2] / radius;
		 }

		 if (texcoords) {
			int texIndex = ( i * (numSlices + 1) + j ) * texcoords->eSize;
			texcoords->buf[texIndex + 0] = (float) j / (float) numSlices;
			texcoords->buf[texIndex + 1] = ( 1.0f - (float) i ) / (float) (numParallels - 1 );
		 }
	  }
	}

	// generate the indices
	if (indices) {
	  unsigned int *indexBuf = indices->buf;
	  for ( i = 0; i < numParallels ; i++) {
		 for ( j = 0; j < numSlices; j++) {
			*indexBuf++  = i * ( numSlices + 1 ) + j;
			*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + j;
			*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );

			*indexBuf++ = i * ( numSlices + 1 ) + j;
			*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
			*indexBuf++ = i * ( numSlices + 1 ) + ( j + 1 );
		 }
	  }
	}
}

void MMS3DPolygonMesh::genTorus(int numwraps, int numperwrap, float majorradius, float minorradius,
				MMS3D_VERTEX_ARRAY	*vertices,
				MMS3D_VERTEX_ARRAY	*normals,
				MMS3D_VERTEX_ARRAY	*texcoords,
				MMS3D_INDEX_ARRAY		*indices) {


	// we use triangle strip, so same number of elements for all buffers
	int eNum = (numwraps * numperwrap + 1) * 2;

	// allocate memory for buffers
	if (vertices) {
		vertices->eSize = 3;
		vertices->eNum = eNum;
		vertices->buf = (float *)malloc(sizeof(float) * vertices->eSize * vertices->eNum);
	}

	if (normals) {
		normals->eSize = 3;
		normals->eNum = eNum;
		normals->buf = (float *)malloc(sizeof(float) * normals->eSize * normals->eNum);
	}

	if (texcoords) {
		texcoords->eSize = 2;
		texcoords->eNum = eNum;
		texcoords->buf = (float *)malloc(sizeof(float) * texcoords->eSize * texcoords->eNum);
	}

	if (indices) {
		indices->type = MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP;
		indices->eNum = eNum;
		indices->buf = (unsigned int *)malloc(sizeof(unsigned int) * indices->eNum);
	}


	// init buffer index
	int index = 0;

	// calculate it
	float PI2 = 2.0f * MMS3D_PI;
	bool final = false;
	while (1) {
		for (int i = 0; i < numwraps; i++) {
			for (int j = 0; j < numperwrap; j++) {

				float wrapFrac = (j % numperwrap) / (float)numperwrap;
				float phi = PI2*wrapFrac;
				float sinphi = sin(phi);
				float cosphi = cos(phi);
				float y = minorradius * sinphi;
				float r = majorradius + minorradius * cosphi;

				for (int k = i; k <= i + 1; k++) {

					float theta = PI2 * (k % numwraps+wrapFrac) / (float)numwraps;
					float sintheta = sin(theta);
					float costheta = cos(theta);
					float x = sintheta * r;
					float z = costheta * r;

					if (normals) {
						normals->buf[index * 3 + 0] = sintheta * cosphi;
						normals->buf[index * 3 + 1] = sinphi;
						normals->buf[index * 3 + 2] = costheta * cosphi;
					}

					if (vertices) {
						vertices->buf[index * 3 + 0] = x;
						vertices->buf[index * 3 + 1] = y;
						vertices->buf[index * 3 + 2] = z;
					}

					if (indices) {
						indices->buf[index] = index;
					}

					index++;
				}

				if (final) break;
			}
			if (final) break;
		}
		if (final) break;
		final = true;
	}
}

void MMS3DPolygonMesh::genCylinder(int numSlices, float height, float radius,
					MMS3D_VERTEX_ARRAY	*vertices,
					MMS3D_VERTEX_ARRAY	*normals,
					MMS3D_VERTEX_ARRAY	*texcoords,
					MMS3D_INDEX_ARRAY		*indices) {


	// we use triangle strip, so same number of elements for all buffers
	int eNum = (numSlices + 1) * 2;

	// allocate memory for buffers
	if (vertices) {
		vertices->eSize = 3;
		vertices->eNum = eNum;
		vertices->buf = (float *)malloc(sizeof(float) * vertices->eSize * vertices->eNum);
	}

	if (normals) {
		normals->eSize = 3;
		normals->eNum = eNum;
		normals->buf = (float *)malloc(sizeof(float) * normals->eSize * normals->eNum);
	}

	if (texcoords) {
		texcoords->eSize = 2;
		texcoords->eNum = eNum;
		texcoords->buf = (float *)malloc(sizeof(float) * texcoords->eSize * texcoords->eNum);
	}

	if (indices) {
		indices->type = MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP;
		indices->eNum = eNum;
		indices->buf = (unsigned int *)malloc(sizeof(unsigned int) * indices->eNum);
	}

	// init buffer index
	int index = 0;

	// calculate it
	float angleStep = 2.0f * MMS3D_PI / ((float) numSlices);
	float z0 = 0.5 * height;
	float z1 = z0 - height;

	for (int j = 0; j <= numSlices; j++) {
		float a = j * angleStep;
		float x = radius * cos(a);
		float y = radius * sin(a);

		if (normals) {
			normals->buf[index * 3 + 0] = normals->buf[index * 3 + 3] = x / radius;
			normals->buf[index * 3 + 1] = normals->buf[index * 3 + 4] = y / radius;
			normals->buf[index * 3 + 2] = normals->buf[index * 3 + 5] = 0;
		}

		if (vertices) {
			vertices->buf[index * 3 + 0] = vertices->buf[index * 3 + 3] = x;
			vertices->buf[index * 3 + 1] = vertices->buf[index * 3 + 4] = y;
			vertices->buf[index * 3 + 2] = z0;
			vertices->buf[index * 3 + 5] = z1;
		}

		if (texcoords) {
			texcoords->buf[index * 2 + 0] = texcoords->buf[index * 2 + 2] = j / (float) numSlices;
			texcoords->buf[index * 2 + 1] = 0;
			texcoords->buf[index * 2 + 3] = 1;
		}

		if (indices) {
			indices->buf[index]   = index;
			indices->buf[index+1] = index+1;
		}

		index+=2;
	}
}


int MMS3DPolygonMesh::findPMItem(MMS3DPM_TYPE type, float identifier[4], int *vertices, int *normals, int *texcoords, int *indices) {
	for (int i = 0; i < this->pm_items_cnt; i++) {
		MMS3DPM_ITEM *item = &this->pm_items[i];

		if (item->type != type)
			continue;
		if (memcmp(item->identifier, identifier, sizeof(identifier)))
			continue;
		if (vertices && item->vertices < 0)
			continue;
		if (normals && item->normals < 0)
			continue;
		if (texcoords && item->texcoords < 0)
			continue;
		if (indices && item->indices < 0)
			continue;

		// successfully found, return buffer indices
		if (vertices) *vertices  = item->vertices;
		if (normals)  *normals   = item->normals;
		if (texcoords)*texcoords = item->texcoords;
		if (indices)  *indices   = item->indices;

		return i;
	}

	return -1;
}

int MMS3DPolygonMesh::newPMItem(MMS3DPM_TYPE type, float identifier[4], int *vertices, int *normals, int *texcoords, int *indices) {
	if (this->pm_items_cnt >= MMS3DPM_ITEM_MAX) {
		// no more space
		return -1;
	}

	// get new item
	MMS3DPM_ITEM *item = &this->pm_items[this->pm_items_cnt];
	this->pm_items_cnt++;
	item->type = type;
	memcpy(item->identifier, identifier, sizeof(identifier));

	// get new indices for the new item
	if (vertices) {
		item->vertices = *vertices = this->varrays_cnt;
		this->varrays[item->vertices] = &this->vabuf[this->varrays_cnt];
		this->varrays_cnt++;
		this->varrays[this->varrays_cnt] = NULL;
	}
	else {
		item->vertices = -1;
	}
	if (normals) {
		item->normals = *normals = this->varrays_cnt;
		this->varrays[item->normals] = &this->vabuf[this->varrays_cnt];
		this->varrays_cnt++;
		this->varrays[this->varrays_cnt] = NULL;
	}
	else {
		item->normals = -1;
	}
	if (texcoords) {
		item->texcoords = *texcoords = varrays_cnt;
		varrays[item->texcoords] = &this->vabuf[this->varrays_cnt];
		varrays_cnt++;
		varrays[varrays_cnt] = NULL;
	}
	else {
		item->texcoords = -1;
	}
	if (indices) {
		item->indices = *indices = this->iarrays_cnt;
		this->iarrays[item->indices] = &this->iabuf[this->iarrays_cnt];
		this->iarrays_cnt++;
		this->iarrays[this->iarrays_cnt] = NULL;
	}
	else {
		item->indices = -1;
	}

	switch (type) {
	case MMS3DPM_TYPE_RECTANGLE:
		genRectangle(identifier[0], identifier[1],
					(vertices) ? this->varrays[*vertices] : NULL,
					(normals)  ? this->varrays[*normals]  : NULL,
					(texcoords)? this->varrays[*texcoords]: NULL,
					(indices)  ? this->iarrays[*indices]  : NULL);
		break;
	case MMS3DPM_TYPE_SPHERE:
		genSphere(identifier[0], identifier[1],
					(vertices) ? this->varrays[*vertices] : NULL,
					(normals)  ? this->varrays[*normals]  : NULL,
					(texcoords)? this->varrays[*texcoords]: NULL,
					(indices)  ? this->iarrays[*indices]  : NULL);
		break;
	case MMS3DPM_TYPE_TORUS:
		genTorus(identifier[0], identifier[1], identifier[2], identifier[3],
					(vertices) ? this->varrays[*vertices] : NULL,
					(normals)  ? this->varrays[*normals]  : NULL,
					(texcoords)? this->varrays[*texcoords]: NULL,
					(indices)  ? this->iarrays[*indices]  : NULL);
		break;
	case MMS3DPM_TYPE_CYLINDER:
		genCylinder(identifier[0], identifier[1], identifier[2],
					(vertices) ? this->varrays[*vertices] : NULL,
					(normals)  ? this->varrays[*normals]  : NULL,
					(texcoords)? this->varrays[*texcoords]: NULL,
					(indices)  ? this->iarrays[*indices]  : NULL);
		break;
	}

	return this->pm_items_cnt - 1;
}



void MMS3DPolygonMesh::getArrays(MMS3D_VERTEX_ARRAY ***varrays, MMS3D_INDEX_ARRAY ***iarrays) {
	*varrays = this->varrays;
	*iarrays = this->iarrays;
}

bool MMS3DPolygonMesh::genRectangle(float width, float height, int *vertices, int	*normals, int *texcoords, int *indices) {
	float identifier[4] = {width, height, 0, 0};
	if (findPMItem(MMS3DPM_TYPE_RECTANGLE, identifier, vertices, normals, texcoords, indices) < 0) {
		// rectangle mesh does not exist, generate it
		return (newPMItem(MMS3DPM_TYPE_RECTANGLE, identifier, vertices, normals, texcoords, indices) >= 0);
	}
	return true;
}

bool MMS3DPolygonMesh::genSphere(int numSlices, float radius, int	*vertices, int *normals, int *texcoords, int *indices) {
	float identifier[4] = {numSlices, radius, 0, 0};
	if (findPMItem(MMS3DPM_TYPE_SPHERE, identifier, vertices, normals, texcoords, indices) < 0) {
		// sphere mesh does not exist, generate it
		return (newPMItem(MMS3DPM_TYPE_SPHERE, identifier, vertices, normals, texcoords, indices) >= 0);
	}
	return true;
}

bool MMS3DPolygonMesh::genTorus(int numwraps, int numperwrap, float majorradius, float minorradius,
				int *vertices, int *normals, int *texcoords, int *indices) {
	float identifier[4] = {numwraps, numperwrap, majorradius, minorradius};
	if (findPMItem(MMS3DPM_TYPE_TORUS, identifier, vertices, normals, texcoords, indices) < 0) {
		// torus mesh does not exist, generate it
		return (newPMItem(MMS3DPM_TYPE_TORUS, identifier, vertices, normals, texcoords, indices) >= 0);
	}
	return true;
}

bool MMS3DPolygonMesh::genCylinder(int numSlices, float height, float radius,
					int *vertices, int *normals, int *texcoords, int *indices) {
	float identifier[4] = {numSlices, height, radius, 0};
	if (findPMItem(MMS3DPM_TYPE_CYLINDER, identifier, vertices, normals, texcoords, indices) < 0) {
		// cylinder mesh does not exist, generate it
		return (newPMItem(MMS3DPM_TYPE_CYLINDER, identifier, vertices, normals, texcoords, indices) >= 0);
	}
	return true;
}

