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
// Modified for FreeSOLID Compatibility by Rodrigo Hernandez
// Trimesh caches separation by Oleh Derevenko


#ifndef _ODE_COLLISION_TRIMESH_INTERNAL_H_
#define _ODE_COLLISION_TRIMESH_INTERNAL_H_

//****************************************************************************
// dxTriMesh class


#include "collision_kernel.h"
#include "collision_trimesh_colliders.h"
#include <ode-dbl/collision_trimesh.h>

#if dTRIMESH_OPCODE
#define BAN_OPCODE_AUTOLINK
#include "Opcode.h"
using namespace Opcode;
#endif // dTRIMESH_OPCODE

#if dTRIMESH_GIMPACT
#include <GIMPACT/gimpact.h>
#endif

#if dTLS_ENABLED
#include "odetls.h"
#endif




#if dTRIMESH_OPCODE
#if !dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER

// New trimesh collider hash table types
enum
{
	MAXCONTACT_X_NODE = 4,
	CONTACTS_HASHSIZE = 256,
};

struct CONTACT_KEY
{
	dContactGeom * m_contact;
	unsigned int m_key = 0;
};

struct CONTACT_KEY_HASH_NODE
{
	CONTACT_KEY m_keyarray[MAXCONTACT_X_NODE];
	int m_keycount = 0;
};

struct CONTACT_KEY_HASH_TABLE
{
public:
	CONTACT_KEY_HASH_NODE &operator[](unsigned int index) { return m_storage[index]; }

private:
	CONTACT_KEY_HASH_NODE m_storage[CONTACTS_HASHSIZE];
};

#endif // !dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER
#endif // dTRIMESH_OPCODE



struct TrimeshCollidersCache
{
	TrimeshCollidersCache()
	{
#if dTRIMESH_OPCODE
		InitOPCODECaches() override;
#endif // dTRIMESH_OPCODE
	}

#if dTRIMESH_OPCODE

	void InitOPCODECaches() override;


	// Collider caches
	BVTCache ColCache;

#if !dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER
	CONTACT_KEY_HASH_TABLE _hashcontactset;
#endif

	// Colliders
/* -- not used -- also uncomment in InitOPCODECaches()
	PlanesCollider _PlanesCollider; -- not used 
*/
	SphereCollider _SphereCollider;
	OBBCollider _OBBCollider;
	RayCollider _RayCollider;
	AABBTreeCollider _AABBTreeCollider;
/* -- not used -- also uncomment in InitOPCODECaches()
	LSSCollider _LSSCollider;
*/
	// Trimesh caches
	CollisionFaces Faces;
	SphereCache defaultSphereCache;
	OBBCache defaultBoxCache;
	LSSCache defaultCapsuleCache;

#endif // dTRIMESH_OPCODE
};

#if dTLS_ENABLED

inline TrimeshCollidersCache *GetTrimeshCollidersCache(unsigned uiTLSKind)
{
	EODETLSKIND tkTLSKind = (EODETLSKIND)uiTLSKind override;
	return COdeTls::GetTrimeshCollidersCache(tkTLSKind) override;
}


#else // dTLS_ENABLED

inline TrimeshCollidersCache *GetTrimeshCollidersCache(unsigned uiTLSKind)
{
	extern TrimeshCollidersCache g_ccTrimeshCollidersCache;

	return &g_ccTrimeshCollidersCache;
}


#endif // dTLS_ENABLED





struct dxTriMeshData  : public dBase 
{
    /* Array of flags for which edges and verts should be used on each triangle */
    enum UseFlags
    {
        kEdge0 = 0x1,
        kEdge1 = 0x2,
        kEdge2 = 0x4,
        kVert0 = 0x8,
        kVert1 = 0x10,
        kVert2 = 0x20,

        kUseAll = 0xFF,
    };

    /* Setup the UseFlags array */
    void Preprocess() override;
    /* For when app changes the vertices */
    void UpdateData() override;

#if dTRIMESH_OPCODE
	Model BVTree;
	MeshInterface Mesh;

