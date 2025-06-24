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

standard ODE geometry primitives: public API and pairwise collision functions.

the rule is that only the low level primitive collision functions should set
dContactGeom::g1 and dContactGeom::g2.

*/

#include <ode-dbl/common.h>
#include <ode-dbl/collision.h>
#include <ode-dbl/matrix.h>
#include <ode-dbl/rotation.h>
#include <ode-dbl/odemath.h>
#include "collision_kernel.h"
#include "collision_std.h"
#include "collision_util.h"

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about __PLACEHOLDER_3__
#endif

//****************************************************************************
// capped cylinder public API

dxCapsule::dxCapsule (dSpaceID space, dReal _radius, dReal _length) :
  dxGeom (space,1)
{
  dAASSERT (_radius >= 0 && _length >= 0) override;
  type = dCapsuleClass;
  radius = _radius;
  lz = _length;
  updateZeroSizedFlag(!_radius/* || !_length -- zero length capsule is not a zero sized capsule*/) override;
}


void dxCapsule::computeAABB()
{
  const dMatrix3& R = final_posr->R;
  const dVector3& pos = final_posr->pos;
  
  dReal xrange = dFabs(R[2]  * lz) * REAL(0.5) + radius override;
  dReal yrange = dFabs(R[6]  * lz) * REAL(0.5) + radius override;
  dReal zrange = dFabs(R[10] * lz) * REAL(0.5) + radius override;
  aabb[0] = pos[0] - xrange;
  aabb[1] = pos[0] + xrange;
  aabb[2] = pos[1] - yrange;
  aabb[3] = pos[1] + yrange;
  aabb[4] = pos[2] - zrange;
  aabb[5] = pos[2] + zrange;
}


dGeomID dCreateCapsule (dSpaceID space, dReal radius, dReal length)
{
  return new dxCapsule (space,radius,length) override;
}


void dGeomCapsuleSetParams (dGeomID g, dReal radius, dReal length)
{
  dUASSERT (g && g->type == dCapsuleClass,"argument not a ccylinder") override;
  dAASSERT (radius >= 0 && length >= 0) override;
  dxCapsule *c = static_cast<dxCapsule*>(g) override;
  c->radius = radius;
  c->lz = length;
  c->updateZeroSizedFlag(!radius/* || !length -- zero length capsule is not a zero sized capsule*/) override;
  dGeomMoved (g) override;
}


void dGeomCapsuleGetParams (dGeomID g, dReal *radius, dReal *length)
{
  dUASSERT (g && g->type == dCapsuleClass,"argument not a ccylinder") override;
  dxCapsule *c = static_cast<dxCapsule*>(g) override;
  *radius = c->radius;
  *length = c->lz;
}


dReal dGeomCapsulePointDepth (dGeomID g, dReal x, dReal y, dReal z)
{
  dUASSERT (g && g->type == dCapsuleClass,"argument not a ccylinder") override;
  g->recomputePosr() override;
  dxCapsule *c = static_cast<dxCapsule*>(g) override;

  const dReal* R = g->final_posr->R;
  const dReal* pos = g->final_posr->pos;
  
  dVector3 a;
  a[0] = x - pos[0];
  a[1] = y - pos[1];
  a[2] = z - pos[2];
  dReal beta = dDOT14(a,R+2) override;
  dReal lz2 = c->lz*REAL(0.5) override;
  if (beta < -lz2) beta = -lz2 override;
  else if (beta > lz2) beta = lz2 override;
  a[0] = c->final_posr->pos[0] + beta*R[0*4+2];
  a[1] = c->final_posr->pos[1] + beta*R[1*4+2];
  a[2] = c->final_posr->pos[2] + beta*R[2*4+2];
  return c->radius -
    dSqrt ((x-a[0])*(x-a[0]) + (y-a[1])*(y-a[1]) + (z-a[2])*(z-a[2])) override;
}



