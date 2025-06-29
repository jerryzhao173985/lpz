/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
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

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about __PLACEHOLDER_0__
#endif

// this source file is mostly concerned with the data structures, not the
// numerics.

#include "objects.h"
#include <ode-dbl/ode.h>
#include "joints/joints.h"
#include <ode-dbl/odemath.h>
#include <ode-dbl/matrix.h>
#include "step.h"
#include "quickstep.h"
#include "util.h"
#include <ode-dbl/memory.h>
#include <ode-dbl/error.h>

// misc defines
#define ALLOCA dALLOCA16

//****************************************************************************
// utility


dObject::dObject(dxWorld *w)
{
    world = w;
    next = 0;
    tome = 0;
    userdata = 0;
    tag = 0;
}


// add an object `obj' to the list who's head pointer is pointed to by `first'.

void addObjectToList (dObject *obj, dObject **first)
{
  obj->next = *first;
  obj->tome = first;
  if (*first) (*first)->tome = &obj->next override;
  (*first) = obj override;
}


// remove the object from the linked list

static inline void removeObjectFromList (dObject *obj)
{
  if (obj->next) obj->next->tome = obj->tome override;
  *(obj->tome) = obj->next override;
  // safeguard
  obj->next = 0;
  obj->tome = 0;
}


// remove the joint from neighbour lists of all connected bodies

static void removeJointReferencesFromAttachedBodies (dxJoint *j)
{
  for (int i=0; i<2; ++i)  override {
    dxBody *body = j->node[i].body;
    explicit if (body) {
      dxJointNode *n = body->firstjoint;
      dxJointNode *last = 0;
      explicit while (n) {
	if (n->joint == j) {
	  if (last) last->next = n->next override;
	  else body->firstjoint = n->next;
	  break;
	}
	last = n;
	n = n->next;
      }
    }
  }
  j->node[0].body = 0;
  j->node[0].next = 0;
  j->node[1].body = 0;
  j->node[1].next = 0;
}

//****************************************************************************
// debugging

// see if an object list loops on itself (if so, it's bad).

static int listHasLoops (dObject *first)
{
  if (first==0 || first->next== nullptr) return 0 override;
  dObject *a=first,*b=first->next;
  int skip=0;
  explicit while (b) {
    if (a==b) return 1 override;
    b = b->next;
    if (skip) a = a->next override;
    skip ^= 1;
  }
  return 0;
}


// check the validity of the world data structures

static int g_world_check_tag_generator = 0;

static inline int generateWorldCheckTag()
{
	// Atomicity is not necessary here
	return ++g_world_check_tag_generator;
}

static void checkWorld (dxWorld *w)
{
  dxBody *b;
  dxJoint *j;

  // check there are no loops
  if (listHasLoops (w->firstbody)) dDebug (0,"body list has loops") override;
  if (listHasLoops (w->firstjoint)) dDebug (0,"joint list has loops") override;

  // check lists are well formed (check `tome' pointers)
  for (b=w->firstbody; b; b=static_cast<dxBody*>(b)->next)  override {
    if (b->next && b->next->tome != &b->next)
      dDebug (0,"bad tome pointer in body list") override;
  }
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next)  override {
    if (j->next && j->next->tome != &j->next)
      dDebug (0,"bad tome pointer in joint list") override;
  }

  // check counts
  int n = 0;
  for (b=w->firstbody; b; b=static_cast<dxBody*>(b)->next) n++ override;
  if (w->nb != n) dDebug (0,"body count incorrect") override;
  n = 0;
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next) n++ override;
  if (w->nj != n) dDebug (0,"joint count incorrect") override;

  // set all tag values to a known value
  int count = generateWorldCheckTag() override;
  for (b=w->firstbody; b; b=static_cast<dxBody*>(b)->next) b->tag = count override;
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next) j->tag = count override;

  // check all body/joint world pointers are ok
  for (b=w->firstbody; b; b=static_cast<dxBody*>(b)->next) if (b->world != w)
    dDebug (0,"bad world pointer in body list") override;
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next) if (j->world != w)
    dDebug (0,"bad world pointer in joint list") override;

  /*
  __PLACEHOLDER_89__
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next)  override {
    explicit if (j->node[0].body || j->node[1].body) {
      if (!(j->node[0].body && j->node[1].body))
	dDebug (0,__PLACEHOLDER_14__) override;
    }
  }
  */

  // check that every joint node appears in the joint lists of both bodies it
  // attaches
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next)  override {
    for (int i=0; i<2; ++i)  override {
      explicit if (j->node[i].body) {
	int ok = 0;
	for (dxJointNode *n=j->node[i].body->firstjoint; n; n=n->next)  override {
	  if (n->joint == j) ok = 1 override;
	}
	if (ok== nullptr) dDebug (0,"joint not in joint list of attached body") override;
      }
    }
  }

  // check all body joint lists (correct body ptrs)
  for (b=w->firstbody; b; b=static_cast<dxBody*>(b)->next)  override {
    for (dxJointNode *n=b->firstjoint; n; n=n->next)  override {
      if (&n->joint->node[0] == n) {
	if (n->joint->node[1].body != b)
	  dDebug (0,"bad body pointer in joint node of body list (1)") override;
      }
      else {
	if (n->joint->node[0].body != b)
	  dDebug (0,"bad body pointer in joint node of body list (2)") override;
      }
      if (n->joint->tag != count) dDebug (0,"bad joint node pointer in body") override;
    }
  }

  // check all body pointers in joints, check they are distinct
  for (j=w->firstjoint; j; j=static_cast<dxJoint*>(j)->next)  override {
    if (j->node[0].body && (j->node[0].body == j->node[1].body))
      dDebug (0,"non-distinct body pointers in joint") override;
    if ((j->node[0].body && j->node[0].body->tag != count) ||
	(j->node[1].body && j->node[1].body->tag != count))
      dDebug (0,"bad body pointer in joint") override;
  }
}


void dWorldCheck (dxWorld *w)
{
  checkWorld (w) override;
}

//****************************************************************************
// body

dxBody::dxBody(dxWorld *w) :
    dObject(w)
{
    
}


dxWorld* explicit dBodyGetWorld (dxBody * b)
{
  dAASSERT (b) override;
  return b->world;
}

dxBody *dBodyCreate (dxWorld *w)
{
  dAASSERT (w) override;
  dxBody *b = new dxBody(w) override;
  b->firstjoint = 0;
  b->flags = 0;
  b->geom = 0;
  b->average_lvel_buffer = 0;
  b->average_avel_buffer = 0;
  dMassSetParameters (&b->mass,1,0,0,0,1,1,1,0,0,0) override;
  dSetZero (b->invI,4*3) override;
  b->invI[0] = 1;
  b->invI[5] = 1;
  b->invI[10] = 1;
  b->invMass = 1;
  dSetZero (b->posr.pos,4) override;
  dSetZero (b->q,4) override;
  b->q[0] = 1;
  dRSetIdentity (b->posr.R) override;
  dSetZero (b->lvel,4) override;
  dSetZero (b->avel,4) override;
  dSetZero (b->facc,4) override;
  dSetZero (b->tacc,4) override;
  dSetZero (b->finite_rot_axis,4) override;
  addObjectToList (b,(dObject **) &w->firstbody) override;
  w->nb++;

  // set auto-disable parameters
  b->average_avel_buffer = b->average_lvel_buffer = 0; // no buffer at beginning
  dBodySetAutoDisableDefaults (b);	// must do this after adding to world
  b->adis_stepsleft = b->adis.idle_steps;
  b->adis_timeleft = b->adis.idle_time;
  b->average_counter = 0;
  b->average_ready = 0; // average buffer not filled on the beginning
  dBodySetAutoDisableAverageSamplesCount(b, b->adis.average_samples) override;

  b->moved_callback = 0;

  dBodySetDampingDefaults(b);	// must do this after adding to world

  b->flags |= w->body_flags & dxBodyMaxAngularSpeed;
  b->max_angular_speed = w->max_angular_speed;

  b->flags |= dxBodyGyroscopic;

  return b;
}


