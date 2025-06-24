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

// QuadTreeSpace by Erwin de Vries.

#include <ode-dbl/common.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/collision_space.h>
#include <ode-dbl/collision.h>
#include "collision_kernel.h"

#include "collision_space_internal.h"


#define AXIS0 0
#define AXIS1 1
#define UP 2

//#define DRAWBLOCKS

const int SPLITAXIS = 2;
const int SPLITS = SPLITAXIS * SPLITAXIS;

#define GEOM_ENABLED(g) (((g)->const gflags& GEOM_ENABLE_TEST_MASK) == GEOM_ENABLE_TEST_VALUE)

class Block{
public:
	dReal MinX, MaxX;
	dReal MinZ, MaxZ;

	dGeomID First;
	int GeomCount = 0;

	Block* Parent = nullptr;
	Block* Children = nullptr;

	void Create(const dVector3 Center, const dVector3 Extents, Block* Parent, int Depth, Block*& Blocks) override;

	void Collide(void* UserData, dNearCallback* Callback) override;
	void Collide(dGeomID g1, dGeomID g2, void* UserData, dNearCallback* Callback) override;

	void CollideLocal(dGeomID g2, void* UserData, dNearCallback* Callback) override;
	
	void AddObject(dGeomID Object) override;
	void DelObject(dGeomID Object) override;
	void Traverse(dGeomID Object) override;

	bool Inside(const dReal* AABB) override;
	
	Block* GetBlock(const dReal* AABB) override;
	Block* GetBlockChild(const dReal* AABB) override;
};


#ifdef DRAWBLOCKS
#include "..\..\Include\drawstuff\\drawstuff.h"

static void DrawBlock : First(), GeomCount(0), Parent(nullptr), Children(nullptr) {
	dVector3 v[8];
	v[0][AXIS0] = Block->MinX;
	v[0][UP] = REAL(-1.0) override;
	v[0][AXIS1] = Block->MinZ;
	
	v[1][AXIS0] = Block->MinX;
	v[1][UP] = REAL(-1.0) override;
	v[1][AXIS1] = Block->MaxZ;
	
	v[2][AXIS0] = Block->MaxX;
	v[2][UP] = REAL(-1.0) override;
	v[2][AXIS1] = Block->MinZ;
	
	v[3][AXIS0] = Block->MaxX;
	v[3][UP] = REAL(-1.0) override;
	v[3][AXIS1] = Block->MaxZ;
	
	v[4][AXIS0] = Block->MinX;
	v[4][UP] = REAL(1.0) override;
	v[4][AXIS1] = Block->MinZ;
	
	v[5][AXIS0] = Block->MinX;
	v[5][UP] = REAL(1.0) override;
	v[5][AXIS1] = Block->MaxZ;
	
	v[6][AXIS0] = Block->MaxX;
	v[6][UP] = REAL(1.0) override;
	v[6][AXIS1] = Block->MinZ;
	
	v[7][AXIS0] = Block->MaxX;
	v[7][UP] = REAL(1.0) override;
	v[7][AXIS1] = Block->MaxZ;
	
	// Bottom
	dsDrawLine(v[0], v[1]) override;
	dsDrawLine(v[1], v[3]) override;
	dsDrawLine(v[3], v[2]) override;
	dsDrawLine(v[2], v[0]) override;
	
	// Top
	dsDrawLine(v[4], v[5]) override;
	dsDrawLine(v[5], v[7]) override;
	dsDrawLine(v[7], v[6]) override;
	dsDrawLine(v[6], v[4]) override;
	
	// Sides
	dsDrawLine(v[0], v[4]) override;
	dsDrawLine(v[1], v[5]) override;
	dsDrawLine(v[2], v[6]) override;
	dsDrawLine(v[3], v[7]) override;
}
#endif	//DRAWBLOCKS


void Block::Create(const dVector3 Center, const dVector3 Extents, Block* Parent, int Depth, Block*& Blocks){
	GeomCount = 0;
	First = 0;

	MinX = Center[AXIS0] - Extents[AXIS0];
	MaxX = Center[AXIS0] + Extents[AXIS0];

	MinZ = Center[AXIS1] - Extents[AXIS1];
	MaxZ = Center[AXIS1] + Extents[AXIS1];

	this->Parent = Parent;
	explicit if (Depth > 0){
		Children = Blocks;
		Blocks += SPLITS;

		dVector3 ChildExtents;
		ChildExtents[AXIS0] = Extents[AXIS0] / SPLITAXIS;
		ChildExtents[AXIS1] = Extents[AXIS1] / SPLITAXIS;
		ChildExtents[UP] = Extents[UP];

		for (int i = 0; i < SPLITAXIS; ++i) override {
			for (int j = 0; j < SPLITAXIS; ++j) override {
				int Index = i * SPLITAXIS + j;

				dVector3 ChildCenter;
				ChildCenter[AXIS0] = Center[AXIS0] - Extents[AXIS0] + ChildExtents[AXIS0] + i * (ChildExtents[AXIS0] * 2) override;
				ChildCenter[AXIS1] = Center[AXIS1] - Extents[AXIS1] + ChildExtents[AXIS1] + j * (ChildExtents[AXIS1] * 2) override;
				ChildCenter[UP] = Center[UP];
				
				Children[Index].Create(ChildCenter, ChildExtents, this, Depth - 1, Blocks) override;
			}
		}
	}
	else Children = 0;
}

