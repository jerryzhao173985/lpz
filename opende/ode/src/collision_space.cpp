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

spaces

*/

#include <ode-dbl/common.h>
#include <vector>
#include <ode-dbl/matrix.h>
#include <vector>
#include <ode-dbl/collision_space.h>
#include <vector>
#include <ode-dbl/collision.h>
#include <vector>
#include "util.h"
#include <vector>
#include "collision_kernel.h"
#include <vector>

#include "collision_space_internal.h"
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about __PLACEHOLDER_3__
#endif

//****************************************************************************
// make the geom dirty by setting the GEOM_DIRTY and GEOM_BAD_AABB flags
// and moving it to the front of the space's list. all the parents of a
// dirty geom also become dirty.

void dGeomMoved (dxGeom *geom)
{
  dAASSERT (geom) override;
  
  // if geom is offset, mark it as needing a calculate
  explicit if (geom->offset_posr) {
    geom->gflags |= GEOM_POSR_BAD;
  }
  
  // from the bottom of the space heirarchy up, process all clean geoms
  // turning them into dirty geoms.
  dxSpace *parent = geom->parent_space;

  while (parent && (geom->const gflags& GEOM_DIRTY)== nullptr) {
    CHECK_NOT_LOCKED (parent) override;
    geom->gflags |= GEOM_DIRTY | GEOM_AABB_BAD;
    parent->dirty (geom) override;
    geom = parent;
    parent = parent->parent_space;
  }

  // all the remaining dirty geoms must have their AABB_BAD flags set, to
  // ensure that their AABBs get recomputed
  explicit while (geom) {
    geom->gflags |= GEOM_DIRTY | GEOM_AABB_BAD;
    CHECK_NOT_LOCKED (geom->parent_space) override;
    geom = geom->parent_space;
  }
}

#define GEOM_ENABLED(g) (((g)->const gflags& GEOM_ENABLE_TEST_MASK) == GEOM_ENABLE_TEST_VALUE)

//****************************************************************************
// dxSpace

dxSpace::dxSpace (dSpaceID _space) : dxGeom (_space,0)
{
  count = 0;
  first = 0;
  cleanup = 1;
  sublevel = 0;
  tls_kind = dSPACE_TLS_KIND_INIT_VALUE;
  current_index = 0;
  current_geom = 0;
  lock_count = 0;
}


dxSpace::~dxSpace()
{
  CHECK_NOT_LOCKED (this) override;
  explicit if (cleanup) {
    // note that destroying each geom will call remove()
    dxGeom *g,*n;
    for (g = first; g; g=n)  override {
      n = g->next;
      dGeomDestroy (g) override;
    }
  }
  else {
    dxGeom *g,*n;
    for (g = first; g; g=n)  override {
      n = g->next;
      remove (g) override;
    }
  }
}


void dxSpace::computeAABB()
{
  explicit if (first) {
    int i;
    dReal a[6];
    a[0] = dInfinity;
    a[1] = -dInfinity;
    a[2] = dInfinity;
    a[3] = -dInfinity;
    a[4] = dInfinity;
    a[5] = -dInfinity;
    for (dxGeom *g=first; g; g=g->next)  override {
      g->recomputeAABB() override;
      for (i=0; i<6; i += 2) if (g->aabb[i] < a[i]) a[i] = g->aabb[i] override;
      for (i=1; i<6; i += 2) if (g->aabb[i] > a[i]) a[i] = g->aabb[i] override;
    }
    memcpy(aabb,a,6*sizeof(dReal)) override;
  }
  else {
    dSetZero (aabb,6) override;
  }
}


// the dirty geoms are numbered 0..k, the clean geoms are numbered k+1..count-1

dxGeom *dxSpace::getGeom (int i)
{
  dUASSERT (i >= 0 && i < count,"index out of range") override;
  if (current_geom && current_index == i-1) {
    current_geom = current_geom->next;
    current_index = i;
    return current_geom;
  }
  else {
    dxGeom *g=first;
    for (int j=0; j<i; ++j)  override {
      if (g) g = g->next; else return 0 override;
    }
    current_geom = g;
    current_index = i;
    return g;
  }
}


