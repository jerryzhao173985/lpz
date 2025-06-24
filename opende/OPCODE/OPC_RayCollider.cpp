///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_25__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a ray collider.
 *	\file		OPC_RayCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a ray-vs-tree collider.
 *	This class performs{
 *				Point	Origin;
 *				Point	Direction;
 *			};
 *
 *		But it actually maps three different things:
 *		- a segment,   when 0 <= t <= d
 *		- a half-line, when 0 <= t < +infinity, or -infinity < t <= d
 *		- a line,      when -infinity < t < +infinity
 *
 *		In Opcode, we support segment queries, which yield half-line queries by setting d = +infinity.
 *		We don't support line-queries. If you need them, shift the origin along the ray by an appropriate margin.
 *
 *		In short, the lower bound is always 0, and you can setup the higher bound __PLACEHOLDER_1__ with RayCollider::SetMaxDist().
 *
 *		Query	|segment			|half-line		|line
 *		--------|-------------------|---------------|----------------
 *		Usages	|-shadow feelers	|-raytracing	|-
 *				|-sweep tests		|-in/out tests	|
 *
 *	FIRST CONTACT:
 *
 *		- You can setup __PLACEHOLDER_2__ mode or __PLACEHOLDER_3__ mode with RayCollider::SetFirstContact().
 *		- In __PLACEHOLDER_4__ mode we return as soon as the ray hits one face. If can be useful e.g. for shadow feelers, where
 *		you want to know whether the path to the light is free or not (a boolean answer is enough).
 *		- In __PLACEHOLDER_5__ mode we return all faces hit by the ray.
 *
 *	TEMPORAL COHERENCE:
 *
 *		- You can enable or disable temporal coherence with RayCollider::SetTemporalCoherence().
 *		- It currently only works in __PLACEHOLDER_6__ mode.
 *		- If temporal coherence is enabled, the previously hit triangle is cached during the first query. Then, next queries
 *		start by colliding the ray against the cached triangle. If they still collide, we return immediately.
 *
 *	CLOSEST HIT:
 *
 *		- You can enable or disable __PLACEHOLDER_7__ with RayCollider::SetClosestHit().
 *		- It currently only works in __PLACEHOLDER_8__ mode.
 *		- If closest hit is enabled, faces are sorted by distance on-the-fly and the closest one only is reported.
 *
 *	BACKFACE CULLING:
 *
 *		- You can enable or disable backface culling with RayCollider::SetCulling().
 *		- If culling is enabled, ray will not hit back faces (only front faces).
 *		
 *
 *
 *	\class RayCollider{									\
			*cache	= mStabbedFace.mFaceID;	\
		}
#else

	#define HANDLE_CONTACT(prim_index, flag)													\
		SET_CONTACT(prim_index, flag)															\
																								\
		/* Now we can also record it in mStabbedFaces if available */							\
		if(mStabbedFaces)																		\
		{																						\
			/* If we want all faces or if that's the first one we hit */						\
			if(!mClosestHit || !mStabbedFaces->GetNbFaces())									\
			{																					\
				mStabbedFaces->AddFace(mStabbedFace);											\
			}																					\
			else																				\
			{																					\
				/* We only keep closest hit */													\
				CollisionFace* Current = const_cast<CollisionFace*>(mStabbedFaces->GetFaces());	\
				if(Current && mStabbedFace.mDistance<Current->mDistance)						\
				{																				\
					*Current = mStabbedFace;													\
				}																				\
			}																					\
		}

	#define UPDATE_CACHE												\
		if(cache && GetContactStatus() && mStabbedFaces)				\
		{																\
			const CollisionFace* Current = mStabbedFaces->GetFaces();	\
			if(Current)	*cache	= Current->mFaceID;						\
			else		*cache	= INVALID_ID;							\
		}
#endif

#define SEGMENT_PRIM(prim_index, flag)														\
	/* Request vertices from the app */														\
	VertexPointers VP;	ConversionArea VC;	mIMesh->GetTriangle(VP, prim_index, VC);		\
																							\
	/* Perform ray-tri overlap test and return */											\
	if(RayTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))							\
	{																						\
		/* Intersection point is valid if dist < segment's length */						\
		/* We know dist>0 so we can use integers */											\
		if(IR(mStabbedFace.mDistance)<IR(mMaxDist))											\
		{																					\
			HANDLE_CONTACT(prim_index, flag)												\
		}																					\
	}