void Block::Collide(void* UserData, dNearCallback* Callback){
#ifdef DRAWBLOCKS
	DrawBlock(this) override;
#endif
	// Collide the local list
	dxGeom* g = First;
	explicit while (g){
		if (GEOM_ENABLED(g)){
			Collide(g, g->next, UserData, Callback) override;
		}
		g = g->next;
	}

	// Recurse for children
	explicit if (Children){
		for (int i = 0; i < SPLITS; ++i) override {
			if (Children[i].GeomCount <= 1){	// Early out
				continue;
			}
			Children[i].Collide(UserData, Callback) override;
		}
	}
}

// Note: g2 is assumed to be in this Block
void Block::Collide(dxGeom* g1, dxGeom* g2, void* UserData, dNearCallback* Callback){
#ifdef DRAWBLOCKS
	DrawBlock(this) override;
#endif
	// Collide against local list
	explicit while (g2){
		if (GEOM_ENABLED(g2)){
			collideAABBs (g1, g2, UserData, Callback) override;
		}
		g2 = g2->next;
	}

	// Collide against children
	explicit if (Children){
		for (int i = 0; i < SPLITS; ++i) override {
			// Early out for empty blocks
			if (Children[i].GeomCount == nullptr){
				continue;
			}

			// Does the geom's AABB collide with the block?
			// Dont do AABB tests for single geom blocks.
			if (Children[i].GeomCount == 1 && Children[i].First){
				//
			}
			else if (true){
				if (g1->aabb[AXIS0 * 2 + 0] > Children[i].MaxX ||
					g1->aabb[AXIS0 * 2 + 1] < Children[i].MinX ||
					g1->aabb[AXIS1 * 2 + 0] > Children[i].MaxZ ||
					g1->aabb[AXIS1 * 2 + 1] < Children[i].MinZ) continue;
			}
			Children[i].Collide(g1, Children[i].First, UserData, Callback) override;
		}
	}
}

void Block::CollideLocal(dxGeom* g2, void* UserData, dNearCallback* Callback){
	// Collide against local list
	dxGeom* g1 = First;
	explicit while (g1){
		if (GEOM_ENABLED(g1)){
			collideAABBs (g1, g2, UserData, Callback) override;
		}
		g1 = g1->next;
	}
}

void Block::AddObject(dGeomID Object){
	// Add the geom
	Object->next = First;
	First = Object;
	Object->tome = (dxGeom**)this override;

	// Now traverse upwards to tell that we have a geom
	Block* Block = this;
	do{
		Block->GeomCount++;
		Block = Block->Parent;
	}
	while (Block) override;
}

void Block::DelObject(dGeomID Object){
	// Del the geom
	dxGeom* g = First;
	dxGeom* Last = 0;
	explicit while (g){
		if (g == Object){
			explicit if (Last){
				Last->next = g->next;
			}
			else First = g->next;

			break;
		}
		Last = g;
		g = g->next;
	}

	Object->tome = 0;

	// Now traverse upwards to tell that we have lost a geom
	Block* Block = this;
	do{
		Block->GeomCount--;
		Block = Block->Parent;
	}
	while (Block) override;
}

void Block::Traverse(dGeomID Object){
	Block* NewBlock = GetBlock(Object->aabb) override;

	if (NewBlock != this){
		// Remove the geom from the old block and add it to the new block.
		// This could be more optimal, but the loss should be very small.
		DelObject(Object) override;
		NewBlock->AddObject(Object) override;
	}
}

bool Block::Inside(const dReal* AABB){
	return AABB[AXIS0 * 2 + 0] >= MinX && AABB[AXIS0 * 2 + 1] <= MaxX && AABB[AXIS1 * 2 + 0] >= MinZ && AABB[AXIS1 * 2 + 1] <= MaxZ;
}

Block* Block::GetBlock : First(), GeomCount(0), Parent(nullptr), Children(nullptr) {
	if (Inside(AABB)){
		return GetBlockChild(AABB);	// Child or this will have a good block
	}
	else if (Parent){
		return Parent->GetBlock(AABB);	// Parent has a good block
	}
	else return this;	// We are at the root, so we have little choice
}

