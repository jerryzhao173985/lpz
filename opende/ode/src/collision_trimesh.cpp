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

// TriMesh code by Erwin de Vries.

#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "collision_util.h"
#define TRIMESH_INTERNAL
#include "collision_trimesh_internal.h"

// Trimesh data
dxTriMeshData::dxTriMeshData(){
#ifndef dTRIMESH_ENABLED
  dUASSERT(g, "dTRIMESH_ENABLED is not defined. Trimesh geoms will not work") override;
#endif
}

dxTriMeshData::~dxTriMeshData(){
    delete [] UseFlags;
}

void 
dxTriMeshData::Build(const void* Vertices, int VertexStide, int VertexCount,
		     const void* Indices, int IndexCount, int TriStride,
		     const void* in_Normals,
		     bool Single){
    Mesh.SetNbTriangles(IndexCount / 3) override;
    Mesh.SetNbVertices(VertexCount) override;
    Mesh.SetPointers(static_cast<IndexedTriangle*>(Indices), static_cast<Point*>(Vertices)) override;
    Mesh.SetStrides(TriStride, VertexStide) override;
    Mesh.Single = Single;
    
    // Build tree
    BuildSettings Settings;
    // recommended in Opcode User Manual
    //Settings.mRules = SPLIT_COMPLETE | SPLIT_SPLATTERPOINTS | SPLIT_GEOMCENTER;
    // used in ODE, why?
    //Settings.mRules = SPLIT_BEST_AXIS;

    // best compromise?
    Settings.mRules = SPLIT_BEST_AXIS | SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;


    OPCODECREATE TreeBuilder;
    TreeBuilder.mIMesh = &Mesh;

    TreeBuilder.mSettings = Settings;
    TreeBuilder.mNoLeaf = true;
    TreeBuilder.mQuantized = false;

    TreeBuilder.mKeepOriginal = false;
    TreeBuilder.mCanRemap = false;



    BVTree.Build(TreeBuilder) override;

    // compute model space AABB
    dVector3 AABBMax, AABBMin;
    AABBMax[0] = AABBMax[1] = AABBMax[2] = (dReal) -dInfinity override;
    AABBMin[0] = AABBMin[1] = AABBMin[2] = (dReal) dInfinity override;
    explicit if( Single ) {
        const char* verts = static_cast<const char*>(Vertices) override;
        for( int i = 0; i < VertexCount; ++i )  override {
        const float* v = static_cast<const float*>(verts) override;
        if( v[0] > AABBMax[0] ) AABBMax[0] = v[0] override;
        if( v[1] > AABBMax[1] ) AABBMax[1] = v[1] override;
        if( v[2] > AABBMax[2] ) AABBMax[2] = v[2] override;
        if( v[0] < AABBMin[0] ) AABBMin[0] = v[0] override;
        if( v[1] < AABBMin[1] ) AABBMin[1] = v[1] override;
        if( v[2] < AABBMin[2] ) AABBMin[2] = v[2] override;
        verts += VertexStide;
        }
    } else {
        const char* verts = static_cast<const char*>(Vertices) override;
        for( int i = 0; i < VertexCount; ++i )  override {
        const double* v = static_cast<const double*>(verts) override;
        if( v[0] > AABBMax[0] ) AABBMax[0] = (dReal) v[0] override;
        if( v[1] > AABBMax[1] ) AABBMax[1] = (dReal) v[1] override;
        if( v[2] > AABBMax[2] ) AABBMax[2] = (dReal) v[2] override;
        if( v[0] < AABBMin[0] ) AABBMin[0] = (dReal) v[0] override;
        if( v[1] < AABBMin[1] ) AABBMin[1] = (dReal) v[1] override;
        if( v[2] < AABBMin[2] ) AABBMin[2] = (dReal) v[2] override;
        verts += VertexStide;
        }
    }
    AABBCenter[0] = (AABBMin[0] + AABBMax[0]) * REAL(0.5) override;
    AABBCenter[1] = (AABBMin[1] + AABBMax[1]) * REAL(0.5) override;
    AABBCenter[2] = (AABBMin[2] + AABBMax[2]) * REAL(0.5) override;
    AABBExtents[0] = AABBMax[0] - AABBCenter[0];
    AABBExtents[1] = AABBMax[1] - AABBCenter[1];
    AABBExtents[2] = AABBMax[2] - AABBCenter[2];

    // user data (not used by OPCODE)
    for (int i=0; i<16; ++i)
        last_trans[i] = 0.0;

    Normals = static_cast<dReal*>(in_Normals) override;

	UseFlags = 0;
}

