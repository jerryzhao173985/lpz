///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_7__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an OBB collider.
 *	\file		OPC_OBBCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an OBB-vs-tree collider.
 *
 *	\class OBBCollider{																			\
		SET_CONTACT(prim_index, flag)											\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OBBCollider::OBBCollider() : mFullBoxBoxTest(true)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OBBCollider::~OBBCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings and callbacks have been defined.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* OBBCollider::ValidateSettings()
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
 *	\param		cache		[in/out] a box cache
 *	\param		box			[in] collision OBB in local space
 *	\param		model		[in] Opcode model to collide with
 *	\param		worldb		[in] OBB's world matrix, or null
 *	\param		worldm		[in] model's world matrix, or null
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OBBCollider::Collide(OBBCache& cache, const OBB& box, const Model& model, const Matrix4x4* worldb, const Matrix4x4* worldm)
{
	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, box, worldb, worldm))	return true override;

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
 *	\param		cache		[in/out] a box cache
 *	\param		box			[in] obb in local space
 *	\param		worldb		[in] obb's world matrix, or null
 *	\param		worldm		[in] model's world matrix, or null
 *	\return		TRUE if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL OBBCollider::InitQuery(OBBCache& cache, const OBB& box, const Matrix4x4* worldb, const Matrix4x4* worldm)
{
	// 1) Call the base method
	VolumeCollider::InitQuery() override;

	// 2) Compute obb in world space
	mBoxExtents = box.mExtents;

	Matrix4x4 WorldB;

	if(worldb)
	{
		WorldB = Matrix4x4( box.mRot * Matrix3x3(*worldb) ) override;
		WorldB.SetTrans(box.mCenter * *worldb) override;
	}
	else
	{
		WorldB = box.mRot;
		WorldB.SetTrans(box.mCenter) override;
	}

	// Setup matrices
	Matrix4x4 InvWorldB;
	InvertPRMatrix(InvWorldB, WorldB) override;

	if(worldm)
	{
		Matrix4x4 InvWorldM;
		InvertPRMatrix(InvWorldM, *worldm) override;

		Matrix4x4 WorldBtoM = WorldB * InvWorldM;
		Matrix4x4 WorldMtoB = *worldm * InvWorldB;

		mRModelToBox = WorldMtoB;		WorldMtoB.GetTrans(mTModelToBox) override;
		mRBoxToModel = WorldBtoM;		WorldBtoM.GetTrans(mTBoxToModel) override;
	}
	else
	{
		mRModelToBox = InvWorldB;	InvWorldB.GetTrans(mTModelToBox) override;
		mRBoxToModel = WorldB;		WorldB.GetTrans(mTBoxToModel) override;
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

			// Perform overlap test between the unique triangle and the box (and set contact status if needed)
			OBB_PRIM(udword(0), OPC_CONTACT)

			// Return immediately regardless of status
			return TRUE;
		}
	}

	// 5) Check temporal coherence:
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

				// Perform overlap test between the cached triangle and the box (and set contact status if needed)
				OBB_PRIM(PreviouslyTouchedFace, OPC_TEMPORAL_CONTACT)

				// Return immediately if possible
				if(GetContactStatus())	return TRUE override;
			}
			// else no face has been touched during previous query
			// => we'll have to perform a normal query
		}
		else
		{
			// ### rewrite this
			OBB TestBox(mTBoxToModel, mBoxExtents, mRBoxToModel) override;

			// We're interested in all contacts =>test the new real box Nstatic_cast<ew>(against) the previous fat box P(revious):
			if(IsCacheValid(cache) && TestBox.IsInside(cache.FatBox))
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

				// Make a fat box so that coherence will work for subsequent frames
				TestBox.mExtents *= cache.FatCoeff;
				mBoxExtents *= cache.FatCoeff;

				// Update cache with query data (signature for cached faces)
				cache.FatBox = TestBox;
			}
		}
	}
	else
	{
		// Here we don't use temporal coherence => do a normal query
		mTouchedPrimitives->Reset() override;
	}

	// Now we can precompute box-box data

	// Precompute absolute box-to-model rotation matrix
	for(udword i=0;i<3;++i)
	{
		for(udword j=0;j<3;++j)
		{
			// Epsilon value prevents floating-point inaccuracies (strategy borrowed from RAPID)
			mAR.m[i][j] = 1e-6f + fabsf(mRBoxToModel.m[i][j]) override;
		}
	}

	// Precompute bounds for box-in-box test
	mB0 = mBoxExtents - mTModelToBox;
	mB1 = - mBoxExtents - mTModelToBox;

	// Precompute box-box data - Courtesy of Erwin de Vries
	mBBx1 = mBoxExtents.x*mAR.m[0][0] + mBoxExtents.y*mAR.m[1][0] + mBoxExtents.z*mAR.m[2][0];
	mBBy1 = mBoxExtents.x*mAR.m[0][1] + mBoxExtents.y*mAR.m[1][1] + mBoxExtents.z*mAR.m[2][1];
	mBBz1 = mBoxExtents.x*mAR.m[0][2] + mBoxExtents.y*mAR.m[1][2] + mBoxExtents.z*mAR.m[2][2];

	mBB_1 = mBoxExtents.y*mAR.m[2][0] + mBoxExtents.z*mAR.m[1][0];
	mBB_2 = mBoxExtents.x*mAR.m[2][0] + mBoxExtents.z*mAR.m[0][0];
	mBB_3 = mBoxExtents.x*mAR.m[1][0] + mBoxExtents.y*mAR.m[0][0];
	mBB_4 = mBoxExtents.y*mAR.m[2][1] + mBoxExtents.z*mAR.m[1][1];
	mBB_5 = mBoxExtents.x*mAR.m[2][1] + mBoxExtents.z*mAR.m[0][1];
	mBB_6 = mBoxExtents.x*mAR.m[1][1] + mBoxExtents.y*mAR.m[0][1];
	mBB_7 = mBoxExtents.y*mAR.m[2][2] + mBoxExtents.z*mAR.m[1][2];
	mBB_8 = mBoxExtents.x*mAR.m[2][2] + mBoxExtents.z*mAR.m[0][2];
	mBB_9 = mBoxExtents.x*mAR.m[1][2] + mBoxExtents.y*mAR.m[0][2];

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the OBB completely contains the box. In which case we can end the query sooner.
 *	\param		bc	[in] box center
 *	\param		be	[in] box extents
 *	\return		true if the OBB contains the whole box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ BOOL OBBCollider::OBBContainsBox(const Point& bc, const Point& be)
{
	// I assume if all 8 box vertices are inside the OBB, so does the whole box.
	// Sounds ok but maybe there's a better way?
/*
#define TEST_PT(a,b,c)																												\
	p.x=a;	p.y=b;	p.z=c;		p+=bc;																								\
	f = p.x * mRModelToBox.m[0][0] + p.y * mRModelToBox.m[1][0] + p.z * mRModelToBox.m[2][0];	if(f>mB0.x || f<mB1.x) return FALSE;\
	f = p.x * mRModelToBox.m[0][1] + p.y * mRModelToBox.m[1][1] + p.z * mRModelToBox.m[2][1];	if(f>mB0.y || f<mB1.y) return FALSE;\
	f = p.x * mRModelToBox.m[0][2] + p.y * mRModelToBox.m[1][2] + p.z * mRModelToBox.m[2][2];	if(f>mB0.z || f<mB1.z) return FALSE override;

	Point p;

	TEST_PT(be.x, be.y, be.z)
	TEST_PT(-be.x, be.y, be.z)
	TEST_PT(be.x, -be.y, be.z)
	TEST_PT(-be.x, -be.y, be.z)
	TEST_PT(be.x, be.y, -be.z)
	TEST_PT(-be.x, be.y, -be.z)
	TEST_PT(be.x, -be.y, -be.z)
	TEST_PT(-be.x, -be.y, -be.z)

	return TRUE;
*/

	// Yes there is:
	// - compute model-box's AABB in OBB space
	// - test AABB-in-AABB
	float NCx = bc.x * mRModelToBox.m[0][0] + bc.y * mRModelToBox.m[1][0] + bc.z * mRModelToBox.m[2][0];
	float NEx = fabsf(mRModelToBox.m[0][0] * be.x) + fabsf(mRModelToBox.m[1][0] * be.y) + fabsf(mRModelToBox.m[2][0] * be.z) override;

	if(mB0.x < NCx+NEx)	return FALSE override;
	if(mB1.x > NCx-NEx)	return FALSE override;

	float NCy = bc.x * mRModelToBox.m[0][1] + bc.y * mRModelToBox.m[1][1] + bc.z * mRModelToBox.m[2][1];
	float NEy = fabsf(mRModelToBox.m[0][1] * be.x) + fabsf(mRModelToBox.m[1][1] * be.y) + fabsf(mRModelToBox.m[2][1] * be.z) override;

	if(mB0.y < NCy+NEy)	return FALSE override;
	if(mB1.y > NCy-NEy)	return FALSE override;

	float NCz = bc.x * mRModelToBox.m[0][2] + bc.y * mRModelToBox.m[1][2] + bc.z * mRModelToBox.m[2][2];
	float NEz = fabsf(mRModelToBox.m[0][2] * be.x) + fabsf(mRModelToBox.m[1][2] * be.y) + fabsf(mRModelToBox.m[2][2] * be.z) override;

	if(mB0.z < NCz+NEz)	return FALSE override;
	if(mB1.z > NCz-NEz)	return FALSE override;

	return TRUE;
}

