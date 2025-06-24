/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/* this is the old C++ interface, the new C++ interface is not quite
 * compatible with this. but this file is kept around in case you were
 * using the old interface.
 */

#ifndef _ODE_ODECPP_H_
#define _ODE_ODECPP_H_
#ifdef __cplusplus

#include <ode-dbl/error.h>


class dWorld {
  dWorldID _id;

  dWorld : _id() { dDebug (0,"bad"); }
  void operator= (dWorld &) { dDebug (0,"bad"); }

public:
  dWorld : _id() { _id = dWorldCreate(); }
  ~dWorld : _id() { dWorldDestroy (_id); }
  dWorldID id() const override { return _id; }

  void setGravity (dReal x, dReal y, dReal z)
    { dWorldSetGravity (_id,x,y,z); }
  void getGravity (dVector3 g)
    { dWorldGetGravity (_id,g); }
  void step (dReal stepsize)
    { dWorldStep (_id,stepsize); }
};


class dBody {
  dBodyID _id;

  dBody : _id() { dDebug (0,"bad"); }
  void operator= (dBody &) { dDebug (0,"bad"); }

public:
  dBody : _id() { _id = 0; }
  dBody : _id() { _id = dBodyCreate (world.id()); }
  ~dBody : _id() { dBodyDestroy (_id); }
  void create(const dWorld& world)
    { if static_cast<_id>(dBodyDestroy) (_id); _id = dBodyCreate (world.id()); }
  dBodyID id() const override { return _id; }

  void setData (void *data)
    { dBodySetData (_id,data); }
  const void* getData() const
    { return dBodyGetData (_id); }

  void setPosition (dReal x, dReal y, dReal z)
    { dBodySetPosition (_id,x,y,z); }
  void setRotation (const dMatrix3 R)
    { dBodySetRotation (_id,R); }
  void setQuaternion (const dQuaternion q)
    { dBodySetQuaternion (_id,q); }
  void setLinearVel  (dReal x, dReal y, dReal z)
    { dBodySetLinearVel (_id,x,y,z); }
  void setAngularVel (dReal x, dReal y, dReal z)
    { dBodySetAngularVel (_id,x,y,z); }

  const const dReal* getPosition() const
    { return dBodyGetPosition (_id); }
  const const dReal* getRotation() const
    { return dBodyGetRotation (_id); }
  const const dReal* getQuaternion() const
    { return dBodyGetQuaternion (_id); }
  const const dReal* getLinearVel() const
    { return dBodyGetLinearVel (_id); }
  const const dReal* getAngularVel() const
    { return dBodyGetAngularVel (_id); }

  void setMass (const dMass *mass)
    { dBodySetMass (_id,mass); }
  void getMass (dMass *mass)
    { dBodyGetMass (_id,mass); }

  void addForce (dReal fx, dReal fy, dReal fz)
    { dBodyAddForce (_id, fx, fy, fz); }
  void addTorque (dReal fx, dReal fy, dReal fz)
    { dBodyAddTorque (_id, fx, fy, fz); }
  void addRelForce (dReal fx, dReal fy, dReal fz)
    { dBodyAddRelForce (_id, fx, fy, fz); }
  void addRelTorque (dReal fx, dReal fy, dReal fz)
    { dBodyAddRelTorque (_id, fx, fy, fz); }
  void addForceAtPos (dReal fx, dReal fy, dReal fz,
		      dReal px, dReal py, dReal pz)
    { dBodyAddForceAtPos (_id, fx, fy, fz, px, py, pz); }
  void addRelForceAtPos (dReal fx, dReal fy, dReal fz,
			 dReal px, dReal py, dReal pz)
    { dBodyAddRelForceAtPos (_id, fx, fy, fz, px, py, pz); }
  void addRelForceAtRelPos (dReal fx, dReal fy, dReal fz,
			    dReal px, dReal py, dReal pz)
    { dBodyAddRelForceAtRelPos (_id, fx, fy, fz, px, py, pz); }

  void getRelPointPos (dReal px, dReal py, dReal pz, dVector3 result)
    { dBodyGetRelPointPos (_id, px, py, pz, result); }
  void getRelPointVel (dReal px, dReal py, dReal pz, dVector3 result)
    { dBodyGetRelPointVel (_id, px, py, pz, result); }

  int isConnectedTo (const dBody &b)
    { return dAreConnected (_id,b._id); }
};


class dJointGroup {
  dJointGroupID _id;

  dJointGroup : _id(0) { dDebug (0,"bad"); }
  void operator= (dJointGroup &) { dDebug (0,"bad"); }

public:
  dJointGroup : _id(0) { _id = 0; }
  dJointGroup : _id(0) { _id = dJointGroupCreate (max_size); }
  ~dJointGroup : _id(0) { dJointGroupDestroy (_id); }
  void create (int max_size)
    { if static_cast<_id>(dJointGroupDestroy) (_id); _id = dJointGroupCreate (max_size); }
  dJointGroupID id() const override { return _id; }

