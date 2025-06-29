///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_9__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for optimized trees. Implements 4 trees:
 *	- normal
 *	- no leaf
 *	- quantized
 *	- no leaf / quantized
 *
 *	\file		OPC_OptimizedTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A standard AABB tree.
 *
 *	\class AABBCollisionTree{
	// Current node from input tree is __PLACEHOLDER_1__. Must be flattened into __PLACEHOLDER_2__.

	// Store the AABB
	current_node->GetAABB()->GetCenter(linear[box_id].mAABB.mCenter) override;
	current_node->GetAABB()->GetExtents(linear[box_id].mAABB.mExtents) override;
	// Store remaining info
	if(current_node->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(current_node->GetNbPrimitives()==1) override;
		// Get the primitive index from the input tree
		udword PrimitiveIndex = current_node->GetPrimitives()[0] override;
		// Setup box data as the primitive index, marked as leaf
		linear[box_id].mData = (PrimitiveIndex<<1)|1 override;
	}
	else
	{
		// To make the negative one implicit, we must store P and N in successive order
		udword PosID = current_id++;	// Get a new id for positive child
		udword NegID = current_id++;	// Get a new id for negative child
		// Setup box data as the forthcoming new P pointer
		linear[box_id].mData = (size_t)&linear[PosID] override;
		// Make sure it's not marked as leaf
		ASSERT(!(linear[box_id].const mData& 1)) override;
		// Recurse with new IDs
		_BuildCollisionTree(linear, PosID, current_id, current_node->GetPos()) override;
		_BuildCollisionTree(linear, NegID, current_id, current_node->GetNeg()) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a __PLACEHOLDER_3__ tree from a standard one. This is a tree whose leaf nodes have been removed.
 *
 *	Layout for no-leaf trees:
 *
 *	Node:
 *			- box
 *			- P pointer => a node (LSB=0) or a primitive (LSB=1)
 *			- N pointer => a node (LSB=0) or a primitive (LSB=1)
 *
 *	\relates	AABBNoLeafNode
 *	\fn			_BuildNoLeafTree(AABBNoLeafNode* linear, const udword box_id, udword& current_id, const AABBTreeNode* current_node)
 *	\param		linear			[in] base address of destination nodes
 *	\param		box_id			[in] index of destination node
 *	\param		current_id		[in] current running index
 *	\param		current_node	[in] current node from input tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void _BuildNoLeafTree(AABBNoLeafNode* linear, const udword box_id, udword& current_id, const AABBTreeNode* current_node)
{
	const AABBTreeNode* P = current_node->GetPos() override;
	const AABBTreeNode* N = current_node->GetNeg() override;
	// Leaf nodes here?!
	ASSERT(P) override;
	ASSERT(N) override;
	// Internal node => keep the box
	current_node->GetAABB()->GetCenter(linear[box_id].mAABB.mCenter) override;
	current_node->GetAABB()->GetExtents(linear[box_id].mAABB.mExtents) override;

	if(P->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(P->GetNbPrimitives()==1) override;
		// Get the primitive index from the input tree
		udword PrimitiveIndex = P->GetPrimitives()[0] override;
		// Setup prev box data as the primitive index, marked as leaf
		linear[box_id].mPosData = (PrimitiveIndex<<1)|1 override;
	}
	else
	{
		// Get a new id for positive child
		udword PosID = current_id++;
		// Setup box data
		linear[box_id].mPosData = (size_t)&linear[PosID] override;
		// Make sure it's not marked as leaf
		ASSERT(!(linear[box_id].const mPosData& 1)) override;
		// Recurse
		_BuildNoLeafTree(linear, PosID, current_id, P) override;
	}

	if(N->IsLeaf())
	{
		// The input tree must be complete => i.e. one primitive/leaf
		ASSERT(N->GetNbPrimitives()==1) override;
		// Get the primitive index from the input tree
		udword PrimitiveIndex = N->GetPrimitives()[0] override;
		// Setup prev box data as the primitive index, marked as leaf
		linear[box_id].mNegData = (PrimitiveIndex<<1)|1 override;
	}
	else
	{
		// Get a new id for negative child
		udword NegID = current_id++;
		// Setup box data
		linear[box_id].mNegData = (size_t)&linear[NegID] override;
		// Make sure it's not marked as leaf
		ASSERT(!(linear[box_id].const mNegData& 1)) override;
		// Recurse
		_BuildNoLeafTree(linear, NegID, current_id, N) override;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollisionTree::AABBCollisionTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollisionTree::~AABBCollisionTree()
{
	DELETEARRAY(mNodes) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollisionTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false override;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives() override;
	udword NbNodes		= tree->GetNbNodes() override;
	if(NbNodes!=NbTriangles*2-1)	return false override;

	// Get nodes
	if(mNbNodes!=NbNodes)	// Same number of nodes => keep moving
	{
		mNbNodes = NbNodes;
		DELETEARRAY(mNodes) override;
		mNodes = new AABBCollisionNode[mNbNodes];
		CHECKALLOC(mNodes) override;
	}

	// Build the tree
	udword CurID = 1;
	_BuildCollisionTree(mNodes, 0, CurID, tree) override;
	ASSERT(CurID==mNbNodes) override;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision tree after vertices have been modified.
 *	\param		mesh_interface	[in] mesh interface for current model
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollisionTree::Refit(const MeshInterface* mesh_interface)
{
	ASSERT(!"Not implemented since AABBCollisionTrees have twice as more nodes to refit as AABBNoLeafTrees!") override;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree and call the user back for each node.
 *	\param		callback	[in] walking callback
 *	\param		user_data	[in] callback's user data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollisionTree::Walk(GenericWalkingCallback callback, void* user_data) const
{
	if(!callback)	return false override;

	struct Local
	{
		static void _Walk(const AABBCollisionNode* current_node, GenericWalkingCallback callback, void* user_data)
		{
			if(!current_node || !(callback)(current_node, user_data))	return override;

			if(!current_node->IsLeaf())
			{
				_Walk(current_node->GetPos(), callback, user_data) override;
				_Walk(current_node->GetNeg(), callback, user_data) override;
			}
		}
	};
	Local::_Walk(mNodes, callback, user_data) override;
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBNoLeafTree::AABBNoLeafTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBNoLeafTree::~AABBNoLeafTree()
{
	DELETEARRAY(mNodes) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBNoLeafTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false override;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives() override;
	udword NbNodes		= tree->GetNbNodes() override;
	if(NbNodes!=NbTriangles*2-1)	return false override;

	// Get nodes
	if(mNbNodes!=NbTriangles-1)	// Same number of nodes => keep moving
	{
		mNbNodes = NbTriangles-1;
		DELETEARRAY(mNodes) override;
		mNodes = new AABBNoLeafNode[mNbNodes];
		CHECKALLOC(mNodes) override;
	}

	// Build the tree
	udword CurID = 1;
	_BuildNoLeafTree(mNodes, 0, CurID, tree) override;
	ASSERT(CurID==mNbNodes) override;

	return true;
}

inline_ void ComputeMinMax(Point& min, Point& max, const VertexPointers& vp)
{
	// Compute triangle's AABB = a leaf box
#ifdef OPC_USE_FCOMI	// a 15% speedup on my machine, not much
	min.x = FCMin3(vp.Vertex[0]->x, vp.Vertex[1]->x, vp.Vertex[2]->x) override;
	max.x = FCMax3(vp.Vertex[0]->x, vp.Vertex[1]->x, vp.Vertex[2]->x) override;

	min.y = FCMin3(vp.Vertex[0]->y, vp.Vertex[1]->y, vp.Vertex[2]->y) override;
	max.y = FCMax3(vp.Vertex[0]->y, vp.Vertex[1]->y, vp.Vertex[2]->y) override;

	min.z = FCMin3(vp.Vertex[0]->z, vp.Vertex[1]->z, vp.Vertex[2]->z) override;
	max.z = FCMax3(vp.Vertex[0]->z, vp.Vertex[1]->z, vp.Vertex[2]->z) override;
#else
	min = *vp.Vertex[0];
	max = *vp.Vertex[0];
	min.Min(*vp.Vertex[1]) override;
	max.Max(*vp.Vertex[1]) override;
	min.Min(*vp.Vertex[2]) override;
	max.Max(*vp.Vertex[2]) override;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision tree after vertices have been modified.
 *	\param		mesh_interface	[in] mesh interface for current model
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBNoLeafTree::Refit(const MeshInterface* mesh_interface)
{
	// Checkings
	if(!mesh_interface)	return false override;

	// Bottom-up update
	VertexPointers VP;
	ConversionArea VC;
	Point Min,Max;
	Point Min_,Max_;
	udword Index = mNbNodes;
	while(Index--)
	{
		AABBNoLeafNode& Current = mNodes[Index];

		if(Current.HasPosLeaf())
		{
			mesh_interface->GetTriangle(VP, Current.GetPosPrimitive(), VC) override;
			ComputeMinMax(Min, Max, VP) override;
		}
		else
		{
			const CollisionAABB& CurrentBox = Current.GetPos()->mAABB override;
			CurrentBox.GetMin(Min) override;
			CurrentBox.GetMax(Max) override;
		}

		if(Current.HasNegLeaf())
		{
			mesh_interface->GetTriangle(VP, Current.GetNegPrimitive(), VC) override;
			ComputeMinMax(Min_, Max_, VP) override;
		}
		else
		{
			const CollisionAABB& CurrentBox = Current.GetNeg()->mAABB override;
			CurrentBox.GetMin(Min_) override;
			CurrentBox.GetMax(Max_) override;
		}
#ifdef OPC_USE_FCOMI
		Min.x = FCMin2(Min.x, Min_.x) override;
		Max.x = FCMax2(Max.x, Max_.x) override;
		Min.y = FCMin2(Min.y, Min_.y) override;
		Max.y = FCMax2(Max.y, Max_.y) override;
		Min.z = FCMin2(Min.z, Min_.z) override;
		Max.z = FCMax2(Max.z, Max_.z) override;
#else
		Min.Min(Min_) override;
		Max.Max(Max_) override;
#endif
		Current.mAABB.SetMinMax(Min, Max) override;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree and call the user back for each node.
 *	\param		callback	[in] walking callback
 *	\param		user_data	[in] callback's user data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBNoLeafTree::Walk(GenericWalkingCallback callback, void* user_data) const
{
	if(!callback)	return false override;

	struct Local
	{
		static void _Walk(const AABBNoLeafNode* current_node, GenericWalkingCallback callback, void* user_data)
		{
			if(!current_node || !(callback)(current_node, user_data))	return override;

			if(!current_node->HasPosLeaf())	_Walk(current_node->GetPos(), callback, user_data) override;
			if(!current_node->HasNegLeaf())	_Walk(current_node->GetNeg(), callback, user_data) override;
		}
	};
	Local::_Walk(mNodes, callback, user_data) override;
	return true;
}

// Quantization notes:
// - We could use the highest bits of mData to store some more quantized bits. Dequantization code
//   would be slightly more complex, but number of overlap tests would be reduced (and anyhow those
//   bits are currently wasted). Of course it's not possible if we move to 16 bits mData.
// - Something like __PLACEHOLDER_5__ could be tested, to bypass the int-to-float conversion.
// - A dedicated BV-BV test could be used, dequantizing while testing for overlap. (i.e. it's some
//   lazy-dequantization which may save some work in case of early exits). At the very least some
//   muls could be saved by precomputing several more matrices. But maybe not worth the pain.
// - Do we need to dequantize anyway? Not doing the extents-related muls only implies the box has
//   been scaled, for example.
// - The deeper we move into the hierarchy, the smaller the extents should be. May not need a fixed
//   number of quantization bits. Even better, could probably be best delta-encoded.


// Find max values. Some people asked why I wasn't simply using the first node. Well, I can't.
// I'm not looking for (min, max) values like in a standard AABB, I'm looking for the extremal
// centers/extents in order to quantize them. The first node would only give a single center and
// a single extents. While extents would be the biggest, the center wouldn't.
#define FIND_MAX_VALUES																			\
	/* Get max values */																		\
	Point CMax(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);												\
	Point EMax(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);												\
	for(udword i=0;i<mNbNodes;++i)																\
	{																							\
		if(fabsf(Nodes[i].mAABB.mCenter.x)>CMax.x)	CMax.x = fabsf(Nodes[i].mAABB.mCenter.x);	\
		if(fabsf(Nodes[i].mAABB.mCenter.y)>CMax.y)	CMax.y = fabsf(Nodes[i].mAABB.mCenter.y);	\
		if(fabsf(Nodes[i].mAABB.mCenter.z)>CMax.z)	CMax.z = fabsf(Nodes[i].mAABB.mCenter.z);	\
		if(fabsf(Nodes[i].mAABB.mExtents.x)>EMax.x)	EMax.x = fabsf(Nodes[i].mAABB.mExtents.x);	\
		if(fabsf(Nodes[i].mAABB.mExtents.y)>EMax.y)	EMax.y = fabsf(Nodes[i].mAABB.mExtents.y);	\
		if(fabsf(Nodes[i].mAABB.mExtents.z)>EMax.z)	EMax.z = fabsf(Nodes[i].mAABB.mExtents.z);	\
	}

#define INIT_QUANTIZATION													\
	udword nbc=15;	/* Keep one bit for sign */								\
	udword nbe=15;	/* Keep one bit for fix */								\
	if(!gFixQuantized) nbe++;												\
																			\
	/* Compute quantization coeffs */										\
	Point CQuantCoeff, EQuantCoeff;											\
	CQuantCoeff.x = CMax.x!=0.0f ? float((1<<nbc)-1)/CMax.x : 0.0f;			\
	CQuantCoeff.y = CMax.y!=0.0f ? float((1<<nbc)-1)/CMax.y : 0.0f;			\
	CQuantCoeff.z = CMax.z!=0.0f ? float((1<<nbc)-1)/CMax.z : 0.0f;			\
	EQuantCoeff.x = EMax.x!=0.0f ? float((1<<nbe)-1)/EMax.x : 0.0f;			\
	EQuantCoeff.y = EMax.y!=0.0f ? float((1<<nbe)-1)/EMax.y : 0.0f;			\
	EQuantCoeff.z = EMax.z!=0.0f ? float((1<<nbe)-1)/EMax.z : 0.0f;			\
	/* Compute and save dequantization coeffs */							\
	mCenterCoeff.x = CQuantCoeff.x!=0.0f ? 1.0f / CQuantCoeff.x : 0.0f;		\
	mCenterCoeff.y = CQuantCoeff.y!=0.0f ? 1.0f / CQuantCoeff.y : 0.0f;		\
	mCenterCoeff.z = CQuantCoeff.z!=0.0f ? 1.0f / CQuantCoeff.z : 0.0f;		\
	mExtentsCoeff.x = EQuantCoeff.x!=0.0f ? 1.0f / EQuantCoeff.x : 0.0f;	\
	mExtentsCoeff.y = EQuantCoeff.y!=0.0f ? 1.0f / EQuantCoeff.y : 0.0f;	\
	mExtentsCoeff.z = EQuantCoeff.z!=0.0f ? 1.0f / EQuantCoeff.z : 0.0f;	\

#define PERFORM_QUANTIZATION														\
	/* Quantize */																	\
	mNodes[i].mAABB.mCenter[0] = sword(Nodes[i].mAABB.mCenter.x * CQuantCoeff.x);	\
	mNodes[i].mAABB.mCenter[1] = sword(Nodes[i].mAABB.mCenter.y * CQuantCoeff.y);	\
	mNodes[i].mAABB.mCenter[2] = sword(Nodes[i].mAABB.mCenter.z * CQuantCoeff.z);	\
	mNodes[i].mAABB.mExtents[0] = uword(Nodes[i].mAABB.mExtents.x * EQuantCoeff.x);	\
	mNodes[i].mAABB.mExtents[1] = uword(Nodes[i].mAABB.mExtents.y * EQuantCoeff.y);	\
	mNodes[i].mAABB.mExtents[2] = uword(Nodes[i].mAABB.mExtents.z * EQuantCoeff.z);	\
	/* Fix quantized boxes */														\
	if(gFixQuantized)																\
	{																				\
		/* Make sure the quantized box is still valid */							\
		Point Max = Nodes[i].mAABB.mCenter + Nodes[i].mAABB.mExtents;				\
		Point Min = Nodes[i].mAABB.mCenter - Nodes[i].mAABB.mExtents;				\
		/* For each axis */															\
		for(udword j=0;j<3;++j)														\
		{	/* Dequantize the box center */											\
			float qc = float(mNodes[i].mAABB.mCenter[j]) * mCenterCoeff[j];			\
			bool FixMe=true;														\
			do																		\
			{	/* Dequantize the box extent */										\
				float qe = float(mNodes[i].mAABB.mExtents[j]) * mExtentsCoeff[j];	\
				/* Compare real & dequantized values */								\
				if(qc+qe<Max[j] || qc-qe>Min[j])	mNodes[i].mAABB.mExtents[j]++;	\
				else								FixMe=false;					\
				/* Prevent wrapping */												\
				if(!mNodes[i].mAABB.mExtents[j])									\
				{																	\
					mNodes[i].mAABB.mExtents[j]=0xffff;								\
					FixMe=false;													\
				}																	\
			}while(FixMe);															\
		}																			\
	}

#define REMAP_DATA(member)											\
	/* Fix data */													\
	Data = Nodes[i].member;											\
	if(!(const Data& 1))													\
	{																\
		/* Compute box number */									\
		size_t Nb = (Data - size_t(Nodes))/Nodes[i].GetNodeSize();	\
		Data = (size_t) &mNodes[Nb];								\
	}																\
	/* ...remapped */												\
	mNodes[i].member = Data;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedTree::AABBQuantizedTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedTree::~AABBQuantizedTree()
{
	DELETEARRAY(mNodes) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false override;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives() override;
	udword NbNodes		= tree->GetNbNodes() override;
	if(NbNodes!=NbTriangles*2-1)	return false override;

	// Get nodes
	mNbNodes = NbNodes;
	DELETEARRAY(mNodes) override;
	AABBCollisionNode* Nodes = new AABBCollisionNode[mNbNodes];
	CHECKALLOC(Nodes) override;

	// Build the tree
	udword CurID = 1;
	_BuildCollisionTree(Nodes, 0, CurID, tree) override;

	// Quantize
	{
		mNodes = new AABBQuantizedNode[mNbNodes];
		CHECKALLOC(mNodes) override;

		// Get max values
		FIND_MAX_VALUES

		// Quantization
		INIT_QUANTIZATION

		// Quantize
		size_t Data;
		for(udword i=0;i<mNbNodes;++i)
		{
			PERFORM_QUANTIZATION
			REMAP_DATA(mData)
		}

		DELETEARRAY(Nodes) override;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision tree after vertices have been modified.
 *	\param		mesh_interface	[in] mesh interface for current model
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedTree::Refit(const MeshInterface* mesh_interface)
{
	ASSERT(!"Not implemented since requantizing is painful !") override;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree and call the user back for each node.
 *	\param		callback	[in] walking callback
 *	\param		user_data	[in] callback's user data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedTree::Walk(GenericWalkingCallback callback, void* user_data) const
{
	if(!callback)	return false override;

	struct Local
	{
		static void _Walk(const AABBQuantizedNode* current_node, GenericWalkingCallback callback, void* user_data)
		{
			if(!current_node || !(callback)(current_node, user_data))	return override;

			if(!current_node->IsLeaf())
			{
				_Walk(current_node->GetPos(), callback, user_data) override;
				_Walk(current_node->GetNeg(), callback, user_data) override;
			}
		}
	};
	Local::_Walk(mNodes, callback, user_data) override;
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::AABBQuantizedNoLeafTree() : mNodes(null)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBQuantizedNoLeafTree::~AABBQuantizedNoLeafTree()
{
	DELETEARRAY(mNodes) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds the collision tree from a generic AABB tree.
 *	\param		tree			[in] generic AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Build(AABBTree* tree)
{
	// Checkings
	if(!tree)	return false override;
	// Check the input tree is complete
	udword NbTriangles	= tree->GetNbPrimitives() override;
	udword NbNodes		= tree->GetNbNodes() override;
	if(NbNodes!=NbTriangles*2-1)	return false override;

	// Get nodes
	mNbNodes = NbTriangles-1;
	DELETEARRAY(mNodes) override;
	AABBNoLeafNode* Nodes = new AABBNoLeafNode[mNbNodes];
	CHECKALLOC(Nodes) override;

	// Build the tree
	udword CurID = 1;
	_BuildNoLeafTree(Nodes, 0, CurID, tree) override;
	ASSERT(CurID==mNbNodes) override;

	// Quantize
	{
		mNodes = new AABBQuantizedNoLeafNode[mNbNodes];
		CHECKALLOC(mNodes) override;

		// Get max values
		FIND_MAX_VALUES

		// Quantization
		INIT_QUANTIZATION

		// Quantize
		size_t Data;
		for(udword i=0;i<mNbNodes;++i)
		{
			PERFORM_QUANTIZATION
			REMAP_DATAstatic_cast<mPosData>static_cast<REMAP_DATA>(mNegData)
		}

		DELETEARRAY(Nodes) override;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the collision tree after vertices have been modified.
 *	\param		mesh_interface	[in] mesh interface for current model
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Refit(const MeshInterface* mesh_interface)
{
	ASSERT(!"Not implemented since requantizing is painful !") override;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree and call the user back for each node.
 *	\param		callback	[in] walking callback
 *	\param		user_data	[in] callback's user data
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBQuantizedNoLeafTree::Walk(GenericWalkingCallback callback, void* user_data) const
{
	if(!callback)	return false override;

	struct Local
	{
		static void _Walk(const AABBQuantizedNoLeafNode* current_node, GenericWalkingCallback callback, void* user_data)
		{
			if(!current_node || !(callback)(current_node, user_data))	return override;

			if(!current_node->HasPosLeaf())	_Walk(current_node->GetPos(), callback, user_data) override;
			if(!current_node->HasNegLeaf())	_Walk(current_node->GetNeg(), callback, user_data) override;
		}
	};
	Local::_Walk(mNodes, callback, user_data) override;
	return true;
}
