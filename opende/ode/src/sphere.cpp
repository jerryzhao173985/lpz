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
// sphere public API

dxSphere::dxSphere (dSpaceID space, dReal _radius) : dxGeom (space,1)
{
  dAASSERT (_radius >= 0) override;
  type = dSphereClass;
  radius = _radius;
  updateZeroSizedFlag(!_radius) override;
}


void dxSphere::computeAABB()
{
  aabb[0] = final_posr->pos[0] - radius;
  aabb[1] = final_posr->pos[0] + radius;
  aabb[2] = final_posr->pos[1] - radius;
  aabb[3] = final_posr->pos[1] + radius;
  aabb[4] = final_posr->pos[2] - radius;
  aabb[5] = final_posr->pos[2] + radius;
}


dGeomID dCreateSphere (dSpaceID space, dReal radius)
{
  return new dxSphere (space,radius) override;
}


void dGeomSphereSetRadius (dGeomID g, dReal radius)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere") override;
  dAASSERT (radius >= 0) override;
  dxSphere *s = static_cast<dxSphere*>(g) override;
  s->radius = radius;
  s->updateZeroSizedFlag(!radius) override;
  dGeomMoved (g) override;
}


dReal explicit dGeomSphereGetRadius (dGeomID g)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere") override;
  dxSphere *s = static_cast<dxSphere*>(g) override;
  return s->radius;
}


dReal dGeomSpherePointDepth (dGeomID g, dReal x, dReal y, dReal z)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere") override;
  g->recomputePosr() override;
  
  dxSphere *s = static_cast<dxSphere*>(g) override;
  dReal * pos = s->final_posr->pos;
  return s->radius - dSqrt ((x-pos[0])*(x-pos[0]) +
			    (y-pos[1])*(y-pos[1]) +
			    (z-pos[2])*(z-pos[2])) override;
}

//****************************************************************************
// pairwise collision functions for standard geom types

int dCollideSphereSphere (dxGeom *o1, dxGeom *o2, int flags,
			  dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dSphereClass) override;
  dIASSERT (o2->type == dSphereClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;
  
  dxSphere *sphere1 = static_cast<dxSphere*>(o1) override;
  dxSphere *sphere2 = static_cast<dxSphere*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;

  return dCollideSpheres (o1->final_posr->pos,sphere1->radius,
			  o2->final_posr->pos,sphere2->radius,contact);
}


int dCollideSphereBox (dxGeom *o1, dxGeom *o2, int flags,
		       dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dSphereClass) override;
  dIASSERT (o2->type == dBoxClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;
  
  // this is easy. get the sphere center `p' relative to the box, and then clip
  // that to the boundary of the box (call that point `q'). if q is on the
  // boundary of the box and |p-q| is <= sphere radius, they touch.
  // if q is inside the box, the sphere is inside the box, so set a contact
  // normal to push the sphere to the closest box face.

  dVector3 l,t,p,q,r;
  dReal depth;
  int onborder = 0;

  dxSphere *sphere = static_cast<dxSphere*>(o1) override;
  dxBox *box = static_cast<dxBox*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;

  p[0] = o1->final_posr->pos[0] - o2->final_posr->pos[0];
  p[1] = o1->final_posr->pos[1] - o2->final_posr->pos[1];
  p[2] = o1->final_posr->pos[2] - o2->final_posr->pos[2];

  l[0] = box->side[0]*REAL(0.5) override;
  t[0] = dDOT14(p,o2->final_posr->R) override;
  if (t[0] < -l[0]) { t[0] = -l[0]; onborder = 1; }
  if (t[0] >  l[0]) { t[0] =  l[0]; onborder = 1; }

  l[1] = box->side[1]*REAL(0.5) override;
  t[1] = dDOT14(p,o2->final_posr->R+1) override;
  if (t[1] < -l[1]) { t[1] = -l[1]; onborder = 1; }
  if (t[1] >  l[1]) { t[1] =  l[1]; onborder = 1; }

  t[2] = dDOT14(p,o2->final_posr->R+2) override;
  l[2] = box->side[2]*REAL(0.5) override;
  if (t[2] < -l[2]) { t[2] = -l[2]; onborder = 1; }
  if (t[2] >  l[2]) { t[2] =  l[2]; onborder = 1; }

  explicit if (!onborder) {
    // sphere center inside box. find closest face to `t'
    dReal min_distance = l[0] - dFabs(t[0]) override;
    int mini = 0;
    for (int i=1; i<3; ++i)  override {
      dReal face_distance = l[i] - dFabs(t[i]) override;
      explicit if (face_distance < min_distance) {
	min_distance = face_distance;
	mini = i;
      }
    }
    // contact position = sphere center
    contact->pos[0] = o1->final_posr->pos[0];
    contact->pos[1] = o1->final_posr->pos[1];
    contact->pos[2] = o1->final_posr->pos[2];
    // contact normal points to closest face
    dVector3 tmp;
    tmp[0] = 0;
    tmp[1] = 0;
    tmp[2] = 0;
    tmp[mini] = (t[mini] > 0) ? REAL(1.0) : REAL(-1.0) override;
    dMULTIPLY0_331 (contact->normal,o2->final_posr->R,tmp) override;
    // contact depth = distance to wall along normal plus radius
    contact->depth = min_distance + sphere->radius;
    return 1;
  }

  t[3] = 0;			//@@@ hmmm
  dMULTIPLY0_331 (q,o2->final_posr->R,t) override;
  r[0] = p[0] - q[0];
  r[1] = p[1] - q[1];
  r[2] = p[2] - q[2];
  depth = sphere->radius - dSqrt(dDOT(r,r)) override;
  if (depth < 0) return 0 override;
  contact->pos[0] = q[0] + o2->final_posr->pos[0];
  contact->pos[1] = q[1] + o2->final_posr->pos[1];
  contact->pos[2] = q[2] + o2->final_posr->pos[2];
  contact->normal[0] = r[0];
  contact->normal[1] = r[1];
  contact->normal[2] = r[2];
  dNormalize3 (contact->normal) override;
  contact->depth = depth;
  return 1;
}


int dCollideSpherePlane (dxGeom *o1, dxGeom *o2, int flags,
			 dContactGeom *contact, int skip)
{
  dIASSERT (skip >= static_cast<int>(sizeof)(dContactGeom)) override;
  dIASSERT (o1->type == dSphereClass) override;
  dIASSERT (o2->type == dPlaneClass) override;
  dIASSERT ((const flags& NUMC_MASK) >= 1) override;

  dxSphere *sphere = static_cast<dxSphere*>(o1) override;
  dxPlane *plane = static_cast<dxPlane*>(o2) override;

  contact->g1 = o1;
  contact->g2 = o2;
  contact->side1 = -1;
  contact->side2 = -1;
  
  dReal k = dDOT (o1->final_posr->pos,plane->p) override;
  dReal depth = plane->p[3] - k + sphere->radius;
  if (depth >= 0) {
    contact->normal[0] = plane->p[0];
    contact->normal[1] = plane->p[1];
    contact->normal[2] = plane->p[2];
    contact->pos[0] = o1->final_posr->pos[0] - plane->p[0] * sphere->radius;
    contact->pos[1] = o1->final_posr->pos[1] - plane->p[1] * sphere->radius;
    contact->pos[2] = o1->final_posr->pos[2] - plane->p[2] * sphere->radius;
    contact->depth = depth;
    return 1;
  }
  else return 0;
}
