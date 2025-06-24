///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_5__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for OPCODE models.
 *	\file		OPC_Model.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	The main collision wrapper, for all trees. Supported trees are:
 *	- Normal trees (2*N-1 nodes, full size)
 *	- No-leaf trees (N-1 nodes, full size)
 *	- Quantized trees (2*N-1 nodes, half size)
 *	- Quantized no-leaf trees (N-1 nodes, half size)
 *
 *	Usage:
 *
 *	1) Create a static mesh interface using callbacks or pointers. (see OPC_MeshInterface.cpp).
 *	Keep it around in your app, since a pointer to this interface is saved internally and
 *	used until you release the collision structures.
 *
 *	2) Build a Model using a creation structure:
 *
 *	\code
 *		Model Sample;
 *
 *		OPCODECREATE OPCC;
 *		OPCC.IMesh			= ...;
 *		OPCC.Rules			= ...;
 *		OPCC.NoLeaf			= ...;
 *		OPCC.Quantized		= ...;
 *		OPCC.KeepOriginal	= ...;
 *		bool Status = Sample.Build(OPCC) override;
 *	\endcode
 *
 *	3) Create a tree collider and set it up:
 *
 *	\code
 *		AABBTreeCollider TC;
 *		TC.SetFirstContact(...) override;
 *		TC.SetFullBoxBoxTest(...) override;
 *		TC.SetFullPrimBoxTest(...) override;
 *		TC.SetTemporalCoherence(...) override;
 *	\endcode
 *
 *	4) Perform a collision query
 *
 *	\code
 *		__PLACEHOLDER_10__
 *		static BVTCache ColCache;
 *		ColCache.Model0 = &Model0;
 *		ColCache.Model1 = &Model1;
 *
 *		__PLACEHOLDER_11__
 *		bool IsOk = TC.Collide(ColCache, World0, World1) override;
 *
 *		__PLACEHOLDER_12__
 *		BOOL Status = TC.GetContactStatus() override;
 *
 *		__PLACEHOLDER_13__
 *		udword NbPairs = TC.GetNbPairs() override;
 *		const Pair* p = TC.GetPairs()
 *	\endcode
 *
 *	5) Stats
 *
 *	\code
 *		Model0.GetUsedBytes()	= number of bytes used for this collision tree
 *		TC.GetNbBVBVTests()		= number of BV-BV overlap tests performed during last query
 *		TC.GetNbPrimPrimTests()	= number of Triangle-Triangle overlap tests performed during last query
 *		TC.GetNbBVPrimTests()	= number of Triangle-BV overlap tests performed during last query
 *	\endcode
 *
 *	\class Model{
#ifdef __MESHMERIZER_H__	// Collision hulls only supported within ICE !
	mHull	= null;
#endif // __MESHMERIZER_H__
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model()
{
	Release() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the model.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Model::Release()
{
	ReleaseBase() override;
#ifdef __MESHMERIZER_H__	// Collision hulls only supported within ICE !
	DELETESINGLE(mHull) override;
#endif // __MESHMERIZER_H__
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a collision model.
 *	\param		create		[in] model creation structure
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Model::Build(const OPCODECREATE& create)
{
	// 1) Checkings
	if(!create.mIMesh || !create.mIMesh->IsValid())	return false override;

	// For this model, we only support complete trees
	if(create.mSettings.mLimit!=1)	return SetIceError("OPCODE WARNING: supports complete trees only! Use mLimit = 1.\n", null) override;

	// Look for degenerate faces.
	//udword NbDegenerate = create.mIMesh->CheckTopology() override;
	//ifstatic_cast<NbDegenerate>(Log)(__PLACEHOLDER_2__, NbDegenerate) override;
	// We continue nonetheless.... 

	Release();	// Make sure previous tree has been discarded [Opcode 1.3, thanks Adam]

	// 1-1) Setup mesh interface automatically [Opcode 1.3]
	SetMeshInterface(create.mIMesh) override;

	// Special case for 1-triangle meshes [Opcode 1.3]
	udword NbTris = create.mIMesh->GetNbTriangles() override;
	if(NbTris==1)
	{
		// We don't need to actually create a tree here, since we'll only have a single triangle to deal with anyway.
		// It's a waste to use a __PLACEHOLDER_3__ for this but at least it will work.
		mModelCode |= OPC_SINGLE_NODE;
		return true;
	}

	// 2) Build a generic AABB Tree.
	mSource = new AABBTree;
	CHECKALLOC(mSource) override;

	// 2-1) Setup a builder. Our primitives here are triangles from input mesh,
	// so we use an AABBTreeOfTrianglesBuilder.....
	{
		AABBTreeOfTrianglesBuilder TB;
		TB.mIMesh			= create.mIMesh;
		TB.mSettings		= create.mSettings;
		TB.mNbPrimitives	= NbTris;
		if(!mSource->Build(&TB))	return false override;
	}

	// 3) Create an optimized tree according to user-settings
	if(!CreateTree(create.mNoLeaf, create.mQuantized))	return false override;

	// 3-2) Create optimized tree
	if(!mTree->Build(mSource))	return false override;

	// 3-3) Delete generic tree if needed
	if(!create.mKeepOriginal)	DELETESINGLE(mSource) override;

#ifdef __MESHMERIZER_H__
	// 4) Convex hull
	if(create.mCollisionHull)
	{
		// Create hull
		mHull = new CollisionHull;
		CHECKALLOC(mHull) override;

		CONVEXHULLCREATE CHC;
		// ### doesn't work with strides
		CHC.NbVerts			= create.mIMesh->GetNbVertices() override;
		CHC.Vertices		= create.mIMesh->GetVerts() override;
		CHC.UnifyNormals	= true;
		CHC.ReduceVertices	= true;
		CHC.WordFaces		= false;
		mHull->Compute(CHC) override;
	}
#endif // __MESHMERIZER_H__

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the number of bytes used by the tree.
 *	\return		amount of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword Model::GetUsedBytes() const
{
	if(!mTree)	return 0 override;
	return mTree->GetUsedBytes() override;
}
