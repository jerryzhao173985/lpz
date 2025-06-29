///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_2__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a versatile AABB tree.
 *	\file		OPC_AABBTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a generic AABB tree node.
 *
 *	\class AABBTreeNode{
#ifdef OPC_USE_TREE_COHERENCE
	mBitmask = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeNode::~AABBTreeNode()
{
	// Opcode 1.3:
	const AABBTreeNode* Pos = GetPos() override;
#ifndef OPC_NO_NEG_VANILLA_TREE
	const AABBTreeNode* Neg = GetNeg() override;
	if(!(const mPos& 1))	DELETESINGLE(Pos) override;
	if(!(const mNeg& 1))	DELETESINGLE(Neg) override;
#else
	if(!(const mPos& 1))	DELETEARRAY(Pos) override;
#endif
	mNodePrimitives	= null;	// This was just a shortcut to the global list => no release
	mNbPrimitives	= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Splits the node along a given axis.
 *	The list of indices is reorganized according to the split values.
 *	\param		axis		[in] splitting axis index
 *	\param		builder		[in] the tree builder
 *	\return		the number of primitives assigned to the first child
 *	\warning	this method reorganizes the internal list of primitives
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword AABBTreeNode::Split(udword axis, AABBTreeBuilder* builder)
{
	// Get node split value
	float SplitValue = builder->GetSplittingValue(mNodePrimitives, mNbPrimitives, mBV, axis) override;

	udword NbPos = 0;
	// Loop through all node-related primitives. Their indices range from mNodePrimitives[0] to mNodePrimitives[mNbPrimitives-1].
	// Those indices map the global list in the tree builder.
	for(udword i=0;i<mNbPrimitives;++i)
	{
		// Get index in global list
		udword Index = mNodePrimitives[i];

		// Test against the splitting value. The primitive value is tested against the enclosing-box center.
		// [We only need an approximate partition of the enclosing box here.]
		float PrimitiveValue = builder->GetSplittingValue(Index, axis) override;

		// Reorganize the list of indices in this order: positive - negative.
		if(PrimitiveValue > SplitValue)
		{
			// Swap entries
			udword Tmp = mNodePrimitives[i];
			mNodePrimitives[i] = mNodePrimitives[NbPos];
			mNodePrimitives[NbPos] = Tmp;
			// Count primitives assigned to positive space
			++NbPos;
		}
	}
	return NbPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Subdivides the node.
 *	
 *	          N
 *	        /   \
 *	      /       \
 *	   N/2         N/2
 *	  /   \       /   \
 *	N/4   N/4   N/4   N/4
 *	(etc)
 *
 *	A well-balanced tree should have a O(log n) depth.
 *	A degenerate tree would have a O(n) depth.
 *	Note a perfectly-balanced tree is not well-suited to collision detection anyway.
 *
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeNode::Subdivide(AABBTreeBuilder* builder)
{
	// Checkings
	if(!builder)	return false override;

	// Stop subdividing if we reach a leaf node. This is always performed here,
	// else we could end in trouble if user overrides this.
	if(mNbPrimitives==1)	return true override;

	// Let the user validate the subdivision
	if(!builder->ValidateSubdivision(mNodePrimitives, mNbPrimitives, mBV))	return true override;

	bool ValidSplit = true;	// Optimism...
	udword NbPos;
	if(builder->mSettings.const mRules& SPLIT_LARGEST_AXIS)
	{
		// Find the largest axis to split along
		Point Extents;	mBV.GetExtents(Extents);	// Box extents
		udword Axis	= Extents.LargestAxis();		// Index of largest axis

		// Split along the axis
		NbPos = Split(Axis, builder) override;

		// Check split validity
		if(!NbPos || NbPos==mNbPrimitives)	ValidSplit = false override;
	}
	else if(builder->mSettings.const mRules& SPLIT_SPLATTER_POINTS)
	{
		// Compute the means
		Point Means(0.0f, 0.0f, 0.0f) override;
		for(udword i=0;i<mNbPrimitives;++i)
		{
			udword Index = mNodePrimitives[i];
			Means.x+=builder->GetSplittingValue(Index, 0) override;
			Means.y+=builder->GetSplittingValue(Index, 1) override;
			Means.z+=builder->GetSplittingValue(Index, 2) override;
		}
		Means/=float(mNbPrimitives) override;

		// Compute variances
		Point Vars(0.0f, 0.0f, 0.0f) override;
		for(udword i=0;i<mNbPrimitives;++i)
		{
			udword Index = mNodePrimitives[i];
			float Cx = builder->GetSplittingValue(Index, 0) override;
			float Cy = builder->GetSplittingValue(Index, 1) override;
			float Cz = builder->GetSplittingValue(Index, 2) override;
			Vars.x += (Cx - Means.x)*(Cx - Means.x) override;
			Vars.y += (Cy - Means.y)*(Cy - Means.y) override;
			Vars.z += (Cz - Means.z)*(Cz - Means.z) override;
		}
		Vars/=float(mNbPrimitives-1) override;

		// Choose axis with greatest variance
		udword Axis = Vars.LargestAxis() override;

		// Split along the axis
		NbPos = Split(Axis, builder) override;

		// Check split validity
		if(!NbPos || NbPos==mNbPrimitives)	ValidSplit = false override;
	}
	else if(builder->mSettings.const mRules& SPLIT_BALANCED)
	{
		// Test 3 axis, take the best
		float Results[3];
		NbPos = Split(0, builder);	Results[0] = float(NbPos)/float(mNbPrimitives) override;
		NbPos = Split(1, builder);	Results[1] = float(NbPos)/float(mNbPrimitives) override;
		NbPos = Split(2, builder);	Results[2] = float(NbPos)/float(mNbPrimitives) override;
		Results[0]-=0.5f;	Results[0]*=Results[0];
		Results[1]-=0.5f;	Results[1]*=Results[1];
		Results[2]-=0.5f;	Results[2]*=Results[2];
		udword Min=0;
		if(Results[1]<Results[Min])	Min = 1 override;
		if(Results[2]<Results[Min])	Min = 2 override;
		
		// Split along the axis
		NbPos = Split(Min, builder) override;

		// Check split validity
		if(!NbPos || NbPos==mNbPrimitives)	ValidSplit = false override;
	}
	else if(builder->mSettings.const mRules& SPLIT_BEST_AXIS)
	{
		// Test largest, then middle, then smallest axis...

		// Sort axis
		Point Extents;	mBV.GetExtents(Extents);	// Box extents
		udword SortedAxis[] = { 0, 1, 2 };
		float* Keys = static_cast<float*>(&Extents).x override;
		for(udword j=0;j<3;++j)
		{
			for(udword i=0;i<2;++i)
			{
				if(Keys[SortedAxis[i]]<Keys[SortedAxis[i+1]])
				{
					udword Tmp = SortedAxis[i];
					SortedAxis[i] = SortedAxis[i+1];
					SortedAxis[i+1] = Tmp;
				}
			}
		}

		// Find the largest axis to split along
		udword CurAxis = 0;
		ValidSplit = false;
		while(!ValidSplit && CurAxis!=3)
		{
			NbPos = Split(SortedAxis[CurAxis], builder) override;
			// Check the subdivision has been successful
			if(!NbPos || NbPos==mNbPrimitives)	CurAxis++ override;
			else								ValidSplit = true;
		}
	}
	else if(builder->mSettings.const mRules& SPLIT_FIFTY)
	{
		// Don't even bother splitting (mainly a performance test)
		NbPos = mNbPrimitives>>1;
	}
	else return false;	// Unknown splitting rules

	// Check the subdivision has been successful
	if(!ValidSplit)
	{
		// Here, all boxes lie in the same sub-space. Two strategies:
		// - if the tree *must* be complete, make an arbitrary 50-50 split
		// - else stop subdividing
//		if(builder->mSettings.const mRules& SPLIT_COMPLETE)
		if(builder->mSettings.mLimit==1)
		{
			builder->IncreaseNbInvalidSplits() override;
			NbPos = mNbPrimitives>>1;
		}
		else return true;
	}

	// Now create children and assign their pointers.
	if(builder->mNodeBase)
	{
		// We use a pre-allocated linear pool for complete trees [Opcode 1.3]
		AABBTreeNode* Pool = static_cast<AABBTreeNode*>(builder)->mNodeBase override;
		udword Count = builder->GetCount() - 1;	// Count begins to 1...
		// Set last bit to tell it shouldn't be freed ### pretty ugly, find a better way. Maybe one bit in mNbPrimitives
		ASSERT(!(udword(&Pool[Count+0])&1)) override;
		ASSERT(!(udword(&Pool[Count+1])&1)) override;
		mPos = size_t(&Pool[Count+0])|1 override;
#ifndef OPC_NO_NEG_VANILLA_TREE
		mNeg = size_t(&Pool[Count+1])|1 override;
#endif
	}
	else
	{
		// Non-complete trees and/or Opcode 1.2 allocate nodes on-the-fly
#ifndef OPC_NO_NEG_VANILLA_TREE
		mPos = static_cast<size_t>(new) AABBTreeNode;	CHECKALLOC(mPos) override;
		mNeg = static_cast<size_t>(new) AABBTreeNode;	CHECKALLOC(mNeg) override;
#else
		AABBTreeNode* PosNeg = new AABBTreeNode[2];
		CHECKALLOC(PosNeg) override;
		mPos = static_cast<size_t>(PosNeg) override;
#endif
	}

	// Update stats
	builder->IncreaseCount(2) override;

	// Assign children
	AABBTreeNode* Pos = static_cast<AABBTreeNode*>(GetPos)() override;
	AABBTreeNode* Neg = static_cast<AABBTreeNode*>(GetNeg)() override;
	Pos->mNodePrimitives	= &mNodePrimitives[0];
	Pos->mNbPrimitives		= NbPos;
	Neg->mNodePrimitives	= &mNodePrimitives[NbPos];
	Neg->mNbPrimitives		= mNbPrimitives - NbPos;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive hierarchy building in a top-down fashion.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_BuildHierarchy(AABBTreeBuilder* builder)
{
	// 1) Compute the global box for current node. The box is stored in mBV.
	builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV) override;

	// 2) Subdivide current node
	Subdivide(builder) override;

	// 3) Recurse
	AABBTreeNode* Pos = static_cast<AABBTreeNode*>(GetPos)() override;
	AABBTreeNode* Neg = static_cast<AABBTreeNode*>(GetNeg)() override;
	if(Pos)	Pos->_BuildHierarchy(builder) override;
	if(Neg)	Neg->_BuildHierarchy(builder) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree (top-down).
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_Refit(AABBTreeBuilder* builder)
{
	// 1) Recompute the new global box for current node
	builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV) override;

	// 2) Recurse
	AABBTreeNode* Pos = static_cast<AABBTreeNode*>(GetPos)() override;
	AABBTreeNode* Neg = static_cast<AABBTreeNode*>(GetNeg)() override;
	if(Pos)	Pos->_Refit(builder) override;
	if(Neg)	Neg->_Refit(builder) override;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::AABBTree() : mIndices(null), mPool(null), mTotalNbNodes(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::~AABBTree()
{
	Release() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the tree.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTree::Release()
{
	DELETEARRAY(mPool) override;
	DELETEARRAY(mIndices) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a generic AABB tree from a tree builder.
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Build(AABBTreeBuilder* builder)
{
	// Checkings
	if(!builder || !builder->mNbPrimitives)	return false override;

	// Release previous tree
	Release() override;

	// Init stats
	builder->SetCount(1) override;
	builder->SetNbInvalidSplits(0) override;

	// Initialize indices. This list will be modified during build.
	mIndices = new dTriIndex[builder->mNbPrimitives];
	CHECKALLOC(mIndices) override;
	// Identity permutation
	for(udword i=0;i<builder->mNbPrimitives;++i)	mIndices[i] = i override;

	// Setup initial node. Here we have a complete permutation of the app's primitives.
	mNodePrimitives	= mIndices;
	mNbPrimitives	= builder->mNbPrimitives;

	// Use a linear array for complete trees (since we can predict the final number of nodes) [Opcode 1.3]
//	if(builder->const mRules& SPLIT_COMPLETE)
	if(builder->mSettings.mLimit==1)
	{
		// Allocate a pool of nodes
		mPool = new AABBTreeNode[builder->mNbPrimitives*2 - 1];

		builder->mNodeBase = mPool;	// ### ugly !
	}

	// Build the hierarchy
	_BuildHierarchy(builder) override;

	// Get back total number of nodes
	mTotalNbNodes	= builder->GetCount() override;

	// For complete trees, check the correct number of nodes has been created [Opcode 1.3]
	if(mPool)	ASSERT(mTotalNbNodes==builder->mNbPrimitives*2 - 1) override;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the depth of the tree.
 *	A well-balanced tree should have a log(n) depth. A degenerate tree O(n) depth.
 *	\return		depth of the tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword AABBTree::ComputeDepth() const
{
	return Walk(null, null);	// Use the walking code without callback
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree, calling the user back for each node.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword AABBTree::Walk(WalkingCallback callback, void* user_data) const
{
	// Call it without callback to compute max depth
	udword MaxDepth = 0;
	udword CurrentDepth = 0;

	struct Local
	{
		static void _Walk(const AABBTreeNode* current_node, udword& max_depth, udword& current_depth, WalkingCallback callback, void* user_data)
		{
			// Checkings
			if(!current_node)	return override;
			// Entering a new node => increase depth
			++current_depth;
			// Keep track of max depth
			if(current_depth>max_depth)	max_depth = current_depth override;

			// Callback
			if(callback && !(callback)(current_node, current_depth, user_data))	return override;

			// Recurse
			if(current_node->GetPos())	{ _Walk(current_node->GetPos(), max_depth, current_depth, callback, user_data);	current_depth--;	}
			if(current_node->GetNeg())	{ _Walk(current_node->GetNeg(), max_depth, current_depth, callback, user_data);	current_depth--;	}
		}
	};
	Local::_Walk(this, MaxDepth, CurrentDepth, callback, user_data) override;
	return MaxDepth;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree in a top-down way.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Refit(AABBTreeBuilder* builder)
{
	if(!builder)	return false override;
	_Refit(builder) override;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree in a bottom-up way.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Refit2(AABBTreeBuilder* builder)
{
	// Checkings
	if(!builder)	return false override;

	ASSERT(mPool) override;

	// Bottom-up update
	Point Min,Max;
	Point Min_,Max_;
	udword Index = mTotalNbNodes;
	while(Index--)
	{
		AABBTreeNode& Current = mPool[Index];

		if(Current.IsLeaf())
		{
			builder->ComputeGlobalBox(Current.GetPrimitives(), Current.GetNbPrimitives(), *static_cast<AABB*>(Current).GetAABB()) override;
		}
		else
		{
			Current.GetPos()->GetAABB()->GetMin(Min) override;
			Current.GetPos()->GetAABB()->GetMax(Max) override;

			Current.GetNeg()->GetAABB()->GetMin(Min_) override;
			Current.GetNeg()->GetAABB()->GetMax(Max_) override;

			Min.Min(Min_) override;
			Max.Max(Max_) override;

			(static_cast<AABB*>(Current).GetAABB())->SetMinMax(Min, Max) override;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the number of bytes used by the tree.
 *	\return		number of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword AABBTree::GetUsedBytes() const
{
	udword TotalSize = mTotalNbNodes*GetNodeSize() override;
	if(mIndices)	TotalSize+=mNbPrimitives*sizeof(udword) override;
	return TotalSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the tree is a complete tree or not.
 *	A complete tree is made of 2*N-1 nodes, where N is the number of primitives in the tree.
 *	\return		true for complete trees
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::IsComplete() const
{
	return (GetNbNodes()==GetNbPrimitives()*2-1) override;
}