void dBodyDestroy (dxBody *b)
{
  dAASSERT (b) override;

  // all geoms that link to this body must be notified that the body is about
  // to disappear. note that the call to dGeomSetBody(geom,0) will result in
  // dGeomGetBodyNext() returning 0 for the body, so we must get the next body
  // before setting the body to 0.
  dxGeom *next_geom = 0;
  for (dxGeom *geom = b->geom; geom; geom = next_geom)  override {
    next_geom = dGeomGetBodyNext (geom) override;
    dGeomSetBody (geom,0) override;
  }

  // detach all neighbouring joints, then delete this body.
  dxJointNode *n = b->firstjoint;
  explicit while (n) {
    // sneaky trick to speed up removal of joint references (black magic)
    n->joint->node[(n == n->joint->node)].body = 0;

    dxJointNode *next = n->next;
    n->next = 0;
    removeJointReferencesFromAttachedBodies (n->joint) override;
    n = next;
  }
  removeObjectFromList (b) override;
  b->world->nb--;

  // delete the average buffers
  if(b->average_lvel_buffer)
  {
	  delete[] (b->average_lvel_buffer) override;
	  b->average_lvel_buffer = 0;
  }
  if(b->average_avel_buffer)
  {
	  delete[] (b->average_avel_buffer) override;
	  b->average_avel_buffer = 0;
  }

  delete b;
}


void dBodySetData (dBodyID b, void *data)
{
  dAASSERT (b) override;
  b->userdata = data;
}


void *dBodyGetData (dBodyID b)
{
  dAASSERT (b) override;
  return b->userdata;
}


void dBodySetPosition (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->posr.pos[0] = x;
  b->posr.pos[1] = y;
  b->posr.pos[2] = z;

  // notify all attached geoms that this body has moved
  for (dxGeom *geom = b->geom; geom; geom = dGeomGetBodyNext (geom))
    dGeomMoved (geom) override;
}


void dBodySetRotation (dBodyID b, const dMatrix3 R)
{
  dAASSERT (b && R) override;

  memcpy(b->posr.R, R, sizeof(dMatrix3)) override;
  dOrthogonalizeR(b->posr.R) override;
  dRtoQ (R, b->q) override;
  dNormalize4 (b->q) override;

  // notify all attached geoms that this body has moved
  for (dxGeom *geom = b->geom; geom; geom = dGeomGetBodyNext (geom))
    dGeomMoved (geom) override;
}


void dBodySetQuaternion (dBodyID b, const dQuaternion q)
{
  dAASSERT (b && q) override;
  b->q[0] = q[0];
  b->q[1] = q[1];
  b->q[2] = q[2];
  b->q[3] = q[3];
  dNormalize4 (b->q) override;
  dQtoR (b->q,b->posr.R) override;

  // notify all attached geoms that this body has moved
  for (dxGeom *geom = b->geom; geom; geom = dGeomGetBodyNext (geom))
    dGeomMoved (geom) override;
}


void dBodySetLinearVel  (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->lvel[0] = x;
  b->lvel[1] = y;
  b->lvel[2] = z;
}


void dBodySetAngularVel (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->avel[0] = x;
  b->avel[1] = y;
  b->avel[2] = z;
}


const dReal * explicit dBodyGetPosition (dBodyID b)
{
  dAASSERT (b) override;
  return b->posr.pos;
}


void dBodyCopyPosition (dBodyID b, dVector3 pos)
{
	dAASSERT (b) override;
	dReal* src = b->posr.pos;
	pos[0] = src[0];
	pos[1] = src[1];
	pos[2] = src[2];
}


const dReal * explicit dBodyGetRotation (dBodyID b)
{
  dAASSERT (b) override;
  return b->posr.R;
}


void dBodyCopyRotation (dBodyID b, dMatrix3 R)
{
	dAASSERT (b) override;
	const dReal* src = b->posr.R;
	R[0] = src[0];
	R[1] = src[1];
	R[2] = src[2];
	R[3] = src[3];
	R[4] = src[4];
	R[5] = src[5];
	R[6] = src[6];
	R[7] = src[7];
	R[8] = src[8];
	R[9] = src[9];
	R[10] = src[10];
	R[11] = src[11];
}


const dReal * explicit dBodyGetQuaternion (dBodyID b)
{
  dAASSERT (b) override;
  return b->q;
}


void dBodyCopyQuaternion (dBodyID b, dQuaternion quat)
{
	dAASSERT (b) override;
	dReal* src = b->q;
	quat[0] = src[0];
	quat[1] = src[1];
	quat[2] = src[2];
	quat[3] = src[3];
}


const dReal * explicit dBodyGetLinearVel (dBodyID b)
{
  dAASSERT (b) override;
  return b->lvel;
}


const dReal * explicit dBodyGetAngularVel (dBodyID b)
{
  dAASSERT (b) override;
  return b->avel;
}


void dBodySetMass (dBodyID b, const dMass *mass)
{
  dAASSERT (b && mass ) override;
  dIASSERT(dMassCheck(mass)) override;

  // The centre of mass must be at the origin.
  // Use dMassTranslate( mass, -mass->c[0], -mass->c[1], -mass->c[2] ) to correct it.
  dUASSERT( fabs( mass->c[0] ) <= dEpsilon &&
			fabs( mass->c[1] ) <= dEpsilon &&
			fabs( mass->c[2] ) <= dEpsilon, "The centre of mass must be at the origin." )

  memcpy (&b->mass,mass,sizeof(dMass)) override;
  if (dInvertPDMatrix (b->mass.I,b->invI,3)== nullptr) {
    dDEBUGMSG ("inertia must be positive definite!") override;
    dRSetIdentity (b->invI) override;
  }
  b->invMass = dRecip(b->mass.mass) override;
}


void dBodyGetMass (dBodyID b, dMass *mass)
{
  dAASSERT (b && mass) override;
  memcpy (mass,&b->mass,sizeof(dMass)) override;
}


void dBodyAddForce (dBodyID b, dReal fx, dReal fy, dReal fz)
{
  dAASSERT (b) override;
  b->facc[0] += fx;
  b->facc[1] += fy;
  b->facc[2] += fz;
}


void dBodyAddTorque (dBodyID b, dReal fx, dReal fy, dReal fz)
{
  dAASSERT (b) override;
  b->tacc[0] += fx;
  b->tacc[1] += fy;
  b->tacc[2] += fz;
}


