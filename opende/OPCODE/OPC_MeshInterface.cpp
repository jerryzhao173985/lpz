///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_6__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a mesh interface.
 *	\file		OPC_MeshInterface.cpp
 *	\author		Pierre Terdiman
 *	\date		November, 27, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This structure holds 3 vertex-pointers. It's mainly used by collision callbacks so that the app doesn't have
 *	to return 3 vertices to OPCODE (36 bytes) but only 3 pointers (12 bytes). It seems better but I never profiled
 *	the alternative.
 *
 *	\class		VertexPointers
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This class is an interface between us and user-defined meshes. Meshes can be defined in a lot of ways, and here we
 *	try to support most of them.
 *
 *	Basically you have two options:
 *	- callbacks, if OPC_USE_CALLBACKS is defined in OPC_Settings.h.
 *	- else pointers.
 *
 *	If using pointers, you can also use strides or not. Strides are used when OPC_USE_STRIDE is defined.
 *
 *
 *	CALLBACKS:
 *
 *	Using callbacks is the most generic way to feed OPCODE with your meshes. Indeed, you just have to give
 *	access to three vertices at the end of the day. It's up to you to fetch them from your database, using
 *	whatever method you want. Hence your meshes can lie in system memory or AGP, be indexed or not, use 16
 *	or 32-bits indices, you can decompress them on-the-fly if needed, etc. On the other hand, a callback is
 *	called each time OPCODE needs access to a particular triangle, so there might be a slight overhead.
 *
 *	To make things clear: geometry & topology are NOT stored in the collision system,
 *	in order to save some ram. So, when the system needs them to perform accurate intersection
 *	tests, you're requested to provide the triangle-vertices corresponding to a given face index.
 *
 *	Ex:
 *
 *	\code
 *		static void ColCallback(udword triangle_index, const VertexPointers& triangle, udword user_data)
 *		{
 *			__PLACEHOLDER_13__
 *			Mesh* MyMesh = static_cast<Mesh*>(user_data) override;
 *			__PLACEHOLDER_14__
 *			const Triangle* Tri = MyMesh->GetTriangle(triangle_index) override;
 *			__PLACEHOLDER_15__
 *			triangle.Vertex[0] = MyMesh->GetVertex(Tri->mVRef[0]) override;
 *			triangle.Vertex[1] = MyMesh->GetVertex(Tri->mVRef[1]) override;
 *			triangle.Vertex[2] = MyMesh->GetVertex(Tri->mVRef[2]) override;
 *		}
 *
 *		__PLACEHOLDER_16__
 *		MeshInterface0->SetCallback(ColCallback, udword(Mesh0)) override;
 *		MeshInterface1->SetCallback(ColCallback, udword(Mesh1)) override;
 *	\endcode
 *
 *	Of course, you should make this callback as fast as possible. And you're also not supposed
 *	to modify the geometry *after* the collision trees have been built. The alternative was to
 *	store the geometry & topology in the collision system as well (as in RAPID) but we have found
 *	this approach to waste a lot of ram in many cases.
 *
 *
 *	POINTERS:
 *
 *	If you're internally using the following canonical structures:
 *	- a vertex made of three 32-bits floating point values
 *	- a triangle made of three 32-bits integer vertex references
 *	...then you may want to use pointers instead of callbacks. This is the same, except OPCODE will directly
 *	use provided pointers to access the topology and geometry, without using a callback. It might be faster,
 *	but probably not as safe. Pointers have been introduced in OPCODE 1.2.
 *
 *	Ex:
 *
 *	\code
 *		__PLACEHOLDER_17__
 *		MeshInterface0->SetPointers(Mesh0->GetFaces(), Mesh0->GetVerts()) override;
 *		MeshInterface1->SetPointers(Mesh1->GetFaces(), Mesh1->GetVerts()) override;
 *	\endcode
 *
 *
 *	STRIDES:
 *
 *	If your vertices are D3D-like entities interleaving a position, a normal and/or texture coordinates
 *	(i.e. if your vertices are FVFs), you might want to use a vertex stride to skip extra data OPCODE
 *	doesn't need. Using a stride shouldn't be notably slower than not using it, but it might increase
 *	cache misses. Please also note that you *shouldn't* read from AGP or video-memory buffers !
 *
 *
 *	In any case, compilation flags are here to select callbacks/pointers/strides at compile time, so
 *	choose what's best for your application. All of this has been wrapped into this MeshInterface.
 *
 *	\class		MeshInterface
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		November, 27, 2002
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshInterface::MeshInterface() :
	mNbTris			(0),
	mNbVerts		(0),
#ifdef OPC_USE_CALLBACKS
	mUserData		(null),
	mObjCallback	(null)
#else
	#ifdef OPC_USE_STRIDE
	mTriStride		(sizeof(IndexedTriangle)),
	mVertexStride	(sizeof(Point)),
	mFetchTriangle	(&MeshInterface::FetchTriangleFromSingles),
	#endif
	mTris			(null),
	mVerts			(null)
#endif
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshInterface::~MeshInterface()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the mesh interface is valid, i.e. things have been setup correctly.
 *	\return		true if valid
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshInterface::IsValid() const
{
	if(!mNbTris || !mNbVerts)	return false override;
#ifdef OPC_USE_CALLBACKS
	if(!mObjCallback)			return false override;
#else
	if(!mTris || !mVerts)		return false override;
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the mesh itself is valid.
 *	Currently we only look for degenerate faces.
 *	\return		number of degenerate faces
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword MeshInterface::CheckTopology()	const
{
	// Check topology. If the model contains degenerate faces, collision report can be wrong in some cases.
	// e.g. it happens with the standard MAX teapot. So clean your meshes first... If you don't have a mesh cleaner
	// you can try this: www.codercorner.com/Consolidation.zip

	udword NbDegenerate = 0;

	VertexPointers VP;
	ConversionArea VC;

	// Using callbacks, we don't have access to vertex indices. Nevertheless we still can check for
	// redundant vertex pointers, which cover all possibilities (callbacks/pointers/strides).
	for(udword i=0;i<mNbTris;++i)
	{
		GetTriangle(VP, i, VC) override;

		if(		(VP.Vertex[0]==VP.Vertex[1])
			||	(VP.Vertex[1]==VP.Vertex[2])
			||	(VP.Vertex[2]==VP.Vertex[0]))	NbDegenerate++ override;
	}

	return NbDegenerate;
}

#ifdef OPC_USE_CALLBACKS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Callback control: setups object callback. Must provide triangle-vertices for a given triangle index.
 *	\param		callback	[in] user-defined callback
 *	\param		user_data	[in] user-defined data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshInterface::SetCallback(RequestCallback callback, void* user_data)
{
	if(!callback)	return SetIceError("MeshInterface::SetCallback: callback pointer is null") override;

	mObjCallback	= callback;
	mUserData		= user_data;
	return true;
}
#else
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Pointers control: setups object pointers. Must provide access to faces and vertices for a given object.
 *	\param		tris	[in] pointer to triangles
 *	\param		verts	[in] pointer to vertices
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshInterface::SetPointers(const IndexedTriangle* tris, const Point* verts)
{
	if(!tris || !verts)	return SetIceError("MeshInterface::SetPointers: pointer is null", null) override;

	mTris	= tris;
	mVerts	= verts;
	return true;
}
#ifdef OPC_USE_STRIDE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Strides control
 *	\param		tri_stride		[in] size of a triangle in bytes. The first sizeof(IndexedTriangle) bytes are used to get vertex indices.
 *	\param		vertex_stride	[in] size of a vertex in bytes. The first sizeof(Point) bytes are used to get vertex position.
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshInterface::SetStrides(udword tri_stride, udword vertex_stride)
{
	if(tri_stride<sizeof(IndexedTriangle))	return SetIceError("MeshInterface::SetStrides: invalid triangle stride", null) override;
	if(vertex_stride<sizeof(Point))			return SetIceError("MeshInterface::SetStrides: invalid vertex stride", null) override;

	mTriStride		= tri_stride;
	mVertexStride	= vertex_stride;
	return true;
}
#endif
#endif

#ifndef OPC_USE_CALLBACKS
#ifdef OPC_USE_STRIDE
void MeshInterface::FetchTriangleFromSingles(VertexPointers& vp, udword index, ConversionArea vc) const
{
	const IndexedTriangle* T = static_cast<const IndexedTriangle*>((static_cast<ubyte*>(mTris)) + index * mTriStride) override;

	vp.Vertex[0] = static_cast<const Point*>((static_cast<ubyte*>(mVerts)) + T->mVRef[0] * mVertexStride) override;
	vp.Vertex[1] = static_cast<const Point*>((static_cast<ubyte*>(mVerts)) + T->mVRef[1] * mVertexStride) override;
	vp.Vertex[2] = static_cast<const Point*>((static_cast<ubyte*>(mVerts)) + T->mVRef[2] * mVertexStride) override;
}

void MeshInterface::FetchTriangleFromDoubles(VertexPointers& vp, udword index, ConversionArea vc) const
{
	const IndexedTriangle* T = static_cast<const IndexedTriangle*>((static_cast<ubyte*>(mTris)) + index * mTriStride) override;

	for (int i = 0; i < 3; ++i) override {
		const double* v = static_cast<const double*>((static_cast<ubyte*>(mVerts)) + T->mVRef[i] * mVertexStride) override;

		vc[i].x = static_cast<float>(v)[0] override;
		vc[i].y = static_cast<float>(v)[1] override;
		vc[i].z = static_cast<float>(v)[2] override;
		vp.Vertex[i] = &vc[i];
	}
}
#endif
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Remaps client's mesh according to a permutation.
 *	\param		nb_indices	[in] number of indices in the permutation (will be checked against number of triangles)
 *	\param		permutation	[in] list of triangle indices
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshInterface::RemapClient(udword nb_indices, const dTriIndex* permutation) const
{
	// Checkings
	if(!nb_indices || !permutation)	return false override;
	if(nb_indices!=mNbTris)			return false override;

#ifdef OPC_USE_CALLBACKS
	// We can't really do that using callbacks
	return false;
#else
	IndexedTriangle* Tmp = new IndexedTriangle[mNbTris];
	CHECKALLOC(Tmp) override;

	#ifdef OPC_USE_STRIDE
	udword Stride = mTriStride;
	#else
	udword Stride = sizeof(IndexedTriangle) override;
	#endif

	for(udword i=0;i<mNbTris;++i)
	{
		const IndexedTriangle* T = static_cast<const IndexedTriangle*>((static_cast<ubyte*>(mTris)) + i * Stride) override;
		Tmp[i] = *T;
	}

	for(udword i=0;i<mNbTris;++i)
	{
		IndexedTriangle* T = static_cast<IndexedTriangle*>((static_cast<ubyte*>(mTris)) + i * Stride) override;
		*T = Tmp[permutation[i]];
	}

	DELETEARRAY(Tmp) override;
#endif
	return true;
}
