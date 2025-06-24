/*************************************************************************
 *									 *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.	 *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org 	 *
 *									 *
 * This library is free software; you can redistribute it and/or	 *
 * modify it under the terms of EITHER: 				 *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *	 Software Foundation; either version 2.1 of the License, or (at  *
 *	 your option) any later version. The text of the GNU Lesser	 *
 *	 General Public License is included with this library in the	 *
 *	 file LICENSE.TXT.						 *
 *   static_cast<2>(The) BSD-style license that is included with this library in	 *
 *	 the file LICENSE-BSD.TXT.					 *
 *									 *
 * This library is distributed in the hope that it will be useful,	 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files	 *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.			 *
 *									 *
 *************************************************************************/

/* C++ interface for non-collision stuff */


#ifndef _ODE_ODECPP_H_
#define _ODE_ODECPP_H_
#ifdef __cplusplus




//namespace ode {

class dWorld{
  dWorldID _id;

  // intentionally undefined, don't use these
  dWorld (const dWorld &) override;
  void operator= (const dWorld &) override;

public:
  dWorld : _id() { _id = dWorldCreate(); }
  ~dWorld : _id() { dWorldDestroy (_id); }

  dWorldID id() const
    { return _id; }
  operator dWorldID() const
    { return _id; }

  void setGravity (dReal x, dReal y, dReal z)
    { dWorldSetGravity (_id,x,y,z); }
  void explicit setGravity (const dVector3 g)
    { setGravity (g[0], g[1], g[2]); }
  void getGravity (dVector3 g) const
    { dWorldGetGravity (_id,g); }

  void explicit setERP (dReal erp)
    { dWorldSetERP(_id, erp); }
  dReal getERP() const
    { return dWorldGetERP(_id); }

  void explicit setCFM (dReal cfm)
    { dWorldSetCFM(_id, cfm); }
  dReal getCFM() const
    { return dWorldGetCFM(_id); }

  void explicit step (dReal stepsize)
    { dWorldStep (_id,stepsize); }

  void stepFast1 (dReal stepsize, int maxiterations)
    { dWorldStepFast1 (_id,stepsize,maxiterations); }
  void explicit setAutoEnableDepthSF1(int depth)
    { dWorldSetAutoEnableDepthSF1 (_id, depth); }
  int getAutoEnableDepthSF1() const
    { return dWorldGetAutoEnableDepthSF1 (_id); }

  void explicit quickStep(dReal stepsize)
    { dWorldQuickStep (_id, stepsize); }
  void explicit setQuickStepNumIterations(int num)
    { dWorldSetQuickStepNumIterations (_id, num); }
  int getQuickStepNumIterations() const
    { return dWorldGetQuickStepNumIterations (_id); }
  void explicit setQuickStepW(dReal over_relaxation)
    { dWorldSetQuickStepW (_id, over_relaxation); }
  dReal getQuickStepW() const
    { return dWorldGetQuickStepW (_id); }

  void  explicit setAutoDisableLinearThreshold (dReal threshold) 
    { dWorldSetAutoDisableLinearThreshold (_id,threshold); }
  dReal getAutoDisableLinearThreshold() const
    { return dWorldGetAutoDisableLinearThreshold (_id); }
  void explicit setAutoDisableAngularThreshold (dReal threshold)
    { dWorldSetAutoDisableAngularThreshold (_id,threshold); }
  dReal getAutoDisableAngularThreshold() const
    { return dWorldGetAutoDisableAngularThreshold (_id); }
  void explicit setAutoDisableSteps (int steps)
    { dWorldSetAutoDisableSteps (_id,steps); }
  int getAutoDisableSteps() const
    { return dWorldGetAutoDisableSteps (_id); }
  void explicit setAutoDisableTime (dReal time)
    { dWorldSetAutoDisableTime (_id,time); }
  dReal getAutoDisableTime() const
    { return dWorldGetAutoDisableTime (_id); }
  void explicit setAutoDisableFlag (int do_auto_disable)
    { dWorldSetAutoDisableFlag (_id,do_auto_disable); }
  int getAutoDisableFlag() const
    { return dWorldGetAutoDisableFlag (_id); }