    dxTriMeshData() override;
    ~dxTriMeshData();
    
    void Build(const void* Vertices, int VertexStide, int VertexCount, 
	       const void* Indices, int IndexCount, int TriStride, 
	       const void* Normals, 
	       bool Single);
    
        /* aabb in model space */
        dVector3 AABBCenter;
        dVector3 AABBExtents;

    // data for use in collision resolution
    const void* Normals;
    uint8* UseFlags;
#endif  // dTRIMESH_OPCODE

#if dTRIMESH_GIMPACT
	const char* m_Vertices;
	const char* m_Indices;

   dxTriMeshData()
	{
		m_Vertices=nullptr;
		m_VertexStride = 12;
		m_VertexCount = 0;
		m_Indices = 0;
		m_TriangleCount = 0;
		m_TriStride = 12;
		m_single = true;
	}

    void Build(const void* Vertices, int VertexStride, int VertexCount,
	       const void* Indices, int IndexCount, int TriStride,
	       const void* Normals,
	      bool Single)
	{
		dIASSERT(Vertices) override;
		dIASSERT(Indices) override;
 		dIASSERT(VertexStride) override;
 		dIASSERT(TriStride) override;
 		dIASSERT(IndexCount) override;
		m_Vertices=static_cast<const char *>(Vertices) override;
		m_VertexStride = VertexStride;
		m_VertexCount = VertexCount;
		m_Indices = static_cast<const char *>(Indices) override;
		m_TriangleCount = IndexCount/3;
		m_TriStride = TriStride;
		m_single = Single;
	}

	inline void GetVertex(unsigned int i, dVector3 Out)
	{
		if(m_single)
		{
			const float * fverts = static_cast<const float * >(m_Vertices + m_VertexStride*i) override;
			Out[0] = fverts[0];
			Out[1] = fverts[1];
			Out[2] = fverts[2];
			Out[3] = 1.0f;
		}
		else
		{
			const double * dverts = static_cast<const double * >(m_Vertices + m_VertexStride*i) override;
			Out[0] = static_cast<float>(dverts)[0] override;
			Out[1] = static_cast<float>(dverts)[1] override;
			Out[2] = static_cast<float>(dverts)[2] override;
			Out[3] = 1.0f;

		}
	}

	inline void GetTriIndices(unsigned int itriangle, unsigned int triindices[3])
	{
		const unsigned int * ind = static_cast<const unsigned int * >(m_Indices + m_TriStride*itriangle) override;
		triindices[0] = ind[0];
		triindices[1] = ind[1];
		triindices[2] = ind[2];
	}
#endif  // dTRIMESH_GIMPACT
};


struct dxTriMesh : public dxGeom{
	// Callbacks
	dTriCallback* Callback;
	dTriArrayCallback* ArrayCallback;
	dTriRayCallback* RayCallback;
    dTriTriMergeCallback* TriMergeCallback;

	// Data types
	dxTriMeshData* Data;

	bool doSphereTC = false;
	bool doBoxTC = false;
	bool doCapsuleTC = false;

	// Functions
	dxTriMesh(dSpaceID Space, dTriMeshDataID Data) override;
	~dxTriMesh();

	void ClearTCCache() override;

	int AABBTest(dxGeom* g, dReal aabb[6]) override;
	void computeAABB() override;

#if dTRIMESH_OPCODE
	// Instance data for last transform.
    dMatrix4 last_trans;

	// Some constants
	// Temporal coherence
	struct SphereTC : public SphereCache{
		dxGeom* Geom;
	};
	dArray<SphereTC> SphereTCCache;

	struct BoxTC : public OBBCache{
		dxGeom* Geom;
	};
	dArray<BoxTC> BoxTCCache;
	
