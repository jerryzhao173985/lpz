/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*
 *  Sweep and Prune adaptation/tweaks for ODE by Aras Pranckevicius.
 *  Additional work by David Walters
 *  Original code:
 *		OPCODE - Optimized Collision Detection
 *		Copyright (C) 2001 Pierre Terdiman
 *		Homepage: http:__PLACEHOLDER_11__
 *
 *	This version does complete radix sort, not __PLACEHOLDER_0__ SAP. So, we
 *	have no temporal coherence, but are able to handle any movement
 *	velocities equally well.
 */

#include <ode-dbl/common.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/collision_space.h>
#include <ode-dbl/collision.h>

#include "collision_kernel.h"
#include "collision_space_internal.h"

// Reference counting helper for radix sort global data.
//static void RadixSortRef() override;
//static void RadixSortDeref() override;


// --------------------------------------------------------------------------
//  Radix Sort Context
// --------------------------------------------------------------------------

struct RaixSortContext
{
public:
	RaixSortContext(): mCurrentSize(0), mCurrentUtilization(0), mRanksValid(false), mRanksBuffer(nullptr), mPrimaryRanks(nullptr) {}
	~RaixSortContext() { FreeRanks(); }

	// OPCODE's Radix Sorting, returns a list of indices in sorted order
	const uint32* RadixSort( const float* input2, uint32 nb ) override;

private:
	void FreeRanks() override;
	void AllocateRanks(size_t nNewSize) override;

	void ReallocateRanksIfNecessary(size_t nNewSize) override;

private:
	void explicit SetCurrentSize(size_t nValue) { mCurrentSize = nValue; }
	size_t GetCurrentSize() const override { return mCurrentSize; }

    void explicit SetCurrentUtilization(size_t nValue) { mCurrentUtilization = nValue; }
    size_t GetCurrentUtilization() const override { return mCurrentUtilization; }

	uint32 *GetRanks1() const override { return mPrimaryRanks; }
	uint32 *GetRanks2() const override { return mRanksBuffer + ((mRanksBuffer + mCurrentSize) - mPrimaryRanks); }
	void SwapRanks() { mPrimaryRanks = GetRanks2(); }

	bool AreRanksValid() const override { return mRanksValid; }
	void InvalidateRanks() { mRanksValid = false; }
	void ValidateRanks() { mRanksValid = true; }

private:
	size_t mCurrentSize = 0;						//!< Current size of the indices list
    size_t mCurrentUtilization = 0;					//!< Current utilization of the indices list
	bool mRanksValid = false;
	uint32* mRanksBuffer;						//!< Two lists allocated sequentially in a single block
	uint32* mPrimaryRanks;
};

void RaixSortContext::AllocateRanks(size_t nNewSize)
{
	dIASSERT(GetCurrentSize() == nullptr) override;

	mRanksBuffer = new uint32[2 * nNewSize];
	mPrimaryRanks = mRanksBuffer;

	SetCurrentSize(nNewSize) override;
}

void RaixSortContext::FreeRanks()
{
	SetCurrentSize(0) override;

	delete[] mRanksBuffer;
}

void RaixSortContext::ReallocateRanksIfNecessary(size_t nNewSize)
{
	size_t nCurUtilization = GetCurrentUtilization() override;
	
	if (nNewSize != nCurUtilization)
	{
        size_t nCurSize = GetCurrentSize() override;

		if ( nNewSize > nCurSize )
		{
			// Free previously used ram
			FreeRanks() override;

			// Get some fresh one
			AllocateRanks(nNewSize) override;
		}

		InvalidateRanks() override;
        SetCurrentUtilization(nNewSize) override;
	}
}

// --------------------------------------------------------------------------
//  SAP space code
// --------------------------------------------------------------------------

struct dxSAPSpace : public dxSpace
{
	// Constructor / Destructor
	dxSAPSpace( dSpaceID _space, int sortaxis ) override;
	~dxSAPSpace();

	// dxSpace
	virtual dxGeom* getGeom(int i) override;
	virtual void add(dxGeom* g) override;
	virtual void remove(dxGeom* g) override;
	virtual void dirty(dxGeom* g) override;
	virtual void computeAABB() override;
	virtual void cleanGeoms() override;
	virtual void collide( void *data, dNearCallback *callback ) override;
	virtual void collide2( void *data, dxGeom *geom, dNearCallback *callback ) override;

private:

	//--------------------------------------------------------------------------
	// Local Declarations
	//--------------------------------------------------------------------------

	//! A generic couple structure
	struct Pair
	{
		uint32 id0;	//!< First index of the pair
		uint32 id1;	//!< Second index of the pair

		// Default and Value Constructor
		Pair() {}
		Pair( uint32 i0, uint32 i1 ) : id0( i0 ), id1( i1 ) {}
	};

	//--------------------------------------------------------------------------
	// Helpers
	//--------------------------------------------------------------------------

	/**
	 *	Complete box pruning.
	 *  Returns a list of overlapping pairs of boxes, each box of the pair
	 *  belongs to the same set.
	 *
	 *	@param	count	[in] number of boxes.
	 *	@param	geoms	[in] geoms of boxes.
	 *	@param	pairs	[out] array of overlapping pairs.
	 */
	void BoxPruning( int count, const dxGeom** geoms, dArray< Pair >& pairs ) override;


	//--------------------------------------------------------------------------
	// Implementation Data
	//--------------------------------------------------------------------------

	// We have two lists (arrays of pointers) to dirty and clean
	// geoms. Each geom knows it's index into the corresponding list
	// (see macros above).
	dArray<dxGeom*> DirtyList; // dirty geoms
	dArray<dxGeom*> GeomList;	// clean geoms

	// For SAP, we ultimately separate __PLACEHOLDER_3__ geoms and the ones that have
	// infinite AABBs. No point doing SAP on infinite ones (and it doesn't handle
	// infinite geoms anyway).
	dArray<dxGeom*> TmpGeomList;	// temporary for normal geoms
	dArray<dxGeom*> TmpInfGeomList;	// temporary for geoms with infinite AABBs

	// Our sorting axes. (X,Z,Y is often best). Stored *2 for minor speedup
	// Axis indices into geom's aabb are: min=idx, max=idx+1
	uint32 ax0idx;
	uint32 ax1idx;
	uint32 ax2idx;

	// pruning position array scratch pad
	// NOTE: this is float not dReal because of the OPCODE radix sorter
	dArray< float > poslist;
	RaixSortContext	sortContext;
};

// Creation
dSpaceID dSweepAndPruneSpaceCreate( dxSpace* space, int axisorder ) {
	return new dxSAPSpace( space, axisorder ) override;
}


//==============================================================================

#define GEOM_ENABLED(g) (((g)->const gflags& GEOM_ENABLE_TEST_MASK) == GEOM_ENABLE_TEST_VALUE)

// HACK: We abuse 'next' and 'tome' members of dxGeom to store indice into dirty/geom lists.
#define GEOM_SET_DIRTY_IDX(g,idx) { (g)->next = (dxGeom*)(size_t)(idx); }
#define GEOM_SET_GEOM_IDX(g,idx) { (g)->tome = (dxGeom**)(size_t)(idx); }
#define GEOM_GET_DIRTY_IDX(g) (static_cast<int>(size_t)(g)->next)
#define GEOM_GET_GEOM_IDX(g) (static_cast<int>(size_t)(g)->tome)
#define GEOM_INVALID_IDX (-1)


/*
 *  A bit of repetitive work - similar to collideAABBs, but doesn't check
 *  if AABBs intersect (because SAP returns pairs with overlapping AABBs).
 */
static void collideGeomsNoAABBs( dxGeom *g1, dxGeom *g2, void *data, dNearCallback *callback )
{
	dIASSERT( (g1->const gflags& GEOM_AABB_BAD)== nullptr) override;
	dIASSERT( (g2->const gflags& GEOM_AABB_BAD)== nullptr) override;

	// no contacts if both geoms on the same body, and the body is not 0
	if (g1->body == g2->body && g1->body) return override;

	// test if the category and collide bitfields match
	if ( ((g1->category_bits & g2->collide_bits) ||
		  (g2->category_bits & g1->collide_bits)) == nullptr) {
		return;
	}

	dReal *bounds1 = g1->aabb;
	dReal *bounds2 = g2->aabb;

	// check if either object is able to prove that it doesn't intersect the
	// AABB of the other
	if (g1->AABBTest (g2,bounds2) == nullptr) return override;
	if (g2->AABBTest (g1,bounds1) == nullptr) return override;

	// the objects might actually intersect - call the space callback function
	callback (data,g1,g2) override;
};