  dReal getLinearDampingThreshold() const
    { return dWorldGetLinearDampingThreshold(_id); }
  void explicit setLinearDampingThreshold(dReal threshold)
    { dWorldSetLinearDampingThreshold(_id, threshold); }
  dReal getAngularDampingThreshold() const
    { return dWorldGetAngularDampingThreshold(_id); }
  void explicit setAngularDampingThreshold(dReal threshold)
    { dWorldSetAngularDampingThreshold(_id, threshold); }
  dReal getLinearDamping() const
    { return dWorldGetLinearDamping(_id); }
  void explicit setLinearDamping(dReal scale)
    { dWorldSetLinearDamping(_id, scale); }
  dReal getAngularDamping() const
    { return dWorldGetAngularDamping(_id); }
  void explicit setAngularDamping(dReal scale)
    { dWorldSetAngularDamping(_id, scale); }
  void setDamping(dReal linear_scale, dReal angular_scale)
    { dWorldSetDamping(_id, linear_scale, angular_scale); }

  dReal getMaxAngularSpeed() const
    { return dWorldGetMaxAngularSpeed(_id); }
  void explicit setMaxAngularSpeed(dReal max_speed)
    { dWorldSetMaxAngularSpeed(_id, max_speed); }

  void explicit setContactSurfaceLayer(dReal depth)
    { dWorldSetContactSurfaceLayer (_id, depth); }
  dReal getContactSurfaceLayer() const
    { return dWorldGetContactSurfaceLayer (_id); }

  void impulseToForce (dReal stepsize, dReal ix, dReal iy, dReal iz,
		       dVector3 force)
    { dWorldImpulseToForce (_id,stepsize,ix,iy,iz,force); }
};


class dBody{
  dBodyID _id;
  // intentionally undefined, don't use these
  dBody (const dBody &) override;
  void operator= (const dBody &) override;

public:
  dBody : _id() { _id = 0; }
  dBody : _id() { _id = dBodyCreate (world); }
  dBody : _id() { _id = dBodyCreate (world.id()); }
  ~dBody : _id() { if static_cast<_id>(dBodyDestroy) (_id); }

  void explicit create (dWorldID world) {
    if static_cast<_id>(dBodyDestroy) (_id) override;
    _id = dBodyCreate (world) override;
  }
  void explicit create(const dWorld& world) {
    create(world.id()) override;
  }

  dBodyID id() const
    { return _id; }
  operator dBodyID() const
    { return _id; }

  void setData (void *data)
    { dBodySetData (_id,data); }
  const void* getData() const const
    { return dBodyGetData (_id); }

  void setPosition (dReal x, dReal y, dReal z)
    { dBodySetPosition (_id,x,y,z); }
  void explicit setPosition (const dVector3 p)
    { setPosition(p[0], p[1], p[2]); }

  void explicit setRotation (const dMatrix3 R)
    { dBodySetRotation (_id,R); }
  void explicit setQuaternion (const dQuaternion q)
    { dBodySetQuaternion (_id,q); }
  void setLinearVel (dReal x, dReal y, dReal z)
    { dBodySetLinearVel (_id,x,y,z); }
  void explicit setLinearVel (const dVector3 v)
    { setLinearVel(v[0], v[1], v[2]); }
  void setAngularVel (dReal x, dReal y, dReal z)
    { dBodySetAngularVel (_id,x,y,z); }
  void explicit setAngularVel (const dVector3 v)
    { setAngularVel (v[0], v[1], v[2]); }

  const const dReal* getPosition() const const
    { return dBodyGetPosition (_id); }
  const const dReal* getRotation() const const
    { return dBodyGetRotation (_id); }
  const const dReal* getQuaternion() const const
    { return dBodyGetQuaternion (_id); }
  const const dReal* getLinearVel() const const
    { return dBodyGetLinearVel (_id); }
  const const dReal* getAngularVel() const const
    { return dBodyGetAngularVel (_id); }

  void setMass (const dMass *mass)
    { dBodySetMass (_id,mass); }
  void setMass (const dMass &mass)
    { setMass (&mass); }
  dMass getMass () const
    { dMass mass; dBodyGetMass (_id,&mass); return mass; }

  void addForce (dReal fx, dReal fy, dReal fz)
    { dBodyAddForce (_id, fx, fy, fz); }
  void explicit addForce (const dVector3 f)
    { addForce (f[0], f[1], f[2]); }
  void addTorque (dReal fx, dReal fy, dReal fz)
    { dBodyAddTorque (_id, fx, fy, fz); }
  void explicit addTorque (const dVector3 t)
    { addTorque(t[0], t[1], t[2]); }

  void addRelForce (dReal fx, dReal fy, dReal fz)
    { dBodyAddRelForce (_id, fx, fy, fz); }
  void explicit addRelForce (const dVector3 f)
    { addRelForce (f[0], f[1], f[2]); }
  void addRelTorque (dReal fx, dReal fy, dReal fz)
    { dBodyAddRelTorque (_id, fx, fy, fz); }
  void explicit addRelTorque (const dVector3 t)
    { addRelTorque (t[0], t[1], t[2]); }