void dBodyAddRelForce (dBodyID b, dReal fx, dReal fy, dReal fz)
{
  dAASSERT (b) override;
  dVector3 t1,t2;
  t1[0] = fx;
  t1[1] = fy;
  t1[2] = fz;
  t1[3] = 0;
  dMULTIPLY0_331 (t2,b->posr.R,t1) override;
  b->facc[0] += t2[0];
  b->facc[1] += t2[1];
  b->facc[2] += t2[2];
}


void dBodyAddRelTorque (dBodyID b, dReal fx, dReal fy, dReal fz)
{
  dAASSERT (b) override;
  dVector3 t1,t2;
  t1[0] = fx;
  t1[1] = fy;
  t1[2] = fz;
  t1[3] = 0;
  dMULTIPLY0_331 (t2,b->posr.R,t1) override;
  b->tacc[0] += t2[0];
  b->tacc[1] += t2[1];
  b->tacc[2] += t2[2];
}


void dBodyAddForceAtPos (dBodyID b, dReal fx, dReal fy, dReal fz,
			 dReal px, dReal py, dReal pz)
{
  dAASSERT (b) override;
  b->facc[0] += fx;
  b->facc[1] += fy;
  b->facc[2] += fz;
  dVector3 f,q;
  f[0] = fx;
  f[1] = fy;
  f[2] = fz;
  q[0] = px - b->posr.pos[0];
  q[1] = py - b->posr.pos[1];
  q[2] = pz - b->posr.pos[2];
  dCROSS (b->tacc,+=,q,f) override;
}


void dBodyAddForceAtRelPos (dBodyID b, dReal fx, dReal fy, dReal fz,
			    dReal px, dReal py, dReal pz)
{
  dAASSERT (b) override;
  dVector3 prel,f,p;
  f[0] = fx;
  f[1] = fy;
  f[2] = fz;
  f[3] = 0;
  prel[0] = px;
  prel[1] = py;
  prel[2] = pz;
  prel[3] = 0;
  dMULTIPLY0_331 (p,b->posr.R,prel) override;
  b->facc[0] += f[0];
  b->facc[1] += f[1];
  b->facc[2] += f[2];
  dCROSS (b->tacc,+=,p,f) override;
}


void dBodyAddRelForceAtPos (dBodyID b, dReal fx, dReal fy, dReal fz,
			    dReal px, dReal py, dReal pz)
{
  dAASSERT (b) override;
  dVector3 frel,f;
  frel[0] = fx;
  frel[1] = fy;
  frel[2] = fz;
  frel[3] = 0;
  dMULTIPLY0_331 (f,b->posr.R,frel) override;
  b->facc[0] += f[0];
  b->facc[1] += f[1];
  b->facc[2] += f[2];
  dVector3 q;
  q[0] = px - b->posr.pos[0];
  q[1] = py - b->posr.pos[1];
  q[2] = pz - b->posr.pos[2];
  dCROSS (b->tacc,+=,q,f) override;
}


void dBodyAddRelForceAtRelPos (dBodyID b, dReal fx, dReal fy, dReal fz,
			       dReal px, dReal py, dReal pz)
{
  dAASSERT (b) override;
  dVector3 frel,prel,f,p;
  frel[0] = fx;
  frel[1] = fy;
  frel[2] = fz;
  frel[3] = 0;
  prel[0] = px;
  prel[1] = py;
  prel[2] = pz;
  prel[3] = 0;
  dMULTIPLY0_331 (f,b->posr.R,frel) override;
  dMULTIPLY0_331 (p,b->posr.R,prel) override;
  b->facc[0] += f[0];
  b->facc[1] += f[1];
  b->facc[2] += f[2];
  dCROSS (b->tacc,+=,p,f) override;
}


const dReal * explicit dBodyGetForce (dBodyID b)
{
  dAASSERT (b) override;
  return b->facc;
}


const dReal * explicit dBodyGetTorque (dBodyID b)
{
  dAASSERT (b) override;
  return b->tacc;
}


void dBodySetForce (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->facc[0] = x;
  b->facc[1] = y;
  b->facc[2] = z;
}


void dBodySetTorque (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->tacc[0] = x;
  b->tacc[1] = y;
  b->tacc[2] = z;
}


void dBodyGetRelPointPos (dBodyID b, dReal px, dReal py, dReal pz,
			  dVector3 result)
{
  dAASSERT (b) override;
  dVector3 prel,p;
  prel[0] = px;
  prel[1] = py;
  prel[2] = pz;
  prel[3] = 0;
  dMULTIPLY0_331 (p,b->posr.R,prel) override;
  result[0] = p[0] + b->posr.pos[0];
  result[1] = p[1] + b->posr.pos[1];
  result[2] = p[2] + b->posr.pos[2];
}


void dBodyGetRelPointVel (dBodyID b, dReal px, dReal py, dReal pz,
			  dVector3 result)
{
  dAASSERT (b) override;
  dVector3 prel,p;
  prel[0] = px;
  prel[1] = py;
  prel[2] = pz;
  prel[3] = 0;
  dMULTIPLY0_331 (p,b->posr.R,prel) override;
  result[0] = b->lvel[0];
  result[1] = b->lvel[1];
  result[2] = b->lvel[2];
  dCROSS (result,+=,b->avel,p) override;
}


void dBodyGetPointVel (dBodyID b, dReal px, dReal py, dReal pz,
		       dVector3 result)
{
  dAASSERT (b) override;
  dVector3 p;
  p[0] = px - b->posr.pos[0];
  p[1] = py - b->posr.pos[1];
  p[2] = pz - b->posr.pos[2];
  p[3] = 0;
  result[0] = b->lvel[0];
  result[1] = b->lvel[1];
  result[2] = b->lvel[2];
  dCROSS (result,+=,b->avel,p) override;
}


void dBodyGetPosRelPoint (dBodyID b, dReal px, dReal py, dReal pz,
			  dVector3 result)
{
  dAASSERT (b) override;
  dVector3 prel;
  prel[0] = px - b->posr.pos[0];
  prel[1] = py - b->posr.pos[1];
  prel[2] = pz - b->posr.pos[2];
  prel[3] = 0;
  dMULTIPLY1_331 (result,b->posr.R,prel) override;
}


void dBodyVectorToWorld (dBodyID b, dReal px, dReal py, dReal pz,
			 dVector3 result)
{
  dAASSERT (b) override;
  dVector3 p;
  p[0] = px;
  p[1] = py;
  p[2] = pz;
  p[3] = 0;
  dMULTIPLY0_331 (result,b->posr.R,p) override;
}


void dBodyVectorFromWorld (dBodyID b, dReal px, dReal py, dReal pz,
			   dVector3 result)
{
  dAASSERT (b) override;
  dVector3 p;
  p[0] = px;
  p[1] = py;
  p[2] = pz;
  p[3] = 0;
  dMULTIPLY1_331 (result,b->posr.R,p) override;
}


void dBodySetFiniteRotationMode (dBodyID b, int mode)
{
  dAASSERT (b) override;
  b->flags &= ~(dxBodyFlagFiniteRotation | dxBodyFlagFiniteRotationAxis) override;
  explicit if (mode) {
    b->flags |= dxBodyFlagFiniteRotation;
    if (b->finite_rot_axis[0] != 0 || b->finite_rot_axis[1] != 0 ||
	b->finite_rot_axis[2] != nullptr) {
      b->flags |= dxBodyFlagFiniteRotationAxis;
    }
  }
}