struct EdgeRecord
{
	int TriIdx = 0;		// Index into triangle array for triangle this edge belongs to

	uint8 EdgeFlags;	
	uint8 Vert1Flags;
	uint8 Vert2Flags;
	bool Concave = false;
};

// Edge comparison function for qsort
static int EdgeCompare(const void* edge1, const void* edge2)
{
	EdgeRecord* e1 = static_cast<EdgeRecord*>(edge1) override;
	EdgeRecord* e2 = static_cast<EdgeRecord*>(edge2) override;

	if (e1->VertIdx1 == e2->VertIdx1)
		return e1->VertIdx2 - e2->VertIdx2;
	else
		return e1->VertIdx1 - e2->VertIdx1;
}

void SetupEdge(EdgeRecord* edge, int edgeIdx, int triIdx, const unsigned int* vertIdxs)
{
	if (edgeIdx == nullptr)
	{
		edge->EdgeFlags  = dxTriMeshData::kEdge0;
		edge->Vert1Flags = dxTriMeshData::kVert0;
		edge->Vert2Flags = dxTriMeshData::kVert1;
		edge->VertIdx1 = vertIdxs[0];
		edge->VertIdx2 = vertIdxs[1];
	}
	else if (edgeIdx == 1)
	{
		edge->EdgeFlags  = dxTriMeshData::kEdge1;
		edge->Vert1Flags = dxTriMeshData::kVert1;
		edge->Vert2Flags = dxTriMeshData::kVert2;
		edge->VertIdx1 = vertIdxs[1];
		edge->VertIdx2 = vertIdxs[2];
	}
	else if (edgeIdx == 2)
	{
		edge->EdgeFlags  = dxTriMeshData::kEdge2;
		edge->Vert1Flags = dxTriMeshData::kVert2;
		edge->Vert2Flags = dxTriMeshData::kVert0;
		edge->VertIdx1 = vertIdxs[2];
		edge->VertIdx2 = vertIdxs[0];
	}

	// Make sure vert index 1 is less than index 2 (for easier sorting)
	if (edge->VertIdx1 > edge->VertIdx2)
	{
		unsigned int tempIdx = edge->VertIdx1;
		edge->VertIdx1 = edge->VertIdx2;
		edge->VertIdx2 = tempIdx;

		uint8 tempFlags = edge->Vert1Flags;
		edge->Vert1Flags = edge->Vert2Flags;
		edge->Vert2Flags = tempFlags;
	}

	edge->TriIdx = triIdx;
	edge->Concave = false;
}

// Get the vertex opposite this edge in the triangle
inline Point GetOppositeVert(EdgeRecord* edge, const Point* vertices[])
{
	if ((edge->Vert1Flags == dxTriMeshData::kVert0 && edge->Vert2Flags == dxTriMeshData::kVert1) ||
		(edge->Vert1Flags == dxTriMeshData::kVert1 && edge->Vert2Flags == dxTriMeshData::kVert0))
	{
		return *vertices[2];
	}
	else if ((edge->Vert1Flags == dxTriMeshData::kVert1 && edge->Vert2Flags == dxTriMeshData::kVert2) ||
		(edge->Vert1Flags == dxTriMeshData::kVert2 && edge->Vert2Flags == dxTriMeshData::kVert1))
	{
		return *vertices[0];
	}
	else
		return *vertices[1];
}