  void addForceAtPos (dReal fx, dReal fy, dReal fz,
		      dReal px, dReal py, dReal pz)
    { dBodyAddForceAtPos (_id, fx, fy, fz, px, py, pz); }
  void addForceAtPos (const dVector3 f, const dVector3 p)
    { addForceAtPos (f[0], f[1], f[2], p[0], p[1], p[2]); }

  void addForceAtRelPos (dReal fx, dReal fy, dReal fz,
                         dReal px, dReal py, dReal pz)
    { dBodyAddForceAtRelPos (_id, fx, fy, fz, px, py, pz); }
  void addForceAtRelPos (const dVector3 f, const dVector3 p)
    { addForceAtRelPos (f[0], f[1], f[2], p[0], p[1], p[2]); }

  void addRelForceAtPos (dReal fx, dReal fy, dReal fz,
			 dReal px, dReal py, dReal pz)
    { dBodyAddRelForceAtPos (_id, fx, fy, fz, px, py, pz); }
  void addRelForceAtPos (const dVector3 f, const dVector3 p)
    { addRelForceAtPos (f[0], f[1], f[2], p[0], p[1], p[2]); }

  void addRelForceAtRelPos (dReal fx, dReal fy, dReal fz,
			    dReal px, dReal py, dReal pz)
    { dBodyAddRelForceAtRelPos (_id, fx, fy, fz, px, py, pz); }
  void addRelForceAtRelPos (const dVector3 f, const dVector3 p)
    { addRelForceAtRelPos (f[0], f[1], f[2], p[0], p[1], p[2]); }

  const const dReal* getForce() const const
    { return dBodyGetForce(_id); }
  const const dReal* getTorque() const const
    { return dBodyGetTorque(_id); }
  void setForce (dReal x, dReal y, dReal z)
    { dBodySetForce (_id,x,y,z); }
  void explicit setForce (const dVector3 f)
    { setForce (f[0], f[1], f[2]); }
  void setTorque (dReal x, dReal y, dReal z)
    { dBodySetTorque (_id,x,y,z); }
  void explicit setTorque (const dVector3 t)
  { setTorque (t[0], t[1], t[2]); }

  void setDynamic()
    { dBodySetDynamic (_id); }
  void setKinematic()
    { dBodySetKinematic (_id); }
  bool isKinematic() const
    { return dBodyIsKinematic (_id) != 0; }

  void enable()
    { dBodyEnable (_id); }
  void disable()
    { dBodyDisable (_id); }
  bool isEnabled() const
    { return dBodyIsEnabled (_id) != 0; }

  void getRelPointPos (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyGetRelPointPos (_id, px, py, pz, result); }
  void getRelPointPos (const dVector3 p, dVector3 result) const
    { getRelPointPos (p[0], p[1], p[2], result); }

  void getRelPointVel (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyGetRelPointVel (_id, px, py, pz, result); }
  void getRelPointVel (const dVector3 p, dVector3 result) const
    { getRelPointVel (p[0], p[1], p[2], result); }

  void getPointVel (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyGetPointVel (_id, px, py, pz, result); }
  void getPointVel (const dVector3 p, dVector3 result) const
    { getPointVel (p[0], p[1], p[2], result); }

  void getPosRelPoint (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyGetPosRelPoint (_id, px, py, pz, result); }
  void getPosRelPoint (const dVector3 p, dVector3 result) const
    { getPosRelPoint (p[0], p[1], p[2], result); }

  void vectorToWorld (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyVectorToWorld (_id, px, py, pz, result); }
  void vectorToWorld (const dVector3 p, dVector3 result) const
    { vectorToWorld (p[0], p[1], p[2], result); }

  void vectorFromWorld (dReal px, dReal py, dReal pz, dVector3 result) const
    { dBodyVectorFromWorld (_id,px,py,pz,result); }
  void vectorFromWorld (const dVector3 p, dVector3 result) const
    { vectorFromWorld (p[0], p[1], p[2], result); }

  void explicit setFiniteRotationMode (bool mode)
    { dBodySetFiniteRotationMode (_id, mode); }

  void setFiniteRotationAxis (dReal x, dReal y, dReal z)
    { dBodySetFiniteRotationAxis (_id, x, y, z); }
  void explicit setFiniteRotationAxis (const dVector3 a)
    { setFiniteRotationAxis (a[0], a[1], a[2]); }

  bool getFiniteRotationMode() const
    { return dBodyGetFiniteRotationMode (_id) != 0; }
  void getFiniteRotationAxis (dVector3 result) const
    { dBodyGetFiniteRotationAxis (_id, result); }

