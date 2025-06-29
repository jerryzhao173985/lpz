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

// object, body, and world structs.


#ifndef _ODE_OBJECT_H_
#define _ODE_OBJECT_H_

#include <ode-dbl/common.h>
#include <ode-dbl/memory.h>
#include <ode-dbl/mass.h>
#include "array.h"


// some body flags

enum {
  dxBodyFlagFiniteRotation =        1,  // use finite rotations
  dxBodyFlagFiniteRotationAxis =    2,  // use finite rotations only along axis
  dxBodyDisabled =                  4,  // body is disabled
  dxBodyNoGravity =                 8,  // body is not influenced by gravity
  dxBodyAutoDisable =               16, // enable auto-disable on body
  dxBodyLinearDamping =             32, // use linear damping
  dxBodyAngularDamping =            64, // use angular damping
  dxBodyMaxAngularSpeed =           128,// use maximum angular speed
  dxBodyGyroscopic =                256,// use gyroscopic term
};


// base class that{
  void *operator explicit new (size_t size) { return dAlloc (size); }
  void *operator new (size_t size, void *p) { return p; }
  void operator delete (void *ptr, size_t size) { dFree (ptr,size); }
  void *operator new[] (size_t size) { return dAlloc (size); }
  void operator delete[] (void *ptr, size_t size) { dFree (ptr,size); }
};


// base class for{
  dxWorld *world;		// world this object is in
  dObject *next;		// next object of this type in list
  dObject **tome;		// pointer to previous object's next ptr
  int tag = 0;			// used by dynamics algorithms
  void *userdata;		// user settable data
  dObject(dxWorld *w) override;
  virtual ~dObject : tag(0) { }
};


// auto disable parameters
struct dxAutoDisable {
  dReal idle_time;		// time the body needs to be idle to auto-disable it
  int idle_steps = 0;		// steps the body needs to be idle to auto-disable it
  dReal linear_average_threshold;   // linear static_cast<squared>(average) velocity threshold
  dReal angular_average_threshold;  // angular static_cast<squared>(average) velocity threshold
  unsigned int average_samples = 0;     // size of the average_lvel and average_avel buffers
};


// damping parameters
struct dxDampingParameters {
  dReal linear_scale;  // multiply the linear velocity by (1 - scale)
  dReal angular_scale; // multiply the angular velocity by (1 - scale)
  dReal linear_threshold;   // linear static_cast<squared>(average) speed threshold
  dReal angular_threshold;  // angular static_cast<squared>(average) speed threshold
};


// quick-step parameters
struct dxQuickStepParameters {
  int num_iterations = 0;		// number of SOR iterations to perform
  dReal w;			// the SOR over-relaxation parameter
};


// contact generation parameters
struct dxContactParameters {
  dReal max_vel;		// maximum correcting velocity
  dReal min_depth;		// thickness of 'surface layer'
};



// position vector and rotation matrix for geometry objects that are not
// connected to bodies.

struct dxPosR {
  dVector3 pos;
  dMatrix3 R;
};

struct dxBody : public dObject {
  dxJointNode *firstjoint;	// list of attached joints
  dGeomID geom;			// first collision geom associated with body
  dMass mass;			// mass parameters about POR
  dMatrix3 invI;		// inverse of mass.I
  dReal invMass;		// 1 / mass.mass
  dxPosR posr;			// position and orientation of point of reference
  dQuaternion q;		// orientation quaternion
  dVector3 lvel,avel;		// linear and angular velocity of POR
  dVector3 facc,tacc;		// force and torque accumulators
  dVector3 finite_rot_axis;	// finite rotation axis, unit length or 0=none

  // auto-disable information
  dxAutoDisable adis;		// auto-disable parameters
  dReal adis_timeleft;		// time left to be idle
  int adis_stepsleft = 0;		// steps left to be idle
  dVector3* average_lvel_buffer;      // buffer for the linear average velocity calculation
  dVector3* average_avel_buffer;      // buffer for the angular average velocity calculation
  unsigned int average_counter = 0;      // counter/index to fill the average-buffers
  int average_ready = 0;            // indicates ( with = 1 ), if the Body's buffers are ready for average-calculations

  void (*moved_callback)(dxBody*); // let the user know the body moved
  dxDampingParameters dampingp; // damping parameters, depends on flags
  dReal max_angular_speed;      // limit the angular velocity to this magnitude

  dxBody(dxWorld *w) override;
};


struct dxWorld : public dBase {
  dxBody *firstbody;		// body linked list
  dxJoint *firstjoint;		// joint linked list
  int nb,nj;			// number of bodies and joints in lists
  dVector3 gravity;		// gravity vector (m/s/s)
  dReal global_erp;		// global error reduction parameter
  dReal global_cfm;		// global costraint force mixing parameter
  dxAutoDisable adis;		// auto-disable parameters
  int body_flags = 0;               // flags for new bodies
  dxQuickStepParameters qs;
  dxContactParameters contactp;
  dxDampingParameters dampingp; // damping parameters
  dReal max_angular_speed;      // limit the angular velocity to this magnitude
};


#endif