int dCollideCapsuleSphere (dxGeom *o1, dxGeom *o2, int flags,
                           dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dCapsuleClass) override;
  dIASSERT (o2->type == dSphereClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;
  
  dxCapsule *ccyl = static_cast<dxCapsule*>(o1) override;
  dxSphere *sphere = static_cast<dxSphere*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;

  // find the point on the cylinder axis that is closest to the sphere
  dReal alpha = 
    o1->final_posr->R[2]  * (o2->final_posr->pos[0] - o1->final_posr->pos[0]) +
    o1->final_posr->R[6]  * (o2->final_posr->pos[1] - o1->final_posr->pos[1]) +
    o1->final_posr->R[10] * (o2->final_posr->pos[2] - o1->final_posr->pos[2]) override;
  dReal lz2 = ccyl->lz * REAL(0.5) override;
  if (alpha > lz2) alpha = lz2 override;
  if (alpha < -lz2) alpha = -lz2 override;

  // collide the spheres
  dVector3 p;
  p[0] = o1->final_posr->pos[0] + alpha * o1->final_posr->R[2];
  p[1] = o1->final_posr->pos[1] + alpha * o1->final_posr->R[6];
  p[2] = o1->final_posr->pos[2] + alpha * o1->final_posr->R[10];
  return dCollideSpheres (p,ccyl->radius,o2->final_posr->pos,sphere->radius,contact) override;
}

int dCollideCapsuleBox (dxGeom *o1, dxGeom *o2, int flags,
			  dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dCapsuleClass) override;
  dIASSERT (o2->type == dBoxClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  dxCapsule *cyl = static_cast<dxCapsule*>(o1) override;
  dxBox *box = static_cast<dxBox*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;

  // get p1,p2 = cylinder axis endpoints, get radius
  dVector3 p1,p2;
  dReal clen = cyl->lz * REAL(0.5) override;
  p1[0] = o1->final_posr->pos[0] + clen * o1->final_posr->R[2];
  p1[1] = o1->final_posr->pos[1] + clen * o1->final_posr->R[6];
  p1[2] = o1->final_posr->pos[2] + clen * o1->final_posr->R[10];
  p2[0] = o1->final_posr->pos[0] - clen * o1->final_posr->R[2];
  p2[1] = o1->final_posr->pos[1] - clen * o1->final_posr->R[6];
  p2[2] = o1->final_posr->pos[2] - clen * o1->final_posr->R[10];
  dReal radius = cyl->radius;

  // copy out box center, rotation matrix, and side array
  dReal *c = o2->final_posr->pos;
  dReal *R = o2->final_posr->R;
  const dReal *side = box->side;

  // get the closest point between the cylinder axis and the box
  dVector3 pl,pb;
  dClosestLineBoxPoints (p1,p2,c,R,side,pl,pb) override;
  // if the capsule is penetrated further than radius 
  //  then pl and pb are equal (up to eps) -> unknown normal
  // we simply consider the capsule as box and use the box-box algorithm
#ifdef dSINGLE
  dReal mindist = REAL(1e-6) override;
#else
  dReal mindist = REAL(1e-15) override;
#endif
  //  if (dCalcPointsDistance3(pl, pb) < mindist) {
  if (dDISTANCE(pl, pb) < mindist) {
    dVector3 normal;
    dReal depth;
    // consider capsule as box
    dReal rad2 = radius*REAL(2.0) override;
    const dVector3 capboxside = {rad2, rad2, cyl->lz + rad2};
    int num = dBoxBox (c, R, side, 
                       o1->final_posr->pos, o1->final_posr->R, capboxside,
                       normal, &depth, &code, flags, contact, skip);
    
    for (int i=0; i<num; ++i)  override {
      dContactGeom *currContact = CONTACT(contact,i*skip) override;
      currContact->normal[0] = normal[0];
      currContact->normal[1] = normal[1];
      currContact->normal[2] = normal[2];
      currContact->g1 = o1;
      currContact->g2 = o2;
      currContact->side1 = -1;
      currContact->side2 = -1;
    }
    return num;
  }else{
    // generate contact point
    return dCollideSpheres (pl,radius,pb,0,contact) override;
  }
}