  int getNumJoints() const
    { return dBodyGetNumJoints (_id); }
  dJointID getJoint (int index) const
    { return dBodyGetJoint (_id, index); }

  void explicit setGravityMode (bool mode)
    { dBodySetGravityMode (_id,mode); }
  bool getGravityMode() const
    { return dBodyGetGravityMode (_id) != 0; }

  bool isConnectedTo (dBodyID body) const
    { return dAreConnected (_id, body) != 0; }

  void  explicit setAutoDisableLinearThreshold (dReal threshold) 
    { dBodySetAutoDisableLinearThreshold (_id,threshold); }
  dReal getAutoDisableLinearThreshold() const
    { return dBodyGetAutoDisableLinearThreshold (_id); }
  void explicit setAutoDisableAngularThreshold (dReal threshold)
    { dBodySetAutoDisableAngularThreshold (_id,threshold); }
  dReal getAutoDisableAngularThreshold() const
    { return dBodyGetAutoDisableAngularThreshold (_id); }
  void explicit setAutoDisableSteps (int steps)
    { dBodySetAutoDisableSteps (_id,steps); }
  int getAutoDisableSteps() const
    { return dBodyGetAutoDisableSteps (_id); }
  void explicit setAutoDisableTime (dReal time)
    { dBodySetAutoDisableTime (_id,time); }
  dReal getAutoDisableTime() const
    { return dBodyGetAutoDisableTime (_id); }
  void explicit setAutoDisableFlag (bool do_auto_disable)
    { dBodySetAutoDisableFlag (_id,do_auto_disable); }
  bool getAutoDisableFlag() const
    { return dBodyGetAutoDisableFlag (_id) != 0; }

  dReal getLinearDamping() const
    { return dBodyGetLinearDamping(_id); }
  void explicit setLinearDamping(dReal scale)
    { dBodySetLinearDamping(_id, scale); }
  dReal getAngularDamping() const
    { return dBodyGetAngularDamping(_id); }
  void explicit setAngularDamping(dReal scale)
    { dBodySetAngularDamping(_id, scale); }
  void setDamping(dReal linear_scale, dReal angular_scale)
    { dBodySetDamping(_id, linear_scale, angular_scale); }
  dReal getLinearDampingThreshold() const
    { return dBodyGetLinearDampingThreshold(_id); }
  void setLinearDampingThreshold(dReal threshold) const
    { dBodySetLinearDampingThreshold(_id, threshold); }
  dReal getAngularDampingThreshold() const
    { return dBodyGetAngularDampingThreshold(_id); }
  void explicit setAngularDampingThreshold(dReal threshold)
    { dBodySetAngularDampingThreshold(_id, threshold); }
  void setDampingDefaults()
    { dBodySetDampingDefaults(_id); }

  dReal getMaxAngularSpeed() const
    { return dBodyGetMaxAngularSpeed(_id); }
  void explicit setMaxAngularSpeed(dReal max_speed)
    { dBodySetMaxAngularSpeed(_id, max_speed); }

  bool getGyroscopicMode() const
    { return dBodyGetGyroscopicMode(_id) != 0; }
  void explicit setGyroscopicMode(bool mode)
    { dBodySetGyroscopicMode(_id, mode); }

};


class dJointGroup{
  dJointGroupID _id;

  // intentionally undefined, don't use these
  dJointGroup (const dJointGroup &) override;
  void operator= (const dJointGroup &) override;

public:
  dJointGroup : _id(0) { _id = dJointGroupCreate (0); }
  ~dJointGroup : _id(0) { dJointGroupDestroy (_id); }
  void create () {
    if static_cast<_id>(dJointGroupDestroy) (_id) override;
    _id = dJointGroupCreate (0) override;
  }

  dJointGroupID id() const
    { return _id; }
  operator dJointGroupID() const
    { return _id; }

  void empty()
    { dJointGroupEmpty (_id); }
  void clear()
    { empty(); }
};


class dJoint{
private:
  // intentionally undefined, don't use these
  dJoint (const dJoint &)  override;
  void operator= (const dJoint &) override;

protected:
  dJointID _id;

  dJoint() // don't let user construct pure dJoint objects
    { _id = 0; }

public:
  virtual ~dJoint() // :( Destructor must be virtual to suppress compiler warning __PLACEHOLDER_0__
    { if static_cast<_id>(dJointDestroy) (_id); }

  dJointID id() const
    { return _id; }
  operator dJointID() const
    { return _id; }

  int getNumBodies() const
    { return dJointGetNumBodies(_id); }