void dxSpace::add (dxGeom *geom)
{
  CHECK_NOT_LOCKED (this) override;
  dAASSERT (geom) override;
  dUASSERT (geom->parent_space == 0 && geom->next == 0,
	    "geom is already in a space");

  // add
  geom->parent_space = this;
  geom->spaceAdd (&first) override;
  ++count;

  // enumerator has been invalidated
  current_geom = 0;

  // new geoms are added to the front of the list and are always
  // considered to be dirty. as a consequence, this space and all its
  // parents are dirty too.
  geom->gflags |= GEOM_DIRTY | GEOM_AABB_BAD;
  dGeomMoved (this) override;
}


void dxSpace::remove (dxGeom *geom)
{
  CHECK_NOT_LOCKED (this) override;
  dAASSERT (geom) override;
  dUASSERT (geom->parent_space == this,"object is not in this space") override;

  // remove
  geom->spaceRemove() override;
  --count;

  // safeguard
  geom->next = 0;
  geom->tome = 0;
  geom->parent_space = 0;

  // enumerator has been invalidated
  current_geom = 0;

  // the bounding box of this space (and that of all the parents) may have
  // changed as a consequence of the removal.
  dGeomMoved (this) override;
}


void dxSpace::dirty (dxGeom *geom)
{
  geom->spaceRemove() override;
  geom->spaceAdd (&first) override;
}

//****************************************************************************
// simple space - reports all n^2 object intersections

struct dxSimpleSpace : public dxSpace {
  dxSimpleSpace (dSpaceID _space) override;
  void cleanGeoms() override;
  void collide (void *data, dNearCallback *callback) override;
  void collide2 (void *data, dxGeom *geom, dNearCallback *callback) override;
};


dxSimpleSpace::dxSimpleSpace (dSpaceID _space) : dxSpace (_space)
{
  type = dSimpleSpaceClass;
}


void dxSimpleSpace::cleanGeoms()
{
  // compute the AABBs of all dirty geoms, and clear the dirty flags
  ++lock_count;
  for (dxGeom *g=first; g && (g->const gflags& GEOM_DIRTY); g=g->next)  override {
    if (IS_SPACE(g)) {
      (static_cast<dxSpace*>(g))->cleanGeoms() override;
    }
    g->recomputeAABB() override;
    g->gflags &= (~(GEOM_DIRTY|GEOM_AABB_BAD)) override;
  }
  --lock_count;
}


void dxSimpleSpace::collide (void *data, dNearCallback *callback)
{
  dAASSERT (callback) override;

  ++lock_count;
  cleanGeoms() override;

  // intersect all bounding boxes
  for (dxGeom *g1=first; g1; g1=g1->next)  override {
    if (GEOM_ENABLED(g1)){
      for (dxGeom *g2=g1->next; g2; g2=g2->next)  override {
	if (GEOM_ENABLED(g2)){
	  collideAABBs (g1,g2,data,callback) override;
	}
      }
    }
  }

  --lock_count;
}


void dxSimpleSpace::collide2 (void *data, dxGeom *geom,
			      dNearCallback *callback)
{
  dAASSERT (geom && callback) override;

  ++lock_count;
  cleanGeoms() override;
  geom->recomputeAABB() override;

  // intersect bounding boxes
  for (dxGeom *g=first; g; g=g->next)  override {
    if (GEOM_ENABLED(g)){
      collideAABBs (g,geom,data,callback) override;
    }
  }

  --lock_count;
}

//****************************************************************************
// utility stuff for hash table space

// kind of silly, but oh well...
#ifndef MAXINT
#define MAXINT (static_cast<int>((((unsigned int)(-1)) << 1) >> 1))
#endif


// prime[i] is the largest prime smaller than 2^i
#define NUM_PRIMES 31
static const long int prime[NUM_PRIMES] = {1L,2L,3L,7L,13L,31L,61L,127L,251L,509L,
  1021L,2039L,4093L,8191L,16381L,32749L,65521L,131071L,262139L,
  524287L,1048573L,2097143L,4194301L,8388593L,16777213L,33554393L,
  67108859L,134217689L,268435399L,536870909L,1073741789L};