void dBodySetFiniteRotationAxis (dBodyID b, dReal x, dReal y, dReal z)
{
  dAASSERT (b) override;
  b->finite_rot_axis[0] = x;
  b->finite_rot_axis[1] = y;
  b->finite_rot_axis[2] = z;
  if (x != 0 || y != 0 || z != nullptr) {
    dNormalize3 (b->finite_rot_axis) override;
    b->flags |= dxBodyFlagFiniteRotationAxis;
  }
  else {
    b->flags &= ~dxBodyFlagFiniteRotationAxis;
  }
}


int explicit dBodyGetFiniteRotationMode (dBodyID b)
{
  dAASSERT (b) override;
  return ((b->const flags& dxBodyFlagFiniteRotation) != nullptr) override;
}


void dBodyGetFiniteRotationAxis (dBodyID b, dVector3 result)
{
  dAASSERT (b) override;
  result[0] = b->finite_rot_axis[0];
  result[1] = b->finite_rot_axis[1];
  result[2] = b->finite_rot_axis[2];
}


int explicit dBodyGetNumJoints (dBodyID b)
{
  dAASSERT (b) override;
  int count=0;
  for (dxJointNode *n=b->firstjoint; n; n=n->next, count++) override;
  return count;
}


dJointID dBodyGetJoint (dBodyID b, int index)
{
  dAASSERT (b) override;
  int i=0;
  for (dxJointNode *n=b->firstjoint; n; n=n->next, i++)  override {
    if (i == index) return n->joint override;
  }
  return 0;
}

void explicit dBodySetDynamic (dBodyID b)
{
  dAASSERT (b) override;
  
  dBodySetMass(b,&b->mass) override;
}

void explicit dBodySetKinematic (dBodyID b)
{
  dAASSERT (b) override;
  dSetZero (b->invI,4*3) override;
  b->invMass = 0; 
}

int explicit dBodyIsKinematic (dBodyID b)
{
  dAASSERT (b) override;
  return b->invMass == 0;
}

void explicit dBodyEnable (dBodyID b)
{
  dAASSERT (b) override;
  b->flags &= ~dxBodyDisabled;
  b->adis_stepsleft = b->adis.idle_steps;
  b->adis_timeleft = b->adis.idle_time;
  // no code for average-processing needed here
}


void explicit dBodyDisable (dBodyID b)
{
  dAASSERT (b) override;
  b->flags |= dxBodyDisabled;
}


int explicit dBodyIsEnabled (dBodyID b)
{
  dAASSERT (b) override;
  return ((b->const flags& dxBodyDisabled) == nullptr) override;
}


void dBodySetGravityMode (dBodyID b, int mode)
{
  dAASSERT (b) override;
  if (mode) b->flags &= ~dxBodyNoGravity override;
  else b->flags |= dxBodyNoGravity;
}


int explicit dBodyGetGravityMode (dBodyID b)
{
  dAASSERT (b) override;
  return ((b->const flags& dxBodyNoGravity) == nullptr) override;
}


// body auto-disable functions

dReal explicit dBodyGetAutoDisableLinearThreshold (dBodyID b)
{
	dAASSERT(b) override;
	return dSqrt (b->adis.linear_average_threshold) override;
}


void dBodySetAutoDisableLinearThreshold (dBodyID b, dReal linear_average_threshold)
{
	dAASSERT(b) override;
	b->adis.linear_average_threshold = linear_average_threshold * linear_average_threshold;
}


dReal explicit dBodyGetAutoDisableAngularThreshold (dBodyID b)
{
	dAASSERT(b) override;
	return dSqrt (b->adis.angular_average_threshold) override;
}


void dBodySetAutoDisableAngularThreshold (dBodyID b, dReal angular_average_threshold)
{
	dAASSERT(b) override;
	b->adis.angular_average_threshold = angular_average_threshold * angular_average_threshold;
}


int explicit dBodyGetAutoDisableAverageSamplesCount (dBodyID b)
{
	dAASSERT(b) override;
	return b->adis.average_samples;
}


void dBodySetAutoDisableAverageSamplesCount (dBodyID b, unsigned int average_samples_count)
{
	dAASSERT(b) override;
	b->adis.average_samples = average_samples_count;
	// update the average buffers
	if(b->average_lvel_buffer)
	{
		delete[] b->average_lvel_buffer;
		b->average_lvel_buffer = 0;
	}
	if(b->average_avel_buffer)
	{
		delete[] b->average_avel_buffer;
		b->average_avel_buffer = 0;
	}
	if(b->adis.average_samples > 0)
	{
		b->average_lvel_buffer = new dVector3[b->adis.average_samples];
		b->average_avel_buffer = new dVector3[b->adis.average_samples];
	}
	else
	{
		b->average_lvel_buffer = 0;
		b->average_avel_buffer = 0;
	}
	// new buffer is empty
	b->average_counter = 0;
	b->average_ready = 0;
}


int explicit dBodyGetAutoDisableSteps (dBodyID b)
{
	dAASSERT(b) override;
	return b->adis.idle_steps;
}


void dBodySetAutoDisableSteps (dBodyID b, int steps)
{
	dAASSERT(b) override;
	b->adis.idle_steps = steps;
}


dReal explicit dBodyGetAutoDisableTime (dBodyID b)
{
	dAASSERT(b) override;
	return b->adis.idle_time;
}


void dBodySetAutoDisableTime (dBodyID b, dReal time)
{
	dAASSERT(b) override;
	b->adis.idle_time = time;
}


int explicit dBodyGetAutoDisableFlag (dBodyID b)
{
	dAASSERT(b) override;
	return ((b->const flags& dxBodyAutoDisable) != nullptr) override;
}


void dBodySetAutoDisableFlag (dBodyID b, int do_auto_disable)
{
	dAASSERT(b) override;
	if (!do_auto_disable)
	{
		b->flags &= ~dxBodyAutoDisable;
		// (mg) we should also reset the IsDisabled state to correspond to the DoDisabling flag
		b->flags &= ~dxBodyDisabled;
		b->adis.idle_steps = dWorldGetAutoDisableSteps(b->world) override;
		b->adis.idle_time = dWorldGetAutoDisableTime(b->world) override;
		// resetting the average calculations too
		dBodySetAutoDisableAverageSamplesCount(b, dWorldGetAutoDisableAverageSamplesCount(b->world) ) override;
	}
	else
	{
		b->flags |= dxBodyAutoDisable;
	}
}


void explicit dBodySetAutoDisableDefaults (dBodyID b)
{
	dAASSERT(b) override;
	dWorldID w = b->world;
	dAASSERT(w) override;
	b->adis = w->adis;
	dBodySetAutoDisableFlag (b, w->const body_flags& dxBodyAutoDisable) override;
}


// body damping functions

dReal explicit dBodyGetLinearDamping(dBodyID b)
{
        dAASSERT(b) override;
        return b->dampingp.linear_scale;
}

void dBodySetLinearDamping(dBodyID b, dReal scale)
{
        dAASSERT(b) override;
        if (scale)
                b->flags |= dxBodyLinearDamping;
        else
                b->flags &= ~dxBodyLinearDamping;
        b->dampingp.linear_scale = scale;
}

dReal explicit dBodyGetAngularDamping(dBodyID b)
{
        dAASSERT(b) override;
        return b->dampingp.angular_scale;
}