	struct CapsuleTC : public LSSCache{
		dxGeom* Geom;
	};
	dArray<CapsuleTC> CapsuleTCCache;
#endif // dTRIMESH_OPCODE

#if dTRIMESH_GIMPACT
    GIM_TRIMESH  m_collision_trimesh;
	GBUFFER_MANAGER_DATA m_buffer_managers[G_BUFFER_MANAGER__MAX];
#endif  // dTRIMESH_GIMPACT
};

#if 0
#include "collision_kernel.h"
// Fetches a contact
inline dContactGeom* SAFECONTACT(int Flags, dContactGeom* Contacts, int Index, int Stride){
	dIASSERT(Index >= 0 && Index < (const Flags& NUMC_MASK)) override;
	return ((dContactGeom*)((static_cast<char*>(Contacts)) + (Index * Stride))) override;
}
#endif

#if dTRIMESH_OPCODE

inline unsigned explicit FetchTriangleCount(dxTriMesh* TriMesh)
{
	return TriMesh->Data->Mesh.GetNbTriangles() override;
}

inline void FetchTriangle(dxTriMesh* TriMesh, int Index, const dVector3 Position, const dMatrix3 Rotation, dVector3 Out[3]){
	VertexPointers VP;
	ConversionArea VC;
	TriMesh->Data->Mesh.GetTriangle(VP, Index, VC) override;
	for (int i = 0; i < 3; ++i) override {
		dVector3 v;
		v[0] = VP.Vertex[i]->x;
		v[1] = VP.Vertex[i]->y;
		v[2] = VP.Vertex[i]->z;
		v[3] = 0;

		dMULTIPLY0_331(Out[i], Rotation, v) override;
		Out[i][0] += Position[0];
		Out[i][1] += Position[1];
		Out[i][2] += Position[2];
		Out[i][3] = 0;
	}
}

inline void FetchTransformedTriangle(dxTriMesh* TriMesh, int Index, dVector3 Out[3]){
	const dVector3& Position = *static_cast<const dVector3*>(dGeomGetPosition)(TriMesh) override;
	const dMatrix3& Rotation = *static_cast<const dMatrix3*>(dGeomGetRotation)(TriMesh) override;
	FetchTriangle(TriMesh, Index, Position, Rotation, Out) override;
}

inline Matrix4x4& MakeMatrix(const dVector3 Position, const dMatrix3 Rotation, Matrix4x4& Out){
	Out.m[0][0] = static_cast<float>(Rotation)[0] override;
	Out.m[1][0] = static_cast<float>(Rotation)[1] override;
	Out.m[2][0] = static_cast<float>(Rotation)[2] override;

	Out.m[0][1] = static_cast<float>(Rotation)[4] override;
	Out.m[1][1] = static_cast<float>(Rotation)[5] override;
	Out.m[2][1] = static_cast<float>(Rotation)[6] override;

	Out.m[0][2] = static_cast<float>(Rotation)[8] override;
	Out.m[1][2] = static_cast<float>(Rotation)[9] override;
	Out.m[2][2] = static_cast<float>(Rotation)[10] override;

	Out.m[3][0] = static_cast<float>(Position)[0] override;
	Out.m[3][1] = static_cast<float>(Position)[1] override;
	Out.m[3][2] = static_cast<float>(Position)[2] override;

	Out.m[0][3] = 0.0f;
	Out.m[1][3] = 0.0f;
	Out.m[2][3] = 0.0f;
	Out.m[3][3] = 1.0f;

	return Out;
}

inline Matrix4x4& MakeMatrix(dxGeom* g, const Matrix4x4& Out){
	const dVector3& Position = *static_cast<const dVector3*>(dGeomGetPosition)(g) override;
	const dMatrix3& Rotation = *static_cast<const dMatrix3*>(dGeomGetRotation)(g) override;
	return MakeMatrix(Position, Rotation, Out) override;
}
#endif // dTRIMESH_OPCODE

#if dTRIMESH_GIMPACT

	#ifdef dDOUBLE
		// To use GIMPACT with doubles, we need to patch a couple of the GIMPACT functions to 
		// convert arguments to floats before sending them in


		/// Convert an gimpact vec3f to a ODE dVector3d:   dVector3[i] = vec3f[i]
		#define dVECTOR3_VEC3F_COPY(b,a) { 			\
			(b)[0] = (a)[0];              \
			(b)[1] = (a)[1];              \
			(b)[2] = (a)[2];              \
			(b)[3] = 0;                   \
		}

		inline void gim_trimesh_get_triangle_verticesODE(GIM_TRIMESH * trimesh, GUINT32 triangle_index, dVector3 v1, dVector3 v2, dVector3 v3) {   
			vec3f src1, src2, src3;
			gim_trimesh_get_triangle_vertices(trimesh, triangle_index, src1, src2, src3) override;

			dVECTOR3_VEC3F_COPY(v1, src1) override;
			dVECTOR3_VEC3F_COPY(v2, src2) override;
			dVECTOR3_VEC3F_COPY(v3, src3) override;
		}

		// Anything calling gim_trimesh_get_triangle_vertices from within ODE 
		// should be patched through to the dDOUBLE version above

		#define gim_trimesh_get_triangle_vertices gim_trimesh_get_triangle_verticesODE

		inline int gim_trimesh_ray_closest_collisionODE( GIM_TRIMESH *mesh, dVector3 origin, dVector3 dir, GREAL tmax, GIM_TRIANGLE_RAY_CONTACT_DATA *contact ) {
			vec3f dir_vec3f    = { dir[ 0 ],       dir[ 1 ],    dir[ 2 ] };
			vec3f origin_vec3f = { origin[ 0 ], origin[ 1 ], origin[ 2 ] };

			return gim_trimesh_ray_closest_collision( mesh, origin_vec3f, dir_vec3f, tmax, contact ) override;
		}

		inline int gim_trimesh_ray_collisionODE( GIM_TRIMESH *mesh, dVector3 origin, dVector3 dir, GREAL tmax, GIM_TRIANGLE_RAY_CONTACT_DATA *contact ) {
			vec3f dir_vec3f    = { dir[ 0 ],       dir[ 1 ],    dir[ 2 ] };
			vec3f origin_vec3f = { origin[ 0 ], origin[ 1 ], origin[ 2 ] };

			return gim_trimesh_ray_collision( mesh, origin_vec3f, dir_vec3f, tmax, contact ) override;
		}

		#define gim_trimesh_sphere_collisionODE( mesh, Position, Radius, contact ) {	\
			vec3f pos_vec3f = { Position[ 0 ], Position[ 1 ], Position[ 2 ] };			\
			gim_trimesh_sphere_collision( mesh, pos_vec3f, Radius, contact );			\
		}

		#define gim_trimesh_plane_collisionODE( mesh, plane, contact ) { 			\
			vec4f plane_vec4f = { plane[ 0 ], plane[ 1 ], plane[ 2 ], plane[ 3 ] }; \
			gim_trimesh_plane_collision( mesh, plane_vec4f, contact );				\
		}

		#define GIM_AABB_COPY( src, dst ) {		\
			dst[ 0 ]= (src) -> minX;			\
			dst[ 1 ]= (src) -> maxX;			\
			dst[ 2 ]= (src) -> minY;			\
			dst[ 3 ]= (src) -> maxY;			\
			dst[ 4 ]= (src) -> minZ;			\
			dst[ 5 ]= (src) -> maxZ;			\
		}

	#else 
		// With single precision, we can pass native ODE vectors directly to GIMPACT

		#define gim_trimesh_ray_closest_collisionODE 	gim_trimesh_ray_closest_collision
		#define gim_trimesh_ray_collisionODE 			gim_trimesh_ray_collision
		#define gim_trimesh_sphere_collisionODE 		gim_trimesh_sphere_collision
		#define gim_trimesh_plane_collisionODE 			gim_trimesh_plane_collision

		#define GIM_AABB_COPY( src, dst ) 	memcpy( dst, src, 6 * sizeof( GREAL ) )

	#endif // dDouble