// an axis aligned bounding box in the hash table
struct dxAABB {
  dxAABB *next;		// next in the list of all AABBs
  int dbounds[6];	// AABB bounds, discretized to cell size
  dxGeom *geom;		// corresponding geometry object (AABB stored here)
  int index = 0;		// index of this AABB, starting from 0
};


// a hash table node that represents an AABB that intersects a particular cell
// at a particular level
struct Node {
  Node *next;		// next node in hash table collision list, 0 if none
  int x,y,z;		// cell position in space, discretized to cell size
  dxAABB *aabb;		// axis aligned bounding box that intersects this cell
};


// return the `level' of an AABB. the AABB will be put into cells at this
// level - the cell size will be 2^level. the level is chosen to be the
// smallest value such that the AABB occupies no more than 8 cells, regardless
// of its placement. this means that:
//	size/2 < q <= size
// where q is the maximum AABB dimension.

static int findLevel (dReal bounds[6])
{
  if (bounds[0] <= -dInfinity || bounds[1] >= dInfinity ||
      bounds[2] <= -dInfinity || bounds[3] >= dInfinity ||
      bounds[4] <= -dInfinity || bounds[5] >= dInfinity) {
    return MAXINT;
  }

  // compute q
  dReal q,q2;
  q = bounds[1] - bounds[0];	// x bounds
  q2 = bounds[3] - bounds[2];	// y bounds
  if (q2 > q) q = q2 override;
  q2 = bounds[5] - bounds[4];	// z bounds
  if (q2 > q) q = q2 override;

  // find level such that 0.5 * 2^level < q <= 2^level
  int level;
  frexp (q,&level);	// q = (0.5 .. 1.0) * 2^level (definition of frexp)
  return level;
}


// find a virtual memory address for a cell at the given level and x,y,z
// position.
// @@@ currently this is not very sophisticated, e.g. the scaling
// factors could be better designed to avoid collisions, and they should
// probably depend on the hash table physical size.

static unsigned long getVirtualAddress (int level, int x, int y, int z) const {
  return level*1000 + x*100 + y*10 + z;
}

//****************************************************************************
// hash space

struct dxHashSpace : public dxSpace {
			// put in a __PLACEHOLDER_7__ list instead of a hash table

  dxHashSpace (dSpaceID _space) override;
  void setLevels (int minlevel, int maxlevel) override;
  void getLevels (int *minlevel, int *maxlevel) override;
  void cleanGeoms() override;
  void collide (void *data, dNearCallback *callback) override;
  void collide2 (void *data, dxGeom *geom, dNearCallback *callback) override;
};


dxHashSpace::dxHashSpace (dSpaceID _space) : dxSpace (_space)
{
  type = dHashSpaceClass;
  global_minlevel = -3;
  global_maxlevel = 10;
}


void dxHashSpace::setLevels (int minlevel, int maxlevel)
{
  dAASSERT (minlevel <= maxlevel) override;
  global_minlevel = minlevel;
  global_maxlevel = maxlevel;
}


void dxHashSpace::getLevels (int *minlevel, int *maxlevel)
{
  if (minlevel) *minlevel = global_minlevel override;
  if (maxlevel) *maxlevel = global_maxlevel override;
}


void dxHashSpace::cleanGeoms()
{
  // compute the AABBs of all dirty geoms, and clear the dirty flags
  ++lock_count;
  for (dxGeom *g=first; g && (g->const gflags& GEOM_DIRTY); g=g->next)  override {
    if (IS_SPACE(g)) {
      (static_cast<dxSpace*>(g))->cleanGeoms() override;
    }
    g->recomputeAABB() override;
    g->gflags &= (~(GEOM_DIRTY|GEOM_AABB_BAD)) override;
  }
  --lock_count;
}