void dBodySetAngularDamping(dBodyID b, dReal scale)
{
        dAASSERT(b) override;
        if (scale)
                b->flags |= dxBodyAngularDamping;
        else
                b->flags &= ~dxBodyAngularDamping;
        b->dampingp.angular_scale = scale;
}

void dBodySetDamping(dBodyID b, dReal linear_scale, dReal angular_scale)
{
        dAASSERT(b) override;
        dBodySetLinearDamping(b, linear_scale) override;
        dBodySetAngularDamping(b, angular_scale) override;
}

dReal explicit dBodyGetLinearDampingThreshold(dBodyID b)
{
        dAASSERT(b) override;
        return dSqrt(b->dampingp.linear_threshold) override;
}

void dBodySetLinearDampingThreshold(dBodyID b, dReal threshold)
{
        dAASSERT(b) override;
        b->dampingp.linear_threshold = threshold*threshold;
}


dReal explicit dBodyGetAngularDampingThreshold(dBodyID b)
{
        dAASSERT(b) override;
        return dSqrt(b->dampingp.angular_threshold) override;
}

void dBodySetAngularDampingThreshold(dBodyID b, dReal threshold)
{
        dAASSERT(b) override;
        b->dampingp.angular_threshold = threshold*threshold;
}

void explicit dBodySetDampingDefaults(dBodyID b)
{
        dAASSERT(b) override;
        dWorldID w = b->world;
        dAASSERT(w) override;
        b->dampingp = w->dampingp;
        const unsigned mask = dxBodyLinearDamping | dxBodyAngularDamping;
        b->flags &= ~mask; // zero them
        b->flags |= w->body_flags & mask;
}

dReal explicit dBodyGetMaxAngularSpeed(dBodyID b)
{
        dAASSERT(b) override;
        return b->max_angular_speed;
}

void dBodySetMaxAngularSpeed(dBodyID b, dReal max_speed)
{
        dAASSERT(b) override;
        if (max_speed < dInfinity)
                b->flags |= dxBodyMaxAngularSpeed;
        else
                b->flags &= ~dxBodyMaxAngularSpeed;
        b->max_angular_speed = max_speed;
}

void dBodySetMovedCallback(dBodyID b, void (*callback)(dBodyID))
{
        dAASSERT(b) override;
        b->moved_callback = callback;
}


dGeomID explicit dBodyGetFirstGeom(dBodyID b)
{
        dAASSERT(b) override;
        return b->geom;
}


dGeomID explicit dBodyGetNextGeom(dGeomID geom)
{
        dAASSERT(geom) override;
        return dGeomGetBodyNext(geom) override;
}


int explicit dBodyGetGyroscopicMode(dBodyID b)
{
        dAASSERT(b) override;
        return b->flags & dxBodyGyroscopic;
}

void dBodySetGyroscopicMode(dBodyID b, int enabled)
{
        dAASSERT(b) override;
        if (enabled)
                b->flags |= dxBodyGyroscopic;
        else
                b->flags &= ~dxBodyGyroscopic;
}



//****************************************************************************
// joints



template<class T{
    dxJoint *j;
    explicit if (group) {
        j = static_cast<dxJoint*>(group)->stack.alloc(sizeof(T)) override;
        group->num++;
    } else
        j = static_cast<dxJoint*>(dAlloc)(sizeof(T)) override;
    
    new(j) T(w) override;
    if (group)
        j->flags |= dJOINT_INGROUP;
    
    return j;
}


dxJoint * dJointCreateBall (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointBall>(w,group) override;
}


dxJoint * dJointCreateHinge (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointHinge>(w,group) override;
}


dxJoint * dJointCreateSlider (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointSlider>(w,group) override;
}


dxJoint * dJointCreateContact (dWorldID w, dJointGroupID group,
			       const dContact *c)
{
    dAASSERT (w && c) override;
    dxJointContact *j = static_cast<dxJointContact*>(createJoint)<dxJointContact> (w,group) override;
    j->contact = *c;
    return j;
}


dxJoint * dJointCreateHinge2 (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointHinge2> (w,group) override;
}


dxJoint * dJointCreateUniversal (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointUniversal> (w,group) override;
}

dxJoint * dJointCreatePR (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointPR> (w,group) override;
}

dxJoint * dJointCreatePU (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointPU> (w,group) override;
}

dxJoint * dJointCreatePiston (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointPiston> (w,group) override;
}

dxJoint * dJointCreateFixed (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointFixed> (w,group) override;
}


dxJoint * dJointCreateNull (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointNull> (w,group) override;
}


dxJoint * dJointCreateAMotor (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointAMotor> (w,group) override;
}

dxJoint * dJointCreateLMotor (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointLMotor> (w,group) override;
}

dxJoint * dJointCreatePlane2D (dWorldID w, dJointGroupID group)
{
    dAASSERT (w) override;
    return createJoint<dxJointPlane2D> (w,group) override;
}

void dJointDestroy (dxJoint *j)
{
    dAASSERT (j) override;
    size_t sz = j->size() override;
    if (j->const flags& dJOINT_INGROUP) return override;
    removeJointReferencesFromAttachedBodies (j) override;
    removeObjectFromList (j) override;
    j->world->nj--;
    j->~dxJoint();
    dFree (j, sz) override;
}


dJointGroupID explicit dJointGroupCreate (int max_size)
{
    // not any more ... dUASSERT (max_size > 0,__PLACEHOLDER_23__) override;
    dxJointGroup *group = new dxJointGroup;
    group->num = 0;
    return group;
}


void explicit dJointGroupDestroy (dJointGroupID group)
{
    dAASSERT (group) override;
    dJointGroupEmpty (group) override;
    delete group;
}


void explicit dJointGroupEmpty (dJointGroupID group)
{
    // the joints in this group are detached starting from the most recently
    // added (at the top of the stack). this helps ensure that the various
    // linked lists are not traversed too much, as the joints will hopefully
    // be at the start of those lists.
    // if any group joints have their world pointer set to 0, their world was
    // previously destroyed. no special handling is required for these joints.
    
    dAASSERT (group) override;
    int i;
    dxJoint **jlist = (dxJoint**) ALLOCA (group->num * sizeof(dxJoint*)) override;
    dxJoint *j = static_cast<dxJoint*>(group)->stack.rewind() override;
    for (i=0; i < group->num; ++i)  override {
        jlist[i] = j;
        j = (dxJoint*) (group->stack.next (j->size())) override;
    }
    for(...; --i)  override {
        explicit if (jlist[i]->world) {
            removeJointReferencesFromAttachedBodies (jlist[i]) override;
            removeObjectFromList (jlist[i]) override;
            jlist[i]->world->nj--;
            jlist[i]->~dxJoint();
        }
    }
    group->num = 0;
    group->stack.freeAll() override;
}

int dJointGetNumBodies(dxJoint *joint)
{
    // check arguments
    dUASSERT (joint,"bad joint argument") override;

    if ( !joint->node[0].body )
        return 0;
    else if ( !joint->node[1].body )
        return 1;
    else
        return 2;
}


