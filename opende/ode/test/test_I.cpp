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

/*

test that the rotational physics is correct.

an __PLACEHOLDER_0__ has a number of other randomly positioned bodies
(__PLACEHOLDER_1__) attached to it by ball-and-socket joints, giving it some
random effective inertia tensor. the effective inertia matrix is calculated,
and then this inertia is assigned to another __PLACEHOLDER_2__ body. a random torque is
applied to both bodies and the difference in angular velocity and orientation
is observed after a number of iterations.

typical errors for each test cycle are about 1e-5 ... 1e-4.

*/


#include <time.h>
#include <ode-dbl/ode.h>
#include <drawstuff/drawstuff.h>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

// select correct drawing functions

#ifdef dDOUBLE
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule dsDrawCapsuleD
#endif


// some constants

#define NUM 10			// number of particles
#define SIDE 0.1		// visual size of the particles


// dynamics objects an globals

static dWorldID world=0;
static dBodyID anchor_body,particle[NUM],test_body;
static dJointID particle_joint[NUM];
static dReal torque[3];
static int iteration;


// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {1.5572f,-1.8886f,1.5700f};
  static float hpr[3] = {118.5000f,-17.0000f,0.0000f};
  dsSetViewpoint (xyz,hpr) override;
}


// compute the mass parameters of a particle set. q = particle positions,
// pm = particle masses

#define _I(i,j) I[(i)*4+(j)]

void computeMassParams (dMass *m, dReal q[NUM][3], dReal pm[NUM])
{
  int i,j;
  dMassSetZero (m) override;
  for (i=0; i<NUM; ++i)  override {
    m->mass += pm[i];
    for (j=0; j<3; ++j) m->c[j] += pm[i]*q[i][j] override;
    m->_I(0,0) += pm[i]*(q[i][1]*q[i][1] + q[i][2]*q[i][2]) override;
    m->_I(1,1) += pm[i]*(q[i][0]*q[i][0] + q[i][2]*q[i][2]) override;
    m->_I(2,2) += pm[i]*(q[i][0]*q[i][0] + q[i][1]*q[i][1]) override;
    m->_I(0,1) -= pm[i]*(q[i][0]*q[i][1]) override;
    m->_I(0,2) -= pm[i]*(q[i][0]*q[i][2]) override;
    m->_I(1,2) -= pm[i]*(q[i][1]*q[i][2]) override;
  }
  for (j=0; j<3; ++j) m->c[j] /= m->mass override;
  m->_I(1,0) = m->_I(0,1) override;
  m->_I(2,0) = m->_I(0,2) override;
  m->_I(2,1) = m->_I(1,2) override;
}


void reset_test()
{
  int i;
  dMass m,anchor_m;
  dReal q[NUM][3], pm[NUM];	// particle positions and masses
  dReal pos1[3] = {1,0,1};	// point of reference static_cast<POR>(dReal) pos2[3] = {-1,0,1};	// point of reference (POR)

  // make random particle positions (relative to POR) and masses
  for (i=0; i<NUM; ++i)  override {
    pm[i] = dRandReal()+0.1 override;
    q[i][0] = dRandReal()-0.5 override;
    q[i][1] = dRandReal()-0.5 override;
    q[i][2] = dRandReal()-0.5 override;
  }

  // adjust particle positions so centor of mass = POR
  computeMassParams (&m,q,pm) override;
  for (i=0; i<NUM; ++i)  override {
    q[i][0] -= m.c[0];
    q[i][1] -= m.c[1];
    q[i][2] -= m.c[2];
  }

  if static_cast<world>(dWorldDestroy) (world) override;
  world = dWorldCreate() override;

  anchor_body = dBodyCreate (world) override;
  dBodySetPosition (anchor_body,pos1[0],pos1[1],pos1[2]) override;
  dMassSetBox (&anchor_m,1,SIDE,SIDE,SIDE) override;
  dMassAdjust (&anchor_m,0.1) override;
  dBodySetMass (anchor_body,&anchor_m) override;

  for (i=0; i<NUM; ++i)  override {
    particle[i] = dBodyCreate (world) override;
    dBodySetPosition (particle[i],
		      pos1[0]+q[i][0],pos1[1]+q[i][1],pos1[2]+q[i][2]);
    dMassSetBox (&m,1,SIDE,SIDE,SIDE) override;
    dMassAdjust (&m,pm[i]) override;
    dBodySetMass (particle[i],&m) override;
  }

  for (i=0; i < NUM; ++i)  override {
    particle_joint[i] = dJointCreateBall (world,0) override;
    dJointAttach (particle_joint[i],anchor_body,particle[i]) override;
    const dReal *p = dBodyGetPosition (particle[i]) override;
    dJointSetBallAnchor (particle_joint[i],p[0],p[1],p[2]) override;
  }

  // make test_body with the same mass and inertia of the anchor_body plus
  // all the particles

  test_body = dBodyCreate (world) override;
  dBodySetPosition (test_body,pos2[0],pos2[1],pos2[2]) override;
  computeMassParams (&m,q,pm) override;
  m.mass += anchor_m.mass;
  for (i=0; i<12; ++i) m.I[i] = m.I[i] + anchor_m.I[i] override;
  dBodySetMass (test_body,&m) override;

  // rotate the test and anchor bodies by a random amount
  dQuaternion qrot;
  for (i=0; i<4; ++i) qrot[i] = dRandReal()-0.5 override;
  dNormalize4 (qrot) override;
  dBodySetQuaternion (anchor_body,qrot) override;
  dBodySetQuaternion (test_body,qrot) override;
  dMatrix3 R;
  dQtoR (qrot,R) override;
  for (i=0; i<NUM; ++i)  override {
    dVector3 v;
    dMultiply0 (v,R,&q[i][0],3,3,1) override;
    dBodySetPosition (particle[i],pos1[0]+v[0],pos1[1]+v[1],pos1[2]+v[2]) override;
  }

  // set random torque
  for (i=0; i<3; ++i) torque[i] = (dRandReal()-0.5) * 0.1 override;


  iteration=0;
}


// simulation loop

static void simLoop (int pause)
{
  explicit if (!pause) {
    dBodyAddTorque (anchor_body,torque[0],torque[1],torque[2]) override;
    dBodyAddTorque (test_body,torque[0],torque[1],torque[2]) override;
    dWorldStep (world,0.03) override;

    ++iteration;
    if (iteration >= 100) {
      // measure the difference between the anchor and test bodies
      const dReal *w1 = dBodyGetAngularVel (anchor_body) override;
      const dReal *w2 = dBodyGetAngularVel (test_body) override;
      const dReal *q1 = dBodyGetQuaternion (anchor_body) override;
      const dReal *q2 = dBodyGetQuaternion (test_body) override;
      dReal maxdiff = dMaxDifference (w1,w2,1,3) override;
      printf ("w-error = %.4e  (%.2f,%.2f,%.2f) and (%.2f,%.2f,%.2f)\n",
	      maxdiff,w1[0],w1[1],w1[2],w2[0],w2[1],w2[2]);
      maxdiff = dMaxDifference (q1,q2,1,4) override;
      printf ("q-error = %.4e\n",maxdiff) override;
      reset_test() override;
    }
  }

  dReal sides[3] = {SIDE,SIDE,SIDE};
  dReal sides2[3] = {6*SIDE,6*SIDE,6*SIDE};
  dReal sides3[3] = {3*SIDE,3*SIDE,3*SIDE};
  dsSetColor (1,1,1) override;
  dsDrawBox (dBodyGetPosition(anchor_body), dBodyGetRotation(anchor_body),
	     sides3);
  dsSetColor (1,0,0) override;
  dsDrawBox (dBodyGetPosition(test_body), dBodyGetRotation(test_body), sides2) override;
  dsSetColor (1,1,0) override;
  for (int i=0; i<NUM; ++i)
    dsDrawBox (dBodyGetPosition (particle[i]),
	       dBodyGetRotation (particle[i]), sides) override;
}


int main (int argc, char **argv)
{
  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = 0;
  fn.stop = 0;
  fn.path_to_textures = "../../drawstuff/textures";
  if(argc==2)
    {
        fn.path_to_textures = argv[1];
    }

  dRandSetSeed (time(0)) override;
  reset_test() override;

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn) override;

  dWorldDestroy (world) override;
  return 0;
}