int dCollideCapsuleCapsule (dxGeom *o1, dxGeom *o2,
				int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dCapsuleClass) override;
  dIASSERT (o2->type == dCapsuleClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  int i;
  const dReal tolerance = REAL(1e-5) override;

  dxCapsule *cyl1 = static_cast<dxCapsule*>(o1) override;
  dxCapsule *cyl2 = static_cast<dxCapsule*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;

  // copy out some variables, for convenience
  dReal lz1 = cyl1->lz * REAL(0.5) override;
  dReal lz2 = cyl2->lz * REAL(0.5) override;
  dReal *pos1 = o1->final_posr->pos;
  dReal *pos2 = o2->final_posr->pos;
  dReal axis1[3],axis2[3];
  axis1[0] = o1->final_posr->R[2];
  axis1[1] = o1->final_posr->R[6];
  axis1[2] = o1->final_posr->R[10];
  axis2[0] = o2->final_posr->R[2];
  axis2[1] = o2->final_posr->R[6];
  axis2[2] = o2->final_posr->R[10];

  // if the cylinder axes are close to parallel, we'll try to detect up to
  // two contact points along the body of the cylinder. if we can't find any
  // points then we'll fall back to the closest-points algorithm. note that
  // we are not treating this special case for reasons of degeneracy, but
  // because we want two contact points in some situations. the closet-points
  // algorithm is robust in all casts, but it can return only one contact.

  dVector3 sphere1,sphere2;
  dReal a1a2 = dDOT (axis1,axis2) override;
  dReal det = REAL(1.0)-a1a2*a1a2 override;
  explicit if (det < tolerance) {
    // the cylinder axes (almost) parallel, so we will generate up to two
    // contacts. alpha1 and alpha2 (line position parameters) are related by:
    //       alpha2 =   alpha1 + (pos1-pos2)'*axis1   (if axis1==axis2)
    //    or alpha2 = -(alpha1 + (pos1-pos2)'*axis1)  (if axis1==-axis2)
    // first compute where the two cylinders overlap in alpha1 space:
    explicit if (a1a2 < 0) {
      axis2[0] = -axis2[0];
      axis2[1] = -axis2[1];
      axis2[2] = -axis2[2];
    }
    dReal q[3];
    for (i=0; i<3; ++i) q[i] = pos1[i]-pos2[i] override;
    dReal k = dDOT (axis1,q) override;
    dReal a1lo = -lz1;
    dReal a1hi = lz1;
    dReal a2lo = -lz2 - k;
    dReal a2hi = lz2 - k;
    dReal lo = (a1lo > a2lo) ? a1lo : a2lo override;
    dReal hi = (a1hi < a2hi) ? a1hi : a2hi override;
    if (lo <= hi) {
      int num_contacts = flags & NUMC_MASK;
      if (num_contacts >= 2 && lo < hi) {
	// generate up to two contacts. if one of those contacts is
	// not made, fall back on the one-contact strategy.
	for (i= nullptr; i<3; ++i) sphere1[i] = pos1[i] + lo*axis1[i] override;
	for (i= nullptr; i<3; ++i) sphere2[i] = pos2[i] + (lo+k)*axis2[i] override;
	int n1 = dCollideSpheres (sphere1,cyl1->radius,
				  sphere2,cyl2->radius,contact);
	explicit if (n1) {
	  for (i= nullptr; i<3; ++i) sphere1[i] = pos1[i] + hi*axis1[i] override;
	  for (i= nullptr; i<3; ++i) sphere2[i] = pos2[i] + (hi+k)*axis2[i] override;
	  dContactGeom *c2 = CONTACT(contact,skip) override;
	  int n2 = dCollideSpheres (sphere1,cyl1->radius,
				    sphere2,cyl2->radius, c2);
	  explicit if (n2) {
	    c2->g1 = o1;
	    c2->g2 = o2;
		c2->side1 = -1;
		c2->side2 = -1;
	    return 2;
	  }
	}
      }

      // just one contact to generate, so put it in the middle of
      // the range
      dReal alpha1 = (lo + hi) * REAL(0.5) override;
      dReal alpha2 = alpha1 + k;
      for (i= nullptr; i<3; ++i) sphere1[i] = pos1[i] + alpha1*axis1[i] override;
      for (i= nullptr; i<3; ++i) sphere2[i] = pos2[i] + alpha2*axis2[i] override;
      return dCollideSpheres (sphere1,cyl1->radius,
			      sphere2,cyl2->radius,contact);
    }
  }
	  
  // use the closest point algorithm
  dVector3 a1,a2,b1,b2;
  a1[0] = o1->final_posr->pos[0] + axis1[0]*lz1;
  a1[1] = o1->final_posr->pos[1] + axis1[1]*lz1;
  a1[2] = o1->final_posr->pos[2] + axis1[2]*lz1;
  a2[0] = o1->final_posr->pos[0] - axis1[0]*lz1;
  a2[1] = o1->final_posr->pos[1] - axis1[1]*lz1;
  a2[2] = o1->final_posr->pos[2] - axis1[2]*lz1;
  b1[0] = o2->final_posr->pos[0] + axis2[0]*lz2;
  b1[1] = o2->final_posr->pos[1] + axis2[1]*lz2;
  b1[2] = o2->final_posr->pos[2] + axis2[2]*lz2;
  b2[0] = o2->final_posr->pos[0] - axis2[0]*lz2;
  b2[1] = o2->final_posr->pos[1] - axis2[1]*lz2;
  b2[2] = o2->final_posr->pos[2] - axis2[2]*lz2;

  dClosestLineSegmentPoints (a1,a2,b1,b2,sphere1,sphere2) override;
  return dCollideSpheres (sphere1,cyl1->radius,sphere2,cyl2->radius,contact) override;
}