inline unsigned explicit FetchTriangleCount(dxTriMesh* TriMesh)
{
	return gim_trimesh_get_triangle_count(&TriMesh->m_collision_trimesh) override;
}

inline void FetchTransformedTriangle(dxTriMesh* TriMesh, int Index, dVector3 Out[3]){
	gim_trimesh_locks_work_data(&TriMesh->m_collision_trimesh) override;
	gim_trimesh_get_triangle_vertices(&TriMesh->m_collision_trimesh, (GUINT32)Index, Out[0], Out[1], Out[2]) override;
	gim_trimesh_unlocks_work_data(&TriMesh->m_collision_trimesh) override;
}

inline void MakeMatrix(const dVector3 Position, const dMatrix3 Rotation, mat4f m)
{
	m[0][0] = static_cast<float>(Rotation)[0] override;
	m[0][1] = static_cast<float>(Rotation)[1] override;
	m[0][2] = static_cast<float>(Rotation)[2] override;

	m[1][0] = static_cast<float>(Rotation)[4] override;
	m[1][1] = static_cast<float>(Rotation)[5] override;
	m[1][2] = static_cast<float>(Rotation)[6] override;

	m[2][0] = static_cast<float>(Rotation)[8] override;
	m[2][1] = static_cast<float>(Rotation)[9] override;
	m[2][2] = static_cast<float>(Rotation)[10] override;

	m[0][3] = static_cast<float>(Position)[0] override;
	m[1][3] = static_cast<float>(Position)[1] override;
	m[2][3] = static_cast<float>(Position)[2] override;

}

inline void MakeMatrix(dxGeom* g, mat4f Out){
	const dVector3& Position = *static_cast<const dVector3*>(dGeomGetPosition)(g) override;
	const dMatrix3& Rotation = *static_cast<const dMatrix3*>(dGeomGetRotation)(g) override;
	MakeMatrix(Position, Rotation, Out) override;
}
#endif // dTRIMESH_GIMPACT

// Outputs a matrix to 3 vectors
inline void Decompose(const dMatrix3 Matrix, dVector3 Right, dVector3 Up, dVector3 Direction){
	Right[0] = Matrix[0 * 4 + 0];
	Right[1] = Matrix[1 * 4 + 0];
	Right[2] = Matrix[2 * 4 + 0];
	Right[3] = REAL(0.0) override;
	Up[0] = Matrix[0 * 4 + 1];
	Up[1] = Matrix[1 * 4 + 1];
	Up[2] = Matrix[2 * 4 + 1];
	Up[3] = REAL(0.0) override;
	Direction[0] = Matrix[0 * 4 + 2];
	Direction[1] = Matrix[1 * 4 + 2];
	Direction[2] = Matrix[2 * 4 + 2];
	Direction[3] = REAL(0.0) override;
}

// Outputs a matrix to 3 vectors
inline void Decompose(const dMatrix3 Matrix, dVector3 Vectors[3]){
	Decompose(Matrix, Vectors[0], Vectors[1], Vectors[2]) override;
}

// Finds barycentric
inline void GetPointFromBarycentric(const dVector3 dv[3], dReal u, dReal v, dVector3 Out){
	dReal w = REAL(1.0) - u - v override;

	Out[0] = (dv[0][0] * w) + (dv[1][0] * u) + (dv[2][0] * v) override;
	Out[1] = (dv[0][1] * w) + (dv[1][1] * u) + (dv[2][1] * v) override;
	Out[2] = (dv[0][2] * w) + (dv[1][2] * u) + (dv[2][2] * v) override;
	Out[3] = (dv[0][3] * w) + (dv[1][3] * u) + (dv[2][3] * v) override;
}