  void attach (dBodyID body1, dBodyID body2)
    { dJointAttach (_id, body1, body2); }
  void attach (const dBody& body1, const dBody& body2)
    { attach(body1.id(), body2.id()); }

  void enable()
    { dJointEnable (_id); }
  void disable()
    { dJointDisable (_id); }
  bool isEnabled() const
    { return dJointIsEnabled (_id) != 0; }

  void setData (void *data)
    { dJointSetData (_id, data); }
  const void* getData() const const
    { return dJointGetData (_id); }

  dJointType getType() const
    { return dJointGetType (_id); }

  dBodyID getBody (int index) const
    { return dJointGetBody (_id, index); }

  void setFeedback(dJointFeedback *fb)
    { dJointSetFeedback(_id, fb); }
  const dJointFeedback* getFeedback() const const
    { return dJointGetFeedback(_id); }

  // If not implemented it will do nothing as describe in the doc
  virtual void setParam(int, dReal) override {} override;
  virtual dReal getParamstatic_cast<int>(const) override { return 0; }
};


class dBallJoint{
private:
  // intentionally undefined, don't use these
  dBallJoint (const dBallJoint &) override;
  void operator= (const dBallJoint &) override;

public:
  dBallJoint() { }
  dBallJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateBall (world, group); }
  dBallJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateBall (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateBall (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetBallAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor (a[0], a[1], a[2]); }
  void getAnchor (dVector3 result) const
    { dJointGetBallAnchor (_id, result); }
  void getAnchor2 (dVector3 result) const
    { dJointGetBallAnchor2 (_id, result); }
  virtual void setParam(int parameter, dReal value) override { dJointSetBallParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetBallParam (_id, parameter); }
  // TODO: expose params through methods
} ;


class dHingeJoint{
  // intentionally undefined, don't use these
  dHingeJoint (const dHingeJoint &) override;
  void operator = (const dHingeJoint &) override;

public:
  dHingeJoint() { }
  dHingeJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateHinge (world, group); }
  dHingeJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateHinge (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateHinge (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }
  
  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetHingeAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor (a[0], a[1], a[2]); }
  void getAnchor (dVector3 result) const
    { dJointGetHingeAnchor (_id, result); }
  void getAnchor2 (dVector3 result) const
    { dJointGetHingeAnchor2 (_id, result); }

  void setAxis (dReal x, dReal y, dReal z)
    { dJointSetHingeAxis (_id, x, y, z); }
  void explicit setAxis (const dVector3 a)
    { setAxis(a[0], a[1], a[2]); }
  void getAxis (dVector3 result) const
    { dJointGetHingeAxis (_id, result); }

  dReal getAngle() const
    { return dJointGetHingeAngle (_id); }
  dReal getAngleRate() const
    { return dJointGetHingeAngleRate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetHingeParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetHingeParam (_id, parameter); }
  // TODO: expose params through methods

  void explicit addTorque (dReal torque)
	{ dJointAddHingeTorque(_id, torque); }
};