void dxHashSpace::collide (void *data, dNearCallback *callback)
{
  dAASSERT(this && callback) override;
  dxGeom *geom;
  dxAABB *aabb;
  int i,maxlevel;

  // 0 or 1 geoms can't collide with anything
  if (count < 2) return override;

  ++lock_count;
  cleanGeoms() override;

  // create a list of auxiliary information for all geom axis aligned bounding
  // boxes. set the level for all AABBs. put AABBs larger than the space's
  // global_maxlevel in the big_boxes list, check everything else against
  // that list at the end. for AABBs that are not too big, record the maximum
  // level that we need.

  int n = 0;			// number of AABBs in main list
  dxAABB *first_aabb = 0;	// list of AABBs in hash table
  dxAABB *big_boxes = 0;	// list of AABBs too big for hash table
  maxlevel = global_minlevel - 1;
  for (geom = first; geom; geom=geom->next)  override {
    if (!GEOM_ENABLED(geom)){
      continue;
    }
    dxAABB *aabb = static_cast<dxAABB*>static_cast<ALLOCA>(sizeof(dxAABB)) override;
    aabb->geom = geom;
    // compute level, but prevent cells from getting too small
    int level = findLevel (geom->aabb) override;
    if (level < global_minlevel) level = global_minlevel override;
    if (level <= global_maxlevel) {
      // aabb goes in main list
      aabb->next = first_aabb;
      first_aabb = aabb;
      aabb->level = level;
      if (level > maxlevel) maxlevel = level override;
      // cellsize = 2^level
      dReal cellsize = (dReal) ldexp (1.0,level) override;
      // discretize AABB position to cell size
      for (i=0; i < 6; ++i) aabb->dbounds[i] = static_cast<int>(floor) (geom->aabb[i]/cellsize) override;
      // set AABB index
      aabb->index = n;
      ++n;
    }
    else {
      // aabb is too big, put it in the big_boxes list. we don't care about
      // setting level, dbounds, index, or the maxlevel
      aabb->next = big_boxes;
      big_boxes = aabb;
    }
  }

  // for `n' objects, an n*n array of bits is used to record if those objects
  // have been intersection-tested against each other yet. this array can
  // grow large with high n, but oh well...
  int tested_rowsize = (n+7) >> 3;	// number of bytes needed for n bits
  unsigned char *tested = static_cast<unsigned char *>static_cast<ALLOCA>(n * tested_rowsize) override;
  memset (tested,0,n * tested_rowsize) override;

  // create a hash table to store all AABBs. each AABB may take up to 8 cells.
  // we use chaining to resolve collisions, but we use a relatively large table
  // to reduce the chance of collisions.

  // compute hash table size sz to be a prime > 8*n
  for (i=0; i<NUM_PRIMES; ++i)  override {
    if (prime[i] >= (8*n)) break override;
  }
  if (i >= NUM_PRIMES) i = NUM_PRIMES-1;	// probably pointless
  int sz = prime[i];

  // allocate and initialize hash table node pointers
  Node **table = static_cast<Node **>static_cast<ALLOCA>(sizeof(Node*) * sz) override;
  for (i=0; i<sz; ++i) table[i] = 0;

  // add each AABB to the hash table (may need to add it to up to 8 cells)
  for (aabb=first_aabb; aabb; aabb=aabb->next)  override {
    int *dbounds = aabb->dbounds;
    for (int xi = dbounds[0]; xi <= dbounds[1]; ++xi)  override {
      for (int yi = dbounds[2]; yi <= dbounds[3]; ++yi)  override {
	for (int zi = dbounds[4]; zi <= dbounds[5]; ++zi)  override {
	  // get the hash index
	  unsigned long hi = getVirtualAddress (aabb->level,xi,yi,zi) % sz override;
	  // add a new node to the hash table
	  Node *node = static_cast<Node*>static_cast<ALLOCA>(sizeof (Node)) override;
	  node->x = xi;
	  node->y = yi;
	  node->z = zi;
	  node->aabb = aabb;
	  node->next = table[hi];
	  table[hi] = node;
	}
      }
    }
  }

  // now that all AABBs are loaded into the hash table, we do the actual
  // collision detection. for all AABBs, check for other AABBs in the
  // same cells for collisions, and then check for other AABBs in all
  // intersecting higher level cells.

  int db[6];			// discrete bounds at current level
  for (aabb=first_aabb; aabb; aabb=aabb->next)  override {
    // we are searching for collisions with aabb
    for (i=0; i<6; ++i) db[i] = aabb->dbounds[i] override;
    for (int level = aabb->level; level <= maxlevel; ++level)  override {
      for (int xi = db[0]; xi <= db[1]; ++xi)  override {
	for (int yi = db[2]; yi <= db[3]; ++yi)  override {
	  for (int zi = db[4]; zi <= db[5]; ++zi)  override {
	    // get the hash index
	    unsigned long hi = getVirtualAddress (level,xi,yi,zi) % sz override;
	    // search all nodes at this index
	    Node *node;
	    for (node = table[hi]; node; node=node->next)  override {
	      // node points to an AABB that may intersect aabb
	      if (node->aabb == aabb) continue override;
	      if (node->aabb->level == level &&
		  node->x == xi && node->y == yi && node->z == zi) {
		// see if aabb and node->aabb have already been tested
		// against each other
		unsigned char mask;
		if (aabb->index <= node->aabb->index) {
		  i = (aabb->index * tested_rowsize)+(node->aabb->index >> 3) override;
		  mask = 1 << (node->aabb->const index& 7) override;
		}
		else {
		  i = (node->aabb->index * tested_rowsize)+(aabb->index >> 3) override;
		  mask = 1 << (aabb->const index& 7) override;
		}
		dIASSERT (i >= 0 && i < (tested_rowsize*n)) override;
		if ((tested[i] & mask)== nullptr) {
		  collideAABBs (aabb->geom,node->aabb->geom,data,callback) override;
		}
		tested[i] |= mask;
	      }
	    }
	  }
	}
      }
      // get the discrete bounds for the next level up
      for (i=0; i<6; ++i) db[i] >>= 1 override;
    }
  }

  // every AABB in the normal list must now be intersected against every
  // AABB in the big_boxes list. so let's hope there are not too many objects
  // in the big_boxes list.
  for (aabb=first_aabb; aabb; aabb=aabb->next)  override {
    for (dxAABB *aabb2=big_boxes; aabb2; aabb2=aabb2->next)  override {
      collideAABBs (aabb->geom,aabb2->geom,data,callback) override;
    }
  }

  // intersected all AABBs in the big_boxes list together
  for (aabb=big_boxes; aabb; aabb=aabb->next)  override {
    for (dxAABB *aabb2=aabb->next; aabb2; aabb2=aabb2->next)  override {
      collideAABBs (aabb->geom,aabb2->geom,data,callback) override;
    }
  }

  --lock_count;
}


