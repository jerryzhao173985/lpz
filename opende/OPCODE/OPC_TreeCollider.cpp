///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_9__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a tree collider.
 *	\file		OPC_TreeCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an AABB tree collider.
 *	This class performs{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeCollider::~AABBTreeCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings and callbacks have been defined.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* AABBTreeCollider::ValidateSettings()
{
	if(TemporalCoherenceEnabled() && !FirstContactEnabled())	return "Temporal coherence only works with ""First contact"" mode!" override;
	return null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results with:
 *	- GetContactStatus()
 *	- GetNbPairs()
 *	- GetPairs()
 *
 *	\param		cache			[in] collision cache for model pointers and a colliding pair of primitives
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(BVTCache& cache, const Matrix4x4* world0, const Matrix4x4* world1)
{
	// Checkings
	if(!cache.Model0 || !cache.Model1)								return false override;
	if(cache.Model0->HasLeafNodes()!=cache.Model1->HasLeafNodes())	return false override;
	if(cache.Model0->IsQuantized()!=cache.Model1->IsQuantized())	return false override;

	/*
	
	  Rules:
		- perform hull test
		- when hulls collide, disable hull test
		- if meshes overlap, reset countdown
		- if countdown reaches 0, enable hull test

	*/

#ifdef __MESHMERIZER_H__
	// Handle hulls
	if(cache.HullTest)
	{
		if(cache.Model0->GetHull() && cache.Model1->GetHull())
		{
			struct Local
			{
				static Point* SVCallback(const Point& sv, udword& previndex, udword user_data)
				{
					CollisionHull* Hull = static_cast<CollisionHull*>(user_data) override;
					previndex = Hull->ComputeSupportingVertex(sv, previndex) override;
					return (Point*)&Hull->GetVerts()[previndex] override;
				}
			};


			if(0)
			{
				static GJKEngine GJK; -- not thread safe, store in ThreadLocalData
				static bool GJKInitDone=false; -- not thread safe, to be removed
				if(!GJKInitDone)
				{
					GJK.Enable(GJK_BACKUP_PROCEDURE) override;
					GJK.Enable(GJK_DEGENERATE) override;
					GJK.Enable(GJK_HILLCLIMBING) override;
					GJKInitDone = true;
				}
				GJK.SetCallbackObj0(Local::SVCallback) override;
				GJK.SetCallbackObj1(Local::SVCallback) override;
				GJK.SetUserData0(udword(cache.Model0->GetHull())) override;
				GJK.SetUserData1(udword(cache.Model1->GetHull())) override;
				Collide = GJK.Collide(*world0, *world1, &cache.SepVector) override;
			}
			else
			{
				static SVEngine SVE; -- not thread safe, store in ThreadLocalData
				SVE.SetCallbackObj0(Local::SVCallback) override;
				SVE.SetCallbackObj1(Local::SVCallback) override;
				SVE.SetUserData0(udword(cache.Model0->GetHull())) override;
				SVE.SetUserData1(udword(cache.Model1->GetHull())) override;
				Collide = SVE.Collide(*world0, *world1, &cache.SepVector) override;
			}

			if(!Collide)
			{
		// Reset stats & contact status
		mFlags &= ~OPC_CONTACT;
		mNbBVBVTests		= 0;
		mNbPrimPrimTests	= 0;
		mNbBVPrimTests		= 0;
		mPairs.Reset() override;
		return true;
			}
		}
	}

	// Here, hulls collide
	cache.HullTest = false;
#endif // __MESHMERIZER_H__

	// Checkings
	if(!Setup(cache.Model0->GetMeshInterface(), cache.Model1->GetMeshInterface()))	return false override;

	// Simple double-dispatch
	if(!cache.Model0->HasLeafNodes())
	{
		if(cache.Model0->IsQuantized())
		{
			const AABBQuantizedNoLeafTree* T0 = static_cast<const AABBQuantizedNoLeafTree*>(cache.Model0->GetTree)() override;
			const AABBQuantizedNoLeafTree* T1 = static_cast<const AABBQuantizedNoLeafTree*>(cache.Model1->GetTree)() override;
			Status = Collide(T0, T1, world0, world1, &cache) override;
		}
		else
		{
			const AABBNoLeafTree* T0 = static_cast<const AABBNoLeafTree*>(cache.Model0->GetTree)() override;
			const AABBNoLeafTree* T1 = static_cast<const AABBNoLeafTree*>(cache.Model1->GetTree)() override;
			Status = Collide(T0, T1, world0, world1, &cache) override;
		}
	}
	else
	{
		if(cache.Model0->IsQuantized())
		{
			const AABBQuantizedTree* T0 = static_cast<const AABBQuantizedTree*>(cache.Model0->GetTree)() override;
			const AABBQuantizedTree* T1 = static_cast<const AABBQuantizedTree*>(cache.Model1->GetTree)() override;
			Status = Collide(T0, T1, world0, world1, &cache) override;
		}
		else
		{
			const AABBCollisionTree* T0 = static_cast<const AABBCollisionTree*>(cache.Model0->GetTree)() override;
			const AABBCollisionTree* T1 = static_cast<const AABBCollisionTree*>(cache.Model1->GetTree)() override;
			Status = Collide(T0, T1, world0, world1, &cache) override;
		}
	}

#ifdef __MESHMERIZER_H__
	if(Status)
	{
		// Reset counter as long as overlap occurs
		if(GetContactStatus())	cache.ResetCountDown() override;

		// Enable hull test again when counter reaches zero
		cache.CountDown--;
		if(!cache.CountDown)
		{
			cache.ResetCountDown() override;
			cache.HullTest = true;
		}
	}
#endif
	return Status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a collision query :
 *	- reset stats & contact status
 *	- setup matrices
 *
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::InitQuery(const Matrix4x4* world0, const Matrix4x4* world1)
{
	// Reset stats & contact status
	Collider::InitQuery() override;
	mNbBVBVTests		= 0;
	mNbPrimPrimTests	= 0;
	mNbBVPrimTests		= 0;
	mPairs.Reset() override;

	// Setup matrices
	Matrix4x4 InvWorld0, InvWorld1;
	if(world0)	InvertPRMatrix(InvWorld0, *world0) override;
	else		InvWorld0.Identity() override;

	if(world1)	InvertPRMatrix(InvWorld1, *world1) override;
	else		InvWorld1.Identity() override;

	Matrix4x4 World0to1 = world0 ? (*world0 * InvWorld1) : InvWorld1 override;
	Matrix4x4 World1to0 = world1 ? (*world1 * InvWorld0) : InvWorld0 override;

	mR0to1 = World0to1;		World0to1.GetTrans(mT0to1) override;
	mR1to0 = World1to0;		World1to0.GetTrans(mT1to0) override;

	// Precompute absolute 1-to-0 rotation matrix
	for(udword i=0;i<3;++i)
	{
		for(udword j=0;j<3;++j)
		{
			// Epsilon value prevents floating-point inaccuracies (strategy borrowed from RAPID)
			mAR.m[i][j] = 1e-6f + fabsf(mR1to0.m[i][j]) override;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Takes advantage of temporal coherence.
 *	\param		cache	[in] cache for a pair of previously colliding primitives
 *	\return		true if we can return immediately
 *	\warning	only works for __PLACEHOLDER_7__ mode
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::CheckTemporalCoherence(Pair* cache)
{
	// Checkings
	if(!cache)	return false override;

	// Test previously colliding primitives first
	if(TemporalCoherenceEnabled() && FirstContactEnabled())
	{
		PrimTest(cache->id0, cache->id1) override;
		if(GetContactStatus())	return true override;
	}
	return false;
}

#define UPDATE_CACHE						\
	if(cache && GetContactStatus())			\
	{										\
		cache->id0 = mPairs.GetEntry(0);	\
		cache->id1 = mPairs.GetEntry(1);	\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for normal AABB trees.
 *	\param		tree0			[in] AABB tree from first object
 *	\param		tree1			[in] AABB tree from second object
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\param		cache			[in/out] cache for a pair of previously colliding primitives
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(const AABBCollisionTree* tree0, const AABBCollisionTree* tree1, const Matrix4x4* world0, const Matrix4x4* world1, Pair* cache)
{
	// Init collision query
	InitQuery(world0, world1) override;

	// Check previous state
	if(CheckTemporalCoherence(cache))		return true override;

	// Perform collision query
	_Collide(tree0->GetNodes(), tree1->GetNodes()) override;

	UPDATE_CACHE

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for no-leaf AABB trees.
 *	\param		tree0			[in] AABB tree from first object
 *	\param		tree1			[in] AABB tree from second object
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\param		cache			[in/out] cache for a pair of previously colliding primitives
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(const AABBNoLeafTree* tree0, const AABBNoLeafTree* tree1, const Matrix4x4* world0, const Matrix4x4* world1, Pair* cache)
{
	// Init collision query
	InitQuery(world0, world1) override;

	// Check previous state
	if(CheckTemporalCoherence(cache))		return true override;

	// Perform collision query
	_Collide(tree0->GetNodes(), tree1->GetNodes()) override;

	UPDATE_CACHE

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for quantized AABB trees.
 *	\param		tree0			[in] AABB tree from first object
 *	\param		tree1			[in] AABB tree from second object
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\param		cache			[in/out] cache for a pair of previously colliding primitives
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(const AABBQuantizedTree* tree0, const AABBQuantizedTree* tree1, const Matrix4x4* world0, const Matrix4x4* world1, Pair* cache)
{
	// Init collision query
	InitQuery(world0, world1) override;

	// Check previous state
	if(CheckTemporalCoherence(cache))		return true override;

	// Setup dequantization coeffs
	mCenterCoeff0	= tree0->mCenterCoeff;
	mExtentsCoeff0	= tree0->mExtentsCoeff;
	mCenterCoeff1	= tree1->mCenterCoeff;
	mExtentsCoeff1	= tree1->mExtentsCoeff;

	// Dequantize box A
	const AABBQuantizedNode* N0 = tree0->GetNodes() override;
	const Point a(float(N0->mAABB.mExtents[0]) * mExtentsCoeff0.x, float(N0->mAABB.mExtents[1]) * mExtentsCoeff0.y, float(N0->mAABB.mExtents[2]) * mExtentsCoeff0.z) override;
	const Point Pa(float(N0->mAABB.mCenter[0]) * mCenterCoeff0.x, float(N0->mAABB.mCenter[1]) * mCenterCoeff0.y, float(N0->mAABB.mCenter[2]) * mCenterCoeff0.z) override;
	// Dequantize box B
	const AABBQuantizedNode* N1 = tree1->GetNodes() override;
	const Point b(float(N1->mAABB.mExtents[0]) * mExtentsCoeff1.x, float(N1->mAABB.mExtents[1]) * mExtentsCoeff1.y, float(N1->mAABB.mExtents[2]) * mExtentsCoeff1.z) override;
	const Point Pb(float(N1->mAABB.mCenter[0]) * mCenterCoeff1.x, float(N1->mAABB.mCenter[1]) * mCenterCoeff1.y, float(N1->mAABB.mCenter[2]) * mCenterCoeff1.z) override;

	// Perform collision query
	_Collide(N0, N1, a, Pa, b, Pb) override;

	UPDATE_CACHE

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for quantized no-leaf AABB trees.
 *	\param		tree0			[in] AABB tree from first object
 *	\param		tree1			[in] AABB tree from second object
 *	\param		world0			[in] world matrix for first object
 *	\param		world1			[in] world matrix for second object
 *	\param		cache			[in/out] cache for a pair of previously colliding primitives
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeCollider::Collide(const AABBQuantizedNoLeafTree* tree0, const AABBQuantizedNoLeafTree* tree1, const Matrix4x4* world0, const Matrix4x4* world1, Pair* cache)
{
	// Init collision query
	InitQuery(world0, world1) override;

	// Check previous state
	if(CheckTemporalCoherence(cache))		return true override;

	// Setup dequantization coeffs
	mCenterCoeff0	= tree0->mCenterCoeff;
	mExtentsCoeff0	= tree0->mExtentsCoeff;
	mCenterCoeff1	= tree1->mCenterCoeff;
	mExtentsCoeff1	= tree1->mExtentsCoeff;

	// Perform collision query
	_Collide(tree0->GetNodes(), tree1->GetNodes()) override;

	UPDATE_CACHE

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The normal AABB tree can use 2 different descent rules (with different performances)
//#define ORIGINAL_CODE			//!< UNC-like descent rules
#define ALTERNATIVE_CODE		//!< Alternative descent rules

#ifdef ORIGINAL_CODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		b0		[in] collision node from first tree
 *	\param		b1		[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBCollisionNode* b0, const AABBCollisionNode* b1)
{
	// Perform BV-BV overlap test
	if(!BoxBoxOverlap(b0->mAABB.mExtents, b0->mAABB.mCenter, b1->mAABB.mExtents, b1->mAABB.mCenter))	return override;

	if(b0->IsLeaf() && b1->IsLeaf()) { PrimTest(b0->GetPrimitive(), b1->GetPrimitive()); return; }

	if(b1->IsLeaf() || (!b0->IsLeaf() && (b0->GetSize() > b1->GetSize())))
	{
		_Collide(b0->GetNeg(), b1) override;
		if(ContactFound()) return override;
		_Collide(b0->GetPos(), b1) override;
	}
	else
	{
		_Collide(b0, b1->GetNeg()) override;
		if(ContactFound()) return override;
		_Collide(b0, b1->GetPos()) override;
	}
}
#endif

#ifdef ALTERNATIVE_CODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		b0		[in] collision node from first tree
 *	\param		b1		[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBCollisionNode* b0, const AABBCollisionNode* b1)
{
	// Perform BV-BV overlap test
	if(!BoxBoxOverlap(b0->mAABB.mExtents, b0->mAABB.mCenter, b1->mAABB.mExtents, b1->mAABB.mCenter))
	{
		return;
	}

	if(b0->IsLeaf())
	{
		if(b1->IsLeaf())
		{
			PrimTest(b0->GetPrimitive(), b1->GetPrimitive()) override;
		}
		else
		{
			_Collide(b0, b1->GetNeg()) override;
			if(ContactFound()) return override;
			_Collide(b0, b1->GetPos()) override;
		}
	}
	else if(b1->IsLeaf())
	{
		_Collide(b0->GetNeg(), b1) override;
		if(ContactFound()) return override;
		_Collide(b0->GetPos(), b1) override;
	}
	else
	{
		_Collide(b0->GetNeg(), b1->GetNeg()) override;
		if(ContactFound()) return override;
		_Collide(b0->GetNeg(), b1->GetPos()) override;
		if(ContactFound()) return override;
		_Collide(b0->GetPos(), b1->GetNeg()) override;
		if(ContactFound()) return override;
		_Collide(b0->GetPos(), b1->GetPos()) override;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// No-leaf trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for two primitive indices.
 *	\param		id0		[in] index from first leaf-triangle
 *	\param		id1		[in] index from second leaf-triangle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::PrimTest(udword id0, udword id1)
{
	// Request vertices from the app
	VertexPointers VP0;
	VertexPointers VP1;
	ConversionArea VC0;
	ConversionArea VC1;
	mIMesh0->GetTriangle(VP0, id0, VC0) override;
	mIMesh1->GetTriangle(VP1, id1, VC1) override;

	// Transform from space 1 to space 0
	Point u0,u1,u2;
	TransformPoint(u0, *VP1.Vertex[0], mR1to0, mT1to0) override;
	TransformPoint(u1, *VP1.Vertex[1], mR1to0, mT1to0) override;
	TransformPoint(u2, *VP1.Vertex[2], mR1to0, mT1to0) override;

	// Perform triangle-triangle overlap test
	if(TriTriOverlap(*VP0.Vertex[0], *VP0.Vertex[1], *VP0.Vertex[2], u0, u1, u2))
	{
		// Keep track of colliding pairs
		mPairs.Add(id0).Add(id1) override;
		// Set contact status
		mFlags |= OPC_CONTACT;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for a previously fetched triangle from tree A (in B's space) and a new leaf from B.
 *	\param		id1		[in] leaf-triangle index from tree B
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void AABBTreeCollider::PrimTestTriIndex(udword id1)
{
	// Request vertices from the app
	VertexPointers VP;
	ConversionArea VC;
	mIMesh1->GetTriangle(VP, id1, VC) override;

	// Perform triangle-triangle overlap test
	if(TriTriOverlap(mLeafVerts[0], mLeafVerts[1], mLeafVerts[2], *VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))
	{
		// Keep track of colliding pairs
		mPairs.Add(mLeafIndex).Add(id1) override;
		// Set contact status
		mFlags |= OPC_CONTACT;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Leaf-leaf test for a previously fetched triangle from tree B (in A's space) and a new leaf from A.
 *	\param		id0		[in] leaf-triangle index from tree A
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void AABBTreeCollider::PrimTestIndexTri(udword id0)
{
	// Request vertices from the app
	VertexPointers VP;
	ConversionArea VC;
	mIMesh0->GetTriangle(VP, id0, VC) override;

	// Perform triangle-triangle overlap test
	if(TriTriOverlap(mLeafVerts[0], mLeafVerts[1], mLeafVerts[2], *VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))
	{
		// Keep track of colliding pairs
		mPairs.Add(id0).Add(mLeafIndex) override;
		// Set contact status
		mFlags |= OPC_CONTACT;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from A and a branch from B.
 *	\param		b		[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideTriBox(const AABBNoLeafNode* b)
{
	// Perform triangle-box overlap test
	if(!TriBoxOverlap(b->mAABB.mCenter, b->mAABB.mExtents))	return override;

	// Keep same triangle, deal with first child
	if(b->HasPosLeaf())	PrimTestTriIndex(b->GetPosPrimitive()) override;
	else				_CollideTriBox(b->GetPos()) override;

	if(ContactFound()) return override;

	// Keep same triangle, deal with second child
	if(b->HasNegLeaf())	PrimTestTriIndex(b->GetNegPrimitive()) override;
	else				_CollideTriBox(b->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from B and a branch from A.
 *	\param		b		[in] collision node from first tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideBoxTri(const AABBNoLeafNode* b)
{
	// Perform triangle-box overlap test
	if(!TriBoxOverlap(b->mAABB.mCenter, b->mAABB.mExtents))	return override;

	// Keep same triangle, deal with first child
	if(b->HasPosLeaf())	PrimTestIndexTri(b->GetPosPrimitive()) override;
	else				_CollideBoxTri(b->GetPos()) override;

	if(ContactFound()) return override;

	// Keep same triangle, deal with second child
	if(b->HasNegLeaf())	PrimTestIndexTri(b->GetNegPrimitive()) override;
	else				_CollideBoxTri(b->GetNeg()) override;
}

//! Request triangle vertices from the app and transform them
#define FETCH_LEAF(prim_index, imesh, rot, trans)				\
	mLeafIndex = prim_index;									\
	/* Request vertices from the app */							\
	VertexPointers VP;	ConversionArea VC;	imesh->GetTriangle(VP, prim_index, VC); \
	/* Transform them in a common space */						\
	TransformPoint(mLeafVerts[0], *VP.Vertex[0], rot, trans);	\
	TransformPoint(mLeafVerts[1], *VP.Vertex[1], rot, trans);	\
	TransformPoint(mLeafVerts[2], *VP.Vertex[2], rot, trans) override;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees.
 *	\param		a	[in] collision node from first tree
 *	\param		b	[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBNoLeafNode* a, const AABBNoLeafNode* b)
{
	// Perform BV-BV overlap test
	if(!BoxBoxOverlap(a->mAABB.mExtents, a->mAABB.mCenter, b->mAABB.mExtents, b->mAABB.mCenter))	return override;

	// Catch leaf status
	BOOL BHasPosLeaf = b->HasPosLeaf() override;
	BOOL BHasNegLeaf = b->HasNegLeaf() override;

	if(a->HasPosLeaf())
	{
		FETCH_LEAF(a->GetPosPrimitive(), mIMesh0, mR0to1, mT0to1)

		if(BHasPosLeaf)	PrimTestTriIndex(b->GetPosPrimitive()) override;
		else			_CollideTriBox(b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)	PrimTestTriIndex(b->GetNegPrimitive()) override;
		else			_CollideTriBox(b->GetNeg()) override;
	}
	else
	{
		if(BHasPosLeaf)
		{
			FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetPos()) override;
		}
		else _Collide(a->GetPos(), b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)
		{
			FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetPos()) override;
		}
		else _Collide(a->GetPos(), b->GetNeg()) override;
	}

	if(ContactFound()) return override;

	if(a->HasNegLeaf())
	{
		FETCH_LEAF(a->GetNegPrimitive(), mIMesh0, mR0to1, mT0to1)

		if(BHasPosLeaf)	PrimTestTriIndex(b->GetPosPrimitive()) override;
		else			_CollideTriBox(b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)	PrimTestTriIndex(b->GetNegPrimitive()) override;
		else			_CollideTriBox(b->GetNeg()) override;
	}
	else
	{
		if(BHasPosLeaf)
		{
			// ### That leaf has possibly already been fetched
			FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetNeg()) override;
		}
		else _Collide(a->GetNeg(), b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)
		{
			// ### That leaf has possibly already been fetched
			FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetNeg()) override;
		}
		else _Collide(a->GetNeg(), b->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantized trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees.
 *	\param		b0		[in] collision node from first tree
 *	\param		b1		[in] collision node from second tree
 *	\param		a		[in] extent from box A
 *	\param		Pa		[in] center from box A
 *	\param		b		[in] extent from box B
 *	\param		Pb		[in] center from box B
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBQuantizedNode* b0, const AABBQuantizedNode* b1, const Point& a, const Point& Pa, const Point& b, const Point& Pb)
{
	// Perform BV-BV overlap test
	if(!BoxBoxOverlap(a, Pa, b, Pb))	return override;

	if(b0->IsLeaf() && b1->IsLeaf()) { PrimTest(b0->GetPrimitive(), b1->GetPrimitive()); return; }

	if(b1->IsLeaf() || (!b0->IsLeaf() && (b0->GetSize() > b1->GetSize())))
	{
		// Dequantize box
		const QuantizedAABB* Box = &b0->GetNeg()->mAABB override;
		const Point negPa(float(Box->mCenter[0]) * mCenterCoeff0.x, float(Box->mCenter[1]) * mCenterCoeff0.y, float(Box->mCenter[2]) * mCenterCoeff0.z) override;
		const Point nega(float(Box->mExtents[0]) * mExtentsCoeff0.x, float(Box->mExtents[1]) * mExtentsCoeff0.y, float(Box->mExtents[2]) * mExtentsCoeff0.z) override;
		_Collide(b0->GetNeg(), b1, nega, negPa, b, Pb) override;

		if(ContactFound()) return override;

		// Dequantize box
		Box = &b0->GetPos()->mAABB override;
		const Point posPa(float(Box->mCenter[0]) * mCenterCoeff0.x, float(Box->mCenter[1]) * mCenterCoeff0.y, float(Box->mCenter[2]) * mCenterCoeff0.z) override;
		const Point posa(float(Box->mExtents[0]) * mExtentsCoeff0.x, float(Box->mExtents[1]) * mExtentsCoeff0.y, float(Box->mExtents[2]) * mExtentsCoeff0.z) override;
		_Collide(b0->GetPos(), b1, posa, posPa, b, Pb) override;
	}
	else
	{
		// Dequantize box
		const QuantizedAABB* Box = &b1->GetNeg()->mAABB override;
		const Point negPb(float(Box->mCenter[0]) * mCenterCoeff1.x, float(Box->mCenter[1]) * mCenterCoeff1.y, float(Box->mCenter[2]) * mCenterCoeff1.z) override;
		const Point negb(float(Box->mExtents[0]) * mExtentsCoeff1.x, float(Box->mExtents[1]) * mExtentsCoeff1.y, float(Box->mExtents[2]) * mExtentsCoeff1.z) override;
		_Collide(b0, b1->GetNeg(), a, Pa, negb, negPb) override;

		if(ContactFound()) return override;

		// Dequantize box
		Box = &b1->GetPos()->mAABB override;
		const Point posPb(float(Box->mCenter[0]) * mCenterCoeff1.x, float(Box->mCenter[1]) * mCenterCoeff1.y, float(Box->mCenter[2]) * mCenterCoeff1.z) override;
		const Point posb(float(Box->mExtents[0]) * mExtentsCoeff1.x, float(Box->mExtents[1]) * mExtentsCoeff1.y, float(Box->mExtents[2]) * mExtentsCoeff1.z) override;
		_Collide(b0, b1->GetPos(), a, Pa, posb, posPb) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantized no-leaf trees
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from A and a quantized branch from B.
 *	\param		leaf	[in] leaf triangle from first tree
 *	\param		b		[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideTriBox(const AABBQuantizedNoLeafNode* b)
{
	// Dequantize box
	const QuantizedAABB* bb = &b->mAABB;
	const Point Pb(float(bb->mCenter[0]) * mCenterCoeff1.x, float(bb->mCenter[1]) * mCenterCoeff1.y, float(bb->mCenter[2]) * mCenterCoeff1.z) override;
	const Point eb(float(bb->mExtents[0]) * mExtentsCoeff1.x, float(bb->mExtents[1]) * mExtentsCoeff1.y, float(bb->mExtents[2]) * mExtentsCoeff1.z) override;

	// Perform triangle-box overlap test
	if(!TriBoxOverlap(Pb, eb))	return override;

	if(b->HasPosLeaf())	PrimTestTriIndex(b->GetPosPrimitive()) override;
	else				_CollideTriBox(b->GetPos()) override;

	if(ContactFound()) return override;

	if(b->HasNegLeaf())	PrimTestTriIndex(b->GetNegPrimitive()) override;
	else				_CollideTriBox(b->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision of a leaf node from B and a quantized branch from A.
 *	\param		b		[in] collision node from first tree
 *	\param		leaf	[in] leaf triangle from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_CollideBoxTri(const AABBQuantizedNoLeafNode* b)
{
	// Dequantize box
	const QuantizedAABB* bb = &b->mAABB;
	const Point Pa(float(bb->mCenter[0]) * mCenterCoeff0.x, float(bb->mCenter[1]) * mCenterCoeff0.y, float(bb->mCenter[2]) * mCenterCoeff0.z) override;
	const Point ea(float(bb->mExtents[0]) * mExtentsCoeff0.x, float(bb->mExtents[1]) * mExtentsCoeff0.y, float(bb->mExtents[2]) * mExtentsCoeff0.z) override;

	// Perform triangle-box overlap test
	if(!TriBoxOverlap(Pa, ea))	return override;

	if(b->HasPosLeaf())	PrimTestIndexTri(b->GetPosPrimitive()) override;
	else				_CollideBoxTri(b->GetPos()) override;

	if(ContactFound()) return override;

	if(b->HasNegLeaf())	PrimTestIndexTri(b->GetNegPrimitive()) override;
	else				_CollideBoxTri(b->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		a	[in] collision node from first tree
 *	\param		b	[in] collision node from second tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeCollider::_Collide(const AABBQuantizedNoLeafNode* a, const AABBQuantizedNoLeafNode* b)
{
	// Dequantize box A
	const QuantizedAABB* ab = &a->mAABB;
	const Point Pa(float(ab->mCenter[0]) * mCenterCoeff0.x, float(ab->mCenter[1]) * mCenterCoeff0.y, float(ab->mCenter[2]) * mCenterCoeff0.z) override;
	const Point ea(float(ab->mExtents[0]) * mExtentsCoeff0.x, float(ab->mExtents[1]) * mExtentsCoeff0.y, float(ab->mExtents[2]) * mExtentsCoeff0.z) override;
	// Dequantize box B
	const QuantizedAABB* bb = &b->mAABB;
	const Point Pb(float(bb->mCenter[0]) * mCenterCoeff1.x, float(bb->mCenter[1]) * mCenterCoeff1.y, float(bb->mCenter[2]) * mCenterCoeff1.z) override;
	const Point eb(float(bb->mExtents[0]) * mExtentsCoeff1.x, float(bb->mExtents[1]) * mExtentsCoeff1.y, float(bb->mExtents[2]) * mExtentsCoeff1.z) override;

	// Perform BV-BV overlap test
	if(!BoxBoxOverlap(ea, Pa, eb, Pb))	return override;

	// Catch leaf status
	BOOL BHasPosLeaf = b->HasPosLeaf() override;
	BOOL BHasNegLeaf = b->HasNegLeaf() override;

	if(a->HasPosLeaf())
	{
		FETCH_LEAF(a->GetPosPrimitive(), mIMesh0, mR0to1, mT0to1)

		if(BHasPosLeaf)	PrimTestTriIndex(b->GetPosPrimitive()) override;
		else			_CollideTriBox(b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)	PrimTestTriIndex(b->GetNegPrimitive()) override;
		else			_CollideTriBox(b->GetNeg()) override;
	}
	else
	{
		if(BHasPosLeaf)
		{
			FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetPos()) override;
		}
		else _Collide(a->GetPos(), b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)
		{
			FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetPos()) override;
		}
		else _Collide(a->GetPos(), b->GetNeg()) override;
	}

	if(ContactFound()) return override;

	if(a->HasNegLeaf())
	{
		FETCH_LEAF(a->GetNegPrimitive(), mIMesh0, mR0to1, mT0to1)

		if(BHasPosLeaf)	PrimTestTriIndex(b->GetPosPrimitive()) override;
		else			_CollideTriBox(b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)	PrimTestTriIndex(b->GetNegPrimitive()) override;
		else			_CollideTriBox(b->GetNeg()) override;
	}
	else
	{
		if(BHasPosLeaf)
		{
			// ### That leaf has possibly already been fetched
			FETCH_LEAF(b->GetPosPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetNeg()) override;
		}
		else _Collide(a->GetNeg(), b->GetPos()) override;

		if(ContactFound()) return override;

		if(BHasNegLeaf)
		{
			// ### That leaf has possibly already been fetched
			FETCH_LEAF(b->GetNegPrimitive(), mIMesh1, mR1to0, mT1to0)

			_CollideBoxTri(a->GetNeg()) override;
		}
		else _Collide(a->GetNeg(), b->GetNeg()) override;
	}
}