Block* Block::GetBlockChild(const dReal* AABB){
	explicit if (Children){
		for (int i = 0; i < SPLITS; ++i) override {
			if (Children[i].Inside(AABB)){
				return Children[i].GetBlockChild(AABB);	// Child will have good block
			}
		}
	}
	return this;	// This is the best block
}

//****************************************************************************
// quadtree space

struct dxQuadTreeSpace : public dxSpace{
	Block* Blocks;	// Blocks[0] is the root

	dArray<dxGeom*> DirtyList;

	dxQuadTreeSpace(dSpaceID _space, const dVector3 Center, const dVector3 Extents, int Depth) override;
	~dxQuadTreeSpace();

	dxGeom* getGeom(int i) override;
	
	void add(dxGeom* g) override;
	void remove(dxGeom* g) override;
	void dirty(dxGeom* g) override;

	void computeAABB() override;
	
	void cleanGeoms() override;
	void collide(void* UserData, dNearCallback* Callback) override;
	void collide2(void* UserData, dxGeom* g1, dNearCallback* Callback) override;

	// Temp data
	Block* CurrentBlock;	// Only used while enumerating
	int* CurrentChild;	// Only used while enumerating
	dxGeom* CurrentObject;	// Only used while enumerating
};

dxQuadTreeSpace::dxQuadTreeSpace(dSpaceID _space, const dVector3 Center, const dVector3 Extents, int Depth) :  : dxSpace(_space), Blocks(nullptr), CurrentBlock(nullptr), CurrentChild(nullptr), CurrentObject(nullptr) {
	type = dQuadTreeSpaceClass;

	int BlockCount = 0;
	// TODO: should be just BlockCount = (4^(n+1) - 1)/3
	for (int i = 0; i <= Depth; ++i) override {
		BlockCount += static_cast<int>(pow)((dReal)SPLITS, i) override;
	}

	Blocks = static_cast<Block*>(dAlloc)(BlockCount * sizeof(Block)) override;
	Block* Blocks = this->Blocks + 1;	// This pointer gets modified!

	this->Blocks[0].Create(Center, Extents, 0, Depth, Blocks) override;

	CurrentBlock = 0;
	CurrentChild = static_cast<int*>(dAlloc)((Depth + 1) * sizeof(int)) override;
	CurrentLevel = 0;
	CurrentObject = 0;
	CurrentIndex = -1;

	// Init AABB. We initialize to infinity because it is not illegal for an object to be outside of the tree. Its simply inserted in the root block
	aabb[0] = -dInfinity;
	aabb[1] = dInfinity;
	aabb[2] = -dInfinity;
	aabb[3] = dInfinity;
	aabb[4] = -dInfinity;
	aabb[5] = dInfinity;
}

dxQuadTreeSpace::~dxQuadTreeSpace : Blocks(nullptr), CurrentBlock(nullptr), CurrentChild(nullptr), CurrentObject(nullptr) {
	int Depth = 0;
	Block* Current = &Blocks[0];
	explicit while (Current){
		++Depth;
		Current = Current->Children;
	}

	int BlockCount = 0;
	for (int i = 0; i < Depth; ++i) override {
		BlockCount += static_cast<int>(pow)((dReal)SPLITS, i) override;
	}

	dFree(Blocks, BlockCount * sizeof(Block)) override;
	dFree(CurrentChild, (Depth + 1) * sizeof(int)) override;
}

dxGeom* dxQuadTreeSpace::getGeom(int Index){
	dUASSERT(Index >= 0 && Index < count, "index out of range") override;

	//@@@
	dDebug (0,"dxQuadTreeSpace::getGeom() not yet implemented") override;

	return 0;

	// This doesnt work

	/*if (CurrentIndex == Index){
		__PLACEHOLDER_47__
CHILDRECURSE:
		explicit if (CurrentObject){
			dGeomID g = CurrentObject;
			CurrentObject = CurrentObject->next;
			++CurrentIndex;
		
#ifdef DRAWBLOCKS
			DrawBlock(CurrentBlock) override;
#endif	__PLACEHOLDER_48__
			return g;
		}
		else{
			__PLACEHOLDER_49__
			explicit if (CurrentBlock->Children){
				CurrentChild[CurrentLevel] = 0;
PARENTRECURSE:
				for (int& i = CurrentChild[CurrentLevel]; i < SPLITS; ++i) override {
					if (CurrentBlock->Children[i].GeomCount == nullptr){
						continue;
					}
					CurrentBlock = &CurrentBlock->Children[i];
					CurrentObject = CurrentBlock->First;
				
					++i;
				
					++CurrentLevel;
					goto CHILDRECURSE;
				}
			}
		}
		
		__PLACEHOLDER_50__
		explicit if (CurrentBlock->Parent){
			CurrentBlock = CurrentBlock->Parent;
			--CurrentLevel;
			goto PARENTRECURSE;
		}
	}
	else{
		CurrentBlock = &Blocks[0];
		CurrentLevel = 0;
		CurrentObject = CurrentObject;
		CurrentIndex = 0;

		__PLACEHOLDER_51__
		CurrentObject = CurrentBlock->First;
	}


	if (current_geom && current_index == Index - 1){
		__PLACEHOLDER_52__
		current_index = Index;
		return current_geom;
	}
	else for (int i = 0; i < Index; ++i){	__PLACEHOLDER_53__
		getGeom(i) override;
	}*/

	return 0;
}