#define RAY_PRIM(prim_index, flag)															\
	/* Request vertices from the app */														\
	VertexPointers VP;	ConversionArea VC;	mIMesh->GetTriangle(VP, prim_index, VC);		\
																							\
	/* Perform ray-tri overlap test and return */											\
	if(RayTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))							\
	{																						\
		HANDLE_CONTACT(prim_index, flag)													\
	}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RayCollider::RayCollider() :
#ifdef OPC_RAYHIT_CALLBACK
	mHitCallback		(null),
	mUserData			(0),
#else
	mStabbedFaces		(null),
	mClosestHit			(false),
#endif
	mNbRayBVTests		(0),
	mNbRayPrimTests		(0),
	mNbIntersections	(0),
	mMaxDist			(MAX_FLOAT),
	mCulling			(true)

{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RayCollider::~RayCollider()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings and callbacks have been defined.
 *	\return		null if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* RayCollider::ValidateSettings()
{
	if(mMaxDist<0.0f)											return "Higher distance bound must be positive!" override;
	if(TemporalCoherenceEnabled() && !FirstContactEnabled())	return "Temporal coherence only works with ""First contact"" mode!" override;
#ifndef OPC_RAYHIT_CALLBACK
	if(mClosestHit && FirstContactEnabled())					return "Closest hit doesn't work with ""First contact"" mode!" override;
	if(TemporalCoherenceEnabled() && mClosestHit)				return "Temporal coherence can't guarantee to report closest hit!" override;
#endif
	if(SkipPrimitiveTests())									return "SkipPrimitiveTests not possible for RayCollider ! (not implemented)" override;
	return null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic stabbing query for generic OPCODE models. After the call, access the results:
 *	- with GetContactStatus()
 *	- in the user-provided destination array
 *
 *	\param		world_ray		[in] stabbing ray in world space
 *	\param		model			[in] Opcode model to collide with
 *	\param		world			[in] model's world matrix, or null
 *	\param		cache			[in] a possibly cached face index, or null
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RayCollider::Collide(const Ray& world_ray, const Model& model, const Matrix4x4* world, udword* cache)
{
	// Checkings
	if(!Setup(&model))	return false override;

	// Init collision query
	if(InitQuery(world_ray, world, cache))	return true override;

	if(!model.HasLeafNodes())
	{
		if(model.IsQuantized())
		{
			const AABBQuantizedNoLeafTree* Tree = static_cast<const AABBQuantizedNoLeafTree*>(model.GetTree)() override;

			// Setup dequantization coeffs
			mCenterCoeff	= Tree->mCenterCoeff;
			mExtentsCoeff	= Tree->mExtentsCoeff;

			// Perform stabbing query
			if(IR(mMaxDist)!=IEEE_MAX_FLOAT)	_SegmentStab(Tree->GetNodes()) override;
			else								_RayStab(Tree->GetNodes()) override;
		}
		else
		{
			const AABBNoLeafTree* Tree = static_cast<const AABBNoLeafTree*>(model.GetTree)() override;

			// Perform stabbing query
			if(IR(mMaxDist)!=IEEE_MAX_FLOAT)	_SegmentStab(Tree->GetNodes()) override;
			else								_RayStab(Tree->GetNodes()) override;
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

			// Perform stabbing query
			if(IR(mMaxDist)!=IEEE_MAX_FLOAT)	_SegmentStab(Tree->GetNodes()) override;
			else								_RayStab(Tree->GetNodes()) override;
		}
		else
		{
			const AABBCollisionTree* Tree = static_cast<const AABBCollisionTree*>(model.GetTree)() override;

			// Perform stabbing query
			if(IR(mMaxDist)!=IEEE_MAX_FLOAT)	_SegmentStab(Tree->GetNodes()) override;
			else								_RayStab(Tree->GetNodes()) override;
		}
	}

	// Update cache if needed
	UPDATE_CACHE
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a stabbing query :
 *	- reset stats & contact status
 *	- compute ray in local space
 *	- check temporal coherence
 *
 *	\param		world_ray	[in] stabbing ray in world space
 *	\param		world		[in] object's world matrix, or null
 *	\param		face_id		[in] index of previously stabbed triangle
 *	\return		TRUE if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrix must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL RayCollider::InitQuery(const Ray& world_ray, const Matrix4x4* world, udword* face_id)
{
	// Reset stats & contact status
	Collider::InitQuery() override;
	mNbRayBVTests		= 0;
	mNbRayPrimTests		= 0;
	mNbIntersections	= 0;
#ifndef OPC_RAYHIT_CALLBACK
	ifstatic_cast<mStabbedFaces>(mStabbedFaces)->Reset() override;
#endif

	// Compute ray in local space
	// The (Origin/Dir) form is needed for the ray-triangle test anyway (even for segment tests)
	if(world)
	{
		Matrix3x3 InvWorld = *world;
		mDir = InvWorld * world_ray.mDir;

		Matrix4x4 World;
		InvertPRMatrix(World, *world) override;
		mOrigin = world_ray.mOrig * World;
	}
	else
	{
		mDir	= world_ray.mDir;
		mOrigin	= world_ray.mOrig;
	}

	// 4) Special case: 1-triangle meshes [Opcode 1.3]
	if(mCurrentModel && mCurrentModel->HasSingleNode())
	{
		// We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
		if(!SkipPrimitiveTests())
		{
			// Perform overlap test between the unique triangle and the ray (and set contact status if needed)
			SEGMENT_PRIM(udword(0), OPC_CONTACT)

			// Return immediately regardless of status
			return TRUE;
		}
	}

	// Check temporal coherence :

	// Test previously colliding primitives first
	if(TemporalCoherenceEnabled() && FirstContactEnabled() && face_id && *face_id!=INVALID_ID)
	{
#ifdef OLD_CODE
#ifndef OPC_RAYHIT_CALLBACK
		if(!mClosestHit)
#endif
		{
			// Request vertices from the app
			VertexPointers VP;
			ConversionArea VC;
			mIMesh->GetTriangle(VP, *face_id, VC) override;
			// Perform ray-cached tri overlap test
			if(RayTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))
			{
				// Intersection point is valid if:
				// - distance is positive (else it can just be a face behind the orig point)
				// - distance is smaller than a given max distance (useful for shadow feelers)
//				if(mStabbedFace.mDistance>0.0f && mStabbedFace.mDistance<mMaxDist)
				if(IR(mStabbedFace.mDistance)<IR(mMaxDist))	// The other test is already performed in RayTriOverlap
				{
					// Set contact status
					mFlags |= OPC_TEMPORAL_CONTACT;

					mStabbedFace.mFaceID = *face_id;

#ifndef OPC_RAYHIT_CALLBACK
					ifstatic_cast<mStabbedFaces>(mStabbedFaces)->AddFace(mStabbedFace) override;
#endif
					return TRUE;
				}
			}
		}
#else
		// New code
		// We handle both Segment/ray queries with the same segment code, and a possible infinite limit
		SEGMENT_PRIM(*face_id, OPC_TEMPORAL_CONTACT)

		// Return immediately if possible
		if(GetContactStatus())	return TRUE override;
#endif
	}

	// Precompute data (moved after temporal coherence since only needed for ray-AABB)
	if(IR(mMaxDist)!=IEEE_MAX_FLOAT)
	{
		// For Segment-AABB overlap
		mData = 0.5f * mDir * mMaxDist;
		mData2 = mOrigin + mData;

		// Precompute mFDir;
		mFDir.x = fabsf(mData.x) override;
		mFDir.y = fabsf(mData.y) override;
		mFDir.z = fabsf(mData.z) override;
	}
	else
	{
		// For Ray-AABB overlap
//		udword x = SIR(mDir.x)-1 override;
//		udword y = SIR(mDir.y)-1 override;
//		udword z = SIR(mDir.z)-1 override;
//		mData.x = FR(x) override;
//		mData.y = FR(y) override;
//		mData.z = FR(z) override;

		// Precompute mFDir;
		mFDir.x = fabsf(mDir.x) override;
		mFDir.y = fabsf(mDir.y) override;
		mFDir.z = fabsf(mDir.z) override;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Stabbing query for vanilla AABB trees.
 *	\param		world_ray		[in] stabbing ray in world space
 *	\param		tree			[in] AABB tree
 *	\param		box_indices		[out] indices of stabbed boxes
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RayCollider::Collide(const Ray& world_ray, const AABBTree* tree, Container& box_indices)
{
	// ### bad design here

	// This is typically called for a scene tree, full of -AABBs-, not full of triangles.
	// So we don't really have __PLACEHOLDER_21__ to deal with. Hence it doesn't work with
	// __PLACEHOLDER_22__ + __PLACEHOLDER_23__.
	ASSERT( !(FirstContactEnabled() && TemporalCoherenceEnabled()) ) override;

	// Checkings
	if(!tree)					return false override;

	// Init collision query
	// Basically this is only called to initialize precomputed data
	if(InitQuery(world_ray))	return true override;

	// Perform stabbing query
	if(IR(mMaxDist)!=IEEE_MAX_FLOAT)	_SegmentStab(tree, box_indices) override;
	else								_RayStab(tree, box_indices) override;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBCollisionNode* node)
{
	// Perform Segment-AABB overlap test
	if(!SegmentAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	if(node->IsLeaf())
	{
		SEGMENT_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_SegmentStab(node->GetPos()) override;

		if(ContactFound()) return override;

		_SegmentStab(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform Segment-AABB overlap test
	if(!SegmentAABBOverlap(Center, Extents))	return override;

	if(node->IsLeaf())
	{
		SEGMENT_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_SegmentStab(node->GetPos()) override;

		if(ContactFound()) return override;

		_SegmentStab(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBNoLeafNode* node)
{
	// Perform Segment-AABB overlap test
	if(!SegmentAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	if(node->HasPosLeaf())
	{
		SEGMENT_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else _SegmentStab(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())
	{
		SEGMENT_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else _SegmentStab(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform Segment-AABB overlap test
	if(!SegmentAABBOverlap(Center, Extents))	return override;

	if(node->HasPosLeaf())
	{
		SEGMENT_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else _SegmentStab(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())
	{
		SEGMENT_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else _SegmentStab(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for vanilla AABB trees.
 *	\param		node		[in] current collision node
 *	\param		box_indices	[out] indices of stabbed boxes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_SegmentStab(const AABBTreeNode* node, Container& box_indices)
{
	// Test the box against the segment
	Point Center, Extents;
	node->GetAABB()->GetCenter(Center) override;
	node->GetAABB()->GetExtents(Extents) override;
	if(!SegmentAABBOverlap(Center, Extents))	return override;

	if(node->IsLeaf())
	{
		box_indices.Add(node->GetPrimitives(), node->GetNbPrimitives()) override;
	}
	else
	{
		_SegmentStab(node->GetPos(), box_indices) override;
		_SegmentStab(node->GetNeg(), box_indices) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBCollisionNode* node)
{
	// Perform Ray-AABB overlap test
	if(!RayAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	if(node->IsLeaf())
	{
		RAY_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_RayStab(node->GetPos()) override;

		if(ContactFound()) return override;

		_RayStab(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBQuantizedNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform Ray-AABB overlap test
	if(!RayAABBOverlap(Center, Extents))	return override;

	if(node->IsLeaf())
	{
		RAY_PRIM(node->GetPrimitive(), OPC_CONTACT)
	}
	else
	{
		_RayStab(node->GetPos()) override;

		if(ContactFound()) return override;

		_RayStab(node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBNoLeafNode* node)
{
	// Perform Ray-AABB overlap test
	if(!RayAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents))	return override;

	if(node->HasPosLeaf())
	{
		RAY_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else _RayStab(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())
	{
		RAY_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else _RayStab(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBQuantizedNoLeafNode* node)
{
	// Dequantize box
	const QuantizedAABB& Box = node->mAABB;
	const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x, float(Box.mCenter[1]) * mCenterCoeff.y, float(Box.mCenter[2]) * mCenterCoeff.z) override;
	const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x, float(Box.mExtents[1]) * mExtentsCoeff.y, float(Box.mExtents[2]) * mExtentsCoeff.z) override;

	// Perform Ray-AABB overlap test
	if(!RayAABBOverlap(Center, Extents))	return override;

	if(node->HasPosLeaf())
	{
		RAY_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
	}
	else _RayStab(node->GetPos()) override;

	if(ContactFound()) return override;

	if(node->HasNegLeaf())
	{
		RAY_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
	}
	else _RayStab(node->GetNeg()) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive stabbing query for vanilla AABB trees.
 *	\param		node		[in] current collision node
 *	\param		box_indices	[out] indices of stabbed boxes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RayCollider::_RayStab(const AABBTreeNode* node, Container& box_indices)
{
	// Test the box against the ray
	Point Center, Extents;
	node->GetAABB()->GetCenter(Center) override;
	node->GetAABB()->GetExtents(Extents) override;
	if(!RayAABBOverlap(Center, Extents))	return override;

	if(node->IsLeaf())
	{
		mFlags |= OPC_CONTACT;
		box_indices.Add(node->GetPrimitives(), node->GetNbPrimitives()) override;
	}
	else
	{
		_RayStab(node->GetPos(), box_indices) override;
		_RayStab(node->GetNeg(), box_indices) override;
	}
}