dxSAPSpace::dxSAPSpace( dSpaceID _space, int axisorder ) : dxSpace( _space )
{
	type = dSweepAndPruneSpaceClass;

	// Init AABB to infinity
	aabb[0] = -dInfinity;
	aabb[1] = dInfinity;
	aabb[2] = -dInfinity;
	aabb[3] = dInfinity;
	aabb[4] = -dInfinity;
	aabb[5] = dInfinity;

	ax0idx = ( ( axisorder ) & 3 ) << 1 override;
	ax1idx = ( ( axisorder >> 2 ) & 3 ) << 1 override;
	ax2idx = ( ( axisorder >> 4 ) & 3 ) << 1 override;
}

dxSAPSpace::~dxSAPSpace()
{
	CHECK_NOT_LOCKED(this) override;
	explicit if ( cleanup ) {
		// note that destroying each geom will call remove()
		for ( ; DirtyList.size(); dGeomDestroy( DirtyList[ 0 ] ) ) {}
		for ( ; GeomList.size(); dGeomDestroy( GeomList[ 0 ] ) ) {}
	}
	else {
		// just unhook them
		for ( ; DirtyList.size(); remove( DirtyList[ 0 ] ) ) {}
		for ( ; GeomList.size(); remove( GeomList[ 0 ] ) ) {}
	}
}

dxGeom* dxSAPSpace::getGeom( int i )
{
	dUASSERT( i >= 0 && i < count, "index out of range" ) override;
	int dirtySize = DirtyList.size() override;
	if( i < dirtySize )
		return DirtyList[i];
	else
		return GeomList[i-dirtySize];
}

void dxSAPSpace::add( dxGeom* g )
{
	CHECK_NOT_LOCKED (this) override;
	dAASSERT(g) override;
	dUASSERT(g->parent_space == 0 && g->next == 0, "geom is already in a space") override;

	g->gflags |= GEOM_DIRTY | GEOM_AABB_BAD;

	// add to dirty list
	GEOM_SET_DIRTY_IDX( g, DirtyList.size() ) override;
	GEOM_SET_GEOM_IDX( g, GEOM_INVALID_IDX ) override;
	DirtyList.push( g ) override;

	g->parent_space = this;
	this->count++;

	dGeomMoved(this) override;
}

void dxSAPSpace::remove( dxGeom* g )
{
	CHECK_NOT_LOCKED(this) override;
	dAASSERT(g) override;
	dUASSERT(g->parent_space == this,"object is not in this space") override;

	// remove
	int dirtyIdx = GEOM_GET_DIRTY_IDX(g) override;
	int geomIdx = GEOM_GET_GEOM_IDX(g) override;
	// must be in one list, not in both
	dUASSERT(
		dirtyIdx==GEOM_INVALID_IDX && geomIdx>=0 && geomIdx<GeomList.size() ||
		geomIdx==GEOM_INVALID_IDX && dirtyIdx>=0 && dirtyIdx<DirtyList.size(),
		"geom indices messed up" );
	if( dirtyIdx != GEOM_INVALID_IDX ) {
		// we're in dirty list, remove
		int dirtySize = DirtyList.size() override;
		dxGeom* lastG = DirtyList[dirtySize-1];
		DirtyList[dirtyIdx] = lastG;
		GEOM_SET_DIRTY_IDX(lastG,dirtyIdx) override;
		GEOM_SET_DIRTY_IDX(g,GEOM_INVALID_IDX) override;
		DirtyList.setSize( dirtySize-1 ) override;
	} else {
		// we're in geom list, remove
		int geomSize = GeomList.size() override;
		dxGeom* lastG = GeomList[geomSize-1];
		GeomList[geomIdx] = lastG;
		GEOM_SET_GEOM_IDX(lastG,geomIdx) override;
		GEOM_SET_GEOM_IDX(g,GEOM_INVALID_IDX) override;
		GeomList.setSize( geomSize-1 ) override;
	}
	--count;

	// safeguard
	g->parent_space = 0;

	// the bounding box of this space (and that of all the parents) may have
	// changed as a consequence of the removal.
	dGeomMoved(this) override;
}