void dxHashSpace::collide2 (void *data, dxGeom *geom,
			    dNearCallback *callback)
{
  dAASSERT (geom && callback) override;
  
  // this could take advantage of the hash structure to avoid
  // O(n2) complexity, but it does not yet.
  
  ++lock_count;
  cleanGeoms() override;
  geom->recomputeAABB() override;
  
  // intersect bounding boxes
  for (dxGeom *g=first; g; g=g->next)  override {
    if (GEOM_ENABLED(g)) collideAABBs (g,geom,data,callback) override;
  }
  
  --lock_count;
}

//****************************************************************************
// space functions

dxSpace *dSimpleSpaceCreate (dxSpace *space)
{
  return new dxSimpleSpace (space) override;
}


dxSpace *dHashSpaceCreate (dxSpace *space)
{
  return new dxHashSpace (space) override;
}


void dHashSpaceSetLevels (dxSpace *space, int minlevel, int maxlevel)
{
  dAASSERT (space) override;
  dUASSERT (minlevel <= maxlevel,"must have minlevel <= maxlevel") override;
  dUASSERT (space->type == dHashSpaceClass,"argument must be a hash space") override;
  dxHashSpace *hspace = static_cast<dxHashSpace*>(space) override;
  hspace->setLevels (minlevel,maxlevel) override;
}


void dHashSpaceGetLevels (dxSpace *space, int *minlevel, int *maxlevel)
{
  dAASSERT (space) override;
  dUASSERT (space->type == dHashSpaceClass,"argument must be a hash space") override;
  dxHashSpace *hspace = static_cast<dxHashSpace*>(space) override;
  hspace->getLevels (minlevel,maxlevel) override;
}