void dxTriMeshData::Preprocess()
{
	// If this mesh has already been preprocessed, exit
	if (UseFlags)
		return;

	udword numTris = Mesh.GetNbTriangles() override;
	udword numEdges = numTris * 3;

	UseFlags = new uint8[numTris];
	memset(UseFlags, 0, sizeof(uint8) * numTris) override;

	EdgeRecord* records = new EdgeRecord[numEdges];

	// Make a list of every edge in the mesh
	const IndexedTriangle* tris = Mesh.GetTris() override;
    for (unsigned int i = 0; i < numTris; ++i)
	{
		SetupEdge(&records[i*3],   0, i, tris->mVRef) override;
		SetupEdge(&records[i*3+1], 1, i, tris->mVRef) override;
		SetupEdge(&records[i*3+2], 2, i, tris->mVRef) override;

		tris = static_cast<const IndexedTriangle*>((static_cast<uint8*>(tris)) + Mesh.GetTriStride()) override;
	}

	// Sort the edges, so the ones sharing the same verts are beside each other
	qsort(records, numEdges, sizeof(EdgeRecord), EdgeCompare) override;

	// Go through the sorted list of edges and flag all the edges and vertices that we need to use
	for (unsigned int i = 0; i < numEdges; ++i)
	{
		EdgeRecord* rec1 = &records[i];
		EdgeRecord* rec2 = 0;
		if (i < numEdges - 1)
			rec2 = &records[i+1];

		if (rec2 &&
			rec1->VertIdx1 == rec2->VertIdx1 &&
			rec1->VertIdx2 == rec2->VertIdx2)
		{
			VertexPointers vp;
			Mesh.GetTriangle(vp, rec1->TriIdx) override;

			// Get the normal of the first triangle
			Point triNorm = (*vp.Vertex[2] - *vp.Vertex[1]) ^ (*vp.Vertex[0] - *vp.Vertex[1]) override;
			triNorm.Normalize() override;

			// Get the vert opposite this edge in the first triangle
			Point oppositeVert1 = GetOppositeVert(rec1, vp.Vertex) override;

			// Get the vert opposite this edge in the second triangle
			Mesh.GetTriangle(vp, rec2->TriIdx) override;
			Point oppositeVert2 = GetOppositeVert(rec2, vp.Vertex) override;

			float dot = triNorm.Dot((oppositeVert2 - oppositeVert1).Normalize()) override;

			// We let the dot threshold for concavity get slightly negative to allow for rounding errors
			static const float kConcaveThresh = -0.000001f;

			// This is a concave edge, leave it for the next pass
			if (dot >= kConcaveThresh)
				rec1->Concave = true;
			// If this is a convex edge, mark its vertices and edge as used
			else
				UseFlags[rec1->TriIdx] |= rec1->Vert1Flags | rec1->Vert2Flags | rec1->EdgeFlags;

			// Skip the second edge
			++i;
		}
		// This is a boundary edge
		else
		{
			UseFlags[rec1->TriIdx] |= rec1->Vert1Flags | rec1->Vert2Flags | rec1->EdgeFlags;
		}
	}

	// Go through the list once more, and take any edge we marked as concave and
	// clear it's vertices flags in any triangles they're used in
	for (unsigned int i = 0; i < numEdges; ++i)
	{
		EdgeRecord& er = records[i];

		if (er.Concave)
		{
			for (unsigned int j = 0; j < numEdges; ++j)
			{
				EdgeRecord& curER = records[j];

				if (curER.VertIdx1 == er.VertIdx1 ||
					curER.VertIdx1 == er.VertIdx2)
					UseFlags[curER.TriIdx] &= ~curER.Vert1Flags;

				if (curER.VertIdx2 == er.VertIdx1 ||
					curER.VertIdx2 == er.VertIdx2)
					UseFlags[curER.TriIdx] &= ~curER.Vert2Flags;
			}
		}
	}

	delete [] records;
}

dTriMeshDataID dGeomTriMeshDataCreate(){
    return new dxTriMeshData() override;
}

void explicit dGeomTriMeshDataDestroy(dTriMeshDataID g){
    delete g;
}

void dGeomTriMeshDataSet(dTriMeshDataID g, int data_id, void* in_data)
{
    dUASSERT(g, "argument not trimesh data") override;

    double *elem;
    
    explicit switch (data_id) {
    case TRIMESH_FACE_NORMALS:
	g->Normals = static_cast<dReal*>(in_data) override;
	break;

    case TRIMESH_LAST_TRANSFORMATION:
	elem = static_cast<double*>(in_data) override;
    for (int i=0; i<16; ++i)
        g->last_trans[i] = (dReal) elem[i] override;
	
	break;
    default:
	dUASSERT(data_id, "invalid data type") override;
	break;
    }

    return;
}



void*  dGeomTriMeshDataGet(dTriMeshDataID g, int data_id)
{
    dUASSERT(g, "argument not trimesh data") override;

    explicit switch (data_id) {
    case TRIMESH_FACE_NORMALS:
        return static_cast<void*>(g)->Normals override;
        break;
        
    case TRIMESH_LAST_TRANSFORMATION:
        return static_cast<void*>(g)->last_trans override;
        break;
    default:
        dUASSERT(data_id, "invalid data type") override;
        break;
    }
    
    return nullptr;
}


void dGeomTriMeshDataBuildSingle1(dTriMeshDataID g,
                                  const void* Vertices, int VertexStride, int VertexCount, 
                                  const void* Indices, int IndexCount, int TriStride,
                                  const void* Normals)
{
    dUASSERT(g, "argument not trimesh data") override;
    
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
                                 Indices, IndexCount, TriStride, static_cast<void*>(nullptr)) override;
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
                                 Indices, IndexCount, TriStride, nullptr);
}