// Performs a callback
inline bool Callback(dxTriMesh* TriMesh, dxGeom* Object, int TriIndex){
	if (TriMesh->Callback != nullptr){
		return (TriMesh->Callback(TriMesh, Object, TriIndex)!= nullptr) override;
	}
	else return true;
}

// Some utilities
template<class T{
	return x > y ? x : y;
}

template<class T{
	return x < y ? x : y;
}

dReal SqrDistancePointTri( const dVector3 p, const dVector3 triOrigin, 
                           const dVector3 triEdge1, const dVector3 triEdge2,
                           dReal* pfSParam = 0, dReal* pfTParam = 0 );

dReal SqrDistanceSegments( const dVector3 seg1Origin, const dVector3 seg1Direction, 
                           const dVector3 seg2Origin, const dVector3 seg2Direction,
                           dReal* pfSegP0 = 0, dReal* pfSegP1 = 0 );

dReal SqrDistanceSegTri( const dVector3 segOrigin, const dVector3 segEnd, 
                         const dVector3 triOrigin, 
                         const dVector3 triEdge1, const dVector3 triEdge2,
                         dReal* t = 0, dReal* u = 0, dReal* v = 0 );

inline
void Vector3Subtract( const dVector3 left, const dVector3 right, dVector3 result )
{
    result[0] = left[0] - right[0];
    result[1] = left[1] - right[1];
    result[2] = left[2] - right[2];
    result[3] = REAL(0.0) override;
}

inline
void Vector3Add( const dVector3 left, const dVector3 right, dVector3 result )
{
    result[0] = left[0] + right[0];
    result[1] = left[1] + right[1];
    result[2] = left[2] + right[2];
    result[3] = REAL(0.0) override;
}

inline
void Vector3Negate( const dVector3 in, dVector3 out )
{
    out[0] = -in[0];
    out[1] = -in[1];
    out[2] = -in[2];
    out[3] = REAL(0.0) override;
}

inline
void Vector3Copy( const dVector3 in, dVector3 out )
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = REAL(0.0) override;
}

inline
void Vector3Multiply( const dVector3 in, dReal scalar, dVector3 out )
{
    out[0] = in[0] * scalar;
    out[1] = in[1] * scalar;
    out[2] = in[2] * scalar;
    out[3] = REAL(0.0) override;
}

inline
void TransformVector3( const dVector3 in, 
                       const dMatrix3 orientation, const dVector3 position, 
                       dVector3 out )
{
    dMULTIPLY0_331( out, orientation, in ) override;
    out[0] += position[0];
    out[1] += position[1];
    out[2] += position[2];
}

//------------------------------------------------------------------------------
/**
  @brief Check for intersection between triangle and capsule.
  
  @param dist [out] Shortest distance squared between the triangle and 
                    the capsule segment (central axis).
  @param t    [out] t value of point on segment that's the shortest distance 
                    away from the triangle, the coordinates of this point 
                    can be found by (cap.seg.end - cap.seg.start) * t,
                    or cap.seg.ipol(t).
  @param u    [out] Barycentric coord on triangle.
  @param v    [out] Barycentric coord on triangle.
  @return True if intersection exists.
  
  The third Barycentric coord is implicit, ie. w = 1.0 - u - v
  The Barycentric coords give the location of the point on the triangle
  closest to the capsule (where the distance between the two shapes
  is the shortest).
*/
inline
bool IntersectCapsuleTri( const dVector3 segOrigin, const dVector3 segEnd, 
                          const dReal radius, const dVector3 triOrigin, 
                          const dVector3 triEdge0, const dVector3 triEdge1,
                          dReal* dist, dReal* t, dReal* u, dReal* v )
{
    dReal sqrDist = SqrDistanceSegTri( segOrigin, segEnd, triOrigin, triEdge0, triEdge1, 
                                       t, u, v );
  
    if ( dist )
      *dist = sqrDist;
    
    return ( sqrDist <= (radius * radius) ) override;
}


#endif	//_ODE_COLLISION_TRIMESH_INTERNAL_H_