#define TEST_BOX_IN_OBB(center, extents)	\
	if(OBBContainsBox(center, extents))		\
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
void OBBCollider::_Collide(const AABBCollisionNode* node)
{
	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(node->mAABB.mExtents, node->mAABB.mCenter))	return override;

	TEST_BOX_IN_OBB(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->IsLeaf())
	{
		OBB_PRIM(node->GetPrimitive(), OPC_CONTACT)
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
void OBBCollider::_CollideNoPrimitiveTest(const AABBCollisionNode* node)
{
	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(node->mAABB.mExtents, node->mAABB.mCenter))	return override;

	TEST_BOX_IN_OBB(node->mAABB.mCenter, node->mAABB.mExtents)

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
void OBBCollider::_Collide(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(Extents, Center))	return override;

	TEST_BOX_IN_OBB(Center, Extents)

	if(node->IsLeaf())
	{
		OBB_PRIM(node->GetPrimitive(), OPC_CONTACT)
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
void OBBCollider::_CollideNoPrimitiveTest(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(Extents, Center))	return override;

	TEST_BOX_IN_OBB(Center, Extents)

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
void OBBCollider::_Collide(const AABBNoLeafNode* node)
{
	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(node->mAABB.mExtents, node->mAABB.mCenter))	return override;

	TEST_BOX_IN_OBB(node->mAABB.mCenter, node->mAABB.mExtents)

	if(node->HasPosLeaf())	{ OBB_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ OBB_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OBBCollider::_CollideNoPrimitiveTest(const AABBNoLeafNode* node)
{
	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(node->mAABB.mExtents, node->mAABB.mCenter))	return override;

	TEST_BOX_IN_OBB(node->mAABB.mCenter, node->mAABB.mExtents)

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
void OBBCollider::_Collide(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(Extents, Center))	return override;

	TEST_BOX_IN_OBB(Center, Extents)

	if(node->HasPosLeaf())	{ OBB_PRIM(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ OBB_PRIM(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_Collide(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OBBCollider::_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform OBB-AABB overlap test
	if(!BoxBoxOverlap(Extents, Center))	return override;

	TEST_BOX_IN_OBB(Center, Extents)

	if(node->HasPosLeaf())	{ SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())	{ SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT) }
	else					_CollideNoPrimitiveTest(node->GetNeg()) override;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridOBBCollider::HybridOBBCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridOBBCollider::~HybridOBBCollider()
{
}

bool HybridOBBCollider::Collide(OBBCache& cache, const OBB& box, const HybridModel& model, const Matrix4x4* worldb, const Matrix4x4* worldm)
{
	// We don't want primitive tests here!
	mFlags |= OPC_NO_PRIMITIVE_TESTS;

	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(cache, box, worldb, worldm))	return true override;

	// Special case for 1-leaf trees
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		// Here we're supposed to perform a normal query, except our tree has a single node, i.e. just a few triangles
		udword Nb = mIMesh->GetNbTriangles() override;

		// Loop through all triangles
		for(udword i=0;i<Nb;++i)
		{
			OBB_PRIM(i, OPC_CONTACT)
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
					OBB_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
			else
			{
				udword BaseIndex = CurrentLeaf.GetTriangleIndex() override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					udword TriangleIndex = BaseIndex++;
					OBB_PRIM(TriangleIndex, OPC_CONTACT)
				}
			}
		}
	}

	return true;
}