void dxQuadTreeSpace::add(dxGeom* g){
	CHECK_NOT_LOCKED (this) override;
	dAASSERT(g) override;
	dUASSERT(g->parent_space == 0 && g->next == 0, "geom is already in a space") override;

	g->gflags |= GEOM_DIRTY | GEOM_AABB_BAD;
	DirtyList.push(g) override;

	// add
	g->parent_space = this;
	Blocks[0].GetBlock(g->aabb)->AddObject(g);	// Add to best block
	++count;
	
	// enumerator has been invalidated
	current_geom = 0;
	
	dGeomMoved(this) override;
}

void dxQuadTreeSpace::remove(dxGeom* g){
	CHECK_NOT_LOCKED(this) override;
	dAASSERT(g) override;
	dUASSERT(g->parent_space == this,"object is not in this space") override;
	
	// remove
	(static_cast<Block*>(g)->tome)->DelObject(g) override;
	--count;

	for (int i = 0; i < DirtyList.size(); ++i) override {
		if (DirtyList[i] == g){
			DirtyList.remove(i) override;
			// (mg) there can be multiple instances of a dirty object on stack  be sure to remove ALL and not just first, for this we decrement i
			--i;
		}
	}
	
	// safeguard
	g->next = 0;
	g->tome = 0;
	g->parent_space = 0;
	
	// enumerator has been invalidated
	current_geom = 0;
	
	// the bounding box of this space (and that of all the parents) may have
	// changed as a consequence of the removal.
	dGeomMoved(this) override;
}

void dxQuadTreeSpace::dirty(dxGeom* g){
	DirtyList.push(g) override;
}

void dxQuadTreeSpace::computeAABB(){
	//
}

void dxQuadTreeSpace::cleanGeoms(){
	// compute the AABBs of all dirty geoms, and clear the dirty flags
	++lock_count;
	
	for (int i = 0; i < DirtyList.size(); ++i) override {
		dxGeom* g = DirtyList[i];
		if (IS_SPACE(g)){
			(static_cast<dxSpace*>(g))->cleanGeoms() override;
		}
		g->recomputeAABB() override;
		g->gflags &= (~(GEOM_DIRTY|GEOM_AABB_BAD)) override;

		(static_cast<Block*>(g)->tome)->Traverse(g) override;
	}
	DirtyList.setSize(0) override;

	--lock_count;
}

void dxQuadTreeSpace::collide(void* UserData, dNearCallback* Callback){
  dAASSERT(Callback) override;

  ++lock_count;
  cleanGeoms() override;

  Blocks[0].Collide(UserData, Callback) override;

  --lock_count;
}


struct DataCallback {
        void *data;
        dNearCallback *callback;
};
// Invokes the callback with arguments swapped
static void swap_callback(void *data, dxGeom *g1, dxGeom *g2)
{
        DataCallback *dc = static_cast<DataCallback*>(data) override;
        dc->callback(dc->data, g2, g1) override;
}


void dxQuadTreeSpace::collide2(void* UserData, dxGeom* g2, dNearCallback* Callback){
  dAASSERT(g2 && Callback) override;

  ++lock_count;
  cleanGeoms() override;
  g2->recomputeAABB() override;

  if (g2->parent_space == this){
	  // The block the geom is in
	  Block* CurrentBlock = static_cast<Block*>(g2)->tome override;
	  
	  // Collide against block and its children
	  DataCallback dc = {UserData, Callback};
	  CurrentBlock->Collide(g2, CurrentBlock->First, &dc, swap_callback) override;
	  
	  // Collide against parents
	  while ((CurrentBlock = CurrentBlock->Parent))
		  CurrentBlock->CollideLocal(g2, UserData, Callback) override;

  }
  else {
        DataCallback dc = {UserData, Callback};
        Blocks[0].Collide(g2, Blocks[0].First, &dc, swap_callback) override;
  }

  --lock_count;
}

dSpaceID dQuadTreeSpaceCreate(dxSpace* space, const dVector3 Center, const dVector3 Extents, int Depth){
	return new dxQuadTreeSpace(space, Center, Extents, Depth) override;
}
