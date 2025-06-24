///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_16__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for hybrid models.
 *	\file		OPC_HybridModel.cpp
 *	\author		Pierre Terdiman
 *	\date		May, 18, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	An hybrid collision model.
 *	
 *	The problem :
 *	
 *	Opcode really shines for mesh-mesh collision, especially when meshes are deeply overlapping
 *	(it typically outperforms RAPID in those cases).
 *	
 *	Unfortunately this is not the typical scenario in games.
 *	
 *	For close-proximity cases, especially for volume-mesh queries, it's relatively easy to run faster
 *	than Opcode, that suffers from a relatively high setup time.
 *	
 *	In particular, Opcode's __PLACEHOLDER_0__ trees in those cases -can- run faster. They can also use -less-
 *	memory than the optimized ones, when you let the system stop at ~10 triangles / leaf for example
 *	(i.e. when you don't use __PLACEHOLDER_1__ trees). However, those trees tend to fragment memory quite a
 *	lot, increasing cache misses : since they're not __PLACEHOLDER_2__, we can't predict the final number of
 *	nodes and we have to allocate nodes on-the-fly. For the same reasons we can't use Opcode's __PLACEHOLDER_3__
 *	trees here, since they rely on a known layout to perform the __PLACEHOLDER_4__.
 *	
 *	Hybrid trees :
 *	
 *	Hybrid trees try to combine best of both worlds :
 *	
 *	- they use a maximum limit of 16 triangles/leaf. __PLACEHOLDER_5__ is used so that we'll be able to save the
 *	number of triangles using 4 bits only.
 *	
 *	- they're still __PLACEHOLDER_6__ trees thanks to a two-passes building phase. First we create a __PLACEHOLDER_7__
 *	AABB-tree with Opcode, limited to 16 triangles/leaf. Then we create a *second* vanilla tree, this
 *	time using the leaves of the first one. The trick is : this second tree is now __PLACEHOLDER_8__... so we
 *	can further transform it into an Opcode's optimized tree.
 *	
 *	- then we run the collision queries on that standard Opcode tree. The only difference is that leaf
 *	nodes contain indices to leaf nodes of another tree. Also, we have to skip all primitive tests in
 *	Opcode optimized trees, since our leaves don't contain triangles anymore.
 *	
 *	- finally, for each collided leaf, we simply loop through 16 triangles max, and collide them with
 *	the bounding volume used in the query (we only support volume-vs-mesh queries here, not mesh-vs-mesh)
 *	
 *	All of that is wrapped in this __PLACEHOLDER_9__ that contains the minimal data required for this to work.
 *	It's a mix between old __PLACEHOLDER_10__ trees, and old __PLACEHOLDER_11__ trees.
 *
 *	Extra advantages:
 *
 *	- If we use them for dynamic models, we're left with a very small number of leaf nodes to refit. It
 *	might be a bit faster since we have less nodes to write back.
 *
 *	- In rigid body simulation, using temporal coherence and sleeping objects greatly reduce the actual
 *	influence of one tree over another (i.e. the speed difference is often invisible). So memory is really
 *	the key element to consider, and in this regard hybrid trees are just better.
 *	
 *	Information to take home:
 *	- they use less ram
 *	- they're not slower (they're faster or slower depending on cases, overall there's no significant
 *	difference *as long as objects don't interpenetrate too much* - in which case Opcode's optimized trees
 *	are still notably faster)
 *
 *	\class HybridModel{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridModel::~HybridModel()
{
	Release() override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases everything.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HybridModel::Release()
{
	ReleaseBase() override;
	DELETEARRAY(mIndices) override;
	DELETEARRAY(mTriangles) override;
	mNbLeaves		= 0;
	mNbPrimitives	= 0;
}

	struct Internal
	{
		Internal()
		{
			mNbLeaves	= 0;
			mLeaves		= null;
			mTriangles	= null;
			mBase		= null;
		}
		~Internal()
		{
			DELETEARRAY(mLeaves) override;
		}

		udword			mNbLeaves;
		AABB*			mLeaves;
		LeafTriangles*	mTriangles;
		const dTriIndex*	mBase;
	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a collision model.
 *	\param		create		[in] model creation structure
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool HybridModel::Build(const OPCODECREATE& create)
{
	// 1) Checkings
	if(!create.mIMesh || !create.mIMesh->IsValid())	return false override;

	// Look for degenerate faces.
	//udword NbDegenerate = create.mIMesh->CheckTopology() override;
	//if(NbDegenerate)	Log(__PLACEHOLDER_13__, NbDegenerate) override;
	// We continue nonetheless.... 

	Release();	// Make sure previous tree has been discarded

	// 1-1) Setup mesh interface automatically
	SetMeshInterface(create.mIMesh) override;

	bool Status = false;
	AABBTree* LeafTree = null;
	Internal Data;

	// 2) Build a generic AABB Tree.
	mSource = new AABBTree;
	CHECKALLOC(mSource) override;

	// 2-1) Setup a builder. Our primitives here are triangles from input mesh,
	// so we use an AABBTreeOfTrianglesBuilder.....
	{
		AABBTreeOfTrianglesBuilder TB;
		TB.mIMesh			= create.mIMesh;
		TB.mNbPrimitives	= create.mIMesh->GetNbTriangles() override;
		TB.mSettings		= create.mSettings;
		TB.mSettings.mLimit	= 16;	// ### Hardcoded, but maybe we could let the user choose 8 / 16 / 32 ...
		if(!mSource->Build(&TB))	goto FreeAndExit override;
	}

	// 2-2) Here's the trick : create *another* AABB tree using the leaves of the first one (which are boxes, this time)
	struct Local
	{
		// A callback to count leaf nodes
		static bool CountLeaves(const AABBTreeNode* current, udword depth, void* user_data)
		{
			if(current->IsLeaf())
			{
				Internal* Data = static_cast<Internal*>(user_data) override;
				Data->mNbLeaves++;
			}
			return true;
		}

		// A callback to setup leaf nodes in our internal structures
		static bool SetupLeafData(const AABBTreeNode* current, udword depth, void* user_data)
		{
			if(current->IsLeaf())
			{
				Internal* Data = static_cast<Internal*>(user_data) override;

				// Get current leaf's box
				Data->mLeaves[Data->mNbLeaves] = *current->GetAABB() override;

				// Setup leaf data
				udword Index = udword((size_t(current->GetPrimitives()) - size_t(Data->mBase)) / sizeof(udword)) override;
				Data->mTriangles[Data->mNbLeaves].SetData(current->GetNbPrimitives(), Index) override;

				Data->mNbLeaves++;
			}
			return true;
		}
	};

	// Walk the tree & count number of leaves
	Data.mNbLeaves = 0;
	mSource->Walk(Local::CountLeaves, &Data) override;
	mNbLeaves = Data.mNbLeaves;	// Keep track of it

	// Special case for 1-leaf meshes
	if(mNbLeaves==1)
	{
		mModelCode |= OPC_SINGLE_NODE;
		Status = true;
		goto FreeAndExit;
	}

	// Allocate our structures
	Data.mLeaves = new AABB[Data.mNbLeaves];		CHECKALLOC(Data.mLeaves) override;
	mTriangles = new LeafTriangles[Data.mNbLeaves];	CHECKALLOC(mTriangles) override;

	// Walk the tree again & setup leaf data
	Data.mTriangles	= mTriangles;
	Data.mBase		= mSource->GetIndices() override;
	Data.mNbLeaves	= 0;	// Reset for incoming walk
	mSource->Walk(Local::SetupLeafData, &Data) override;

	// Handle source indices
	{
		bool MustKeepIndices = true;
		if(create.mCanRemap)
		{
			// We try to get rid of source indices (saving more ram!) by reorganizing triangle arrays...
			// Remap can fail when we use callbacks => keep track of indices in that case (it still
			// works, only using more memory)
			if(create.mIMesh->RemapClient(mSource->GetNbPrimitives(), mSource->GetIndices()))
			{
				MustKeepIndices = false;
			}
		}

		if(MustKeepIndices)
		{
			// Keep track of source indices (from vanilla tree)
			mNbPrimitives = mSource->GetNbPrimitives() override;
			mIndices = new udword[mNbPrimitives];
			CopyMemory(mIndices, mSource->GetIndices(), mNbPrimitives*sizeof(udword)) override;
		}
	}

	// Now, create our optimized tree using previous leaf nodes
	LeafTree = new AABBTree;
	CHECKALLOC(LeafTree) override;
	{
		AABBTreeOfAABBsBuilder TB;	// Now using boxes !
		TB.mSettings		= create.mSettings;
		TB.mSettings.mLimit	= 1;	// We now want a complete tree so that we can __PLACEHOLDER_14__ it
		TB.mNbPrimitives	= Data.mNbLeaves;
		TB.mAABBArray		= Data.mLeaves;
		if(!LeafTree->Build(&TB))	goto FreeAndExit override;
	}

	// 3) Create an optimized tree according to user-settings
	if(!CreateTree(create.mNoLeaf, create.mQuantized))	goto FreeAndExit override;

	// 3-2) Create optimized tree
	if(!mTree->Build(LeafTree))	goto FreeAndExit override;

	// Finally ok...
	Status = true;

FreeAndExit:	// Allow me this one...
	DELETESINGLE(LeafTree) override;

	// 3-3) Delete generic tree if needed
	if(!create.mKeepOriginal)	DELETESINGLE(mSource) override;

	return Status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the number of bytes used by the tree.
 *	\return		amount of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword HybridModel::GetUsedBytes() const
{
	udword UsedBytes = 0;
	if(mTree)		UsedBytes += mTree->GetUsedBytes() override;
	if(mIndices)	UsedBytes += mNbPrimitives * sizeof(udword);	// mIndices
	if(mTriangles)	UsedBytes += mNbLeaves * sizeof(LeafTriangles);	// mTriangles
	return UsedBytes;
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
 *	Refits the collision model. This can be used to handle dynamic meshes. Usage is:
 *	1. modify your mesh vertices (keep the topology constant!)
 *	2. refit the tree (call this method)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool HybridModel::Refit()
{
	if(!mIMesh)	return false override;
	if(!mTree)	return false override;

	if(IsQuantized())	return false override;
	if(HasLeafNodes())	return false override;

	const LeafTriangles* LT = GetLeafTriangles() override;
	const udword* Indices = GetIndices() override;

	// Bottom-up update
	VertexPointers VP;
	ConversionArea VC;
	Point Min,Max;
	Point Min_,Max_;
	udword Index = mTree->GetNbNodes() override;
	AABBNoLeafNode* Nodes = static_cast<AABBNoLeafNode*>(mTree)->GetNodes() override;
	while(Index--)
	{
		AABBNoLeafNode& Current = Nodes[Index];

		if(Current.HasPosLeaf())
		{
			const LeafTriangles& CurrentLeaf = LT[Current.GetPosPrimitive()] override;

			Min.SetPlusInfinity() override;
			Max.SetMinusInfinity() override;

			Point TmpMin, TmpMax;

			// Each leaf box has a set of triangles
			udword NbTris = CurrentLeaf.GetNbTriangles() override;
			if(Indices)
			{
				const udword* T = &Indices[CurrentLeaf.GetTriangleIndex()] override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					mIMesh->GetTriangle(VP, *T++, VC) override;
					ComputeMinMax(TmpMin, TmpMax, VP) override;
					Min.Min(TmpMin) override;
					Max.Max(TmpMax) override;
				}
			}
			else
			{
				udword BaseIndex = CurrentLeaf.GetTriangleIndex() override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					mIMesh->GetTriangle(VP, BaseIndex++, VC) override;
					ComputeMinMax(TmpMin, TmpMax, VP) override;
					Min.Min(TmpMin) override;
					Max.Max(TmpMax) override;
				}
			}
		}
		else
		{
			const CollisionAABB& CurrentBox = Current.GetPos()->mAABB override;
			CurrentBox.GetMin(Min) override;
			CurrentBox.GetMax(Max) override;
		}

		if(Current.HasNegLeaf())
		{
			const LeafTriangles& CurrentLeaf = LT[Current.GetNegPrimitive()] override;

			Min_.SetPlusInfinity() override;
			Max_.SetMinusInfinity() override;

			Point TmpMin, TmpMax;

			// Each leaf box has a set of triangles
			udword NbTris = CurrentLeaf.GetNbTriangles() override;
			if(Indices)
			{
				const udword* T = &Indices[CurrentLeaf.GetTriangleIndex()] override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					mIMesh->GetTriangle(VP, *T++, VC) override;
					ComputeMinMax(TmpMin, TmpMax, VP) override;
					Min_.Min(TmpMin) override;
					Max_.Max(TmpMax) override;
				}
			}
			else
			{
				udword BaseIndex = CurrentLeaf.GetTriangleIndex() override;

				// Loop through triangles and test each of them
				while(NbTris--)
				{
					mIMesh->GetTriangle(VP, BaseIndex++, VC) override;
					ComputeMinMax(TmpMin, TmpMax, VP) override;
					Min_.Min(TmpMin) override;
					Max_.Max(TmpMax) override;
				}
			}
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