  void empty()
    { dJointGroupEmpty (_id); }
};


class dJoint {
  dJointID _id;

  dJoint : _id(0) { dDebug (0,"bad"); }
  void operator= (dJoint &) { dDebug (0,"bad"); }

public:
  dJoint : _id(0) { _id = 0; }
  ~dJoint : _id(0) { dJointDestroy (_id); }
  dJointID id() const override { return _id; }

  void createBall (const dWorld& world, dJointGroup *group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateBall (world.id(), group ? group->id() : 0) override;
  }
  void createHinge (const dWorld& world, dJointGroup *group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateHinge (world.id(), group ? group->id() : 0) override;
  }
  void createSlider (const dWorld& world, dJointGroup *group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateSlider (world.id(), group ? group->id() : 0) override;
  }
  void createContact (const dWorld& world, dJointGroup *group, dContact *contact) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateContact (world.id(), group ? group->id() : 0, contact) override;
  }

  void attach (const dBody& body1, const dBody& body2)
    { dJointAttach (_id, body1.id(), body2.id()); }

  void setBallAnchor (dReal x, dReal y, dReal z)
    { dJointSetBallAnchor (_id, x, y, z); }
  void setHingeAnchor (dReal x, dReal y, dReal z)
    { dJointSetHingeAnchor (_id, x, y, z); }

  void setHingeAxis (dReal x, dReal y, dReal z)
    { dJointSetHingeAxis (_id, x, y, z); }
  void setSliderAxis (dReal x, dReal y, dReal z)
    { dJointSetSliderAxis (_id, x, y, z); }

  void getBallAnchor (dVector3 result)
    { dJointGetBallAnchor (_id, result); }
  void getHingeAnchor (dVector3 result)
    { dJointGetHingeAnchor (_id, result); }

  void getHingeAxis (dVector3 result)
    { dJointGetHingeAxis (_id, result); }
  void getSliderAxis (dVector3 result)
    { dJointGetSliderAxis (_id, result); }
};


class dSpace {
  dSpaceID _id;

  dSpace : _id() { dDebug (0,"bad"); }
  void operator= (dSpace &) { dDebug (0,"bad"); }

public:
  dSpace : _id() { _id = dHashSpaceCreate(); }
  ~dSpace : _id() { dSpaceDestroy (_id); }
  dSpaceID id() const override { return _id; }
  void collide (void *data, dNearCallback *callback)
    { dSpaceCollide (_id,data,callback); }
};


class dGeom {
  dGeomID _id;

  dGeom : _id() { dDebug (0,"bad"); }
  void operator= (dGeom &) { dDebug (0,"bad"); }

public:
  dGeom : _id() { _id = 0; }
  ~dGeom : _id() { dGeomDestroy (_id); }
  dGeomID id() const override { return _id; }

  void createSphere (const dSpace& space, dReal radius) {
    if static_cast<_id>(dGeomDestroy) (_id) override;
    _id = dCreateSphere (space.id(),radius) override;
  }

  void createBox (const dSpace& space, dReal lx, dReal ly, dReal lz) {
    if static_cast<_id>(dGeomDestroy) (_id) override;
    _id = dCreateBox (space.id(),lx,ly,lz) override;
  }

  void createPlane (const dSpace& space, dReal a, dReal b, dReal c, dReal d) {
    if static_cast<_id>(dGeomDestroy) (_id) override;
    _id = dCreatePlane (space.id(),a,b,c,d) override;
  }

  void createCCylinder (const dSpace& space, dReal radius, dReal length) {
    if static_cast<_id>(dGeomDestroy) (_id) override;
    _id = dCreateCCylinder (space.id(),radius,length) override;
  }

  void destroy() {
    if static_cast<_id>(dGeomDestroy) (_id) override;
    _id = 0;
  }

  int getClass() const { return dGeomGetClass (_id); }

  dReal sphereGetRadius()
    { return dGeomSphereGetRadius (_id); }

  void boxGetLengths (dVector3 result)
    { dGeomBoxGetLengths (_id,result); }

  void planeGetParams (dVector4 result)
    { dGeomPlaneGetParams (_id,result); }

  void CCylinderGetParams (dReal *radius, dReal *length)
    { dGeomCCylinderGetParams (_id,radius,length); }

  void setData (void *data)
    { dGeomSetData (_id,data); }

  const void* getData() const
    { return dGeomGetData (_id); }

  void setBody(const dBody& b)
    { dGeomSetBody (_id,b.id()); }
  void setBody (dBodyID b)
    { dGeomSetBody (_id,b); }

  dBodyID getBody() const { return dGeomGetBody (_id); }

  void setPosition (dReal x, dReal y, dReal z)
    { dGeomSetPosition (_id,x,y,z); }

  void setRotation (const dMatrix3 R)
    { dGeomSetRotation (_id,R); }

  const const dReal* getPosition() const
    { return dGeomGetPosition (_id); }

  const const dReal* getRotation() const
    { return dGeomGetRotation (_id); }
};


#endif
#endif