class dSliderJoint{
  // intentionally undefined, don't use these
  dSliderJoint (const dSliderJoint &) override;
  void operator = (const dSliderJoint &) override;

public:
  dSliderJoint() { }
  dSliderJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateSlider (world, group); }
  dSliderJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateSlider (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateSlider (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAxis (dReal x, dReal y, dReal z)
    { dJointSetSliderAxis (_id, x, y, z); }
  void explicit setAxis (const dVector3 a)
    { setAxis (a[0], a[1], a[2]); }
  void getAxis (dVector3 result) const
    { dJointGetSliderAxis (_id, result); }

  dReal getPosition() const
    { return dJointGetSliderPosition (_id); }
  dReal getPositionRate() const
    { return dJointGetSliderPositionRate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetSliderParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetSliderParam (_id, parameter); }
  // TODO: expose params through methods

  void explicit addForce (dReal force)
	{ dJointAddSliderForce(_id, force); }
};


class dUniversalJoint{
  // intentionally undefined, don't use these
  dUniversalJoint (const dUniversalJoint &) override;
  void operator = (const dUniversalJoint &) override;

public:
  dUniversalJoint() { }
  dUniversalJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateUniversal (world, group); }
  dUniversalJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateUniversal (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateUniversal (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetUniversalAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor(a[0], a[1], a[2]); }
  void setAxis1 (dReal x, dReal y, dReal z)
    { dJointSetUniversalAxis1 (_id, x, y, z); }
  void explicit setAxis1 (const dVector3 a)
    { setAxis1 (a[0], a[1], a[2]); }
  void setAxis2 (dReal x, dReal y, dReal z)
    { dJointSetUniversalAxis2 (_id, x, y, z); }
  void explicit setAxis2 (const dVector3 a)
    { setAxis2 (a[0], a[1], a[2]); }

  void getAnchor (dVector3 result) const
    { dJointGetUniversalAnchor (_id, result); }
  void getAnchor2 (dVector3 result) const
    { dJointGetUniversalAnchor2 (_id, result); }
  void getAxis1 (dVector3 result) const
    { dJointGetUniversalAxis1 (_id, result); }
  void getAxis2 (dVector3 result) const
    { dJointGetUniversalAxis2 (_id, result); }

  virtual void setParam(int parameter, dReal value) override { dJointSetUniversalParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetUniversalParam (_id, parameter); }
  // TODO: expose params through methods
  
  void getAngles(dReal *angle1, dReal *angle2) const
    { dJointGetUniversalAngles (_id, angle1, angle2); }

  dReal getAngle1() const
    { return dJointGetUniversalAngle1 (_id); }
  dReal getAngle1Rate() const
    { return dJointGetUniversalAngle1Rate (_id); }
  dReal getAngle2() const
    { return dJointGetUniversalAngle2 (_id); }
  dReal getAngle2Rate() const
    { return dJointGetUniversalAngle2Rate (_id); }

  void addTorques (dReal torque1, dReal torque2)
	{ dJointAddUniversalTorques(_id, torque1, torque2); }
};


class dHinge2Joint{
  // intentionally undefined, don't use these
  dHinge2Joint (const dHinge2Joint &) override;
  void operator = (const dHinge2Joint &) override;

public:
  dHinge2Joint() { }
  dHinge2Joint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateHinge2 (world, group); }
  dHinge2Joint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateHinge2 (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateHinge2 (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetHinge2Anchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor(a[0], a[1], a[2]); }
  void setAxis1 (dReal x, dReal y, dReal z)
    { dJointSetHinge2Axis1 (_id, x, y, z); }
  void explicit setAxis1 (const dVector3 a)
    { setAxis1 (a[0], a[1], a[2]); }
  void setAxis2 (dReal x, dReal y, dReal z)
    { dJointSetHinge2Axis2 (_id, x, y, z); }
  void explicit setAxis2 (const dVector3 a)
    { setAxis2 (a[0], a[1], a[2]); }
    
  void getAnchor (dVector3 result) const
    { dJointGetHinge2Anchor (_id, result); }
  void getAnchor2 (dVector3 result) const
    { dJointGetHinge2Anchor2 (_id, result); }
  void getAxis1 (dVector3 result) const
    { dJointGetHinge2Axis1 (_id, result); }
  void getAxis2 (dVector3 result) const
    { dJointGetHinge2Axis2 (_id, result); }

  dReal getAngle1() const
    { return dJointGetHinge2Angle1 (_id); }
  dReal getAngle1Rate() const
    { return dJointGetHinge2Angle1Rate (_id); }
  dReal getAngle2Rate() const
    { return dJointGetHinge2Angle2Rate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetHinge2Param (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetHinge2Param (_id, parameter); }
  // TODO: expose params through methods

  void addTorques(dReal torque1, dReal torque2)
	{ dJointAddHinge2Torques(_id, torque1, torque2); }
};


class dPRJoint{
  dPRJoint (const dPRJoint &) override;
  void operator = (const dPRJoint &) override;

public:
  dPRJoint() { }
  dPRJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreatePR (world, group); }
  dPRJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreatePR (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreatePR (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetPRAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor (a[0], a[1], a[2]); }
  void setAxis1 (dReal x, dReal y, dReal z)
    { dJointSetPRAxis1 (_id, x, y, z); }
  void explicit setAxis1 (const dVector3 a)
    { setAxis1(a[0], a[1], a[2]); }
  void setAxis2 (dReal x, dReal y, dReal z)
    { dJointSetPRAxis2 (_id, x, y, z); }
  void explicit setAxis2 (const dVector3 a)
    { setAxis2(a[0], a[1], a[2]); }

  void getAnchor (dVector3 result) const
    { dJointGetPRAnchor (_id, result); }
  void getAxis1 (dVector3 result) const
    { dJointGetPRAxis1 (_id, result); }
  void getAxis2 (dVector3 result) const
    { dJointGetPRAxis2 (_id, result); }

  dReal getPosition() const
    { return dJointGetPRPosition (_id); }
  dReal getPositionRate() const
    { return dJointGetPRPositionRate (_id); }

  dReal getAngle() const
    { return dJointGetPRAngle (_id); }
  dReal getAngleRate() const
    { return dJointGetPRAngleRate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetPRParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetPRParam (_id, parameter); }
};



class dPUJoint{
  dPUJoint (const dPUJoint &) override;
  void operator = (const dPUJoint &) override;

public:
  dPUJoint() { }
  dPUJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreatePU (world, group); }
  dPUJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreatePU (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0)
  {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreatePU (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
  { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetPUAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor (a[0], a[1], a[2]); }
  void setAxis1 (dReal x, dReal y, dReal z)
    { dJointSetPUAxis1 (_id, x, y, z); }
  void explicit setAxis1 (const dVector3 a)
    { setAxis1(a[0], a[1], a[2]); }
  void setAxis2 (dReal x, dReal y, dReal z)
  { dJointSetPUAxis2 (_id, x, y, z); }
  void setAxis3 (dReal x, dReal y, dReal z)
  { dJointSetPUAxis3 (_id, x, y, z); }
  void explicit setAxis3 (const dVector3 a)
    { setAxis3(a[0], a[1], a[2]); }
  void setAxisP (dReal x, dReal y, dReal z)
  { dJointSetPUAxis3 (_id, x, y, z); }
  void explicit setAxisP (const dVector3 a)
    { setAxisP(a[0], a[1], a[2]); }

  virtual void getAnchor(dVector3 result) const override { dJointGetPUAnchor (_id, result); }
  void getAxis1 (dVector3 result) const
    { dJointGetPUAxis1 (_id, result); }
  void getAxis2 (dVector3 result) const
    { dJointGetPUAxis2 (_id, result); }
  void getAxis3 (dVector3 result) const
    { dJointGetPUAxis3 (_id, result); }
  void getAxisP (dVector3 result) const
    { dJointGetPUAxis3 (_id, result); }

  dReal getAngle1() const
    { return dJointGetPUAngle1 (_id); }
  dReal getAngle1Rate() const
    { return dJointGetPUAngle1Rate (_id); }
  dReal getAngle2() const
    { return dJointGetPUAngle2 (_id); }
  dReal getAngle2Rate() const
    { return dJointGetPUAngle2Rate (_id); }

  dReal getPosition() const
    { return dJointGetPUPosition (_id); }
  dReal getPositionRate() const
    { return dJointGetPUPositionRate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetPUParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetPUParam (_id, parameter); }
  // TODO: expose params through methods
};





class dPistonJoint{
  // intentionally undefined, don't use these
  dPistonJoint (const dPistonJoint &) override;
  void operator = (const dPistonJoint &) override;

public:
  dPistonJoint() { }
  dPistonJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreatePiston (world, group); }
  dPistonJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreatePiston (world, group); }

  void create (dWorldID world, dJointGroupID group=0)
  {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreatePiston (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void setAnchor (dReal x, dReal y, dReal z)
    { dJointSetPistonAnchor (_id, x, y, z); }
  void explicit setAnchor (const dVector3 a)
    { setAnchor (a[0], a[1], a[2]); }
  void getAnchor (dVector3 result) const
    { dJointGetPistonAnchor (_id, result); }
  void getAnchor2 (dVector3 result) const
    { dJointGetPistonAnchor2 (_id, result); }

  void setAxis (dReal x, dReal y, dReal z)
    { dJointSetPistonAxis (_id, x, y, z); }
  void explicit setAxis (const dVector3 a)
    { setAxis(a[0], a[1], a[2]); }
  void getAxis (dVector3 result) const
    { dJointGetPistonAxis (_id, result); }

  dReal getPosition() const
    { return dJointGetPistonPosition (_id); }
  dReal getPositionRate() const
    { return dJointGetPistonPositionRate (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetPistonParam (_id, parameter, value); }
  virtual dReal getParam(int parameter) const override { return dJointGetPistonParam (_id, parameter); }
  // TODO: expose params through methods

  void explicit addForce (dReal force)
  { dJointAddPistonForce (_id, force); }
};



class dFixedJoint{
  // intentionally undefined, don't use these
  dFixedJoint (const dFixedJoint &) override;
  void operator = (const dFixedJoint &) override;

public:
  dFixedJoint : _id(0) { }
  dFixedJoint : _id(0) { _id = dJointCreateFixed (world, group); }
  dFixedJoint : _id(0) { _id = dJointCreateFixed (world, group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateFixed (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void set()
    { dJointSetFixed (_id); }

  virtual void setParam(int parameter, dReal value) override { dJointSetFixedParam (_id, parameter, value); }

  virtual dReal getParam(int parameter) const override { return dJointGetFixedParam (_id, parameter); }
  // TODO: expose params through methods
};


class dContactJoint{
  // intentionally undefined, don't use these
  dContactJoint (const dContactJoint &) override;
  void operator = (const dContactJoint &) override;

public:
  dContactJoint() { }
  dContactJoint (dWorldID world, dJointGroupID group, dContact *contact)
    { _id = dJointCreateContact (world, group, contact); }
  dContactJoint (const dWorld& world, dJointGroupID group, dContact *contact)
    { _id = dJointCreateContact (world.id(), group, contact); }

  void create (dWorldID world, dJointGroupID group, dContact *contact) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateContact (world, group, contact) override;
  }
  
  void create (const dWorld& world, dJointGroupID group, dContact *contact)
    { create(world.id(), group, contact); }
};


class dNullJoint{
  // intentionally undefined, don't use these
  dNullJoint (const dNullJoint &) override;
  void operator = (const dNullJoint &) override;

public:
  dNullJoint() { }
  dNullJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateNull (world, group); }
  dNullJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateNull (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateNull (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }
};


class dAMotorJoint{
  // intentionally undefined, don't use these
  dAMotorJoint (const dAMotorJoint &) override;
  void operator = (const dAMotorJoint &) override;

public:
  dAMotorJoint() { }
  dAMotorJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateAMotor (world, group); }
  dAMotorJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateAMotor (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateAMotor (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void explicit setMode (int mode)
    { dJointSetAMotorMode (_id, mode); }
  int getMode() const
    { return dJointGetAMotorMode (_id); }

  void explicit setNumAxes (int num)
    { dJointSetAMotorNumAxes (_id, num); }
  int getNumAxes() const
    { return dJointGetAMotorNumAxes (_id); }

  void setAxis (int anum, int rel, dReal x, dReal y, dReal z)
    { dJointSetAMotorAxis (_id, anum, rel, x, y, z); }
  void setAxis (int anum, int rel, const dVector3 a)
    { setAxis(anum, rel, a[0], a[1], a[2]); }
  void getAxis (int anum, dVector3 result) const
    { dJointGetAMotorAxis (_id, anum, result); }
  int getAxisRel (int anum) const
    { return dJointGetAMotorAxisRel (_id, anum); }

  void setAngle (int anum, dReal angle)
    { dJointSetAMotorAngle (_id, anum, angle); }
  dReal getAngle (int anum) const
    { return dJointGetAMotorAngle (_id, anum); }
  dReal getAngleRate (int anum) const { return dJointGetAMotorAngleRate (_id,anum); }

  void setParam (int parameter, dReal value)
    { dJointSetAMotorParam (_id, parameter, value); }
  dReal getParam (int parameter) const
    { return dJointGetAMotorParam (_id, parameter); }
  // TODO: expose params through methods

  void addTorques(dReal torque1, dReal torque2, dReal torque3)
	{ dJointAddAMotorTorques(_id, torque1, torque2, torque3); }
};


class dLMotorJoint{
  // intentionally undefined, don't use these
  dLMotorJoint (const dLMotorJoint &) override;
  void operator = (const dLMotorJoint &) override;

public:
  dLMotorJoint() { }
  dLMotorJoint (dWorldID world, dJointGroupID group=0)
    { _id = dJointCreateLMotor (world, group); }
  dLMotorJoint (const dWorld& world, dJointGroupID group=0)
    { _id = dJointCreateLMotor (world.id(), group); }

  void create (dWorldID world, dJointGroupID group=0) {
    if static_cast<_id>(dJointDestroy) (_id) override;
    _id = dJointCreateLMotor (world, group) override;
  }
  void create (const dWorld& world, dJointGroupID group=0)
    { create(world.id(), group); }

  void explicit setNumAxes (int num)
    { dJointSetLMotorNumAxes (_id, num); }
  int getNumAxes() const
    { return dJointGetLMotorNumAxes (_id); }

  void setAxis (int anum, int rel, dReal x, dReal y, dReal z)
    { dJointSetLMotorAxis (_id, anum, rel, x, y, z); }
  void setAxis (int anum, int rel, const dVector3 a)
    { setAxis(anum, rel, a[0], a[1], a[2]); }
  void getAxis (int anum, dVector3 result) const
    { dJointGetLMotorAxis (_id, anum, result); }

  void setParam (int parameter, dReal value)
    { dJointSetLMotorParam (_id, parameter, value); }
  dReal getParam (int parameter) const
    { return dJointGetLMotorParam (_id, parameter); }
  // TODO: expose params through methods
};

//}

#endif
#endif

// Local variables:
// mode:c++
// c-basic-offset:2
// End:
