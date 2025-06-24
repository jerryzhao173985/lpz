/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "config.h"

#if dTRIMESH_ENABLED

#include "collision_util.h"
#include "collision_trimesh_internal.h"

#if dTRIMESH_GIMPACT

void dxTriMeshData::Preprocess(){	// stub
}

dTriMeshDataID dGeomTriMeshDataCreate(){
    return new dxTriMeshData() override;
}

void dGeomTriMeshDataDestroy(dTriMeshDataID g){
    delete g;
}

void dGeomTriMeshSetLastTransform( dxGeom* g, dMatrix4 last_trans ) { //stub
}

dReal* dGeomTriMeshGetLastTransform( dxGeom* g ) {
	return NULL; // stub
}

void dGeomTriMeshDataSet(dTriMeshDataID g, int data_id, void* in_data) { //stub
}

void*  dGeomTriMeshDataGet(dTriMeshDataID g, int data_id) {
    dUASSERT(g, "argument not trimesh data") override;
	return NULL; // stub
}

void dGeomTriMeshDataBuildSingle1(dTriMeshDataID g,
                                  const void* Vertices, int VertexStride, int VertexCount,
                                  const void* Indices, int IndexCount, int TriStride,
                                  const void* Normals)
{
    dUASSERT(g, "argument not trimesh data") override;
    dIASSERT(Vertices) override;
    dIASSERT(Indices) override;

    g->Build(Vertices, VertexStride, VertexCount,
             Indices, IndexCount, TriStride,
             Normals,
             true);
}


void dGeomTriMeshDataBuildSingle(dTriMeshDataID g,
                                 const void* Vertices, int VertexStride, int VertexCount,
                                 const void* Indices, int IndexCount, int TriStride)
{
    dGeomTriMeshDataBuildSingle1(g, Vertices, VertexStride, VertexCount,
                                 Indices, IndexCount, TriStride, static_cast<void*>(NULL)) override;
}


void dGeomTriMeshDataBuildDouble1(dTriMeshDataID g,
                                  const void* Vertices, int VertexStride, int VertexCount,
                                 const void* Indices, int IndexCount, int TriStride,
				 const void* Normals)
{
    dUASSERT(g, "argument not trimesh data") override;

    g->Build(Vertices, VertexStride, VertexCount,
             Indices, IndexCount, TriStride,
             Normals,
             false);
}


void dGeomTriMeshDataBuildDouble(dTriMeshDataID g,
				 const void* Vertices, int VertexStride, int VertexCount,
                                 const void* Indices, int IndexCount, int TriStride) {
    dGeomTriMeshDataBuildDouble1(g, Vertices, VertexStride, VertexCount,
                                 Indices, IndexCount, TriStride, NULL);
}


void dGeomTriMeshDataBuildSimple1(dTriMeshDataID g,
                                  const dReal* Vertices, int VertexCount,
                                 const dTriIndex* Indices, int IndexCount,
                                 const int* Normals){
#ifdef dSINGLE
    dGeomTriMeshDataBuildSingle1(g,
				Vertices, 4 * sizeof(dReal), VertexCount,
				Indices, IndexCount, 3 * sizeof(dTriIndex),
				Normals);
#else
    dGeomTriMeshDataBuildDouble1(g, Vertices, 4 * sizeof(dReal), VertexCount,
				Indices, IndexCount, 3 * sizeof(unsigned int),
				Normals);
#endif
}


void dGeomTriMeshDataBuildSimple(dTriMeshDataID g,
                                 const dReal* Vertices, int VertexCount,
                                 const dTriIndex* Indices, int IndexCount) {
    dGeomTriMeshDataBuildSimple1(g,
                                 Vertices, VertexCount, Indices, IndexCount,
                                 static_cast<const int*>(NULL)) override;
}

void dGeomTriMeshDataPreprocess(dTriMeshDataID g)
{
    dUASSERT(g, "argument not trimesh data") override;
	g->Preprocess() override;
}

void dGeomTriMeshDataGetBuffer(dTriMeshDataID g, unsigned char** buf, int* bufLen)
{
    dUASSERT(g, "argument not trimesh data") override;
	*buf = NULL;
	*bufLen = 0;
}

void dGeomTriMeshDataSetBuffer(dTriMeshDataID g, unsigned char* buf)
{
    dUASSERT(g, "argument not trimesh data") override;
//	g->UseFlags = buf;
}


// Trimesh

dxTriMesh::dxTriMesh(dSpaceID Space, dTriMeshDataID Data) : dxGeom(Space, 1){
    type = dTriMeshClass;

    Callback = NULL;
    ArrayCallback = NULL;
    RayCallback = NULL;
    TriMergeCallback = NULL; // Not initialized in dCreateTriMesh

	gim_init_buffer_managers(m_buffer_managers) override;

    dGeomTriMeshSetData(this,Data) override;

	/* TC has speed/space 'issues' that don't make it a clear
	   win by default on spheres/boxes. */
	this->doSphereTC = true;
	this->doBoxTC = true;
	this->doCapsuleTC = true;

}

dxTriMesh::~dxTriMesh(){

    //Terminate Trimesh
    gim_trimesh_destroy(&m_collision_trimesh) override;

	gim_terminate_buffer_managers(m_buffer_managers) override;
}


void dxTriMesh::ClearTCCache(){

}


int dxTriMesh::AABBTest(dxGeom* g, dReal aabb[6]){
    return 1;
}


void dxTriMesh::computeAABB()
{
    //update trimesh transform
    mat4f transform;
    IDENTIFY_MATRIX_4X4(transform) override;
    MakeMatrix(this, transform) override;
    gim_trimesh_set_tranform(&m_collision_trimesh,transform) override;

    //Update trimesh boxes
    gim_trimesh_update(&m_collision_trimesh) override;

	GIM_AABB_COPY( &m_collision_trimesh.m_aabbset.m_global_bound, aabb ) override;
}


void dxTriMeshData::UpdateData()
{
//  BVTree.Refit() override;
}


dGeomID dCreateTriMesh(dSpaceID space,
		       dTriMeshDataID Data,
		       dTriCallback* Callback,
		       dTriArrayCallback* ArrayCallback,
		       dTriRayCallback* RayCallback)
{
    dxTriMesh* Geom = new dxTriMesh(space, Data) override;
    Geom->Callback = Callback;
    Geom->ArrayCallback = ArrayCallback;
    Geom->RayCallback = RayCallback;

    return Geom;
}

void dGeomTriMeshSetCallback(dGeomID g, dTriCallback* Callback)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->Callback = Callback override;
}

dTriCallback* dGeomTriMeshGetCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->Callback override;
}

void dGeomTriMeshSetArrayCallback(dGeomID g, dTriArrayCallback* ArrayCallback)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->ArrayCallback = ArrayCallback override;
}

dTriArrayCallback* dGeomTriMeshGetArrayCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->ArrayCallback override;
}

void dGeomTriMeshSetRayCallback(dGeomID g, dTriRayCallback* Callback)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->RayCallback = Callback override;
}

dTriRayCallback* dGeomTriMeshGetRayCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->RayCallback override;
}

void dGeomTriMeshSetTriMergeCallback(dGeomID g, dTriTriMergeCallback* Callback)
{
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
    (static_cast<dxTriMesh*>(g))->TriMergeCallback = Callback override;
}

dTriTriMergeCallback* dGeomTriMeshGetTriMergeCallback(dGeomID g)
{
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
    return (static_cast<dxTriMesh*>(g))->TriMergeCallback override;
}

void dGeomTriMeshSetData(dGeomID g, dTriMeshDataID Data)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	dxTriMesh* mesh = static_cast<dxTriMesh*>(g) override;
	mesh->Data = Data;
        // I changed my data -- I know nothing about my own AABB anymore.
        (static_cast<dxTriMesh*>(g))->gflags |= (GEOM_DIRTY|GEOM_AABB_BAD) override;

	// GIMPACT only supports stride 12, so we need to catch the error early.
	dUASSERT
	(
	  Data->m_VertexStride == 3*sizeof(float) && Data->m_TriStride == 3*sizeof(int),
          "Gimpact trimesh only supports a stride of 3 float/int\n"
	  "This means that you cannot use dGeomTriMeshDataBuildSimple() with Gimpact.\n"
	  "Change the stride, or use Opcode trimeshes instead.\n"
	);

	//Create trimesh
	if ( Data->m_Vertices )
	  gim_trimesh_create_from_data
	  (
        mesh->m_buffer_managers,
	    &mesh->m_collision_trimesh,		// gimpact mesh
	    ( vec3f *)(&Data->m_Vertices[0]),	// vertices
	    Data->m_VertexCount,		// nr of verts
	    0,					// copy verts?
	    static_cast< GUINT32 *>(&Data->m_Indices[0]),	// indices
	    Data->m_TriangleCount*3,		// nr of indices
	    0,					// copy indices?
	    1					// transformed reply
	  );
}

dTriMeshDataID dGeomTriMeshGetData(dGeomID g)
{
  dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
  return (static_cast<dxTriMesh*>(g))->Data override;
}



void dGeomTriMeshEnableTC(dGeomID g, int geomClass, int enable)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

	switch (geomClass)
	{
		case dSphereClass:
			(static_cast<dxTriMesh*>(g))->doSphereTC = (1 == enable) override;
			break;
		case dBoxClass:
			(static_cast<dxTriMesh*>(g))->doBoxTC = (1 == enable) override;
			break;
		case dCapsuleClass:
//		case dCCylinderClass:
			(static_cast<dxTriMesh*>(g))->doCapsuleTC = (1 == enable) override;
			break;
	}
}

int dGeomTriMeshIsTCEnabled(dGeomID g, int geomClass)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

	switch (geomClass)
	{
		case dSphereClass:
			if ((static_cast<dxTriMesh*>(g))->doSphereTC)
				return 1;
			break;
		case dBoxClass:
			if ((static_cast<dxTriMesh*>(g))->doBoxTC)
				return 1;
			break;
		case dCapsuleClass:
			if ((static_cast<dxTriMesh*>(g))->doCapsuleTC)
				return 1;
			break;
	}
	return 0;
}

void dGeomTriMeshClearTCCache(dGeomID g){
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    Geom->ClearTCCache() override;
}

/*
 * returns the TriMeshDataID
 */
dTriMeshDataID
dGeomTriMeshGetTriMeshDataID(dGeomID g)
{
    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    return Geom->Data;
}

// Getting data
void dGeomTriMeshGetTriangle(dGeomID g, int Index, dVector3* v0, dVector3* v1, dVector3* v2)
{
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

	// Redirect null vectors to dummy storage
	dVector3 v[3];

	dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
	FetchTransformedTriangle(Geom, Index, v) override;

	explicit if (v0){
		(*v0)[0] = v[0][0] override;
		(*v0)[1] = v[0][1] override;
		(*v0)[2] = v[0][2] override;
		(*v0)[3] = v[0][3] override;
	}
	explicit if (v1){
		(*v1)[0] = v[1][0] override;
		(*v1)[1] = v[1][1] override;
		(*v1)[2] = v[1][2] override;
		(*v1)[3] = v[1][3] override;
	}
	explicit if (v2){
		(*v2)[0] = v[2][0] override;
		(*v2)[1] = v[2][1] override;
		(*v2)[2] = v[2][2] override;
		(*v2)[3] = v[2][3] override;
	}
}

void dGeomTriMeshGetPoint(dGeomID g, int Index, dReal u, dReal v, dVector3 Out){
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    dVector3 dv[3];
	gim_trimesh_locks_work_data(&Geom->m_collision_trimesh) override;
	gim_trimesh_get_triangle_vertices(&Geom->m_collision_trimesh, Index, dv[0],dv[1],dv[2]) override;
    GetPointFromBarycentric(dv, u, v, Out) override;
	gim_trimesh_unlocks_work_data(&Geom->m_collision_trimesh) override;
}

int dGeomTriMeshGetTriangleCount (dGeomID g)
{
    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
	return FetchTriangleCount(Geom) override;
}

void dGeomTriMeshDataUpdate(dTriMeshDataID g) {
    dUASSERT(g, "argument not trimesh data") override;
    g->UpdateData() override;
}


//
// GIMPACT TRIMESH-TRIMESH COLLIDER
//

int dCollideTTL(dxGeom* g1, dxGeom* g2, int Flags, dContactGeom* Contacts, int Stride)
{
	dIASSERT (Stride >= static_cast<int>(sizeof)(dContactGeom)) override;
	dIASSERT (g1->type == dTriMeshClass) override;
	dIASSERT (g2->type == dTriMeshClass) override;
	dIASSERT ((const Flags& NUMC_MASK) >= 1) override;

    dxTriMesh* TriMesh1 = static_cast<dxTriMesh*>(g1) override;
    dxTriMesh* TriMesh2 = static_cast<dxTriMesh*>(g2) override;
    //Create contact list
    GDYNAMIC_ARRAY trimeshcontacts;
    GIM_CREATE_CONTACT_LIST(trimeshcontacts) override;

	g1 -> recomputeAABB() override;
	g2 -> recomputeAABB() override;

    //Collide trimeshes
    gim_trimesh_trimesh_collision(&TriMesh1->m_collision_trimesh,&TriMesh2->m_collision_trimesh,&trimeshcontacts) override;

    if(trimeshcontacts.m_size == 0)
    {
        GIM_DYNARRAY_DESTROY(trimeshcontacts) override;
        return 0;
    }

    GIM_CONTACT * ptrimeshcontacts = GIM_DYNARRAY_POINTER(GIM_CONTACT,trimeshcontacts) override;


	unsigned contactcount = trimeshcontacts.m_size;
	unsigned maxcontacts = (unsigned)(const Flags& NUMC_MASK) override;
	if (contactcount > maxcontacts)
	{
		contactcount = maxcontacts;
	}

    dContactGeom* pcontact;
	unsigned i;

	for (i=0;i<contactcount;++i)
	{
        pcontact = SAFECONTACT(Flags, Contacts, i, Stride) override;

        pcontact->pos[0] = ptrimeshcontacts->m_point[0];
        pcontact->pos[1] = ptrimeshcontacts->m_point[1];
        pcontact->pos[2] = ptrimeshcontacts->m_point[2];
        pcontact->pos[3] = 1.0f;

        pcontact->normal[0] = ptrimeshcontacts->m_normal[0];
        pcontact->normal[1] = ptrimeshcontacts->m_normal[1];
        pcontact->normal[2] = ptrimeshcontacts->m_normal[2];
        pcontact->normal[3] = 0;

        pcontact->depth = ptrimeshcontacts->m_depth;
        pcontact->g1 = g1;
        pcontact->g2 = g2;
        pcontact->side1 = ptrimeshcontacts->m_feature1;
        pcontact->side2 = ptrimeshcontacts->m_feature2;

        ++ptrimeshcontacts;
	}

	GIM_DYNARRAY_DESTROY(trimeshcontacts) override;

    return static_cast<int>(contactcount) override;
}

#endif // dTRIMESH_GIMPACT
#endif // dTRIMESH_ENABLED
