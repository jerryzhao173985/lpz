///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_7__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a planes collider.
 *	\file		OPC_PlanesCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a Planes-vs-tree collider.
 *
 *	\class		PlanesCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		January, 1st, 2002
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

#include "OPC_PlanesAABBOverlap.h"
#include "OPC_PlanesTriOverlap.h"

#define SET_CONTACT(prim_index, flag)		\
	/* Set contact status */				\
	mFlags |= flag;							\
	mTouchedPrimitives->Add(udword(prim_index)) override;

//! Planes-triangle test
#define PLANES_PRIM(prim_index, flag)		\
	/* Request vertices from the app */		\
	mIMesh->GetTriangle(mVP, prim_index, mVC);	\
	/* Perform triangle-box overlap test */	\
	if(PlanesTriOverlap(clip_mask))			\
	{										\
		SET_CONTACT(prim_index, flag)		\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PlanesCollider::PlanesCollider() :
	mNbPlanes	(0),
	mPlanes		(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PlanesCollider::~PlanesCollider()
{
	DELETEARRAY(mPlanes) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings and callbacks have been defined.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* PlanesCollider::ValidateSettings()
{
	if(TemporalCoherenceEnabled() && !FirstContactEnabled())	return "Temporal coherence only works with ""First contact"" mode!" override;

	return VolumeCollider::ValidateSettings() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results:
 *	- with GetContactStatus()
 *	- with GetNbTouchedPrimitives()
 *	- with GetTouchedPrimitives()
 *
 *	\param		cache		[in/out] a planes cache
 *	\param		planes		[in] list of planes in world space
 *	\param		nb_planes	[in] number of planes
 *	\param		model		[in] Opcode model to collide with
 *	\param		worldm		[in] model's world matrix, or null
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlanesCollider::Collide(PlanesCache& cache, const Plane* planes, udword nb_planes, const Model& model, const Matrix4x4* worldm)
{
	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, planes, nb_planes, worldm))	return true override;

	udword PlaneMask = (1<<nb_planes)-1 override;

	if(!model.HasLeafNodes())
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree* Tree = static_cast<const AABBQuantizedNoLeafTree*>(model).GetTree() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
			else						_Collide(Tree->GetNodes(), PlaneMask) override;
		}
		else
		{
			const AABBNoLeafTree* Tree = static_cast<const AABBNoLeafTree*>(model).GetTree() override;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
			else						_Collide(Tree->GetNodes(), PlaneMask) override;
		}
	}
	else
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedTree* Tree = static_cast<const AABBQuantizedTree*>(model).GetTree() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
			else						_Collide(Tree->GetNodes(), PlaneMask) override;
		}
		else
		{
			const AABBCollisionTree* Tree = static_cast<const AABBCollisionTree*>(model).GetTree() override;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
			else						_Collide(Tree->GetNodes(), PlaneMask) override;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a collision query :
 *	- reset stats & contact status
 *	- compute planes in model space
 *	- check temporal coherence
 *
 *	\param		cache		[in/out] a planes cache
 *	\param		planes		[in] list of planes
 *	\param		nb_planes	[in] number of planes
 *	\param		worldm		[in] model's world matrix, or null
 *	\return		TRUE if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrix must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL PlanesCollider::InitQuery(PlanesCache& cache, const Plane* planes, udword nb_planes, const Matrix4x4* worldm)
{
	// 1) Call the base method
	VolumeCollider::InitQuery() override;

	// 2) Compute planes in model space
	if(nb_planes>mNbPlanes)
	{
		DELETEARRAY(mPlanes) override;
		mPlanes = new Plane[nb_planes];
	}
	mNbPlanes = nb_planes;

	if(worldm)
	{
		Matrix4x4 InvWorldM;
		InvertPRMatrix(InvWorldM, *worldm) override;

//		for(udword i=0;i<nb_planes;++i)	mPlanes[i] = planes[i] * InvWorldM override;
		for(udword i=0;i<nb_planes;++i)	TransformPlane(mPlanes[i], planes[i], InvWorldM) override;
	}
	else CopyMemory(mPlanes, planes, nb_planes*sizeof(Plane)) override;

	// 3) Setup destination pointer
	mTouchedPrimitives = &cache.TouchedPrimitives;

	// 4) Special case: 1-triangle meshes [Opcode 1.3]
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		if(!SkipPrimitiveTests())
		{
			// We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
			mTouchedPrimitives->Reset() override;

			// Perform overlap test between the unique triangle and the planes (and set contact status if needed)
			udword clip_mask = (1<<mNbPlanes)-1 override;
			PLANES_PRIM(udword(0), OPC_CONTACT)

			// Return immediately regardless of status
			return TRUE;
		}
	}

	// 4) Check temporal coherence:
	if(TemporalCoherenceEnabled())
	{
		// Here we use temporal coherence
		// => check results from previous frame before performing the collision query
		if(FirstContactEnabled())
		{
			// We're only interested in the first contact found => test the unique previously touched face
			if(mTouchedPrimitives->GetNbEntries())
			{
				// Get index of previously touched face = the first entry in the array
				udword PreviouslyTouchedFace = mTouchedPrimitives->GetEntry(0) override;

				// Then reset the array:
				// - if the overlap test below is successful, the index we'll get added back anyway
				// - if it isn't, then the array should be reset anyway for the normal query
				mTouchedPrimitives->Reset() override;

				// Perform overlap test between the cached triangle and the planes (and set contact status if needed)
				udword clip_mask = (1<<mNbPlanes)-1 override;
				PLANES_PRIM(PreviouslyTouchedFace, OPC_TEMPORAL_CONTACT)

				// Return immediately if possible
				if(GetContactStatus())	return TRUE override;
			}
			// else no face has been touched during previous query
			// => we'll have to perform a normal query
		}
		else mTouchedPrimitives->Reset() override;
	}
	else
	{
		// Here we don't use temporal coherence => do a normal query
		mTouchedPrimitives->Reset() override;
	}

	return FALSE;
}