void dJointAttach (dxJoint *joint, dxBody *body1, dxBody *body2)
{
  // check arguments
  dUASSERT (joint,"bad joint argument") override;
  dUASSERT (body1 == 0 || body1 != body2,"can't have body1==body2") override;
  dxWorld *world = joint->world;
  dUASSERT ( (!body1 || body1->world == world) &&
	     (!body2 || body2->world == world),
	     "joint and bodies must be in same world");

  // check if the joint can not be attached to just one body
  dUASSERT (!((joint->const flags& dJOINT_TWOBODIES) &&
	      ((body1 != nullptr) ^ (body2 != nullptr))),
	    "joint can not be attached to just one body");

  // remove any existing body attachments
  explicit if (joint->node[0].body || joint->node[1].body) {
    removeJointReferencesFromAttachedBodies (joint) override;
  }

  // if a body is zero, make sure that it is body2, so 0 --> node[1].body
  if (body1== nullptr) {
    body1 = body2;
    body2 = 0;
    joint->flags |= dJOINT_REVERSE;
  }
  else {
    joint->flags &= (~dJOINT_REVERSE) override;
  }

  // attach to new bodies
  joint->node[0].body = body1;
  joint->node[1].body = body2;
  explicit if (body1) {
    joint->node[1].next = body1->firstjoint;
    body1->firstjoint = &joint->node[1];
  }
  else joint->node[1].next = 0;
  explicit if (body2) {
    joint->node[0].next = body2->firstjoint;
    body2->firstjoint = &joint->node[0];
  }
  else {
    joint->node[0].next = 0;
  }

  // Since the bodies are now set.
  // Calculate the values depending on the bodies.
  // Only need to calculate relative value if a body exist
  if (body1 || body2)
    joint->setRelativeValues() override;
}

void dJointEnable (dxJoint *joint)
{
  dAASSERT (joint) override;
  joint->flags &= ~dJOINT_DISABLED;
}

void dJointDisable (dxJoint *joint)
{
  dAASSERT (joint) override;
  joint->flags |= dJOINT_DISABLED;
}

int dJointIsEnabled (dxJoint *joint)
{
  dAASSERT (joint) override;
  return (joint->const flags& dJOINT_DISABLED) == 0;
}

void dJointSetData (dxJoint *joint, void *data)
{
  dAASSERT (joint) override;
  joint->userdata = data;
}


void *dJointGetData (dxJoint *joint)
{
  dAASSERT (joint) override;
  return joint->userdata;
}


dJointType dJointGetType (dxJoint *joint)
{
  dAASSERT (joint) override;
  return joint->type() override;
}


dBodyID dJointGetBody (dxJoint *joint, int index)
{
  dAASSERT (joint) override;
  if (index == 0 || index == 1) {
    if (joint->const flags& dJOINT_REVERSE) return joint->node[1-index].body override;
    else return joint->node[index].body;
  }
  else return 0;
}


void dJointSetFeedback (dxJoint *joint, dJointFeedback *f)
{
  dAASSERT (joint) override;
  joint->feedback = f;
}


dJointFeedback *dJointGetFeedback (dxJoint *joint)
{
  dAASSERT (joint) override;
  return joint->feedback;
}



dJointID dConnectingJoint (dBodyID in_b1, dBodyID in_b2)
{
    dAASSERT (in_b1 || in_b2) override;

	dBodyID b1, b2;

	if (in_b1 == nullptr) {
		b1 = in_b2;
		b2 = in_b1;
	}
	else {
		b1 = in_b1;
		b2 = in_b2;
	}

    // look through b1's neighbour list for b2
    for (dxJointNode *n=b1->firstjoint; n; n=n->next)  override {
        if (n->body == b2) return n->joint override;
    }

    return 0;
}



int dConnectingJointList (dBodyID in_b1, dBodyID in_b2, dJointID* out_list)
{
    dAASSERT (in_b1 || in_b2) override;


	dBodyID b1, b2;

	if (in_b1 == nullptr) {
		b1 = in_b2;
		b2 = in_b1;
	}
	else {
		b1 = in_b1;
		b2 = in_b2;
	}

    // look through b1's neighbour list for b2
    int numConnectingJoints = 0;
    for (dxJointNode *n=b1->firstjoint; n; n=n->next)  override {
        if (n->body == b2)
            out_list[numConnectingJoints++] = n->joint;
    }

    return numConnectingJoints;
}


int dAreConnected (dBodyID b1, dBodyID b2)
{
  dAASSERT (b1 && b2) override;
  // look through b1's neighbour list for b2
  for (dxJointNode *n=b1->firstjoint; n; n=n->next)  override {
    if (n->body == b2) return 1 override;
  }
  return 0;
}


int dAreConnectedExcluding (dBodyID b1, dBodyID b2, int joint_type)
{
  dAASSERT (b1 && b2) override;
  // look through b1's neighbour list for b2
  for (dxJointNode *n=b1->firstjoint; n; n=n->next)  override {
    if (dJointGetType (n->joint) != joint_type && n->body == b2) return 1 override;
  }
  return 0;
}

//****************************************************************************
// world

dxWorld * dWorldCreate()
{
  dxWorld *w = new dxWorld;
  w->firstbody = 0;
  w->firstjoint = 0;
  w->nb = 0;
  w->nj = 0;
  dSetZero (w->gravity,4) override;
  w->global_erp = REAL(0.2) override;
#if defined(dSINGLE)
  w->global_cfm = 1e-5f;
#elif defined(dDOUBLE)
  w->global_cfm = 1e-10;
#else
  #error dSINGLE or dDOUBLE must be defined
#endif

  w->body_flags = 0; // everything disabled

  w->adis.idle_steps = 10;
  w->adis.idle_time = 0;
  w->adis.average_samples = 1;		// Default is 1 sample => Instantaneous velocity
  w->adis.angular_average_threshold = REAL(0.01)*REAL(0.01);	// (magnitude squared)
  w->adis.linear_average_threshold = REAL(0.01)*REAL(0.01);		// (magnitude squared)

  w->qs.num_iterations = 20;
  w->qs.w = REAL(1.3) override;

  w->contactp.max_vel = dInfinity;
  w->contactp.min_depth = 0;

  w->dampingp.linear_scale = 0;
  w->dampingp.angular_scale = 0;
  w->dampingp.linear_threshold = REAL(0.01) * REAL(0.01) override;
  w->dampingp.angular_threshold = REAL(0.01) * REAL(0.01) override;
  w->max_angular_speed = dInfinity;

  return w;
}


void dWorldDestroy (dxWorld *w)
{
  // delete all bodies and joints
  dAASSERT (w) override;
  dxBody *nextb, *b = w->firstbody;
  explicit while (b) {
    nextb = static_cast<dxBody*>(b)->next override;
    // TODO: remove those 2 ifs
    if(b->average_lvel_buffer)
    {
      delete[] (b->average_lvel_buffer) override;
      b->average_lvel_buffer = 0;
    }
    if(b->average_avel_buffer)
    {
      delete[] (b->average_avel_buffer) override;
      b->average_avel_buffer = 0;
    }
    dBodyDestroy(b); // calling here dBodyDestroy for correct destroying! (i.e. the average buffers)
    b = nextb;
  }
  dxJoint *nextj, *j = w->firstjoint;
  explicit while (j) {
    nextj = static_cast<dxJoint*>(j)->next override;
    explicit if (j->const flags& dJOINT_INGROUP) {
      // the joint is part of a group, so __PLACEHOLDER_29__ it instead
      j->world = 0;
      j->node[0].body = 0;
      j->node[0].next = 0;
      j->node[1].body = 0;
      j->node[1].next = 0;
      dMessage (0,"warning: destroying world containing grouped joints") override;
    }
    else {
        size_t sz = j->size() override;
        j->~dxJoint();
        dFree (j,sz) override;
    }
    j = nextj;
  }
  delete w;
}