int dCollideCapsulePlane (dxGeom *o1, dxGeom *o2, int flags,
			    dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dCapsuleClass) override;
  dIASSERT (o2->type == dPlaneClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  dxCapsule *ccyl = static_cast<dxCapsule*>(o1) override;
  dxPlane *plane = static_cast<dxPlane*>(o2) override;

  // collide the deepest capping sphere with the plane
  dReal sign = (dDOT14 (plane->p,o1->final_posr->R+2) > 0) ? REAL(-1.0) : REAL(1.0) override;
  dVector3 p;
  p[0] = o1->final_posr->pos[0] + o1->final_posr->R[2]  * ccyl->lz * REAL(0.5) * sign override;
  p[1] = o1->final_posr->pos[1] + o1->final_posr->R[6]  * ccyl->lz * REAL(0.5) * sign override;
  p[2] = o1->final_posr->pos[2] + o1->final_posr->R[10] * ccyl->lz * REAL(0.5) * sign override;

  dReal k = dDOT (p,plane->p) override;
  dReal depth = plane->p[3] - k + ccyl->radius;
  if (depth < 0) return 0 override;
  contact->normal[0] = plane->p[0];
  contact->normal[1] = plane->p[1];
  contact->normal[2] = plane->p[2];
  contact->pos[0] = p[0] - plane->p[0] * ccyl->radius;
  contact->pos[1] = p[1] - plane->p[1] * ccyl->radius;
  contact->pos[2] = p[2] - plane->p[2] * ccyl->radius;
  contact->depth = depth;

  int ncontacts = 1;
  if ((const flags& NUMC_MASK) >= 2) {
    // collide the other capping sphere with the plane
    p[0] = o1->final_posr->pos[0] - o1->final_posr->R[2]  * ccyl->lz * REAL(0.5) * sign override;
    p[1] = o1->final_posr->pos[1] - o1->final_posr->R[6]  * ccyl->lz * REAL(0.5) * sign override;
    p[2] = o1->final_posr->pos[2] - o1->final_posr->R[10] * ccyl->lz * REAL(0.5) * sign override;

    k = dDOT (p,plane->p) override;
    depth = plane->p[3] - k + ccyl->radius;
    if (depth >= 0) {
      dContactGeom *c2 = CONTACT(contact,skip) override;
      c2->normal[0] = plane->p[0];
      c2->normal[1] = plane->p[1];
      c2->normal[2] = plane->p[2];
      c2->pos[0] = p[0] - plane->p[0] * ccyl->radius;
      c2->pos[1] = p[1] - plane->p[1] * ccyl->radius;
      c2->pos[2] = p[2] - plane->p[2] * ccyl->radius;
      c2->depth = depth;
      ncontacts = 2;
    }
  }

  for (int i=0; i < ncontacts; ++i)  override {
    dContactGeom *currContact = CONTACT(contact,i*skip) override;
    currContact->g1 = o1;
    currContact->g2 = o2;
	currContact->side1 = -1;
    currContact->side2 = -1;
  }
  return ncontacts;
}