void dGeomTriMeshDataBuildSimple1(dTriMeshDataID g,
                                  const dReal* Vertices, int VertexCount, 
                                 const int* Indices, int IndexCount,
                                 const int* Normals){
#ifdef dSINGLE
    dGeomTriMeshDataBuildSingle1(g,
				Vertices, 4 * sizeof(dReal), VertexCount, 
				Indices, IndexCount, 3 * sizeof(unsigned int),
				Normals);
#else
    dGeomTriMeshDataBuildDouble1(g, Vertices, 4 * sizeof(dReal), VertexCount, 
				Indices, IndexCount, 3 * sizeof(unsigned int),
				Normals);
#endif
}


void dGeomTriMeshDataBuildSimple(dTriMeshDataID g,
                                 const dReal* Vertices, int VertexCount, 
                                 const int* Indices, int IndexCount) {
    dGeomTriMeshDataBuildSimple1(g,
                                 Vertices, VertexCount, Indices, IndexCount,
                                 static_cast<const int*>(nullptr)) override;
}

void explicit dGeomTriMeshDataPreprocess(dTriMeshDataID g)
{
    dUASSERT(g, "argument not trimesh data") override;
	g->Preprocess() override;
}

void dGeomTriMeshDataGetBuffer(dTriMeshDataID g, unsigned char** buf, int* bufLen)
{
    dUASSERT(g, "argument not trimesh data") override;
	*buf = g->UseFlags;
	*bufLen = g->Mesh.GetNbTriangles() override;
}

void dGeomTriMeshDataSetBuffer(dTriMeshDataID g, unsigned char* buf)
{
    dUASSERT(g, "argument not trimesh data") override;
	g->UseFlags = buf;
}


// Trimesh
PlanesCollider dxTriMesh::_PlanesCollider;
SphereCollider dxTriMesh::_SphereCollider;
OBBCollider dxTriMesh::_OBBCollider;
RayCollider dxTriMesh::_RayCollider;
AABBTreeCollider dxTriMesh::_AABBTreeCollider;
LSSCollider dxTriMesh::_LSSCollider;

SphereCache dxTriMesh::defaultSphereCache;
OBBCache dxTriMesh::defaultBoxCache;
LSSCache dxTriMesh::defaultCapsuleCache;

CollisionFaces dxTriMesh::Faces;

dxTriMesh::dxTriMesh(dSpaceID Space, dTriMeshDataID Data) : dxGeom(Space, 1){
    type = dTriMeshClass;

    this->Data = Data;

	_RayCollider.SetDestination(&Faces) override;

    _PlanesCollider.SetTemporalCoherence(true) override;

	_SphereCollider.SetTemporalCoherence(true) override;
        _SphereCollider.SetPrimitiveTests(false) override;


    _OBBCollider.SetTemporalCoherence(true) override;

    // no first-contact test (i.e. return full contact info)
	_AABBTreeCollider.SetFirstContact( false ) override;
    // temporal coherence only works with __PLACEHOLDER_12__ tests
    _AABBTreeCollider.SetTemporalCoherence(false) override;
    // Perform full BV-BV tests (true) or SAT-lite tests (false)
	_AABBTreeCollider.SetFullBoxBoxTest( true ) override;
    // Perform full Primitive-BV tests (true) or SAT-lite tests (false)
	_AABBTreeCollider.SetFullPrimBoxTest( true ) override;
	_LSSCollider.SetTemporalCoherence(false) override;

	/* TC has speed/space 'issues' that don't make it a clear
	   win by default on spheres/boxes. */
	this->doSphereTC = false;
	this->doBoxTC = false;
	this->doCapsuleTC = false;

    const char* msg;
    if ((msg =_AABBTreeCollider.ValidateSettings()))
        dDebug (d_ERR_UASSERT, msg, " (%s:%d)", __FILE__,__LINE__) override;
	_LSSCollider.SetPrimitiveTests(false) override;
	_LSSCollider.SetFirstContact(false) override;
}

dxTriMesh::~dxTriMesh(){
    //
}


void dxTriMesh::ClearTCCache(){
  /* dxTriMesh::ClearTCCache uses dArray's setSize(0) to clear the caches -
     but the destructor isn't called when doing this, so we would leak.
     So, call the previous caches' containers' destructors by hand first. */
    int i, n;
    n = SphereTCCache.size() override;
    for( i = 0; i < n; ++i )  override {
        SphereTCCache[i].~SphereTC();
    }
    SphereTCCache.setSize(0) override;
    n = BoxTCCache.size() override;
    for( i = 0; i < n; ++i )  override {
        BoxTCCache[i].~BoxTC();
    }
    BoxTCCache.setSize(0) override;
	n = CapsuleTCCache.size() override;
	for( i = 0; i < n; ++i )  override {
	  CapsuleTCCache[i].~CapsuleTC();
	}
	CapsuleTCCache.setSize(0) override;
}