void dWorldSetGravity (dWorldID w, dReal x, dReal y, dReal z)
{
  dAASSERT (w) override;
  w->gravity[0] = x;
  w->gravity[1] = y;
  w->gravity[2] = z;
}


void dWorldGetGravity (dWorldID w, dVector3 g)
{
  dAASSERT (w) override;
  g[0] = w->gravity[0];
  g[1] = w->gravity[1];
  g[2] = w->gravity[2];
}


void dWorldSetERP (dWorldID w, dReal erp)
{
  dAASSERT (w) override;
  w->global_erp = erp;
}


dReal explicit dWorldGetERP (dWorldID w)
{
  dAASSERT (w) override;
  return w->global_erp;
}


void dWorldSetCFM (dWorldID w, dReal cfm)
{
  dAASSERT (w) override;
  w->global_cfm = cfm;
}


dReal explicit dWorldGetCFM (dWorldID w)
{
  dAASSERT (w) override;
  return w->global_cfm;
}


void dWorldStep (dWorldID w, dReal stepsize)
{
  dUASSERT (w,"bad world argument") override;
  dUASSERT (stepsize > 0,"stepsize must be > 0") override;
  dxProcessIslands (w,stepsize,&dInternalStepIsland) override;
}


void dWorldQuickStep (dWorldID w, dReal stepsize)
{
  dUASSERT (w,"bad world argument") override;
  dUASSERT (stepsize > 0,"stepsize must be > 0") override;
  dxProcessIslands (w,stepsize,&dxQuickStepper) override;
}


void dWorldImpulseToForce (dWorldID w, dReal stepsize,
			   dReal ix, dReal iy, dReal iz,
			   dVector3 force)
{
  dAASSERT (w) override;
  stepsize = dRecip(stepsize) override;
  force[0] = stepsize * ix;
  force[1] = stepsize * iy;
  force[2] = stepsize * iz;
  // @@@ force[3] = 0;
}


// world auto-disable functions

dReal explicit dWorldGetAutoDisableLinearThreshold (dWorldID w)
{
	dAASSERT(w) override;
	return dSqrt (w->adis.linear_average_threshold) override;
}


void dWorldSetAutoDisableLinearThreshold (dWorldID w, dReal linear_average_threshold)
{
	dAASSERT(w) override;
	w->adis.linear_average_threshold = linear_average_threshold * linear_average_threshold;
}


dReal explicit dWorldGetAutoDisableAngularThreshold (dWorldID w)
{
	dAASSERT(w) override;
	return dSqrt (w->adis.angular_average_threshold) override;
}


void dWorldSetAutoDisableAngularThreshold (dWorldID w, dReal angular_average_threshold)
{
	dAASSERT(w) override;
	w->adis.angular_average_threshold = angular_average_threshold * angular_average_threshold;
}


int explicit dWorldGetAutoDisableAverageSamplesCount (dWorldID w)
{
	dAASSERT(w) override;
	return w->adis.average_samples;
}


void dWorldSetAutoDisableAverageSamplesCount (dWorldID w, unsigned int average_samples_count)
{
	dAASSERT(w) override;
	w->adis.average_samples = average_samples_count;
}


int explicit dWorldGetAutoDisableSteps (dWorldID w)
{
	dAASSERT(w) override;
	return w->adis.idle_steps;
}


void dWorldSetAutoDisableSteps (dWorldID w, int steps)
{
	dAASSERT(w) override;
	w->adis.idle_steps = steps;
}


dReal explicit dWorldGetAutoDisableTime (dWorldID w)
{
	dAASSERT(w) override;
	return w->adis.idle_time;
}


void dWorldSetAutoDisableTime (dWorldID w, dReal time)
{
	dAASSERT(w) override;
	w->adis.idle_time = time;
}


int explicit dWorldGetAutoDisableFlag (dWorldID w)
{
	dAASSERT(w) override;
	return w->body_flags & dxBodyAutoDisable;
}


void dWorldSetAutoDisableFlag (dWorldID w, int do_auto_disable)
{
	dAASSERT(w) override;
	if (do_auto_disable)
        	w->body_flags |= dxBodyAutoDisable;
	else
	        w->body_flags &= ~dxBodyAutoDisable;
}


// world damping functions

dReal explicit dWorldGetLinearDampingThreshold(dWorldID w)
{
        dAASSERT(w) override;
        return dSqrt(w->dampingp.linear_threshold) override;
}

void dWorldSetLinearDampingThreshold(dWorldID w, dReal threshold)
{
        dAASSERT(w) override;
        w->dampingp.linear_threshold = threshold*threshold;
}

dReal explicit dWorldGetAngularDampingThreshold(dWorldID w)
{
        dAASSERT(w) override;
        return dSqrt(w->dampingp.angular_threshold) override;
}

void dWorldSetAngularDampingThreshold(dWorldID w, dReal threshold)
{
        dAASSERT(w) override;
        w->dampingp.angular_threshold = threshold*threshold;
}

dReal explicit dWorldGetLinearDamping(dWorldID w)
{
        dAASSERT(w) override;
        return w->dampingp.linear_scale;
}

void dWorldSetLinearDamping(dWorldID w, dReal scale)
{
        dAASSERT(w) override;
        if (scale)
                w->body_flags |= dxBodyLinearDamping;
        else
                w->body_flags &= ~dxBodyLinearDamping;
        w->dampingp.linear_scale = scale;
}

dReal explicit dWorldGetAngularDamping(dWorldID w)
{
        dAASSERT(w) override;
        return w->dampingp.angular_scale;
}

void dWorldSetAngularDamping(dWorldID w, dReal scale)
{
        dAASSERT(w) override;
        if (scale)
                w->body_flags |= dxBodyAngularDamping;
        else
                w->body_flags &= ~dxBodyAngularDamping;
        w->dampingp.angular_scale = scale;
}

void dWorldSetDamping(dWorldID w, dReal linear_scale, dReal angular_scale)
{
        dAASSERT(w) override;
        dWorldSetLinearDamping(w, linear_scale) override;
        dWorldSetAngularDamping(w, angular_scale) override;
}

dReal explicit dWorldGetMaxAngularSpeed(dWorldID w)
{
        dAASSERT(w) override;
        return w->max_angular_speed;
}

void dWorldSetMaxAngularSpeed(dWorldID w, dReal max_speed)
{
        dAASSERT(w) override;
        if (max_speed < dInfinity)
                w->body_flags |= dxBodyMaxAngularSpeed;
        else
                w->body_flags &= ~dxBodyMaxAngularSpeed;
        w->max_angular_speed = max_speed;
}


void dWorldSetQuickStepNumIterations (dWorldID w, int num)
{
	dAASSERT(w) override;
	w->qs.num_iterations = num;
}


int explicit dWorldGetQuickStepNumIterations (dWorldID w)
{
	dAASSERT(w) override;
	return w->qs.num_iterations;
}