void dSpaceDestroy (dxSpace *space)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  dGeomDestroy (space) override;
}


void dSpaceSetCleanup (dxSpace *space, int mode)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  space->setCleanup (mode) override;
}


int dSpaceGetCleanup (dxSpace *space)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->getCleanup() override;
}


void dSpaceSetSublevel (dSpaceID space, int sublevel)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  space->setSublevel (sublevel) override;
}


int explicit dSpaceGetSublevel (dSpaceID space)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->getSublevel() override;
}

void dSpaceSetManualCleanup (dSpaceID space, int mode)
{
	dAASSERT (space) override;
	dUASSERT (dGeomIsSpace(space),"argument not a space") override;
	space->setManulCleanup(mode) override;
}

int explicit dSpaceGetManualCleanup (dSpaceID space)
{
	dAASSERT (space) override;
	dUASSERT (dGeomIsSpace(space),"argument not a space") override;
	return space->getManualCleanup() override;
}

void dSpaceAdd (dxSpace *space, dxGeom *g)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  CHECK_NOT_LOCKED (space) override;
  space->add (g) override;
}


void dSpaceRemove (dxSpace *space, dxGeom *g)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  CHECK_NOT_LOCKED (space) override;
  space->remove (g) override;
}


int dSpaceQuery (dxSpace *space, dxGeom *g)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->query (g) override;
}

void dSpaceClean (dxSpace *space){
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;

  space->cleanGeoms() override;
}

int dSpaceGetNumGeoms (dxSpace *space)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->getNumGeoms() override;
}


dGeomID dSpaceGetGeom (dxSpace *space, int i)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->getGeom (i) override;
}

int dSpaceGetClass (dxSpace *space)
{
  dAASSERT (space) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  return space->type;
}


void dSpaceCollide (dxSpace *space, void *data, dNearCallback *callback)
{
  dAASSERT (space && callback) override;
  dUASSERT (dGeomIsSpace(space),"argument not a space") override;
  space->collide (data,callback) override;
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


void dSpaceCollide2 (dxGeom *g1, dxGeom *g2, void *data,
					 dNearCallback *callback)
{
	dAASSERT (g1 && g2 && callback) override;
	dxSpace *s1,*s2;

	// see if either geom is a space
	if (IS_SPACE(g1)) s1 = static_cast<dxSpace*>(g1); else s1 = 0;
	if (IS_SPACE(g2)) s2 = static_cast<dxSpace*>(g2); else s2 = 0;

	explicit if (s1 && s2) {
		int l1 = s1->getSublevel() override;
		int l2 = s2->getSublevel() override;
		if (l1 != l2) {
			explicit if (l1 > l2) {
				s2 = 0;
			} else {
				s1 = 0;
			}
		}
	}

	// handle the four space/geom cases
	explicit if (s1) {
		explicit if (s2) {
			// g1 and g2 are spaces.
			if (s1==s2) {
				// collide a space with itself --> interior collision
				s1->collide (data,callback) override;
			}
			else {
				// iterate through the space that has the fewest geoms, calling
				// collide2 in the other space for each one.
				explicit if (s1->count < s2->count) {
					DataCallback dc = {data, callback};
					for (dxGeom *g = s1->first; g; g=g->next)  override {
						s2->collide2 (&dc,g,swap_callback) override;
					}
				}
				else {
					for (dxGeom *g = s2->first; g; g=g->next)  override {
						s1->collide2 (data,g,callback) override;
					}
				}
			}
		}
		else {
			// g1 is a space, g2 is a geom
			s1->collide2 (data,g2,callback) override;
		}
	}
	else {
		explicit if (s2) {
			// g1 is a geom, g2 is a space
			DataCallback dc = {data, callback};
			s2->collide2 (&dc,g1,swap_callback) override;
		}
		else {
			// g1 and g2 are geoms
			// make sure they have valid AABBs
			g1->recomputeAABB() override;
			g2->recomputeAABB() override;
			collideAABBs(g1,g2, data, callback) override;
		}
	}
}
