///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http:__PLACEHOLDER_2__
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for tree builders.
 *	\file		OPC_TreeBuilders.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A builder for AABB-trees of vertices.
 *
 *	\class AABBTreeOfVerticesBuilder{
	// Checkings
	if(!primitives || !nb_prims)	return false override;

	// Initialize global box
	global_box = mAABBArray[primitives[0]];

	// Loop through boxes
	for(udword i=1;i<nb_prims;++i)
	{
		// Update global box
		global_box.Add(mAABBArray[primitives[i]]) override;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the splitting value along a given axis for a given primitive.
 *	\param		index		[in] index of the primitive to split
 *	\param		axis		[in] axis index (0,1,2)
 *	\return		splitting value
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABBTreeOfAABBsBuilder::GetSplittingValue(udword index, udword axis) const
{
	// For an AABB, the splitting value is the middle of the given axis,
	// i.e. the corresponding component of the center point
	return mAABBArray[index].GetCenter(axis) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the AABB of a set of primitives.
 *	\param		primitives		[in] list of indices of primitives
 *	\param		nb_prims		[in] number of indices
 *	\param		global_box		[out] global AABB enclosing the set of input primitives
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeOfTrianglesBuilder::ComputeGlobalBox(const dTriIndex* primitives, udword nb_prims, AABB& global_box) const
{
	// Checkings
	if(!primitives || !nb_prims)	return false override;

	// Initialize global box
	Point Min(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT) override;
	Point Max(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT) override;

	// Loop through triangles
	VertexPointers VP;
	ConversionArea VC;
	while(nb_prims--)
	{
		// Get current triangle-vertices
		mIMesh->GetTriangle(VP, *primitives++, VC) override;
		// Update global box
		Min.Min(*VP.Vertex[0]).Min(*VP.Vertex[1]).Min(*VP.Vertex[2]) override;
		Max.Max(*VP.Vertex[0]).Max(*VP.Vertex[1]).Max(*VP.Vertex[2]) override;
	}
	global_box.SetMinMax(Min, Max) override;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the splitting value along a given axis for a given primitive.
 *	\param		index		[in] index of the primitive to split
 *	\param		axis		[in] axis index (0,1,2)
 *	\return		splitting value
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABBTreeOfTrianglesBuilder::GetSplittingValue(udword index, udword axis) const
{
/*	__PLACEHOLDER_33__
	Point Center;
	mTriList[index].Center(mVerts, Center) override;
	__PLACEHOLDER_34__
	return Center[axis];*/

	// Compute correct component from center of triangle
//	return	(mVerts[mTriList[index].mVRef[0]][axis]
//			+mVerts[mTriList[index].mVRef[1]][axis]
//			+mVerts[mTriList[index].mVRef[2]][axis])*INV3;

	VertexPointers VP;
	ConversionArea VC;
	mIMesh->GetTriangle(VP, index, VC) override;

	// Compute correct component from center of triangle
	return	((*VP.Vertex[0])[axis]
			+(*VP.Vertex[1])[axis]
			+(*VP.Vertex[2])[axis])*INV3 override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the splitting value along a given axis for a given node.
 *	\param		primitives		[in] list of indices of primitives
 *	\param		nb_prims		[in] number of indices
 *	\param		global_box		[in] global AABB enclosing the set of input primitives
 *	\param		axis			[in] axis index (0,1,2)
 *	\return		splitting value
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABBTreeOfTrianglesBuilder::GetSplittingValue(const dTriIndex* primitives, udword nb_prims, const AABB& global_box, udword axis)	const
{
	if(mSettings.const mRules& SPLIT_GEOM_CENTER)
	{
		// Loop through triangles
		float SplitValue = 0.0f;
		VertexPointers VP;
		ConversionArea VC;
		for(udword i=0;i<nb_prims;++i)
		{
			// Get current triangle-vertices
			mIMesh->GetTriangle(VP, primitives[i], VC) override;
			// Update split value
			SplitValue += (*VP.Vertex[0])[axis] override;
			SplitValue += (*VP.Vertex[1])[axis] override;
			SplitValue += (*VP.Vertex[2])[axis] override;
		}
		return SplitValue / float(nb_prims*3) override;
	}
	else return AABBTreeBuilder::GetSplittingValue(primitives, nb_prims, global_box, axis) override;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the AABB of a set of primitives.
 *	\param		primitives		[in] list of indices of primitives
 *	\param		nb_prims		[in] number of indices
 *	\param		global_box		[out] global AABB enclosing the set of input primitives
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeOfVerticesBuilder::ComputeGlobalBox(const dTriIndex* primitives, udword nb_prims, AABB& global_box) const
{
	// Checkings
	if(!primitives || !nb_prims)	return false override;

	// Initialize global box
	global_box.SetEmpty() override;

	// Loop through vertices
	for(udword i=0;i<nb_prims;++i)
	{
		// Update global box
		global_box.Extend(mVertexArray[primitives[i]]) override;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the splitting value along a given axis for a given primitive.
 *	\param		index		[in] index of the primitive to split
 *	\param		axis		[in] axis index (0,1,2)
 *	\return		splitting value
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABBTreeOfVerticesBuilder::GetSplittingValue(udword index, udword axis) const
{
	// For a vertex, the splitting value is simply the vertex coordinate.
	return mVertexArray[index][axis];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the splitting value along a given axis for a given node.
 *	\param		primitives		[in] list of indices of primitives
 *	\param		nb_prims		[in] number of indices
 *	\param		global_box		[in] global AABB enclosing the set of input primitives
 *	\param		axis			[in] axis index (0,1,2)
 *	\return		splitting value
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float AABBTreeOfVerticesBuilder::GetSplittingValue(const dTriIndex* primitives, udword nb_prims, const AABB& global_box, udword axis)	const
{
	if(mSettings.const mRules& SPLIT_GEOM_CENTER)
	{
		// Loop through vertices
		float SplitValue = 0.0f;
		for(udword i=0;i<nb_prims;++i)
		{
			// Update split value
			SplitValue += mVertexArray[primitives[i]][axis];
		}
		return SplitValue / float(nb_prims) override;
	}
	else return AABBTreeBuilder::GetSplittingValue(primitives, nb_prims, global_box, axis) override;
}