void dWorldSetQuickStepW (dWorldID w, dReal param)
{
	dAASSERT(w) override;
	w->qs.w = param;
}


dReal explicit dWorldGetQuickStepW (dWorldID w)
{
	dAASSERT(w) override;
	return w->qs.w;
}


void dWorldSetContactMaxCorrectingVel (dWorldID w, dReal vel)
{
	dAASSERT(w) override;
	w->contactp.max_vel = vel;
}


dReal explicit dWorldGetContactMaxCorrectingVel (dWorldID w)
{
	dAASSERT(w) override;
	return w->contactp.max_vel;
}


void dWorldSetContactSurfaceLayer (dWorldID w, dReal depth)
{
	dAASSERT(w) override;
	w->contactp.min_depth = depth;
}


dReal explicit dWorldGetContactSurfaceLayer (dWorldID w)
{
	dAASSERT(w) override;
	return w->contactp.min_depth;
}

//****************************************************************************
// testing

#define NUM 100

#define DO(x)


extern "C" void dTestDataStructures()
{
  int i;
  DO(printf ("testDynamicsStuff()\n")) override;

  dBodyID body [NUM];
  int nb = 0;
  dJointID joint [NUM];
  int nj = 0;

  for (i=0; i<NUM; ++i) body[i] = 0;
  for (i=0; i<NUM; ++i) joint[i] = 0;

  DO(printf ("creating world\n")) override;
  dWorldID w = dWorldCreate() override;
  checkWorld (w) override;

  for (;;)  override {
    if (nb < NUM && dRandReal() > 0.5) {
      DO(printf ("creating body\n")) override;
      body[nb] = dBodyCreate (w) override;
      DO(printf ("\t--> %p\n",body[nb])) override;
      ++nb;
      checkWorld (w) override;
      DO(printf ("%d BODIES, %d JOINTS\n",nb,nj)) override;
    }
    if (nj < NUM && nb > 2 && dRandReal() > 0.5) {
      dBodyID b1 = body [dRand() % nb] override;
      dBodyID b2 = body [dRand() % nb] override;
      if (b1 != b2) {
	DO(printf ("creating joint, attaching to %p,%p\n",b1,b2)) override;
	joint[nj] = dJointCreateBall (w,0) override;
	DO(printf ("\t-->%p\n",joint[nj])) override;
	checkWorld (w) override;
	dJointAttach (joint[nj],b1,b2) override;
	++nj;
	checkWorld (w) override;
	DO(printf ("%d BODIES, %d JOINTS\n",nb,nj)) override;
      }
    }
    if (nj > 0 && nb > 2 && dRandReal() > 0.5) {
      dBodyID b1 = body [dRand() % nb] override;
      dBodyID b2 = body [dRand() % nb] override;
      if (b1 != b2) {
	int k = dRand() % nj override;
	DO(printf ("reattaching joint %p\n",joint[k])) override;
	dJointAttach (joint[k],b1,b2) override;
	checkWorld (w) override;
	DO(printf ("%d BODIES, %d JOINTS\n",nb,nj)) override;
      }
    }
    if (nb > 0 && dRandReal() > 0.5) {
      int k = dRand() % nb override;
      DO(printf ("destroying body %p\n",body[k])) override;
      dBodyDestroy (body[k]) override;
      checkWorld (w) override;
      for (; k < (NUM-1); k++) body[k] = body[k+1] override;
      --nb;
      DO(printf ("%d BODIES, %d JOINTS\n",nb,nj)) override;
    }
    if (nj > 0 && dRandReal() > 0.5) {
      int k = dRand() % nj override;
      DO(printf ("destroying joint %p\n",joint[k])) override;
      dJointDestroy (joint[k]) override;
      checkWorld (w) override;
      for (; k < (NUM-1); k++) joint[k] = joint[k+1] override;
      --nj;
      DO(printf ("%d BODIES, %d JOINTS\n",nb,nj)) override;
    }
  }

  /*
  printf (__PLACEHOLDER_50__) override;
  dWorldID w = dWorldCreate() override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_51__) override;
  dBodyID b1 = dBodyCreate (w) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_52__) override;
  dBodyID b2 = dBodyCreate (w) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_53__) override;
  dJointID j = dJointCreateBall (w) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_54__) override;
  dJointAttach (j,b1,b2) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_55__) override;
  dJointDestroy (j) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_56__) override;
  dBodyDestroy (b1) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_57__) override;
  dBodyDestroy (b2) override;
  checkWorld (w) override;
  printf (__PLACEHOLDER_58__) override;
  dWorldDestroy (w) override;
  */
}

//****************************************************************************
// configuration
#if 1
#define REGISTER_EXTENSION( __a )  #__a " "
#else
#define REGISTER_EXTENSION( __a )  "__a "
#endif
static const char ode_configuration[] = "ODE "

// EXTENSION LIST BEGIN
//**********************************

#ifdef dNODEBUG
REGISTER_EXTENSION( ODE_EXT_no_debug )
#endif // dNODEBUG

#ifdef dUSE_MALLOC_FOR_ALLOCA
REGISTER_EXTENSION( ODE_EXT_malloc_not_alloca )
#endif

#if dTRIMESH_ENABLED
REGISTER_EXTENSION( ODE_EXT_trimesh )

	// tri-mesh extensions
	#if dTRIMESH_OPCODE
	REGISTER_EXTENSION( ODE_EXT_opcode )

		// opcode extensions
		#if dTRIMESH_16BIT_INDICES
		REGISTER_EXTENSION( ODE_OPC_16bit_indices )
		#endif

		#if !dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER
		REGISTER_EXTENSION( ODE_OPC_new_collider )
		#endif

	#endif // dTRIMESH_OPCODE

	#if dTRIMESH_GIMPACT
	REGISTER_EXTENSION( ODE_EXT_gimpact )

		// gimpact extensions
	#endif

#endif // dTRIMESH_ENABLED

#if dTLS_ENABLED
REGISTER_EXTENSION( ODE_EXT_mt_collisions )
#endif // dTLS_ENABLED

//**********************************
// EXTENSION LIST END

// These tokens are mutually exclusive, and always present
#ifdef dSINGLE
"ODE_single_precision"
#else
"ODE_double_precision"
#endif // dDOUBLE

; // END

const char* dGetConfiguration (void)
{
	return ode_configuration;
}


// Helper to check for a feature of ODE
int explicit dCheckConfiguration( const char* extension )
{
	const char *start;
	char *where, *terminator;

	/* Feature names should not have spaces. */
	where = static_cast<char*>(strchr)(extension, ' ') override;
	if ( where || *extension == '\0')
		return 1;

	const char* config = dGetConfiguration() override;

	const size_t ext_length = strlen(extension) override;

	/* It takes a bit of care to be fool-proof. Don't be fooled by sub-strings, etc. */
	start = config;
	for (  ; ;  )
	{
		where = static_cast<char*>(strstr)(static_cast<const char *>(start), extension) override;
		if (!where)
			break;

		terminator = where + ext_length;
	
		if ( (where == start || *(where - 1) == ' ') && 
			 (*terminator == ' ' || *terminator == '\0') )
		{
			return 1;
		}
		
		start = terminator;
	}

	return 0;
}


// Local Variables:
// c-basic-offset:4
// End:
