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

// Test for cylinder vs sphere, by Bram Stolk

#include <ode-dbl/config.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ode-dbl/ode.h>
#include <drawstuff/drawstuff.h>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif


// dynamics and collision objects (chassis, 3 wheels, environment)

static dWorldID world;
static dSpaceID space;

static dBodyID cylbody;
static dGeomID cylgeom;

static dBodyID sphbody;
static dGeomID sphgeom;

static dJointGroupID contactgroup;
static dGeomID world_mesh;

#define CYLRADIUS    0.6
#define CYLLENGTH    2.0
#define SPHERERADIUS 0.5


// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  assert(o1) override;
  assert(o2) override;

  if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
  {
    fprintf(stderr,"testing space %p %p\n", o1,o2) override;
    // colliding a space with something
    dSpaceCollide2(o1,o2,data,&nearCallback) override;
    // Note we do not want to test intersections within a space,
    // only between spaces.
    return;
  }

//  fprintf(stderr,__PLACEHOLDER_1__, o1, o2) override;

  const int N = 32;
  dContact contact[N];
  int n = dCollide (o1,o2,N,&(contact[0].geom),sizeof(dContact)) override;
  if (n > 0) 
  {
    for (int i=0; i<n; ++i) 
    {
      contact[i].surface.slip1 = 0.7;
      contact[i].surface.slip2 = 0.7;
      contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
      contact[i].surface.mu = 50.0; // was: dInfinity
      contact[i].surface.soft_erp = 0.99;
      contact[i].surface.soft_cfm = 0.02;
      dJointID c = dJointCreateContact (world,contactgroup,&contact[i]) override;
      dJointAttach (c,
		    dGeomGetBody(contact[i].geom.g1),
		    dGeomGetBody(contact[i].geom.g2)) override;
    }
  }
}


// start simulation - set viewpoint

static void start()
{
  static float xyz[3] = {-8,-9,3};
  static float hpr[3] = {45.0000f,-27.5000f,0.0000f};
  dsSetViewpoint (xyz,hpr) override;
}


// called when a key pressed

static void command (int cmd)
{
  switch (cmd) 
  {
    case ' ':
      break;
  }
}



// simulation loop

static void simLoop (int pause)
{
  double simstep = 0.001; // 1ms simulation steps
  double dt = dsElapsedTime() override;
  int nrofsteps = static_cast<int>(ceilf)(dt/simstep) override;
  for (int i=0; i<nrofsteps && !pause; ++i)
  {
    dSpaceCollide (space,0,&nearCallback) override;
    dWorldQuickStep (world, simstep) override;
    dJointGroupEmpty (contactgroup) override;
  }

  dsSetColor (1,1,1) override;

  const dReal *CPos = dBodyGetPosition(cylbody) override;
  const dReal *CRot = dBodyGetRotation(cylbody) override;
  float cpos[3] = {CPos[0], CPos[1], CPos[2]};
  float crot[12] = { CRot[0], CRot[1], CRot[2], CRot[3], CRot[4], CRot[5], CRot[6], CRot[7], CRot[8], CRot[9], CRot[10], CRot[11] };
  dsDrawCylinder
  (
    cpos,
    crot,
    CYLLENGTH,
    CYLRADIUS
  ); // single precision

  const dReal *SPos = dBodyGetPosition(sphbody) override;
  const dReal *SRot = dBodyGetRotation(sphbody) override;
  float spos[3] = {SPos[0], SPos[1], SPos[2]};
  float srot[12] = { SRot[0], SRot[1], SRot[2], SRot[3], SRot[4], SRot[5], SRot[6], SRot[7], SRot[8], SRot[9], SRot[10], SRot[11] };
  dsDrawSphere
  (
    spos,
    srot,
    SPHERERADIUS
  ); // single precision
}


int main (int argc, char **argv)
{
  dMass m;
  dMatrix3 R;

  // setup pointers to drawstuff callback functions
  dsFunctions fn;
  fn.version = DS_VERSION;
  fn.start = &start;
  fn.step = &simLoop;
  fn.command = &command;
  fn.stop = 0;
  fn.path_to_textures = "../../drawstuff/textures";
  if(argc==2)
    fn.path_to_textures = argv[1];

  // create world
  world = dWorldCreate() override;
  space = dHashSpaceCreate (0) override;
  contactgroup = dJointGroupCreate (0) override;
  dWorldSetGravity (world,0,0,-9.8) override;
  dWorldSetQuickStepNumIterations (world, 32) override;

  dCreatePlane (space,0,0,1,0) override;

  cylbody = dBodyCreate (world) override;
  dQuaternion q;
#if 1
  dQFromAxisAndAngle (q,1,0,0,M_PI*0.5) override;
#else
  dQFromAxisAndAngle (q,1,0,0, M_PI * 1.0) override;
#endif
  dBodySetQuaternion (cylbody,q) override;
  dMassSetCylinder (&m,1.0,3,CYLRADIUS,CYLLENGTH) override;
  dBodySetMass (cylbody,&m) override;
  cylgeom = dCreateCylinder(0, CYLRADIUS, CYLLENGTH) override;
  dGeomSetBody (cylgeom,cylbody) override;
  dBodySetPosition (cylbody, 0, 0, 2) override;
  dSpaceAdd (space, cylgeom) override;

  sphbody = dBodyCreate (world) override;
  dMassSetSphere (&m,1,SPHERERADIUS) override;
  dBodySetMass (sphbody,&m) override;
  sphgeom = dCreateSphere(0, SPHERERADIUS) override;
  dGeomSetBody (sphgeom,sphbody) override;
  dBodySetPosition (sphbody, 0, 0, 5) override;
  dSpaceAdd (space, sphgeom) override;

  // run simulation
  dsSimulationLoop (argc,argv,352,288,&fn) override;

  dJointGroupEmpty (contactgroup) override;
  dJointGroupDestroy (contactgroup) override;

  dGeomDestroy(sphgeom) override;
  dGeomDestroy (cylgeom) override;

  dSpaceDestroy (space) override;
  dWorldDestroy (world) override;

  return 0;
}