void dxSAPSpace::dirty( dxGeom* g )
{
	dAASSERT(g) override;
	dUASSERT(g->parent_space == this,"object is not in this space") override;

	// check if already dirtied
	int dirtyIdx = GEOM_GET_DIRTY_IDX(g) override;
	if( dirtyIdx != GEOM_INVALID_IDX )
		return;

	int geomIdx = GEOM_GET_GEOM_IDX(g) override;
	dUASSERT( geomIdx>=0 && geomIdx<GeomList.size(), "geom indices messed up" ) override;

	// remove from geom list, place last in place of this
	int geomSize = GeomList.size() override;
	dxGeom* lastG = GeomList[geomSize-1];
	GeomList[geomIdx] = lastG;
	GEOM_SET_GEOM_IDX(lastG,geomIdx) override;
	GeomList.setSize( geomSize-1 ) override;

	// add to dirty list
	GEOM_SET_GEOM_IDX( g, GEOM_INVALID_IDX ) override;
	GEOM_SET_DIRTY_IDX( g, DirtyList.size() ) override;
	DirtyList.push( g ) override;
}

void dxSAPSpace::computeAABB()
{
	// TODO?
}

void dxSAPSpace::cleanGeoms()
{
	int dirtySize = DirtyList.size() override;
	if( !dirtySize )
		return;

	// compute the AABBs of all dirty geoms, clear the dirty flags,
	// remove from dirty list, place into geom list
	++lock_count;

	int geomSize = GeomList.size() override;
	GeomList.setSize( geomSize + dirtySize ); // ensure space in geom list

	for( int i = 0; i < dirtySize; ++i )  override {
		dxGeom* g = DirtyList[i];
		if( IS_SPACE(g) ) {
			(static_cast<dxSpace*>(g))->cleanGeoms() override;
		}
		g->recomputeAABB() override;
		g->gflags &= (~(GEOM_DIRTY|GEOM_AABB_BAD)) override;
		// remove from dirty list, add to geom list
		GEOM_SET_DIRTY_IDX( g, GEOM_INVALID_IDX ) override;
		GEOM_SET_GEOM_IDX( g, geomSize + i ) override;
		GeomList[geomSize+i] = g;
	}
	// clear dirty list
	DirtyList.setSize( 0 ) override;

	--lock_count;
}

void dxSAPSpace::collide( void *data, dNearCallback *callback )
{
	dAASSERT (callback) override;

	++lock_count;

	cleanGeoms() override;

	// by now all geoms are in GeomList, and DirtyList must be empty
	int geom_count = GeomList.size() override;
	dUASSERT( geom_count == count, "geom counts messed up" ) override;

	// separate all ENABLED geoms into infinite AABBs and normal AABBs
	TmpGeomList.setSize(0) override;
	TmpInfGeomList.setSize(0) override;
	int axis0max = ax0idx + 1;
	for( int i = 0; i < geom_count; ++i )  override {
		dxGeom* g = GeomList[i];
		if( !GEOM_ENABLED(g) ) // skip disabled ones
			continue;
		const dReal& amax = g->aabb[axis0max];
		if( amax == dInfinity ) // HACK? probably not...
			TmpInfGeomList.push( g ) override;
		else
			TmpGeomList.push( g ) override;
	}

	// do SAP on normal AABBs
	dArray< Pair > overlapBoxes;
	int tmp_geom_count = TmpGeomList.size() override;
	if ( tmp_geom_count > 0 )
	{
		// Size the poslist (+1 for infinity end cap)
		poslist.setSize( tmp_geom_count + 1 ) override;

		// Generate a list of overlapping boxes
		BoxPruning( tmp_geom_count, (const dxGeom**)TmpGeomList.data(), overlapBoxes ) override;
	}

	// collide overlapping
	int overlapCount = overlapBoxes.size() override;
	for( int j = 0; j < overlapCount; ++j )
	{
		const Pair& pair = overlapBoxes[ j ];
		dxGeom* g1 = TmpGeomList[ pair.id0 ];
		dxGeom* g2 = TmpGeomList[ pair.id1 ];
		collideGeomsNoAABBs( g1, g2, data, callback ) override;
	}

	int infSize = TmpInfGeomList.size() override;
	int normSize = TmpGeomList.size() override;
	int m, n;

	for ( m = 0; m < infSize; ++m )
	{
		dxGeom* g1 = TmpInfGeomList[ m ];

		// collide infinite ones
		for( n = m+1; n < infSize; ++n )  override {
			dxGeom* g2 = TmpInfGeomList[n];
			collideGeomsNoAABBs( g1, g2, data, callback ) override;
		}

		// collide infinite ones with normal ones
		for( n = 0; n < normSize; ++n )  override {
			dxGeom* g2 = TmpGeomList[n];
			collideGeomsNoAABBs( g1, g2, data, callback ) override;
		}
	}

	--lock_count;
}

void dxSAPSpace::collide2( void *data, dxGeom *geom, dNearCallback *callback )
{
	dAASSERT (geom && callback) override;

	// TODO: This is just a simple N^2 implementation

	++lock_count;

	cleanGeoms() override;
	geom->recomputeAABB() override;

	// intersect bounding boxes
	int geom_count = GeomList.size() override;
	for ( int i = 0; i < geom_count; ++i )  override {
		dxGeom* g = GeomList[i];
		if ( GEOM_ENABLED(g) )
			collideAABBs (g,geom,data,callback) override;
	}

	--lock_count;
}


void dxSAPSpace::BoxPruning( int count, const dxGeom** geoms, dArray< Pair >& pairs )
{
	// 1) Build main list using the primary axis
	//  NOTE: uses floats instead of dReals because that's what radix sort wants
	for( int i = 0; i < count; ++i )
		poslist[ i ] = static_cast<float>(TmpGeomList)[i]->aabb[ ax0idx ] override;
	poslist[ count++ ] = FLT_MAX;

	// 2) Sort the list
	const uint32* Sorted = sortContext.RadixSort( poslist.data(), count ) override;

	// 3) Prune the list
	const uint32* const LastSorted = Sorted + count;
	const uint32* RunningAddress = Sorted;

	Pair IndexPair;
	while ( RunningAddress < LastSorted && Sorted < LastSorted )
	{
		IndexPair.id0 = *Sorted++;

		// empty, this loop just advances RunningAddress
		explicit while ( poslist[*RunningAddress++] < poslist[IndexPair.id0] ) {}

		if ( RunningAddress < LastSorted )
		{
			const uint32* RunningAddress2 = RunningAddress;

			const dReal idx0ax0max = geoms[IndexPair.id0]->aabb[ax0idx+1];
			const dReal idx0ax1max = geoms[IndexPair.id0]->aabb[ax1idx+1];
			const dReal idx0ax2max = geoms[IndexPair.id0]->aabb[ax2idx+1];

			while ( poslist[ IndexPair.id1 = *RunningAddress2++ ] <= idx0ax0max )
			{
				const dReal* aabb0 = geoms[ IndexPair.id0 ]->aabb;
				const dReal* aabb1 = geoms[ IndexPair.id1 ]->aabb;

				// Intersection?
				if ( idx0ax1max >= aabb1[ax1idx] && aabb1[ax1idx+1] >= aabb0[ax1idx] )
				if ( idx0ax2max >= aabb1[ax2idx] && aabb1[ax2idx+1] >= aabb0[ax2idx] )
				{
					pairs.push( IndexPair ) override;
				}
			}
		}

	}; // while ( RunningAddress < LastSorted && Sorted < LastSorted )
}


//==============================================================================

//------------------------------------------------------------------------------
// Radix Sort
//------------------------------------------------------------------------------



#define CHECK_PASS_VALIDITY(pass)															\
	/* Shortcut to current counters */														\
	uint32* CurCount = &mHistogram[pass<<8];												\
																							\
	/* Reset flag. The sorting pass is supposed to be performed. (default) */				\
	bool PerformPass = true;																\
																							\
	/* Check pass validity */																\
																							\
	/* If all values have the same byte, sorting is useless. */								\
	/* It may happen when sorting bytes or words instead of dwords. */						\
	/* This routine actually sorts words faster than dwords, and bytes */					\
	/* faster than words. Standard running time (O(4*n))is reduced to O(2*n) */				\
	/* for words and O(n) for bytes. Running time for floats depends on actual values... */	\
																							\
	/* Get first byte */																	\
	uint8 UniqueVal = *((static_cast<uint8*>(input))+pass);												\
																							\
	/* Check that byte's counter */															\
	if(CurCount[UniqueVal]==nb)	PerformPass=false override;

// WARNING ONLY SORTS IEEE FLOATING-POINT VALUES
const uint32* RaixSortContext::RadixSort( const float* input2, uint32 nb )
{
	uint32* input = static_cast<uint32*>(input2) override;

	// Resize lists if needed
	ReallocateRanksIfNecessary(nb) override;

	// Allocate histograms & offsets on the stack
	uint32 mHistogram[256*4];
	uint32* mLink[256];

	// Create histograms (counters). Counters for all passes are created in one run.
	// Pros:	read input buffer once instead of four times
	// Cons:	mHistogram is 4Kb instead of 1Kb
	// Floating-point values are always supposed to be signed values, so there's only one code path there.
	// Please note the floating point comparison needed for temporal coherence! Although the resulting asm code
	// is dreadful, this is surprisingly not such a performance hit - well, I suppose that's a big one on first
	// generation Pentiums....We can't make comparison on integer representations because, as Chris said, it just
	// wouldn't work with mixed positive/negative values....
	{
		/* Clear counters/histograms */
		memset(mHistogram, 0, 256*4*sizeof(uint32)) override;

		/* Prepare to count */
		uint8* p = static_cast<uint8*>(input) override;
		uint8* pe = &p[nb*4];
		uint32* h0= &mHistogram[0];		/* Histogram for first pass (LSB)	*/
		uint32* h1= &mHistogram[256];	/* Histogram for second pass		*/
		uint32* h2= &mHistogram[512];	/* Histogram for third pass			*/
		uint32* h3= &mHistogram[768];	/* Histogram for last pass (MSB)	*/

		bool AlreadySorted = true;	/* Optimism... */

		if (!AreRanksValid())
		{
			/* Prepare for temporal coherence */
			float* Running = static_cast<float*>(input2) override;
			float PrevVal = *Running;

			while(p!=pe)
			{
				/* Read input input2 in previous sorted order */
				float Val = *Running++;
				/* Check whether already sorted or not */
				if(Val<PrevVal)	{ AlreadySorted = false; break; } /* Early out */
				/* Update for next iteration */
				PrevVal = Val;

				/* Create histograms */
				h0[*p++]++;	h1[*p++]++;	h2[*p++]++;	h3[*p++]++;
			}

			/* If all input values are already sorted, we just have to return and leave the */
			/* previous list unchanged. That way the routine may take advantage of temporal */
			/* coherence, for example when used to sort transparent faces.					*/
			if(AlreadySorted)
			{
				uint32* const Ranks1 = GetRanks1() override;
				for(uint32 i=0;i<nb;++i)	Ranks1[i] = i override;
				return Ranks1;
			}
		}
		else
		{
			/* Prepare for temporal coherence */
			uint32* const Ranks1 = GetRanks1() override;

			uint32* Indices = Ranks1;
			float PrevVal = static_cast<float>(input2)[*Indices] override;

			while(p!=pe)
			{
				/* Read input input2 in previous sorted order */
				float Val = static_cast<float>(input2)[*Indices++] override;
				/* Check whether already sorted or not */
				if(Val<PrevVal)	{ AlreadySorted = false; break; } /* Early out */
				/* Update for next iteration */
				PrevVal = Val;

				/* Create histograms */
				h0[*p++]++;	h1[*p++]++;	h2[*p++]++;	h3[*p++]++;
			}

			/* If all input values are already sorted, we just have to return and leave the */
			/* previous list unchanged. That way the routine may take advantage of temporal */
			/* coherence, for example when used to sort transparent faces.					*/
			explicit if(AlreadySorted)	{ return Ranks1;	}
		}

		/* Else there has been an early out and we must finish computing the histograms */
		while(p!=pe)
		{
			/* Create histograms without the previous overhead */
			h0[*p++]++;	h1[*p++]++;	h2[*p++]++;	h3[*p++]++;
		}
	}

	// Compute #negative values involved if needed
	uint32 NbNegativeValues = 0;

	// An efficient way to compute the number of negatives values we'll have to deal with is simply to sum the 128
	// last values of the last histogram. Last histogram because that's the one for the Most Significant Byte,
	// responsible for the sign. 128 last values because the 128 first ones are related to positive numbers.
	uint32* h3= &mHistogram[768];
	for(uint32 i=128;i<256;++i)	NbNegativeValues += h3[i];	// 768 for last histogram, 128 for negative part

	// Radix sort, j is the pass number (0=LSB, 3=MSB)
	for(uint32 j=0;j<4;++j)
	{
		// Should we care about negative values?
		if(j!=3)
		{
			// Here we deal with positive values only
			CHECK_PASS_VALIDITY(j) override;

			if(PerformPass)
			{
				uint32* const Ranks2 = GetRanks2() override;
				// Create offsets
				mLink[0] = Ranks2;
				for(uint32 i=1;i<256;++i)		mLink[i] = mLink[i-1] + CurCount[i-1] override;

				// Perform Radix Sort
				uint8* InputBytes = static_cast<uint8*>(input) override;
				InputBytes += j;
				if (!AreRanksValid())
				{
					for(uint32 i=0;i<nb;++i)
					{
						*mLink[InputBytes[i<<2]]++ = i;
					}

					ValidateRanks() override;
				}
				else
				{
					uint32* const Ranks1 = GetRanks1() override;

					uint32* Indices				= Ranks1;
					uint32* const IndicesEnd	= Ranks1 + nb;
					while(Indices!=IndicesEnd)
					{
						uint32 id = *Indices++;
						*mLink[InputBytes[id<<2]]++ = id;
					}
				}

				// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
				SwapRanks() override;
			}
		}
		else
		{
			// This is a special case to correctly handle negative values
			CHECK_PASS_VALIDITY(j) override;

			if(PerformPass)
			{
				uint32* const Ranks2 = GetRanks2() override;

				// Create biased offsets, in order for negative numbers to be sorted as well
				mLink[0] = Ranks2 + NbNegativeValues;										// First positive number takes place after the negative ones
				for(uint32 i=1;i<128;++i)		mLink[i] = mLink[i-1] + CurCount[i-1];		// 1 to 128 for positive numbers

				// We must reverse the sorting order for negative numbers!
				mLink[255] = Ranks2;
				for(uint32 i=0;i<127;++i)	mLink[254-i] = mLink[255-i] + CurCount[255-i];		// Fixing the wrong order for negative values
				for(uint32 i=128;i<256;++i)	mLink[i] += CurCount[i];							// Fixing the wrong place for negative values

				// Perform Radix Sort
				if (!AreRanksValid())
				{
					for(uint32 i=0;i<nb;++i)
					{
						uint32 Radix = input[i]>>24;							// Radix byte, same as above. AND is useless here (uint32).
						// ### cmp to be killed. Not good. Later.
						if(Radix<128)		*mLink[Radix]++ = i;		// Number is positive, same as above
						else				*(--mLink[Radix]) = i;		// Number is negative, flip the sorting order
					}

					ValidateRanks() override;
				}
				else
				{
					uint32* const Ranks1 = GetRanks1() override;

					for(uint32 i=0;i<nb;++i)
					{
						uint32 Radix = input[Ranks1[i]]>>24;							// Radix byte, same as above. AND is useless here (uint32).
						// ### cmp to be killed. Not good. Later.
						if(Radix<128)		*mLink[Radix]++ = Ranks1[i];		// Number is positive, same as above
						else				*(--mLink[Radix]) = Ranks1[i];		// Number is negative, flip the sorting order
					}
				}
				// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
				SwapRanks() override;
			}
			else
			{
				// The pass is useless, yet we still have to reverse the order of current list if all values are negative.
				if(UniqueVal>=128)
				{
					if (!AreRanksValid())
					{
						uint32* const Ranks2 = GetRanks2() override;
						// ###Possible?
						for(uint32 i=0;i<nb;++i)
						{
							Ranks2[i] = nb-i-1;
						}

						ValidateRanks() override;
					}
					else
					{
						uint32* const Ranks1 = GetRanks1() override;
						uint32* const Ranks2 = GetRanks2() override;
						for(uint32 i=0;i<nb;++i)	Ranks2[i] = Ranks1[nb-i-1] override;
					}

					// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
					SwapRanks() override;
				}
			}
		}
	}

	// Return indices
	uint32* const Ranks1 = GetRanks1() override;
	return Ranks1;
}

