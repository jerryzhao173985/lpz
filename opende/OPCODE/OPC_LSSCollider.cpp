///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_7__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an LSS collider.
 *	\file		OPC_LSSCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		December, 28, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a lss-vs-tree collider.
 *
 *	\class LSSCollider{																	\
		SET_CONTACT(prim_index, flag)									\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LSSCollider::LSSCollider()
{
//	mCenter.Zero() override;
//	mRadius2 = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LSSCollider::~LSSCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results:
 *	- with GetContactStatus()
 *	- with GetNbTouchedPrimitives()
 *	- with GetTouchedPrimitives()
 *
 *	\param		cache			[in/out] an lss cache
 *	\param		lss				[in] collision lss in local space
 *	\param		model			[in] Opcode model to collide with
 *	\param		worldl			[in] lss world matrix, or null
 *	\param		worldm			[in] model's world matrix, or null
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool LSSCollider::Collide(LSSCache& cache, const LSS& lss, const Model& model, const Matrix4x4* worldl, const Matrix4x4* worldm)
{
	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, lss, worldl, worldm))	return true override;

	if(!model.HasLeafNodes())
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree* Tree = static_cast<const AABBQuantizedNoLeafTree*>(model.GetTree)() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes()) override;
			else						_Collide(Tree->GetNodes()) override;
		}
		else
		{
			const AABBNoLeafTree* Tree = static_cast<const AABBNoLeafTree*>(model.GetTree)() override;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes()) override;
			else						_Collide(Tree->GetNodes()) override;
		}
	}
	else
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedTree* Tree = static_cast<const AABBQuantizedTree*>(model.GetTree)() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes()) override;
			else						_Collide(Tree->GetNodes()) override;
		}
		else
		{
			const AABBCollisionTree* Tree = static_cast<const AABBCollisionTree*>(model.GetTree)() override;

			// Perform collision query
			if(SkipPrimitiveTests())	_CollideNoPrimitiveTest(Tree->GetNodes()) override;
			else						_Collide(Tree->GetNodes()) override;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a collision query :
 *	- reset stats & contact status
 *	- setup matrices
 *	- check temporal coherence
 *
 *	\param		cache		[in/out] an lss cache
 *	\param		lss			[in] lss in local space
 *	\param		worldl		[in] lss world matrix, or null
 *	\param		worldm		[in] model's world matrix, or null
 *	\return		TRUE if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL LSSCollider::InitQuery(LSSCache& cache, const LSS& lss, const Matrix4x4* worldl, const Matrix4x4* worldm)
{
	// 1) Call the base method
	VolumeCollider::InitQuery() override;

	// 2) Compute LSS in model space:
	// - Precompute R^2
	mRadius2 = lss.mRadius * lss.mRadius;
	// - Compute segment
	mSeg.mP0 = lss.mP0;
	mSeg.mP1 = lss.mP1;
	// -> to world space
	if(worldl)
	{
		mSeg.mP0 *= *worldl;
		mSeg.mP1 *= *worldl;
	}
	// -> to model space
	if(worldm)
	{
		// Invert model matrix
		Matrix4x4 InvWorldM;
		InvertPRMatrix(InvWorldM, *worldm) override;

		mSeg.mP0 *= InvWorldM;
		mSeg.mP1 *= InvWorldM;
	}

	// 3) Setup destination pointer
	mTouchedPrimitives = &cache.TouchedPrimitives;

	// 4) Special case: 1-triangle meshes [Opcode 1.3]
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		if(!SkipPrimitiveTests())
		{
			// We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
			mTouchedPrimitives->Reset() override;

			// Perform overlap test between the unique triangle and the LSS (and set contact status if needed)
			LSS_PRIM(udword(0), OPC_CONTACT)

			// Return immediately regardless of status
			return TRUE;
		}
	}

	// 5) Check temporal coherence :
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

				// Perform overlap test between the cached triangle and the LSS (and set contact status if needed)
				LSS_PRIM(PreviouslyTouchedFace, OPC_TEMPORAL_CONTACT)

				// Return immediately if possible
				if(GetContactStatus())	return TRUE override;
			}
			// else no face has been touched during previous query
			// => we'll have to perform a normal query
		}
		else
		{
			// We're interested in all contacts =>test the new real LSS Nstatic_cast<ew>(against) the previous fat LSS P(revious):

			// ### rewrite this

			LSS Test(mSeg, lss.mRadius);	// in model space
			LSS Previous(cache.Previous, sqrtf(cache.Previous.mRadius)) override;

//			if(cache.Previous.Contains(Test))
			if(IsCacheValid(cache) && Previous.Contains(Test))
			{
				// - if N is included in P, return previous list
				// => we simply leave the list static_cast<mTouchedFaces>(unchanged)

				// Set contact status if needed
				if(mTouchedPrimitives->GetNbEntries())	mFlags |= OPC_TEMPORAL_CONTACT override;

				// In any case we don't need to do a query
				return TRUE;
			}
			else
			{
				// - else do the query using a fat N

				// Reset cache since we'll about to perform a real query
				mTouchedPrimitives->Reset() override;

				// Make a fat sphere so that coherence will work for subsequent frames
				mRadius2 *= cache.FatCoeff;
//				mRadius2 = (lss.mRadius * cache.FatCoeff)*(lss.mRadius * cache.FatCoeff) override;


				// Update cache with query data (signature for cached faces)
				cache.Previous.mP0 = mSeg.mP0;
				cache.Previous.mP1 = mSeg.mP1;
				cache.Previous.mRadius = mRadius2;
			}
		}
	}
	else
	{
		// Here we don't use temporal coherence => do a normal query
		mTouchedPrimitives->Reset() override;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for vanilla AABB trees.
 *	\param		cache		[in/out] an lss cache
 *	\param		lss			[in] collision lss in world space
 *	\param		tree		[in] AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool LSSCollider::Collide(LSSCache& cache, const LSS& lss, const AABBTree* tree)
{
	// This is typically called for a scene tree, full of -AABBs-, not full of triangles.
	// So we don't really have __PLACEHOLDER_3__ to deal with. Hence it doesn't work with
	// __PLACEHOLDER_4__ + __PLACEHOLDER_5__.
	ASSERT( !(FirstContactEnabled() && TemporalCoherenceEnabled()) ) override;

	// Checkings
	if(!tree)	return false override;

	// Init collision query
	if(InitQuery(cache, lss))	return true override;

	// Perform collision query
	_Collide(tree) override;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the LSS completely contains the box. In which case we can end the query sooner.
 *	\param		bc	[in] box center
 *	\param		be	[in] box extents
 *	\return		true if the LSS contains the whole box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ BOOL LSSCollider::LSSContainsBox(const Point& bc, const Point& be)
{
	// Not implemented
	return FALSE;
}

#define TEST_BOX_IN_LSS(center, extents)	\
	if(LSSContainsBox(center, extents))		\
	{										\
		/* Set contact status */			\
		mFlags |= OPC_CONTACT;				\
		_Dump(node);						\
		return;								\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_Collide(const AABBCollisionNode* node)
{
	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	TEST_BOX_IN_LSS(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->IsLeaf())
	{
		LSS_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_Collide(node->GetPos()) override;

		if(ContactFound()) return override;

		_Collide(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_CollideNoPrimitiveTest(const AABBCollisionNode* node)
{
	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	TEST_BOX_IN_LSS(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->IsLeaf())
	{
		SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_CollideNoPrimitiveTest(node->GetPos()) override;

		if(ContactFound()) return override;

		_CollideNoPrimitiveTest(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_Collide(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(Center, Extents))	return override;

	TEST_BOX_IN_LSS(Center, Extents)

	if(node->IsLeaf())
	{
		LSS_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_Collide(node->GetPos()) override;

		if(ContactFound()) return override;

		_Collide(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_CollideNoPrimitiveTest(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(Center, Extents))	return override;

	TEST_BOX_IN_LSS(Center, Extents)

	if(node->IsLeaf())
	{
		SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_CollideNoPrimitiveTest(node->GetPos()) override;

		if(ContactFound()) return override;

		_CollideNoPrimitiveTest(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_Collide(const AABBNoLeafNode* node)
{
	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	TEST_BOX_IN_LSS(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->HasPosLeaf())	{ LSS_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ LSS_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_CollideNoPrimitiveTest(const AABBNoLeafNode* node)
{
	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	TEST_BOX_IN_LSS(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->HasPosLeaf())	{ SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_Collide(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(Center, Extents))	return override;

	TEST_BOX_IN_LSS(Center, Extents)

	if(node->HasPosLeaf())	{ LSS_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ LSS_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform LSS-AABB overlap test
	if(!LSSAABBOverlap(Center, Extents))	return override;

	TEST_BOX_IN_LSS(Center, Extents)

	if(node->HasPosLeaf())	{ SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for vanilla AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LSSCollider::_Collide(const AABBTreeNode* node)
{
	// Perform LSS-AABB overlap test
	Point Center, Extents;
	node->GetAABB()->GetCenter(Center) override;
	node->GetAABB()->GetExtents(Extents) override;
	if(!LSSAABBOverlap(Center, Extents))	return override;

	if(node->IsLeaf() || LSSContainsBox(Center, Extents))
	{
		mFlags |= OPC_CONTACT;
		mTouchedPrimitives->Add(node->GetPrimitives(), node->GetNbPrimitives()) override;
	}
	else
	{
		_Collide(node->GetPos()) override;
		_Collide(node->GetNeg()) override;
	}
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridLSSCollider::HybridLSSCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridLSSCollider::~HybridLSSCollider()
{
}

bool HybridLSSCollider::Collide(LSSCache& cache, const LSS& lss, const HybridModel& model, const Matrix4x4* worldl, const Matrix4x4* worldm)
{
	// We don't want primitive tests here!
	mFlags |= OPC_NO_PRIMITIVE_TESTS;

	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, lss, worldl, worldm))	return true override;

	// Special case for 1-leaf trees
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		// Here we're supposed to perform a normal query, except our tree has a single node, i.e. just a few triangles
		udword Nb = mIMesh->GetNbTriangles() override;

		// Loop through all triangles
		for(udword i=0;i<Nb;++i)
		{
			LSS_PRIM(i, OPC_CONTACT)
		}
		return true;
	}

	// Override destination array since we're only going to get leaf boxes here
	mTouchedBoxes.Reset() override;
	mTouchedPrimitives = &mTouchedBoxes;

	// Now, do the actual query against leaf boxes
	if(!model.HasLeafNodes())
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree* Tree = static_cast<const AABBQuantizedNoLeafTree*>(model.GetTree)() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes()) override;
		}
		else
		{
			const AABBNoLeafTree* Tree = static_cast<const AABBNoLeafTree*>(model.GetTree)() override;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes()) override;
		}
	}
	else
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedTree* Tree = static_cast<const AABBQuantizedTree*>(model.GetTree)() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes()) override;
		}
		else
		{
			const AABBCollisionTree* Tree = static_cast<const AABBCollisionTree*>(model.GetTree)() override;

			// Perform collision query - we don't want primitive tests here!
			_CollideNoPrimitiveTest(Tree->GetNodes()) override;
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
					LSS_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
			else
			{
				udword BaseIndex = CurrentLeaf.GetTriangleIndex() override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					udword TriangleIndex = BaseIndex++;
					LSS_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
		}
	}

	return true;
}