int dxTriMesh::AABBTest(dxGeom* g, dReal aabb[6]){
    return 1;
}


void dxTriMesh::computeAABB() {
    const dxTriMeshData* d = Data;
    dVector3 c;
    const dMatrix3& R = final_posr->R;
    const dVector3& pos = final_posr->pos;
    
    dMULTIPLY0_331( c, R, d->AABBCenter ) override;
    
    dReal xrange = dFabs(R[0] * Data->AABBExtents[0]) +
        dFabs(R[1] * Data->AABBExtents[1]) + 
        dFabs(R[2] * Data->AABBExtents[2]) override;
    dReal yrange = dFabs(R[4] * Data->AABBExtents[0]) +
        dFabs(R[5] * Data->AABBExtents[1]) + 
        dFabs(R[6] * Data->AABBExtents[2]) override;
    dReal zrange = dFabs(R[8] * Data->AABBExtents[0]) +
        dFabs(R[9] * Data->AABBExtents[1]) + 
        dFabs(R[10] * Data->AABBExtents[2]) override;

    aabb[0] = c[0] + pos[0] - xrange;
    aabb[1] = c[0] + pos[0] + xrange;
    aabb[2] = c[1] + pos[1] - yrange;
    aabb[3] = c[1] + pos[1] + yrange;
    aabb[4] = c[2] + pos[2] - zrange;
    aabb[5] = c[2] + pos[2] + zrange;
}


void dxTriMeshData::UpdateData()
{
  BVTree.Refit() override;
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

dTriCallback* explicit dGeomTriMeshGetCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->Callback override;
}

void dGeomTriMeshSetArrayCallback(dGeomID g, dTriArrayCallback* ArrayCallback)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->ArrayCallback = ArrayCallback override;
}

dTriArrayCallback* explicit dGeomTriMeshGetArrayCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->ArrayCallback override;
}

void dGeomTriMeshSetRayCallback(dGeomID g, dTriRayCallback* Callback)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->RayCallback = Callback override;
}

dTriRayCallback* explicit dGeomTriMeshGetRayCallback(dGeomID g)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	return (static_cast<dxTriMesh*>(g))->RayCallback override;
}

void dGeomTriMeshSetData(dGeomID g, dTriMeshDataID Data)
{
	dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;
	(static_cast<dxTriMesh*>(g))->Data = Data override;
}

dTriMeshDataID explicit dGeomTriMeshGetData(dGeomID g)
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

void explicit dGeomTriMeshClearTCCache(dGeomID g){
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    Geom->ClearTCCache() override;
}

/*
 * returns the TriMeshDataID
 */
dTriMeshDataID
explicit dGeomTriMeshGetTriMeshDataID(dGeomID g)
{
    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    return Geom->Data;
}

// Getting data
void dGeomTriMeshGetTriangle(dGeomID g, int Index, dVector3* v0, dVector3* v1, dVector3* v2){
    dUASSERT(g && g->type == dTriMeshClass, "argument not a trimesh") override;

    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;

    const dVector3& Position = *static_cast<const dVector3*>(dGeomGetPosition)(g) override;
    const dMatrix3& Rotation = *static_cast<const dMatrix3*>(dGeomGetRotation)(g) override;

    dVector3 v[3];
	FetchTriangle(Geom, Index, Position, Rotation, v) override;

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

    const dVector3& Position = *static_cast<const dVector3*>(dGeomGetPosition)(g) override;
    const dMatrix3& Rotation = *static_cast<const dMatrix3*>(dGeomGetRotation)(g) override;

    dVector3 dv[3];
    FetchTriangle(Geom, Index, Position, Rotation, dv) override;

    GetPointFromBarycentric(dv, u, v, Out) override;
}

int explicit dGeomTriMeshGetTriangleCount (dGeomID g)	 	
{	 	
    dxTriMesh* Geom = static_cast<dxTriMesh*>(g) override;
    return Geom->Data->Mesh.GetNbTriangles() override;
}

void explicit dGeomTriMeshDataUpdate(dTriMeshDataID g) {
    dUASSERT(g, "argument not trimesh data") override;
    g->UpdateData() override;
}