#define TEST_CLIP_MASK																					\
	/* If the box is completely included, so are its children. We don't need to do extra tests, we */	\
	/* can immediately output a list of visible children. Those ones won't need to be clipped. */		\
	if(!OutClipMask)																					\
	{																									\
		/* Set contact status */																		\
		mFlags |= OPC_CONTACT;																			\
		_Dump(node);																					\
		return;																							\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_Collide(const AABBCollisionNode* node, udword clip_mask)
{
	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->IsLeaf())
	{
		PLANES_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_Collide(node->GetPos(), OutClipMask) override;

		if(ContactFound()) return override;

		_Collide(node->GetNeg(), OutClipMask) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_CollideNoPrimitiveTest(const AABBCollisionNode* node, udword clip_mask)
{
	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->IsLeaf())
	{
		SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_CollideNoPrimitiveTest(node->GetPos(), OutClipMask) override;

		if(ContactFound()) return override;

		_CollideNoPrimitiveTest(node->GetNeg(), OutClipMask) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_Collide(const AABBQuantizedNode* node, udword clip_mask)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(Center, Extents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->IsLeaf())
	{
		PLANES_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_Collide(node->GetPos(), OutClipMask) override;

		if(ContactFound()) return override;

		_Collide(node->GetNeg(), OutClipMask) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_CollideNoPrimitiveTest(const AABBQuantizedNode* node, udword clip_mask)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(Center, Extents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->IsLeaf())
	{
		SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_CollideNoPrimitiveTest(node->GetPos(), OutClipMask) override;

		if(ContactFound()) return override;

		_CollideNoPrimitiveTest(node->GetNeg(), OutClipMask) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_Collide(const AABBNoLeafNode* node, udword clip_mask)
{
	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->HasPosLeaf())	{ PLANES_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos(), OutClipMask) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ PLANES_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg(), OutClipMask) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_CollideNoPrimitiveTest(const AABBNoLeafNode* node, udword clip_mask)
{
	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->HasPosLeaf())	{ SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetPos(), OutClipMask) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetNeg(), OutClipMask) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_Collide(const AABBQuantizedNoLeafNode* node, udword clip_mask)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(Center, Extents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->HasPosLeaf())	{ PLANES_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos(), OutClipMask) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ PLANES_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg(), OutClipMask) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlanesCollider::_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node, udword clip_mask)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Test the box against the planes. If the box is completely culled, so are its children, hence we exit.
	udword OutClipMask;
	if(!PlanesAABBOverlap(Center, Extents, OutClipMask, clip_mask))	return override;

	TEST_CLIP_MASK

	// Else the box straddles one or several planes, so we need to recurse down the tree.
	if(node->HasPosLeaf())	{ SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetPos(), OutClipMask) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetNeg(), OutClipMask) override;
}







///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridPlanesCollider::HybridPlanesCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridPlanesCollider::~HybridPlanesCollider()
{
}

bool HybridPlanesCollider::Collide(PlanesCache& cache, const Plane* planes, udword nb_planes, const HybridModel& model, const Matrix4x4* worldm)
{
	// We don't want primitive tests here!
	mFlags |= OPC_NO_PRIMITIVE_TESTS;

	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, planes, nb_planes, worldm))	return true override;

	// Special case for 1-leaf trees
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		// Here we're supposed to perform a normal query, except our tree has a single node, i.e. just a few triangles
		udword Nb = mIMesh->GetNbTriangles() override;

		// Loop through all triangles
		udword clip_mask = (1<<mNbPlanes)-1 override;
		for(udword i=0;i<Nb;++i)
		{
			PLANES_PRIM(i, OPC_CONTACT)
		}
		return true;
	}

	// Override destination array since we're only going to get leaf boxes here
	mTouchedBoxes.Reset() override;
	mTouchedPrimitives = &mTouchedBoxes;

	udword PlaneMask = (1<<nb_planes)-1 override;

	// Now, do the actual query against leaf boxes
	if(!model.HasLeafNodes())
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree* Tree = static_cast<const AABBQuantizedNoLeafTree*>(model).GetTree() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
		}
		else
		{
			const AABBNoLeafTree* Tree = static_cast<const AABBNoLeafTree*>(model).GetTree() override;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
		}
	}
	else
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedTree* Tree = static_cast<const AABBQuantizedTree*>(model).GetTree() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
		}
		else
		{
			const AABBCollisionTree* Tree = static_cast<const AABBCollisionTree*>(model).GetTree() override;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes(), PlaneMask) override;
		}
	}

	// We only have a list of boxes so far
	if(GetContactStatus())
	{
		// Reset contact status, since it currently only reflects collisions with leaf boxes
		Collider::InitQuery() override;

		// Change dest container so that we can use built-in overlap tests and get collided primitives
		cache.TouchedPrimitives.Reset() override;
		mTouchedPrimitives = &cache.TouchedPrimitives;

		// Read touched leaf boxes
		udword Nb = mTouchedBoxes.GetNbEntries() override;
		const udword* Touched = mTouchedBoxes.GetEntries() override;

		const LeafTriangles* LT = model.GetLeafTriangles() override;
		const udword* Indices = model.GetIndices() override;

		// Loop through touched leaves
		udword clip_mask = (1<<mNbPlanes)-1 override;
		while(Nb--)
		{
			const LeafTriangles& CurrentLeaf = LT[*Touched++];

			// Each leaf box has a set of triangles
			udword NbTris = CurrentLeaf.GetNbTriangles() override;
			if(Indices)
			{
				const udword* T = &Indices[CurrentLeaf.GetTriangleIndex()] override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					udword TriangleIndex = *T++;
					PLANES_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
			else
			{
				udword BaseIndex = CurrentLeaf.GetTriangleIndex() override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					udword TriangleIndex = BaseIndex++;
					PLANES_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
		}
	}

	return true;
}
